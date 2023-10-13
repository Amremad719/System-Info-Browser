#include <iostream>
#include <map>
#include <chrono> //Needed for time functions
#include <iomanip> //Needed for setprecision()
#include <sstream> //Needed for stringstream
#include <curses.h> //to display the info
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string

/**
* Takes a float and converts it into a std::string
* @param f The float to be converted
* @param precision the number of decimal digits to take from the float
* @param fixed converts the string with fixed precision if true
* @return The given float as a std::string
*/
std::string toString(const float& f, const int precision = 4, const bool fixed = 0)
{
    //stream to be used in the conversion
    std::stringstream ss;
    
    //the result to be returned at the end
    std::string res;

    //set if the conversion precision is fixed
    if (fixed)
    {
        ss << std::fixed;
    }

    //set the precision and output the float onto the stream
    ss << std::setprecision(precision) << f;

    //input the result string from the stream
    ss >> res;

    //return the result
    return res;
}


/**
* A map used to store what row is the hardware sensor on
* Initialized by printStaticHarwareInfo() and used by updateAndPrintSensorData()
* @key a pair of the hardware index and sensor index
* @value row number on screen
*/
std::map<std::pair<int, int>, int> sensor_screen_row;


/**
* Updates and prints all sensors data from the computer object on the window object
* Uses the sensor_screen_row map to know what row the sensor value should be printed on
* @see printStaticHarwareInfo()
* @param computer The computer object to get the hardware info from
* @param window The Curses window to print the info on
*/
void updateAndPrintSensorData(OpenHardwareMonitor::Hardware::Computer^ computer, WINDOW* window)
{
    //Iterate over all of the available hardware
    for (int i = 0; i < computer->Hardware->Length; i++)
    {
        //Update hardware data
        computer->Hardware[i]->Update();

        //Iterate over all available sensors
        for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++) {

            std::string value; //stores the value to print

            //Error handling
            //if has value set it else set it to "NULL" text
            if (computer->Hardware[i]->Sensors[j]->Value.HasValue) {
                value = toString(computer->Hardware[i]->Sensors[j]->Value.Value);
            }
            else {
                value = "NULL";
            }

            //print data
            mvwprintw(window, sensor_screen_row[std::make_pair(i, j)], 50, value.c_str());
        }
    }
}

/**
* Prints the Hardware name and sensor name and data type once as they do not need to be updated
* @see updateAndPrintSensorData()
* @param computer The computer object to get the hardware info from
* @param window The Curses window to print the info on
* @return The number of rows taken to print all of the info
*/
int printStaticHarwareInfo(OpenHardwareMonitor::Hardware::Computer^ computer, WINDOW* window)
{
    int current_display_row = 0; //keeps track of what row we are displaying on

    for (int hardware_index = 0; hardware_index < computer->Hardware->Length; hardware_index++, current_display_row += 2)
    {
        //convert string and print it
        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Name);
        mvwprintw(window, current_display_row, 0, HardwareName.c_str());

        current_display_row++; //go to the next row

        //Iterate over all available sensors
        for (int sensor_index = 0; sensor_index < computer->Hardware[hardware_index]->Sensors->Length; sensor_index++, current_display_row++) {

            //convert string and print it
            std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->Name);
            mvwprintw(window, current_display_row, 15, SensorName.c_str());

            //convert string and print it
            std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->SensorType.ToString());
            mvwprintw(window, current_display_row, 35, SensorType.c_str());

            //save the position of this sensor to be used in updating the sensor value in printing the dynamic sensor data
            sensor_screen_row[std::make_pair(hardware_index, sensor_index)] = current_display_row;
        }
    }

    return current_display_row;
}



int main()
{
    //init curses screen
    initscr();

    //print wait screen and refresh to show to user
    printw("Loading...... Please wait.");
    refresh();
    
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

    //turn off cursor
    curs_set(0);

    //Enable all mouse events
    mousemask(ALL_MOUSE_EVENTS, NULL);

    //create pad that is needed to be able to print as much data as we want and scroll in it
    WINDOW* pad = newpad(200, 150);

    //enable keypad keys input
    keypad(pad, TRUE);

    //set timeout period for the wgetch() function and therefor limit the update rate of the program
    wtimeout(pad, 5);

    //get max rows and cols of the terminal to be used later in displaying the updating info
    int mxrows = 0, mxcols = 0;
    getmaxyx(stdscr, mxrows, mxcols);

    //display the static information that does not get updated by time
    int totalRows = printStaticHarwareInfo(computer, pad);

    //Clear the waiting text from the display to start displaying the data
    clear();

    //keeps track of what row of the pad we are on
    int mypadpos = 0;
    
    //keeps track of the last time we polled the data to be used in limiting the poll rate
    auto prev_time = std::chrono::high_resolution_clock::now();

    //makrs if it is our time polling the data to avoid waiting for the poll rate limit the first time
    bool first_poll = 1;

    //the delay for the poll rate of the data in milliseconds
    int poll_delay = 500;
    
    //main runtime loop
    while (1)
    {
        //current time to be compared to the last time we polled to limit the poll rate
        auto now_time = std::chrono::high_resolution_clock::now();

        //check if duration since the last time we polled is grater than the desired rate or if it is the first time polling the data
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now_time - prev_time).count() >= poll_delay || first_poll)
        {
            //mark the first_poll boolean as false as it is not the first poll no more
            first_poll &= 0;

            //update last data poll time
            prev_time = now_time;

            //Call updating and printing function
            updateAndPrintSensorData(computer, pad);
        }

        //update pad within the window
        prefresh(pad, mypadpos, 0, 0, 0, mxrows -1, mxcols - 1);
        
        //mouse event to be used to determine what mouse button was pressed
        MEVENT event;

        //get input form user
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
                //check mouse wheel down
                else if ((event.bstate & BUTTON5_PRESSED) &&
                    mypadpos < totalRows - (mxrows > totalRows ? totalRows : mxrows))
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