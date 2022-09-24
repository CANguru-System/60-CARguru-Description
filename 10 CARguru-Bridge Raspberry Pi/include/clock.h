#ifndef GTKMM_EXAMPLE_CLOCK_H
#define GTKMM_EXAMPLE_CLOCK_H

#include <gtkmm/drawingarea.h>

class Clock : public Gtk::DrawingArea
{
    public:
        Clock();
        virtual ~Clock();

    protected:
        double m_radius;
        double m_lineWidth;
        //Override default signal handler:
        bool onSecondElapsed(void);
        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif // GTKMM_EXAMPLE_CLOCK_H