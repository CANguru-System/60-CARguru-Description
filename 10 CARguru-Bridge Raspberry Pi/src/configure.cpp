#include <iostream>
#include <gtkmm.h>
#include "classdefs.h"

Configure::Configure()
{
  configure_data_loaded = false;

  ///// LABEL /////////////////////////////////////////////////////////////
  label->set_label("Clients");

  ///// ScrolledWindow /////////////////////////////////////////////////////////////
  // Create a scrolling pane with the vertical scroll bar always showing
  // and the horizontal scroll bar only showing when needed
  Gtk::ScrolledWindow *scrollWindow = Gtk::manage(new Gtk::ScrolledWindow);
  scrollWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  // Add grid inside scrolling pane, and set pane to expand and contract
  // as the window is resized (other elements would remain fixed size)
  scrollWindow->set_hexpand(false);
  scrollWindow->set_vexpand(false);
  //  scrollWindow->set_size_request(column_spacing * right_end, row_spacing * list_height);
  scrollWindow->set_shadow_type(Gtk::SHADOW_OUT);
  scrollWindow->set_size_request(window_width * 0.4, window_height * 0.6);
  // Add the scrolling pane in which the grid resides to the vertical box
  fixedBackground->put(*scrollWindow, leftEdge, firstBox);

  ///// ListStore /////////////////////////////////////////////////////////////
  // Create the Tree MAIN model:
  m_ref_Main_TreeModel = Gtk::ListStore::create(m_Main_Columns);
  m_Main_TreeView.set_model(m_ref_Main_TreeModel);

  // Add the TreeView's view columns:
  // This number will be shown with the default numeric formatting.
  m_Main_TreeView.append_column("HASH", m_Main_Columns.m_Main_col_hash);
  m_Main_TreeView.append_column("Name", m_Main_Columns.m_Main_col_name);
  m_Main_TreeView.append_column("IP-Adresse", m_Main_Columns.m_Main_col_ip);

  // Add the TreeView, inside a ScrolledWindow, with the button underneath:
  scrollWindow->add(m_Main_TreeView);
  ///// createEmptyTreemodel ///////////////////////////////////////////////////////
  m_Columns.add(m_col_number_Title);
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  // Tree view listing the target_addobjectives entities
  // Create our TreeView
  m_TreeView = Gtk::manage(new Gtk::TreeView());
  m_TreeView->set_model(m_refTreeModel);
  m_TreeView->append_column("Data", m_col_number_Title);
  // Add our tree to the window
  Gtk::TreeModel::Row row = *(m_refTreeModel->append());
  row[m_col_number_Title] = "Empty";
  // Add the scrolling pane in which the grid resides to the vertical box
  fixedBackground->put(*m_TreeView, leftEdge, window_height * 0.7);

  m_TreeView->show_all_children();
  ///// BUTTON SAVE ///////////////////////////////////////////////////////
  button_save = Gtk::manage(new carguruButton());
  button_save->set_label("Speichern");
  button_save->signal_clicked().connect(sigc::mem_fun(*this, &Configure::on_save_Data));
  // Attach the new button to the grid
  fixedBackground->put(*button_save, leftEdge, window_height * 0.95);
  ///// BUTTON OTA ///////////////////////////////////////////////////////
  button_OTA = Gtk::manage(new carguruButton());
  button_OTA->set_label("OTA");
  button_OTA->signal_clicked().connect(sigc::mem_fun(*this, &Configure::on_OTA));
  // Attach the new button to the grid
  fixedBackground->put(*button_OTA, leftEdge + window_width * 0.2, window_height * 0.95);
  ///// PROGRESSBAR ///////////////////////////////////////////////////////
  m_progressbar = Gtk::manage(new Gtk::ProgressBar());
  m_progressbar->set_orientation(Gtk::ORIENTATION_VERTICAL);
  // Aktuellen Fortschritt der horizontalen ProgressBar auf 0 setzen
  //  m_progressbar->set_margin_end(5);
  m_progressbar->set_halign(Gtk::Align::ALIGN_FILL);
  m_progressbar->set_valign(Gtk::Align::ALIGN_FILL);
  m_progressbar->set_inverted(true);
  m_progressbar->set_hexpand(false);
  m_progressbar->set_vexpand(false);
  /*
  GdkColor color;
  gdk_color_parse( "#0080FF", &color );
  gtk_widget_modify_fg( GTK_WIDGET(button), GTK_STATE_SELECTED, &color );
  gtk_widget_modify_fg( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );  */
  m_progressbar->get_style_context();
}

void Configure::on_progress_step(double cur)
{
  // Prüfen, ob 1.0 oder mehr erreicht wurden (wenn ja: zurücksetzen auf 0)
  if (cur >= 1.0)
    cur = 0;
  // Neuen Fortschrittstand setzen
  m_progressbar->set_fraction(cur);
  // Prozentanzeige (Text im Hintergrund der ProgressBar) setzen
  m_progressbar->property_show_text() = true;
}

void Configure::on_save_Data()
{
  pFrame->send_config_data(current_nbr, 0x00);
}

// List item selection handler.
void Configure::on_OTA()
{
  Glib::RefPtr<Gtk::TreeSelection> treeselection = m_Main_TreeView.get_selection();
  Gtk::TreeIter iter = treeselection->get_selected();
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    current_nbr = row[m_Main_Columns.is_Main_ID];
    pFrame->send_OTA(current_nbr);
  }
}

void Configure::treeviewcolumn_validated_on_cell_data(
    Gtk::CellRenderer * /* renderer */,
    const Gtk::TreeModel::iterator &iter)
{
  // Get the value from the model and show it appropriately in the view:
  /*  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;
      current_nbr = row[m_Main_Columns.is_Main_ID];
        int model_value = row[m_col_number_Titles[0x01]];

        // This is just an example.
        // In this case, it would be easier to use append_column_editable() or
        // append_column_numeric_editable()
        char buffer[buflng20];
        sprintf(buffer, "%d", model_value);

        Glib::ustring view_text = buffer;
      m_cellrenderer_validated.property_text() = row[m_col_number_Titles[0x01]]; // view_text;
    }*/
}

void Configure::cellrenderer_validated_on_editing_started(
    Gtk::CellEditable *cell_editable, const Glib::ustring &path)
{
  // Start editing with previously-entered (but invalid) text,
  // if we are allowing the user to correct some invalid data.
  if (m_validate_retry)
  {
    // This is the CellEditable inside the CellRenderer.
    Gtk::CellEditable *celleditable_validated = cell_editable;

    // It's usually an Entry, at least for a CellRendererText:
    Gtk::Entry *pEntry = dynamic_cast<Gtk::Entry *>(celleditable_validated);
    if (pEntry)
    {
      pEntry->set_text(m_invalid_text_for_retry);
      m_validate_retry = false;
      m_invalid_text_for_retry.clear();
    }
  }
}

void Configure::cellrenderer_validated_on_edited(
    const Glib::ustring &path_string,
    const Glib::ustring &new_text,
    uint8_t current_col)
{
  Gtk::TreePath path(path_string);
#ifdef IS_TEST
  printf("column: %d\n", current_col);
#endif
  // Convert the inputed text to an integer, as needed by our model column:
  char *pchEnd = 0;
  int new_value = strtol(new_text.c_str(), &pchEnd, 10);
  loLimit = pFrame->canguru[current_nbr].Kanaele.at(current_col).untererWert;
  hiLimit = pFrame->canguru[current_nbr].Kanaele.at(current_col).obererWert;
  if ((new_value > hiLimit) || (new_value < loLimit))
  {
    // Prevent entry of number is out off limit.

    // Tell the user:
    static gchar text[buflng30];
    g_snprintf(text, buflng30, "Die Eingabe muss zwischen %d und %d liegen!\n", loLimit, hiLimit);
    Gtk::MessageDialog dialog(text, false, Gtk::MESSAGE_ERROR);
    dialog.run();

    // Start editing again, with the bad text, so that the user can correct it.
    // A real application should probably allow the user to revert to the
    // previous text.

    // Set the text to be used in the start_editing signal handler:
    m_invalid_text_for_retry = new_text;
    m_validate_retry = true;

    // Start editing again:
    m_TreeView->set_cursor(path, m_treeviewcolumn[current_col], m_cellrenderer_spin[current_col], true /* start_editing */);
  }
  else
  {
    // Put the new value in the model:
    m_cellrenderer_spin[current_col].property_text() = new_text;
    g_snprintf(pFrame->canguru[current_nbr].Kanaele.at(current_col).newValue, buflng10, "%s", new_text.c_str());
  }
}

void Configure::createDynamicTreemodel()
{
  // aktuellen (angeklickten) Datensatz laden
  static gchar value[buflng10];
  static gchar titles[buflng10];
  uint8_t nbr_channels = pFrame->canguru[current_nbr].numberofchannels;
  m_col_number_Titles.clear();
  m_TreeView->remove_all_columns();
  m_refTreeModel->clear();
  m_col_number_Titles.resize(nbr_channels * 2);
  for (uint8_t c = 0; c < nbr_channels * 2; c++)
    m_Columns.add(m_col_number_Titles[c]);
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  m_TreeView->set_model(m_refTreeModel);
  uint8_t cc = 0;
  m_cellrenderer_spin.clear();
  m_cellrenderer_text.clear();
  m_cellrenderer_spin.resize(nbr_channels);
  m_cellrenderer_text.resize(nbr_channels);
  m_treeviewcolumn.clear();
  m_treeviewcolumn.resize(nbr_channels * 2);
  // Add our tree to the window
  row = *(m_refTreeModel->append());
  for (uint8_t c = 0; c < nbr_channels * 2; c++)
  {
    if (c % 2 == 0)
    {
      // alle geraden Nummern erhalten die auswahlbezeichnung
      g_snprintf(titles, buflng10, "%s", pFrame->canguru[current_nbr].Kanaele.at(cc).auswahlbezeichnung);
      m_treeviewcolumn[c].pack_start(m_cellrenderer_spin[cc]);
      m_cellrenderer_spin[cc].property_xalign() = 1.0;
      // Tell the view column how to render the model values:
      m_treeviewcolumn[c].set_cell_data_func(m_cellrenderer_spin[cc], sigc::mem_fun(*this, &Configure::treeviewcolumn_validated_on_cell_data));

      // Make the CellRenderer editable, and handle its editing signals:
      m_cellrenderer_spin[cc].property_editable() = true;
      m_cellrenderer_spin[cc].signal_editing_started().connect(sigc::mem_fun(*this, &Configure::cellrenderer_validated_on_editing_started));
      m_cellrenderer_spin[cc].signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &Configure::cellrenderer_validated_on_edited), cc));

      g_snprintf(value, buflng10, "%d", pFrame->canguru[current_nbr].Kanaele.at(cc).aktuellerWert);
      m_cellrenderer_spin[cc].property_text() = value;
      g_snprintf(pFrame->canguru[current_nbr].Kanaele.at(cc).newValue, buflng10, "%s", value);
    }
    else
    {
      // alle ungeraden Nummern erhalten als Überschrift
      m_treeviewcolumn[c].pack_start(m_cellrenderer_text[cc]);
      m_cellrenderer_text[cc].property_xalign() = 0.25;
      ///////////////////////////////////
      // If this was a CellRendererSpin then you would have to set the adjustment:
      uint16_t currvalue = pFrame->canguru[current_nbr].Kanaele.at(cc).aktuellerWert;
      uint16_t lower = pFrame->canguru[current_nbr].Kanaele.at(cc).untererWert;
      uint16_t upper = pFrame->canguru[current_nbr].Kanaele.at(cc).obererWert;

      Glib::RefPtr<Gtk::Adjustment> m_spin_adjustment = Gtk::Adjustment::create(currvalue, lower, upper);
      m_cellrenderer_spin[cc].property_adjustment() = m_spin_adjustment;
      ///////////////////////////////////
      g_snprintf(titles, buflng10, "%d≤W≤%d", pFrame->canguru[current_nbr].Kanaele.at(cc).untererWert, pFrame->canguru[current_nbr].Kanaele.at(cc).obererWert);
      g_snprintf(value, buflng10, "%s", pFrame->canguru[current_nbr].Kanaele.at(cc).einheit);
      m_cellrenderer_text[cc].property_text() = value;
      cc++;
    }
    m_treeviewcolumn[c].set_title(titles);
    m_TreeView->append_column(m_treeviewcolumn[c]);
  }
  fixedBackground->put(*m_TreeView, leftEdge, window_height * 0.7);
  m_TreeView->show_all_children();
  fixedBackground->remove(*m_progressbar);
}

void Configure::on_treeview_row_activated(const Gtk::TreeModel::Path &path,
                                          Gtk::TreeViewColumn * /* column */)
{
  Gtk::TreeModel::iterator iter = m_ref_Main_TreeModel->get_iter(path);
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    current_nbr = row[m_Main_Columns.is_Main_ID];
#ifdef IS_TEST
    Glib::ustring hash = row[m_Main_Columns.m_Main_col_hash];
    Glib::ustring name = row[m_Main_Columns.m_Main_col_name];
    printf("Row %d activated: ID= %s, Name= %s\n", current_nbr, hash.c_str(), name.c_str());
#endif
    fixedBackground->remove(*m_TreeView);
    m_progressbar->set_fraction(0.0);
    fixedBackground->put(*m_progressbar, window_width*0.4, firstBox);
    m_progressbar->set_size_request(window_width * 0.1, window_height * 0.5);
    m_progressbar->show_now();
    pFrame->get_config_data(current_nbr);
  }
}

void Configure::fillComboBox(Frames *frame)
{
  static gchar buffhash[buflng10] = {0};
  static gchar buffip[buflng10] = {0};
  if (configure_data_loaded == true)
    return;
  pFrame = frame;
  // Fill the TreeView's model
  Gtk::TreeModel::Row row = *(m_ref_Main_TreeModel->append());
#ifdef IS_TEST
//  printf("CONFIGURE:Client: %d\n", canguru_nbr);
#endif

  for (uint8_t nbr = 0; nbr < pFrame->canguru_nbr; nbr++)
  {
    // Fill the ComboBox's Tree Model:
    g_snprintf(buffhash, buflng10, "%02X%02X",
               pFrame->canguru[nbr].HASH[0], pFrame->canguru[nbr].HASH[1]);
    g_snprintf(buffip, buflng10, "%d.%d.%d.%d",
               pFrame->canguru[nbr].IP[0], pFrame->canguru[nbr].IP[1], pFrame->canguru[nbr].IP[2], pFrame->canguru[nbr].IP[3]);
    if (nbr > 0)
      row = *(m_ref_Main_TreeModel->append());
    row[m_Main_Columns.is_Main_ID] = nbr;
    row[m_Main_Columns.m_Main_col_hash] = buffhash;
    row[m_Main_Columns.m_Main_col_name] = pFrame->canguru[nbr].name;
    row[m_Main_Columns.m_Main_col_ip] = buffip;
#ifdef IS_TEST
    printf("Kanäle: %d\n", pFrame->canguru[nbr].numberofchannels);
    printf("UID: %s\n", buffhash);
#endif
  }
  configure_data_loaded = true;
  // Connect signal:
  m_Main_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &Configure::on_treeview_row_activated));
  m_Main_TreeView.show_all_children();
}
