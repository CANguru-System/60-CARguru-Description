#ifndef _CLASSDEFS_H_
#define _CLASSDEFS_H_

#include <gtkmm.h>
#include "definitions.h"
#include "classdefs.h"
#include "clock.h"

/////// FORWARD /////////////////////////////////////////////////////////////////
class Frames;
class Play;
class Configure;

/////// MAINWINDOW///////////////////////////////////////
class Mainwindow : public Gtk::Window
{
public:
  Mainwindow(Glib::RefPtr<Gtk::Application>);
  Frames *pageFrames;
  Configure *pageConfigure;
  Play *pagePlay;

private:
  Gtk::Fixed *fixedBackground;
  Gtk::Notebook *ptabControl;

  Glib::RefPtr<Gtk::Application> app;

protected:
  // Signal handlers:
  virtual void on_notebook_switch_page(Gtk::Widget *page, guint page_num);
};

/////// carguruButton /////////////////////////////////////////////////////////////////
class carguruButton : public Gtk::Button
{
public:
  carguruButton()
  {
    set_size_request(window_width * 0.1, window_height * 0.075);
    set_border_width(20);
  };
};

/////// smallButton /////////////////////////////////////////////////////////////////
class smallButton : public Gtk::Button
{
public:
  smallButton(uint16_t bW, uint16_t bH)
  {
    ///// LAYOUT //////////////////////////////////
    buttonWidth = bW;
    buttonHeight = bH;

    set_size_request(buttonWidth, buttonHeight);
    override_background_color(Gdk::RGBA("dark blue"));
  };

  Gtk::Image *shrink_the_pixbuff(const std::string &filename)
  {
    // Load the source image
    // Create, fill and save the destination image
    Glib::RefPtr<Gdk::Pixbuf> pimg = Gdk::Pixbuf::create_from_file(filename);
    uint16_t bW = buttonWidth-10;
    uint16_t bH = buttonHeight-12;
    double scale_x = ((double)bW / (double)pimg->get_width());
    double scale_y = ((double)(bH) / (double)pimg->get_height());
    Glib::RefPtr<Gdk::Pixbuf> pimg_tmp = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, (double)bW, (double)bH);
    pimg_tmp->fill(0x70707070);
    pimg->scale(pimg_tmp, 0, 0, bW, bH, 0, 0, scale_x, scale_y, Gdk::INTERP_NEAREST);
    Gtk::Image *img = new Gtk::Image;
    img->set(pimg_tmp);
    return img;
  }

  uint8_t function;
  press_t on_off;
  uint16_t buttonWidth;
  uint16_t buttonHeight;
  uint16_t btnDistFromTop = buttonHeight * 0.7;
  uint16_t distance = buttonWidth * 0.075;
};

/////// CAR /////////////////////////////////////////////////////////////////
class CAR : public Gtk::Frame
{
public:
  CAR();
  virtual ~CAR();
  void get_data_from_PLAY(Frames *_pFrame, const uint8_t _nbr, const uint16_t _cw, const uint16_t _bE);
  void setup_CAR();

  uint16_t Lok_speed_dest;
  uint16_t Lok_speed_curr;
  direction_t direction;
  uint8_t steps = 8;
  uint8_t m_scale_factor = 10;
  uint16_t adr;

private:
  void on_button_pressed(uint8_t btn_nbr);
  void on_stop_func(uint8_t btn_nbr);
  void on_speed_value_changed();
  Frames *pFrame;
  Gtk::Fixed *fixed;
  Glib::RefPtr<Gtk::Adjustment> m_scale_adjustment;

  uint16_t _carWidth;
  uint16_t _carHeight;
  uint16_t _btnleftEdge;
  double m_scale_value;
  std::vector<Gtk::HBox*> v_hBox;

protected:
  uint8_t nbr;
  // Child widgets:
  Gtk::Scale *scale;
  Gtk::Grid *grid;
  smallButton *button;

  smallButton *forwardBtn;
  smallButton *backwardBtn;

  Gtk::Alignment *alignment;
  Gtk::Frame *functionFrame;
  Gtk::Frame *speedFrame;
  std::vector<smallButton *> func_btns;
};

/////// BASEFRAME /////////////////////////////////////////////////////////////////
class EmptyFrame : public Gtk::Frame
{
public:
  EmptyFrame(){};
};

/////// BASEFRAME /////////////////////////////////////////////////////////////////
class BaseFrame : public Gtk::Frame
{
public:
  BaseFrame()
  {
    ///// FIXED /////////////////////////////////////////////////////////////
    fixedBackground = new Gtk::Fixed;
    add(*fixedBackground);
    fixedBackground->show();

    ///// SIZE /////////////////////////////////////////////////////////////
    fixedBackground->set_size_request(window_width * 0.9, window_height * 0.9);

    ///// LABEL /////////////////////////////////////////////////////////////
    label = Gtk::manage(new Gtk::Label());
    label->set_halign(Gtk::ALIGN_START);
    label->set_valign(Gtk::ALIGN_CENTER);
    // Attach the new label to the grid
    fixedBackground->put(*label, leftEdge, upperEdge);

    ///// BOX   /////////////////////////////////////
    vBox = new Gtk::Box;
    vBox->set_size_request(window_width * 0.1, window_width * 0.1);
    fixedBackground->put(*vBox, window_width * 0.9, upperEdge);

    ///// CLOCK /////////////////////////////////////
    clock = Gtk::manage(new Clock());
    vBox->pack_start(*clock, Gtk::PACK_EXPAND_WIDGET);
    clock->show();

    fixedBackground->show();
  };
  // data
  Clock *clock;

  Gtk::Fixed *fixedBackground;
  Gtk::Label *label;
  Gtk::Box *vBox;
};

/////// FRAMES /////////////////////////////////////////////////////////////////
class Frames : public BaseFrame
{
public:
  Frames();

  void get_config_data(uint8_t nbr);
  void get_single_config_data(uint8_t nbr, uint8_t index);
  void send_config_data(uint8_t nbr, uint8_t index);
  void send_OTA(uint8_t nbr);
  void send_a_frame(send_frames_t send_frame);
  void sendFunctions2CARS(const uint8_t _function, const uint8_t nbr, const uint8_t val);
  void sendFunctions2CARS(const uint8_t _function, const uint8_t nbr);
  void sendSpeed2CARS(const uint16_t adr);
  void sendDirection2CARS(uint8_t _dir, const uint8_t nbr);

  void getConfigure_ptr(Configure *ptrC, Gtk::Notebook *pTc)
  {
    pConfigure = ptrC;
    ptabControl = pTc;
  }

  void getPlay_ptr(Play *ptr)
  {
    pPlay = ptr;
  }

  canguru_t canguru[maxcanguru] = {0};
  uint8_t canguru_nbr = 0;
  uint8_t current_nbr;

private:
  Gtk::Notebook *ptabControl;
  Configure *pConfigure;
  Play *pPlay;
  // Signal handlers:
  void send_the_config_frame(uint8_t nbr, uint8_t index);
  bool serial_handler(int m_timer_number);
  bool timer(func_t func_data);
  void Setup();
  void sendFRAME2Bridge(const uint8_t *buffer);
  void handleFrame(uint8_t *buffer);
  void on_startPeliCAN();
  void showTextBuffer(const char *buffer);
  void printaFRAME(uint8_t *frame);
  void MSGFromTheBridge(uint8_t msg);
  void get_item_name(uint8_t *buffer, uint8_t &r, uint8_t &c, gchar *name);
  int lookup_my_IP(Glib::ustring &ip_str);

  Gtk::ScrolledWindow *scrollWindow;
  carguruButton *button_start;
  carguruButton *button_quit;
  Gtk::Label *ip_label;

  Gtk::TextView *textEditor;
  Glib::RefPtr<Gtk::TextBuffer> textBuffer;

  uint8_t current_idx;
    status_input_t status_input;
  enum enumData dataType;

  static const uint8_t CAN_FRAME_SIZE = 13;
  static const uint8_t CAN_FRAME_DATA_SIZE = 8;
  const uint8_t offset = 0x10;
  static const uint8_t MAX_CONFIG_CHANNELS = 20;
  const uint8_t scrollWindow_width = 3;

  const uint8_t max_char = 12;

  uint8_t recvdFrame[CAN_FRAME_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t configBuffer[MAX_CONFIG_CHANNELS][CAN_FRAME_DATA_SIZE] = {0x00};

  int uart_handle;

  int m_timer_number = 0;
  uint16_t timer_value = 1;
  func_t func_data;

  Glib::RefPtr<Gtk::Application> app;
};

/////// PLAY /////////////////////////////////////////////////////////////////
class Play : public BaseFrame
{
public:
  Play();
  void set_playcards(Frames *frame);

  std::vector<CAR *> v_CAR;

private:
  // Signal handlers:

  // data
  uint16_t carWidth = window_width * 0.2;
  uint16_t btnleftEdge = carWidth * 0.075;

  Frames *pFrame;
  bool play_data_loaded;
  CAR *car;
};

/////// CONFIGURE /////////////////////////////////////////////////////////////////
class Configure : public BaseFrame
{
public:
  Configure();

  void fillComboBox(Frames *pFrame);
  void createDynamicTreemodel();
  void on_progress_step(double cur);

private:
  Frames *pFrame;
  bool configure_data_loaded;
  // Signal handlers:
  void on_treeview_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn * /* column */);
  void on_save_Data();
  void on_OTA();

  uint8_t current_nbr;

  Gtk::ScrolledWindow *scrollWindow;
  carguruButton *button_save;
  carguruButton *button_OTA;
  Gtk::ProgressBar *m_progressbar;

  const uint8_t scrollWindow_width = 2;
  const uint8_t treeView_width = 1;
  const uint8_t treeView_height = 1;

  // Tree MAIN  model columns: //////////////////////////////////////
  class Main_ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    Main_ModelColumns()
    {
      add(is_Main_ID);
      add(m_Main_col_hash);
      add(m_Main_col_name);
      add(m_Main_col_ip);
    }

    Gtk::TreeModelColumn<uint8_t> is_Main_ID;
    Gtk::TreeModelColumn<Glib::ustring> m_Main_col_hash;
    Gtk::TreeModelColumn<Glib::ustring> m_Main_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_Main_col_ip;
  };

  Main_ModelColumns m_Main_Columns;
  Gtk::TreeView m_Main_TreeView;
  Glib::RefPtr<Gtk::ListStore> m_ref_Main_TreeModel;

  // Dynamic Tree model /////////////////////////////////////////

  virtual void treeviewcolumn_validated_on_cell_data(Gtk::CellRenderer *renderer, const Gtk::TreeModel::iterator &iter);
  virtual void cellrenderer_validated_on_editing_started(Gtk::CellEditable *cell_editable, const Glib::ustring &path);
  virtual void cellrenderer_validated_on_edited(const Glib::ustring &path_string, const Glib::ustring &new_text, uint8_t current_col);

  Gtk::TreeModel::ColumnRecord m_Columns;
  Gtk::TreeModel::Row row;

  Gtk::TreeModelColumn<Glib::ustring> m_col_number_Title;
  std::vector<Gtk::TreeModelColumn<Glib::ustring>> m_col_number_Titles;

  Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
  Gtk::TreeView *m_TreeView;

  // For the validated columns:
  // You could also use a CellRendererSpin or a CellRendererProgress:
  std::vector<Gtk::CellRendererSpin> m_cellrenderer_spin;
  std::vector<Gtk::CellRendererText> m_cellrenderer_text;
  Gtk::TreeView::Column m_treeviewcolumn_validated;
  std::vector<Gtk::TreeView::Column> m_treeviewcolumn;
  bool m_validate_retry;
  Glib::ustring m_invalid_text_for_retry;
  uint8_t hiLimit;
  uint8_t loLimit;
};

#endif
