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
    curs_set(0);

    //display the static information that does not get updated by time
    
    int r = 0; //keeps track of what row we are displaying on
    
    for (int i = 0; i < computer->Hardware->Length; i++, r+=2)
    {
        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Name);
        mvprintw(r, 0, HardwareName.c_str());

        r++; //go to the next roow

        //Iterate over all available sensors
        for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++, r++) {

            std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->Name);
            mvprintw(r, 15, SensorName.c_str());
            
            std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->SensorType.ToString());
            mvprintw(r, 35, SensorType.c_str());
        }
    }

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

                std::string value;
                    
                //Error handling
                if (computer->Hardware[i]->Sensors[j]->Value.HasValue){
                    value = toString(computer->Hardware[i]->Sensors[j]->Value.Value);
                }
                else {
                    value = "NULL";
                }

                mvprintw(r, 50, value.c_str());
            }
        }

        //refresh screen and wait
        refresh();
        Sleep(300);
    }

    //end curses window
    endwin();

    return 0;
}