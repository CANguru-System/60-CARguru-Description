#include <iostream>
#include <gtkmm.h>
#include "classdefs.h"

Play::Play()
{
    play_data_loaded = false;
    ///// LABEL /////////////////////////////////////////////////////////////
    label->set_label("Play");
}

void Play::set_playcards(Frames *frame)
{
    uint8_t nbr_cars = 0;
    if (play_data_loaded == true)
        return;
    pFrame = frame;
#ifdef IS_TEST
//    printf("PLAY:Client: %d\n", canguru_nbr);
#endif

    for (uint8_t nbr = 0; nbr < pFrame->canguru_nbr; nbr++)
    {
        if (pFrame->canguru[nbr].device_kind == DEVTYPE_CAR_CAR)
        {
// Fill the ComboBox's Tree Model:
#ifdef IS_TEST
            printf("Kanäle: %d\n", pFrame->canguru[nbr].numberofchannels);
            printf("Device: %d %s\n", nbr, pFrame->canguru[nbr].name);
#endif
            ///// CAR /////////////////////////////////////////////////////////////
            car = new CAR();
            car->get_data_from_PLAY(pFrame, nbr, carWidth, btnleftEdge);
            car->setup_CAR();
            car->show();
            fixedBackground->put(*car, nbr_cars * carWidth * 1.3 + leftEdge, firstBox);
            v_CAR.push_back(car);
            nbr_cars++;
        }
    }
    //    grid->show_all_children();
    play_data_loaded = true;
}
