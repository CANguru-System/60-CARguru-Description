
#include <gtkmm.h>
#include "definitions.h"
#include "classdefs.h"

Mainwindow::Mainwindow(Glib::RefPtr<Gtk::Application> app)
{
  this->app = app;
  ///// FIXED /////////////////////////////////////////////////////////////
  fixedBackground = new Gtk::Fixed;
  add(*fixedBackground);
  fixedBackground->show();

  ///// SIZE /////////////////////////////////////////////////////////////
  set_size_request(window_width, window_height);
  set_title("CARguru Version 1.00");
  set_position(Gtk::WIN_POS_CENTER);

  ///// ptabControl / NOTEBOOK ///////////////////////////////////////////
  ptabControl = new Gtk::Notebook();
  ptabControl->set_size_request(window_width * 0.1, window_height - frame_offset);
  fixedBackground->add(*ptabControl);
  fixedBackground->move(*ptabControl, frame_offset, frame_offset);
  ptabControl->set_tab_pos(Gtk::POS_LEFT);

  pageFrames = new Frames();
  pageFrames->override_background_color(Gdk::RGBA("light grey"));
  ptabControl->insert_page(*pageFrames, "Frames", frames);

  pageConfigure = new Configure();
  pageConfigure->override_background_color(Gdk::RGBA("light grey"));
  ptabControl->insert_page(*pageConfigure, "Configure", configure);
  pageFrames->getConfigure_ptr(pageConfigure, ptabControl);

  pagePlay = new Play();
  pagePlay->override_background_color(Gdk::RGBA("light grey"));
  ptabControl->insert_page(*pagePlay, "Play", play);
  pageFrames->getPlay_ptr(pagePlay);

  EmptyFrame *pageEmpty = new EmptyFrame;
  ptabControl->insert_page(*pageEmpty, "QUIT", empty);

  //////////////////////////////////////////////////////////////
  ptabControl->signal_switch_page().connect(sigc::mem_fun(*this, &Mainwindow::on_notebook_switch_page));

  ///// CSS SET STYLE //////////////////////////////////////////
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "css/style.css", NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  //////////////////////////////////////////////////////////////
  show_all_children();
  // hide tabs until start is done
  ptabControl->get_nth_page(configure)->set_visible(false);
  ptabControl->get_nth_page(play)->set_visible(false);
}

void Mainwindow::on_notebook_switch_page(Gtk::Widget *page, guint page_num)
{
#ifdef IS_TEST
  printf("Switched to tab with index %d\n", page_num);
#endif
  if (page_num == configure)
    //    printf("Im on CONFIGURE\n");
    pageConfigure->fillComboBox(pageFrames);
  // You can also use m_Notebook.get_current_page() to get this index.
  if (page_num == play)
    //    printf("Im on CONFIGURE\n");
    pagePlay->set_playcards(pageFrames);
  if (page_num == empty)
  {
    pageFrames->send_a_frame(restart);
    app->quit();
  }
}
