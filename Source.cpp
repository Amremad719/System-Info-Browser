#include <iostream>
#include <iomanip> //Needed for setprecision()
#include <sstream> //Needed for stringstream
#include <curses.h> //to display the info
#include <windows.h> //Needed for the Sleep() function
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string

//converts a float to a std::string
std::string toString(const float& f, const int precision = 4)
{
    std::stringstream ss;
    
    std::string res;

    ss << std::setprecision(precision) << f;
    ss >> res;

    return res;
}

int main()
{
    //Initialize main object
    OpenHardwareMonitor::Hardware::Computer^ computer = gcnew OpenHardwareMonitor::Hardware::Computer();

    //Tell the library what hardware we want to monitor
    computer->CPUEnabled = true;
    computer->GPUEnabled = true;
    computer->HDDEnabled = true;
    computer->RAMEnabled = true;
    computer->MainboardEnabled = true;
    computer->FanControllerEnabled = true;

    //Start the session
    computer->Open();

    //init curses screen
    initscr();

    //turn off cursor
    curs_set(0);

    //Enable all mouse events
    mousemask(ALL_MOUSE_EVENTS, NULL);

    // create pad
    WINDOW* pad = newpad(200, 150);

    //enable keypad keys input
    keypad(pad, TRUE);

    wtimeout(pad, 300);

    //get max rows and cols of the terminal to be used later
    int mxrows = 0, mxcols = 0;
    getmaxyx(stdscr, mxrows, mxcols);

    //display the static information that does not get updated by time
    
    int r = 0; //keeps track of what row we are displaying on
    
    for (int i = 0; i < computer->Hardware->Length; i++, r+=2)
    {
        //convert string and print it
        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Name);
        mvwprintw(pad, r, 0, HardwareName.c_str());

        r++; //go to the next row

        //Iterate over all available sensors
        for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++, r++) {

            //convert string and print it
            std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->Name);
            mvwprintw(pad, r, 15, SensorName.c_str());
            
            //convert string and print it
            std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->SensorType.ToString());
            mvwprintw(pad, r, 35, SensorType.c_str());
        }
    }

    //keeps track of what row of the pad we are on
    int mypadpos = 0;
    
    //main runtime loop
    while (1)
    {
        //reset output row
        r = 1;

        //Iterate over all of the available hardware
        for (int i = 0; i < computer->Hardware->Length; i++, r+=3)
        {
            //Update hardware data
            computer->Hardware[i]->Update();

            //Iterate over all available sensors
            for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++, r++){

                std::string value; //stores the value to print
                    
                //Error handling
                //if has value set it else set it to "NULL" text
                if (computer->Hardware[i]->Sensors[j]->Value.HasValue){
                    value = toString(computer->Hardware[i]->Sensors[j]->Value.Value);
                }
                else {
                    value = "NULL";
                }

                //print data
                mvwprintw(pad, r, 50, value.c_str());
            }
        }

        //update pad within the window
        prefresh(pad, mypadpos, 0, 0, 0, mxrows -1, mxcols - 1);
        
        //mouse event
        MEVENT event;

        //get input
        char ch = wgetch(pad);

        //check if input is a mouse input
        if (ch == 27)
        {
            //if fetched mouse input without errors
            if (nc_getmouse(&event) == OK)
            {
                //check mouse wheel up
                if ((event.bstate & BUTTON4_PRESSED) && mypadpos > 0)
                {
                    mypadpos--;
                }
                //check mouse wheel up
                else if ((event.bstate & BUTTON5_PRESSED) && mypadpos < 200)
                {
                    mypadpos++;
                }
            }
        }
    }

    //end curses window
    endwin();

    return 0;
}