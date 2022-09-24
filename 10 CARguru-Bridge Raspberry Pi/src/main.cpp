
#include <gtkmm.h>
#include "classdefs.h"

int main(int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "CARguru.app");
  Mainwindow mainwindow(app);
  return app->run(mainwindow);
}
