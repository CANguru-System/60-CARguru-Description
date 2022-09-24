#include <iostream>
#include <gtkmm.h>
#include "classdefs.h"

CAR::CAR()
{
}

void CAR::setup_CAR()
{
  // carwidth = margin + functionframe( = 3*(button + margin)) + margin + speedframe + margin
  std::vector<Glib::ustring> tool_tip = {
      "Abblendlicht",
      "Blinker links",
      "Blinker rechts",
      "Warnblinker",
      "Fernlicht",
      "Lichthupe",
      "Hupe",
      "Martinshorn",
      "Frontblinker",
      "Langsamer",
      "S T O P",
      "Schneller"};

  _carHeight = _carWidth;
  set_size_request(_carWidth, _carHeight);
  //  set_title("Fixed Container");
  set_border_width(10);

  // Sets the margin around the frame
  set_margin_left(window_width * 0.01);
  set_margin_top(window_width * 0.01);
  set_margin_right(window_width * 0.01);
  set_margin_bottom(window_width * 0.01);

  Lok_speed_dest = 0;
  Lok_speed_curr = 0;
  direction = no_dir;
  m_scale_value = 0;

  adr = pFrame->canguru[nbr].adr;
  printf("ADR: %d\n", adr);
  // Set the frames label
  char buflbl[buflng20];
  sprintf(buflbl, "CAR: %d", adr);
  set_label(buflbl);
  // Align the label at the start of the frame
  set_label_align(Gtk::Align::ALIGN_START);

  ///// FIXED hier kommt alles rein/////////////////////////////////////////////
  fixed = new Gtk::Fixed;
  add(*fixed);
  fixed->show();

  ///// FUNCTIONFRAME /////////////////////////////////////////////////////////////
  functionFrame = new Gtk::Frame("Functions"); // Gtk::Orientation::ORIENTATION_HORIZONTAL, 2);
  // The number of pixels to place between children:
  functionFrame->set_border_width(2);
  // Set the frames label
  functionFrame->set_size_request(50, 100); 
  functionFrame->set_hexpand(false);
  functionFrame->set_vexpand(false);

  ///// FONT ////////////////////////////////////
  Pango::FontDescription fdesc0("Monospace 12");
  Pango::FontDescription fdesc1("Monospace 8");

  ///// VBOX   /////////////////////////////////////
  // Put a vertical box container as the Window contents
  Gtk::VBox *vBox = Gtk::manage(new Gtk::VBox(false, 0));
  fixed->add(*vBox);
  vBox->pack_start(*functionFrame, Gtk::PACK_SHRINK);

  ///// HBOX   /////////////////////////////////////
  // Put a horizontal box container as the Window contents
  v_hBox.push_back(Gtk::manage(new Gtk::HBox(false, 0)));

  ///// GRID /////////////////////////////////////////////////////////////
  grid = new Gtk::Grid();
  grid->set_hexpand(false);
  grid->set_vexpand(false);

  ///// BUTTONS /////////////////////////////////////////////////////////////

  uint8_t line = 0;
  for (uint8_t btn = Abblendlicht; btn < no_btn; btn++)
  {
    button = new smallButton(window_width * 0.06, window_height * 0.07);
    v_hBox[line]->pack_start(*button, false, false);
    button->on_off = released;
    button->set_tooltip_text(tool_tip[btn]);
    switch (btn)
    {
    case Abblendlicht:
      //  F00:  Abblendlicht
      button->set_image(*button->shrink_the_pixbuff("licht.png"));
      break;
    case Blinker_links:
      //  F01:  Blinker links
      button->set_image(*button->shrink_the_pixbuff("blinker_links.png"));
      break;
    case Blinker_rechts:
      //  F02:  Blinker rechts
      button->set_image(*button->shrink_the_pixbuff("blinker_rechts.png"));
      break;
    case Warnblinker:
      //  F03:  Warnblinker
      button->set_image(*button->shrink_the_pixbuff("warnblinker.png"));
      break;
    case Fernlicht:
      button->set_image(*button->shrink_the_pixbuff("licht.png"));
      break;
      //  F04:  Fernlicht   TASTER
    case Lichthupe_TASTER:
      //  F05:  Lichthupe   TASTER
      button->set_image(*button->shrink_the_pixbuff("lichthupe.png"));
      break;
    case Hupe_TASTER:
      //  F06:  Hupe        TASTER
      button->set_image(*button->shrink_the_pixbuff("hupe.png"));
      break;
    case Martinshorn:
      //  F07:  Martinshorn
      button->set_image(*button->shrink_the_pixbuff("martinshorn.png"));
      break;
    case Frontblinker:
      //  F08:  Frontblinker
      button->set_image(*button->shrink_the_pixbuff("frontblitzer.png"));
      break;
    case go_slower:
      //  slower
      button->set_image(*button->shrink_the_pixbuff("go_slower.png"));
      break;
    case STOP:
      //  F09:  STOP
      button->set_image(*button->shrink_the_pixbuff("stop.png"));
      break;
    case go_faster:
      //  faster
      button->set_image(*button->shrink_the_pixbuff("go_faster.png"));
      break;
    }

    if (((btn + 1) % 3) == 0)
    {
      // Fill Grid:
      grid->attach(*v_hBox[line], 0, line);
      line++;
      v_hBox.push_back(Gtk::manage(new Gtk::HBox(false, 0)));
    }

    button->function = 0x00;
    func_btns.push_back(button);

    button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &CAR::on_button_pressed), btn));
  }

  Gtk::Alignment *alignment = new Gtk::Alignment(0.5, 0.5, 1.0, 1.0);
  alignment->add(*grid);
  functionFrame->add(*alignment);

  ///// SCALE zur Regelung der Geschwindigkeit//////////////////////////////////
  // ALTERNATIVE: https://zetcode.com/gui/gtk2/customwidget/
  // create(double value, double lower, double upper, double step_increment =  1, double page_increment =  10, double page_size =  0);
  m_scale_adjustment = Gtk::Adjustment::create(0.0, 0.0, 100.0, 0.1, 1.0, 1.0);
  scale = Gtk::manage(new Gtk::Scale(m_scale_adjustment, Gtk::Orientation::ORIENTATION_VERTICAL));
  scale->set_digits(1);
  scale->set_value_pos(Gtk::PositionType::POS_TOP);
  scale->set_draw_value();
  scale->set_inverted(); // highest value at top
  scale->set_size_request(_carWidth * 0.3, _carWidth * 0.9);
  m_scale_adjustment->signal_value_changed().connect(sigc::mem_fun(*this, &CAR::on_speed_value_changed));

  ///// SPEEDFRAME /////////////////////////////////////////////////////////////
  speedFrame = new Gtk::Frame("Speed"); // Gtk::Orientation::ORIENTATION_HORIZONTAL, 2);
  // The number of pixels to place between children:
  speedFrame->set_border_width(2);
  // Set the frames label
  speedFrame->add(*scale);
  fixed->put(*speedFrame, _carWidth - _btnleftEdge, upperEdge);

  show_all_children();
}

// void Frames::sendDirection2CARS(uint8_t _dir, const uint8_t nbr)

void CAR::on_speed_value_changed()
{
  const uint16_t max_speed = 1000;
  direction_t tmp_dir = direction;
  Lok_speed_curr = m_scale_value * m_scale_factor;
  m_scale_value = m_scale_adjustment->get_value();
  //  scale->set_digits((int)val);
  Lok_speed_dest = m_scale_value * m_scale_factor;
  if (Lok_speed_dest > Lok_speed_curr)
  {
    direction = faster;
    Lok_speed_curr += steps;
    if (Lok_speed_curr > Lok_speed_dest)
      Lok_speed_curr = Lok_speed_dest;
    if (Lok_speed_curr > max_speed)
      Lok_speed_curr = max_speed;
  }
  else
  {
    direction = slower;
    Lok_speed_curr -= steps;
    if (Lok_speed_curr < Lok_speed_dest)
      Lok_speed_curr = Lok_speed_dest;
    if (Lok_speed_curr > max_speed)
      Lok_speed_curr = 0;
  }
  // if it is running, send no start-message
  if (tmp_dir != no_dir)
    return;
  pFrame->sendSpeed2CARS(adr); // die maximale Geschwindigkeit ist auf 1000 festgelegt
}

void CAR::on_stop_func(uint8_t btn_nbr){
  if (func_btns[btn_nbr]->on_off == released)
    return;
  func_btns[btn_nbr]->on_off = released;
  func_btns[btn_nbr]->override_background_color(Gdk::RGBA("dark blue"));
  func_btns[btn_nbr]->function ^= 1;
  pFrame->sendFunctions2CARS(btn_nbr, nbr, func_btns[btn_nbr]->function);
}

void CAR::on_button_pressed(uint8_t btn_nbr)
{
  double new_value;
  Gtk::Image *img0;
  Gtk::Image *img1;
  switch (btn_nbr)
  {
  // switch-buttons: switch on or off ****************************************
  case Abblendlicht:
  case Blinker_links:
  case Blinker_rechts:
  case Warnblinker:
  case Fernlicht:
  case Martinshorn:
  case Frontblinker:
    if (func_btns[btn_nbr]->on_off == released)
    {
      // switch on
      func_btns[btn_nbr]->override_background_color(Gdk::RGBA("red"));
      func_btns[btn_nbr]->on_off = pressed;
      if (btn_nbr == Blinker_links)
        on_stop_func(Blinker_rechts);
      if (btn_nbr == Blinker_rechts)
        on_stop_func(Blinker_links);
      if (btn_nbr == Warnblinker)
      {
        on_stop_func(Blinker_rechts);
        on_stop_func(Blinker_links);
      }
    }
    else
    {
      // switch off
      if (btn_nbr == Abblendlicht)
        on_stop_func(Fernlicht);
      func_btns[btn_nbr]->override_background_color(Gdk::RGBA("dark blue"));
      func_btns[btn_nbr]->on_off = released;
    }
    // switch on or off
    func_btns[btn_nbr]->function ^= 1;
    pFrame->sendFunctions2CARS(btn_nbr, nbr, func_btns[btn_nbr]->function);
    break;
  // just buttons: switch on and off ********************************************
  case Lichthupe_TASTER:
  case Hupe_TASTER:
    pFrame->sendFunctions2CARS(btn_nbr, nbr);
    break;
  // STOP ***********************************************************************
  case STOP:
    //    img0 = new Gtk::Image;
    if (func_btns[STOP]->on_off == released)
    {
      // switch on
      func_btns[STOP]->override_background_color(Gdk::RGBA("red"));
      func_btns[STOP]->on_off = pressed;
    }
    else
    {
      // switch off
      func_btns[STOP]->override_background_color(Gdk::RGBA("green"));
      func_btns[STOP]->on_off = released;
    }
    //    func_btns[btn_nbr]->set_image(*img0);
    direction = stop;
    Lok_speed_dest = 0;
    Lok_speed_curr--;
    m_scale_adjustment->set_value(0.0);
    m_scale_adjustment->value_changed();
    pFrame->sendSpeed2CARS(adr); // die maximale Geschwindigkeit ist auf 1000 festgelegt
    break;
  // go_slower ********************************************************************
  case go_slower:
    if (func_btns[go_slower]->on_off == released)
    {
      // switch on
      func_btns[go_slower]->set_image(*func_btns[9]->shrink_the_pixbuff("go_slower.png"));
    /*  func_btns[go_slower]->on_off = pressed;
      func_btns[go_faster]->set_image(*func_btns[11]->shrink_the_pixbuff("normal.png"));
      func_btns[go_faster]->on_off = released;*/
    }
    //
    m_scale_adjustment->set_value(Lok_speed_dest/m_scale_factor-steps);
    m_scale_adjustment->value_changed();
    break;
  // go_faster ********************************************************************
  case go_faster:
    if (func_btns[11]->on_off == released)
    {
      // switch on
      func_btns[go_faster]->set_image(*func_btns[11]->shrink_the_pixbuff("go_faster.png"));
/*      func_btns[go_faster]->on_off = pressed;
      func_btns[go_slower]->set_image(*func_btns[9]->shrink_the_pixbuff("normal.png"));
      func_btns[go_slower]->on_off = released;*/
    }
    //
    new_value = Lok_speed_dest/m_scale_factor+steps;
    if (new_value<40.0)
      new_value = 40.0;
    m_scale_adjustment->set_value(new_value);
    m_scale_adjustment->value_changed();
    break;

  default:
    break;
  }
}

void CAR::get_data_from_PLAY(Frames *_pFrame, const uint8_t _nbr, const uint16_t _cw, const uint16_t _bE)
{
  pFrame = _pFrame;
  nbr = _nbr;
  _carWidth = _cw;
  _btnleftEdge = _bE;

  ///// FRAME /////////////////////////////////////////////////////////////
}

CAR::~CAR()
{
}