#include <iostream>
#include <gtkmm.h>
#include "classdefs.h"
#include "wiringPi.h"
#include "wiringSerial.h"
#include <vector>
#include <netdb.h>
#include <arpa/inet.h>

Frames::Frames()
{
    ///// LABEL /////////////////////////////////////////////////////////////
    label->set_label("Frames");

    ///// ScrolledWindow /////////////////////////////////////////////////////////////
    // Create a scrolling pane with the vertical scroll bar always showing
    // and the horizontal scroll bar only showing when needed
    Gtk::ScrolledWindow *scrollWindow = Gtk::manage(new Gtk::ScrolledWindow);
    scrollWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    // Add grid inside scrolling pane, and set pane to expand and contract
    // as the window is resized (other elements would remain fixed size)
    scrollWindow->set_hexpand(false);
    scrollWindow->set_vexpand(false);
    //    scrollWindow->set_size_request(column_spacing * right_end, row_spacing * bottom_end);
    scrollWindow->set_shadow_type(Gtk::SHADOW_OUT);
    scrollWindow->set_size_request(window_width * 0.5, window_height * 0.9);
    // Add the scrolling pane in which the grid resides to the vertical box
    fixedBackground->put(*scrollWindow, leftEdge, firstBox);

    ///// textEditor /////////////////////////////////////////////////////////////
    textEditor = Gtk::manage(new Gtk::TextView());
    textEditor->set_editable(false);
    textEditor->set_cursor_visible(false);
    textBuffer = textEditor->get_buffer();

    ///// SET TEXT & ADD 2 WINDOW ////////////////////////////////////
    textBuffer->set_text("Push START-Button\n");

    ///// FONT ////////////////////////////////////
    Pango::FontDescription fdesc("Monospace 10");
    textEditor->override_font(fdesc);

    scrollWindow->add(*textEditor);

    ///// BUTTON START ///////////////////////////////////////////////////////
    button_start = Gtk::manage(new carguruButton());
    button_start->set_label("START");
    button_start->signal_clicked().connect(sigc::mem_fun(*this, &Frames::on_startPeliCAN));
    // Attach the new button to the grid
    fixedBackground->put(*button_start, leftEdge, window_height * 0.975);

    ///// START SERIAL_HANDLER /////////////////////////////////////////////////
    // Creation of a new object prevents long lines and shows us a little
    // how slots work.  We have 0 parameters and bool as a return value
    // after calling sigc::bind.
    sigc::slot<bool()> my_slot = sigc::bind(sigc::mem_fun(*this, &Frames::serial_handler), m_timer_number);

    // This is where we connect the slot to the Glib::signal_timeout()
    auto conn = Glib::signal_timeout().connect(my_slot, timer_value);

    ///// SHOW_ALL /////////////////////////////////////////////////////////////
    // Make the box and everything in it visible
    //    grid->show_all();
    Setup();
}

void Frames::showTextBuffer(const char *buffer)
{
    ///// INSERT NEW TEXT /////////////////////////////////////////////////////////////
    Gtk::TextIter iter = textBuffer->get_iter_at_offset(textBuffer->get_char_count());
    textBuffer->insert(iter, buffer);
    ///// SCROLL 2 BOTTOM /////////////////////////////////////////////////////////////
    Glib::RefPtr<Gtk::Adjustment> adj = textEditor->get_vadjustment();
    adj->set_value(adj->get_upper());
}

void Frames::printaFRAME(uint8_t *frame)
{
    static gchar inBuffer[buflng];
    static gchar direction[buflng10];
    char c;
    switch (frame[0x00])
    {
    case toBridge:
        g_snprintf(direction, buflng10, "-->Brdg");
        break;
    case fromBridge:
        g_snprintf(direction, buflng10, "Brdg-->");
        break;
    case toMaster:
        g_snprintf(direction, buflng10, "-->Mstr");
        break;
    case fromMaster:
        g_snprintf(direction, buflng10, "<--Mstr");
        break;
    case toClnt:
        g_snprintf(direction, buflng10, "<--Clnt");
        break;
    case fromClnt:
        g_snprintf(direction, buflng10, "Clnt-->");
        break;
    default:
        g_snprintf(direction, buflng10, "-->xxxx");
        break;
    }
    showTextBuffer(direction);
    // Richtungskenner wird nicht mehr benötigt
    frame[0x00] = 0x00;
    if (frame[0x01] % 2 == 0)
        c = ' ';
    else
        c = 'R';
    char str[10] = {'.', '.', '.', '.', '.', '.', '.', '.', '\n', '\0'};
    for (uint8_t i = 5; i < CAN_FRAME_SIZE; i++)
    {
        if (frame[i] >= ' ' && frame[i] <= 'z')
        {
            str[i - 5] = frame[i];
        }
    }
    g_snprintf(inBuffer, buflng, "%02X(%02X)%02X%02X %c [%02X] %02X %02X %02X %02X %02X %02X %02X %02X %s",
               frame[0x00], frame[0x01], frame[0x02], frame[0x03], c, frame[0x04],
               frame[0x05], frame[0x06], frame[0x07], frame[0x08], frame[0x09],
               frame[0x0A], frame[0x0B], frame[0x0C], str);

    showTextBuffer(inBuffer);
}

void Frames::MSGFromTheBridge(uint8_t msg)
{
    static gchar message[buflng20];
    switch (msg)
    {
    case 0x00:
        g_snprintf(message, buflng20, "received CAN ping");
        break;
    case 0x01:
        g_snprintf(message, buflng20, "CAN magic 60113 start");
        break;
    case 0x02:
        g_snprintf(message, buflng20, "CAN enabled all loco protos");
        break;
    case 0x03:
        g_snprintf(message, buflng20, "Start Train-Application\n");
        break;

    default:
        g_snprintf(message, buflng20, "don't know");
        break;
    }
    //   String[] messages = {
    //        /*00*/ "received CAN ping",
    //        /*01*/ "CAN magic 60113 start",
    //        /*02*/ "CAN enabled all loco protos",
    //        /*03*/ "Start Train-Application",
    //        /*04*/ " -- No Slaves!",
    //        /*05*/ "replied CAN ping (fake member)",
    //        /*06*/ "Meldeüunkt0",
    //        /*07*/ "Meldeüunkt1",
    //        /*08*/ "Meldeüunkt2"};
    showTextBuffer(message);
}

void Frames::sendFRAME2Bridge(const uint8_t *buffer)
{
    serialPutchar(uart_handle, data);
    for (uint8_t i = 0; i < CAN_FRAME_SIZE; i++)
    {
        char buf[3];
        sprintf(buf, "%02X", buffer[i]);
        //  printf ("[%s] is a string %d chars long\n",buffer,n);
        // Format einer HEX-Zahl: Index plus 16 als Zahl - erstes Byte in ASCII- zweittes Byte in ASCII
        serialPutchar(uart_handle, i + offset);
        serialPutchar(uart_handle, buf[0]);
        serialPutchar(uart_handle, buf[1]);
    }
    // Ende der Datenübertragung
}

void Frames::sendDirection2CARS(uint8_t _dir, const uint8_t nbr)
{
    uint8_t SEND_DIR[] = {toClnt, Lok_Direction, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t adr = DCC_TRACK + canguru[nbr].adr;
    SEND_DIR[0x07] = adr >> 8;
    SEND_DIR[0x08] = adr;
    // 0 = Fahrtrichtung bleibt
    // 1 = Fahrtrichtung vorwärts
    // 2 = Fahrtrichtung rückwärts
    // 3 = Fahrtrichtung umschalten
    SEND_DIR[0x09] = _dir + 1;
    printaFRAME(SEND_DIR);
    sendFRAME2Bridge(SEND_DIR);
    //    printf("Dir: %d Nbr: %d\n", _dir, nbr);
}

void Frames::sendSpeed2CARS(const uint16_t adr)
{
    uint8_t SEND_SPEED[] = {toClnt, Lok_Speed, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t _adr;
    for (auto car : pPlay->v_CAR)
    {
        if (car->adr == adr)
        {
            _adr = adr + DCC_TRACK;
            SEND_SPEED[data2] = _adr >> 8;
            SEND_SPEED[data3] = _adr;
            SEND_SPEED[data4] = car->Lok_speed_curr >> 8;
            SEND_SPEED[data5] = car->Lok_speed_curr;
            printaFRAME(SEND_SPEED);
            sendFRAME2Bridge(SEND_SPEED);
        }
    }
}

void Frames::sendFunctions2CARS(const uint8_t _function, const uint8_t nbr, const uint8_t val)
{
    uint8_t SEND_FUNC[] = {toClnt, Lok_Function, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t adr = DCC_TRACK + canguru[nbr].adr;
    SEND_FUNC[0x07] = adr >> 8;
    SEND_FUNC[0x08] = adr;
    SEND_FUNC[0x09] = _function;
    SEND_FUNC[0x0A] = val;
    printaFRAME(SEND_FUNC);
    sendFRAME2Bridge(SEND_FUNC);
}

void Frames::sendFunctions2CARS(const uint8_t _function, const uint8_t nbr)
{
    // switch function on
    sendFunctions2CARS(_function, nbr, 0x01);
    func_data.nbr = nbr;
    func_data.timer_number = 1;
    func_data.fction = _function;
    sigc::slot<bool()> my_slot = sigc::bind(sigc::mem_fun(*this, &Frames::timer), func_data);

    // This is where we connect the slot to the Glib::signal_timeout()
    auto conn = Glib::signal_timeout().connect(my_slot, 500);
}
bool Frames::timer(func_t func_data)
{
    //    printf("TIMER: %d\n", func_data.nbr);
    // switch function off
    sendFunctions2CARS(func_data.fction, func_data.nbr, 0x00);
    return false;
}

///////// BUTTON-Funktionen ///////////////////////////////////////////////////

/* our usual callback function */
void callback(GtkWidget *widget, gpointer *data)
{
    g_print("Hello again - %s was pressed\n", (char *)data);
}

void Frames::on_startPeliCAN()
{
    send_a_frame(start);
}

///////// SENDE-Funktionen ///////////////////////////////////////////////////
void Frames::get_config_data(uint8_t nbr)
{
    current_nbr = nbr;
    current_idx = 0x01;
    send_the_config_frame(current_nbr, current_idx);
}

void Frames::send_config_data(uint8_t nbr, uint8_t index)
{
    uint8_t SEND_CONFIG[] = {toClnt, SYS_CMD, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00};
    uint8_t new_value;
    char *pchEnd = 0;
    if ((nbr + 1) > canguru_nbr)
        return;
    uint8_t nbr_channels = canguru[nbr].numberofchannels;
    if ((index + 1) > nbr_channels)
        return;
    current_idx = index;
    if (index == 0x00)
    {
        current_nbr = nbr;
    }
    // UID eintragen
    for (uint8_t uid = 0; uid < uid_lng; uid++)
        SEND_CONFIG[5 + uid] = canguru[nbr].UID[uid];
    SEND_CONFIG[0x0A] = index + 1;
    new_value = strtol(canguru[current_nbr].Kanaele.at(index).newValue, &pchEnd, 10);
    SEND_CONFIG[0x0C] = new_value;
    printaFRAME(SEND_CONFIG);
    sendFRAME2Bridge(SEND_CONFIG);
}

void Frames::send_OTA(uint8_t nbr)
{
    uint8_t OTA_START[] = {0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, START_OTA, 0x00, 0x00, 0x00};
    // UID eintragen
    for (uint8_t uid = 0; uid < uid_lng; uid++)
        OTA_START[5 + uid] = canguru[nbr].UID[uid];
    printaFRAME(OTA_START);
    sendFRAME2Bridge(OTA_START);
}

void Frames::send_the_config_frame(uint8_t nbr, uint8_t index)
{
    uint8_t GET_CONFIG[] = {toClnt, CONFIG_Status, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if ((nbr + 1) > canguru_nbr)
        return;
    // UID eintragen
    for (uint8_t uid = 0; uid < uid_lng; uid++)
        GET_CONFIG[5 + uid] = canguru[nbr].UID[uid];
    // Paketnummer
    GET_CONFIG[0x09] = index;
    printaFRAME(GET_CONFIG);
    sendFRAME2Bridge(GET_CONFIG);
}

void Frames::send_a_frame(send_frames_t send_frame)
{
    uint8_t sendFrame[CAN_FRAME_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t GET_PING[] = {toClnt, PING, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t GET_IP[] = {toClnt, SEND_IP, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t RESTART_BRIDGE[] = {0x00, restartBridge, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t startFrame[] = {toBridge, CALL4CONNECT, 0x47, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    switch (send_frame)
    {
    case ping:
        memcpy(sendFrame, GET_PING, CAN_FRAME_SIZE);
        break;
    case ip:
        memcpy(sendFrame, GET_IP, CAN_FRAME_SIZE);
        break;
    case start:
        memcpy(sendFrame, startFrame, CAN_FRAME_SIZE);
        break;
    case restart:
        memcpy(sendFrame, RESTART_BRIDGE, CAN_FRAME_SIZE);
        break;
    }
    printaFRAME(sendFrame);
    sendFRAME2Bridge(sendFrame);
}

void Frames::get_item_name(uint8_t *buffer, uint8_t &r, uint8_t &c, gchar *name)
{
    bool max_reached = false;
    uint8_t i = 0x00;
    while (configBuffer[r][c] != 0x00 && !max_reached)
    {
        switch (configBuffer[r][c])
        {
        case 0xD6: // Ö
            name[i] = 'O';
            i++;
            name[i] = 'E';
            break;
        case 0xF6: // ö
            name[i] = 'o';
            i++;
            name[i] = 'e';
            break;

        default:
            name[i] = (configBuffer[r][c] & 0x7F);
            break;
        }
        i++;
        c++;
        if (c == 0x08)
        {
            // nach der Spalte 8 wweiter in der nächsten Zeile vorne
            c = 0x00;
            r++;
        } // if
        if (i >= max_char)
        {
            //
            name[i] = '.';
            i++;
            max_reached = true;
        } // if
    }
    name[i] = 0x00;
    c++;
    if (c == 0x08)
    {
        // nach der Spalte 8 wweiter in der nächsten Zeile vorne
        c = 0x00;
        r++;
    } // if
}

void Frames::handleFrame(uint8_t *buffer)
{
    uint8_t row;
    uint8_t col;
    kanal_t *new_kanal;
    static uint8_t index;
    bool found = false;
    uint16_t adr;
    uint16_t speed;
    //    printf("CMD: %02X\n", buffer[opCmd]);
    switch (buffer[opCmd])
    {
    case Lok_Speed_R:
        // 00 (09) D7 15 R [06] 00 00 C0 CA 00 05 00 00
        /*
        uint16_t adr = DCC_TRACK + canguru[nbr].adr;
        SEND_SPEED[0x07] = adr >> 8;
        SEND_SPEED[0x08] = adr;
        SEND_SPEED[0x09] = int(_speed) >> 8;
        SEND_SPEED[0x0A] = int(_speed);
        */
        printaFRAME(buffer);
        adr = (buffer[data2] << 8) + buffer[data3] - DCC_TRACK;
        speed = (buffer[data4] << 8) + buffer[data5];
        for (auto car : pPlay->v_CAR)
        {
            if (car->adr == adr)
            {
                // stop immediatly
                if (car->direction == stop)
                {
                    speed = 0;
                    car->direction = no_dir;
                }
                else
                {
                    // keep running
                    if (car->direction == no_dir)
                        break;
                    // go faster
                    if (car->direction == faster)
                    {
                        // speed is increasing
                        if (speed < car->Lok_speed_dest)
                            speed += car->steps;
                        if (speed >= car->Lok_speed_dest)
                        {
                            if (speed > 1024)
                                speed = 1024;
                            speed = car->Lok_speed_dest;
                            car->direction = no_dir;
                        }
                    }
                    // go slower
                    if (car->direction == slower)
                    {
                        // speed is increasing
                        if (speed > car->Lok_speed_dest)
                            speed -= car->steps;
                        if ((speed > 1024) || (speed <= car->Lok_speed_dest))
                        {
                            car->direction = no_dir;
                            speed = car->Lok_speed_dest;
                        }
                    }
                }
                car->Lok_speed_curr = speed;
                buffer[dest] = toClnt;
                buffer[opCmd] = Lok_Speed;
                buffer[data4] = int(speed) >> 8;
                buffer[data5] = int(speed);
                printaFRAME(buffer);
                //            printf("RET: %d\n", int(car->Lok_speed_curr));
                sendFRAME2Bridge(buffer);
            }
        }
        break;
    case SYS_CMD_R:
        printaFRAME(buffer);
        send_config_data(current_nbr, current_idx + 1);
        break;
    case PING_R:
        // jeder Decoder meldet sich und teilt seinen HASH-Wert (z.B. D7 17) mit
        printaFRAME(buffer);
        found = false;
        for (uint8_t c = 0; c < canguru_nbr; c++)
        {
            // ein freier Speicherplatz
            if ((canguru[c].HASH[0x00] == 0x00) && (canguru[c].HASH[0x01]) == 0x00)
            {
                canguru[c].HASH[0x00] = buffer[0x02];
                canguru[c].HASH[0x01] = buffer[0x03];
                for (uint8_t uid = 0; uid < uid_lng; uid++)
                {
                    canguru[c].UID[uid] = buffer[5 + uid];
                }
                canguru[c].device_kind = buffer[0x0C];
#ifdef IS_TEST
                printf("HASH: %d successfully written\n", c + 1);
#endif
                found = true;
                if (c == (canguru_nbr - 1))
                {
                    // Wenn sich alle Decoder gemeldet haben,
                    // wird die Aufforderung zum Senden der IP-Adresse
                    // der Decoder verschickt
                    send_a_frame(ip);
                }
            }
            if (found == true)
                break;
        }
        break;
    case SEND_IP_R: // get IP
        // jeder Decoder meldet sich, identifiziert sich mit seinen HASH-Wert und teilt seine IP-Adresse (z.B. 192.168.178.62) mit
        printaFRAME(buffer);
        found = false;
        for (uint8_t c = 0; c < canguru_nbr; c++)
        {
            if ((canguru[c].HASH[0x00] == buffer[0x02]) && (canguru[c].HASH[0x01]) == buffer[0x03])
            {
                for (uint8_t ip = 0; ip < 4; ip++)
                {
                    canguru[c].IP[ip] = buffer[5 + ip];
                }
                canguru[c].adr = (buffer[0x09] << 8) + buffer[0x0A];
#ifdef IS_TEST
                printf("IP: %d successfully written\n", c + 1);
#endif
                found = true;
                if (c == (canguru_nbr - 1))
                    // Wenn sich alle Decoder gemeldet haben,
                    // wird die Aufforderung zum Senden der CONFIG-Daten
                    // der Decoder verschickt
                    send_the_config_frame(0x00, 0x00);
            }
            if (found == true)
                break;
        }
        break;
    case CONFIG_Status_R:
        printaFRAME(buffer);
        // Zwischenspeichern; der index wird in jeder Zeile auf Position 3 statt HASH geliefert
        if (buffer[dataLength] == 0x08)
            index = buffer[hash1] - 1;
        else
            index++;
        memcpy(configBuffer[index], &buffer[data0], CAN_FRAME_DATA_SIZE);
        // Letzte Zeile ist 6 Byte lang, sonst 8 Byte
        // jetzt wird ausgewertet
        // printf("CONFIG_Status: %02X\n", buffer[dataLength]);
        if (buffer[dataLength] == 0x06)
        {
            // bei current_nbr gleich 0xFF werden die Kopfdaten des Client abgefragt
            if (current_nbr == 0xFF) // current_idx == 0x00)
            {
                found = false;
                // Suche den passenden Client (anhand des Hashwertes)
                for (uint8_t c = 0; c < canguru_nbr; c++)
                {
                    if ((canguru[c].HASH[0x00] == buffer[0x02]) && (canguru[c].HASH[0x01]) == buffer[0x03])
                    {
                        found = true;
                        current_nbr = c;
#ifdef IS_TEST
                        printf("Config: %02X%02X client\n", buffer[0x02], buffer[0x03]);
#endif
                        // numberofchannels
                        canguru[current_nbr].numberofchannels = configBuffer[0x00][0x01];
                        // Arraygröße dynamisch anpassen
                        canguru[current_nbr].Kanaele.resize(canguru[current_nbr].numberofchannels);
                        // name
                        row = 0x02;
                        col = 0x00;
                        get_item_name(configBuffer[row], row, col, canguru[current_nbr].name);
                    } // if
                    if (found == true)
                    {
                        memset(configBuffer, 0x00, sizeof(configBuffer));
#ifdef IS_TEST
                        printf("Name: %s\n", canguru[current_nbr].name);
#endif
                        send_the_config_frame(current_nbr + 0x01, 0x00);
                        // auch bei der evtl. nächsten Abfrage nur Kopfdaten
                        current_nbr = 0xFF;
                        break;
                    } // if
                }     // for
            }         // if
            else
            {
                // bei current_nbr ungleich 0xFF werden die restlichen Daten des Client current_nbr abgefragt
                current_idx = configBuffer[0x00][0x00];
                // Clnt-->00(3B)0001 R [08] 01 02 00 0A 00 64 00 1E .....d..
                // Char    Konfigurationskanalnummer   0x05: Setzen unter Kanal 5
                canguru[current_nbr].Kanaele[current_idx - 1].kanalnbr = current_idx;
                // Char    Kenner Slider               Wert 2
                canguru[current_nbr].Kanaele[current_idx - 1].grundformat = configBuffer[0x00][0x01];
                // Word    Unterer Wert                0
                canguru[current_nbr].Kanaele[current_idx - 1].untererWert = (configBuffer[0x00][0x02] << 8) + configBuffer[0x00][0x03];
                // Word    Oberer Wert                 660
                canguru[current_nbr].Kanaele[current_idx - 1].obererWert = (configBuffer[0x00][0x04] << 8) + configBuffer[0x00][0x05];
                // Word    Aktuelle Einstellung        500
                canguru[current_nbr].Kanaele[current_idx - 1].aktuellerWert = (configBuffer[0x00][0x06] << 8) + configBuffer[0x00][0x07];
                // Clnt-->00(3B)0002 R [08] 53 65 72 76 6F 76 65 72 Servover
                // Clnt-->00(3B)0003 R [08] 7A F6 67 65 72 75 6E 67 z.gerung
                // Clnt-->00(3B)0004 R [08] 00 3A 00 31 30 30 00 6D .:.100.m
                // Clnt-->00(3B)0005 R [08] 73 00 00 00 00 00 00 00 s.......
                // Clnt-->00(3B)D716 R [06] 45 00 91 96 01 05 00 00 E.......
                // String  Auswahlbezeichnung          Variable Strombegrenzung\0
                row = 0x01;
                col = 0x00;
                get_item_name(configBuffer[row], row, col, canguru[current_nbr].Kanaele[current_idx - 1].auswahlbezeichnung);
#ifdef IS_TEST
                printf("auswahlbezeichnung: %s\n", canguru[current_nbr].Kanaele[current_idx - 1].auswahlbezeichnung);
#endif
                // String  Bezeichnung Start           0.000\0
                get_item_name(configBuffer[row], row, col, canguru[current_nbr].Kanaele[current_idx - 1].start);
#ifdef IS_TEST
                printf("start: %s\n", canguru[current_nbr].Kanaele[current_idx - 1].start);
#endif
                // String  Bezeichnung Ende            2.500\0
                get_item_name(configBuffer[row], row, col, canguru[current_nbr].Kanaele[current_idx - 1].ende);
#ifdef IS_TEST
                printf("ende: %s\n", canguru[current_nbr].Kanaele[current_idx - 1].ende);
#endif
                // String  Einheit                     Achsen\0 oder A\0
                get_item_name(configBuffer[row], row, col, canguru[current_nbr].Kanaele[current_idx - 1].einheit);
#ifdef IS_TEST
                printf("einheit: %s\n", canguru[current_nbr].Kanaele[current_idx - 1].einheit);
#endif
                //
                if (canguru[current_nbr].numberofchannels > current_idx)
                {
                    current_idx++;
                    send_the_config_frame(current_nbr, current_idx);
#ifdef IS_TEST
                    printf("index1: %d von %d\n", current_idx, canguru[current_nbr].numberofchannels);
#endif
                    double percent = (double)(current_idx - 1) / (double)canguru[current_nbr].numberofchannels;
                    pConfigure->on_progress_step(percent);
                } // if
                else
                {
                    current_nbr = 0xFF;
                    pConfigure->createDynamicTreemodel();
                } // else
            }     // else
        }         // if
        break;
    case CALL4CONNECT_R:
        printaFRAME(buffer);
        // Antwort der Bridge
        // in 0x05 steht die Anzahl der Decoder
        canguru_nbr = buffer[0x05];
        if (canguru_nbr > 0)
        {
            // show tabs when start is done
            ptabControl->get_nth_page(configure)->set_visible(true);
            ptabControl->get_nth_page(play)->set_visible(true);
        }
#ifdef IS_TEST
        printf("Found: %d clients\n", canguru_nbr);
#endif
        // versendet anschließend einen PING an alle Decoder
        send_a_frame(ping);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool Frames::serial_handler(int m_timer_number)
{
    static gchar inString[buflng];
    static uint8_t uindex = 0;
    static char bytes[] = "00";
    uint8_t c;

    // Beginn der Übertragung
    // Datenübertragung
    switch (status_input)
    {
    case waiting:
        dataType = no_data;
        // Beginn der Übertragung
        if (serialDataAvail(uart_handle) > 0)
            dataType = enumData(serialGetchar(uart_handle));
        if (dataType == no_data)
            return TRUE;
        // Datenübertragung
        switch (dataType)
        {
        case data:
        case strng:
            uindex = 0;
            status_input = dataBytes;
#ifdef IS_TEST
            printf("data: %d\n", dataType);
#endif
            break;
        }
        break;
    case dataBytes:
        switch (dataType)
        {
        case data:
            if (serialDataAvail(uart_handle) > 0)
            {
                c = serialGetchar(uart_handle);
#ifdef IS_TEST
                printf("data: %d\n", c);
#endif
                recvdFrame[uindex] = c;
                uindex++;
                if (uindex >= CAN_FRAME_SIZE)
                {
                    status_input = waiting;
                    if (recvdFrame[0x00] == MSGfromBridge)
                        MSGFromTheBridge(recvdFrame[0x01]);
                    else
                        handleFrame(recvdFrame);
                    memset(recvdFrame, 0x00, CAN_FRAME_SIZE);
                }
            }
            break;
        case strng:
            if (serialDataAvail(uart_handle) > 0)
            {
                c = serialGetchar(uart_handle);
                switch (c)
                {
                case '\0':
                    status_input = waiting;
                    break;
                case '\n':
                    inString[uindex] = c;
                    showTextBuffer(inString);
                    memset(inString, 0x00, buflng);
                    uindex = 0;
                    status_input = waiting;
                    break;
                default:
                    inString[uindex] = c;
                    uindex++;
                    break;
                }
            }
        }
        break;
        // Format einer HEX-Zahl: Index plus 16 als Zahl - erstes Byte in ASCII- zweittes Byte in ASCII
    }
    return TRUE;
}

void Frames::Setup()
{
    ///// UART Initialization /////////////////////////////////////////////////////////////
    /*
A very quick example:
handle = serialOpen ("/dev/ttyAMA0", 115200) ;
that will open the on-board serial port. It you want to talk to a USB serial, then you need to identify its device – usually /dev/ttyUSB0 or /dev/ttyACM0, and use that in the open call:
handle = serialOpen ("/dev/ttyUSB0", 9600) ;
for example.
Then you can use handle in subsequent calls – e.g.
data = serialGetchar (handle) ;
and so on.
Hope this helps.
link: http://wiringpi.com/reference/serial-library/
    */
    uart_handle = serialOpen("/dev/ttyS0", baudrate);
    if (uart_handle != -1)
        printf("UART started successfully\n");
    status_input = waiting;
    canguru_nbr = 0;
    current_nbr = 0xFF;
    current_idx = 0x00;
    ///// IP_LABEL /////////////////////////////////////////////////////////////
    Glib::ustring ipBuffer;
    lookup_my_IP(ipBuffer);
    ip_label = Gtk::manage(new Gtk::Label());
    ip_label->set_label(ipBuffer);
    ip_label->set_halign(Gtk::ALIGN_START);
    ip_label->set_valign(Gtk::ALIGN_CENTER);
    ip_label->set_lines(2);
    // Attach the new label to the grid
    fixedBackground->put(*ip_label, window_width * 0.7, upperEdge);
}

////////////////////////////////////////////////////////////

int Frames::lookup_my_IP(Glib::ustring &ip_str)
{
    struct addrinfo hints, *res, *result;
    int errcode;
    char addrstr[buflng30];
    char addrstr0[buflng10];
    char addrstr1[buflng10];
    void *ptr;
    uint8_t ip_nbr = 0;
    char hostname[100];

    if (gethostname(hostname, 100) == 0)
        strcat(hostname, ".");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(hostname, NULL, &hints, &result);
    if (errcode != 0)
    {
        perror("getaddrinfo");
        return -1;
    }

    res = result;

    while (res)
    {
        //        inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family)
        {
        case AF_INET:
            ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
            break;
            //        case AF_INET6:
            //            ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
            //            break;
        }
        if (ip_nbr == 0)
            inet_ntop(res->ai_family, ptr, addrstr0, 100);
        else
            inet_ntop(res->ai_family, ptr, addrstr1, 100);
        //        printf("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        //        printf("IPv4 address: %s (%d)\n", addrstr, res->ai_protocol);
        ip_nbr++;
        res = res->ai_next;
    }
    g_snprintf(addrstr, buflng30, "IP0: %s\nIP1: %s", addrstr0, addrstr1);
    ip_str = addrstr;
    freeaddrinfo(result);

    return 0;
}

////////////////////////////////////////////////////////////