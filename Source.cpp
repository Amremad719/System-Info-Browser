#include <iostream>
#include <map>
#include <algorithm>
#include <chrono> //Needed for time functions
#include <iomanip> //Needed for setprecision()
#include <sstream> //Needed for stringstream
#include <curses.h> //to display the info
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string
#include "SessionRecorder.h"
#include "StorageInformation.h"
#include "GlobalFunctions.h"
#include "NetworkInformation.h"

/**
* A map used to store what row is the hardware sensor on
* Initialized by printStaticHarwareInfo() and used by updateAndPrintSensorData()
* @key a pair of the hardware index and sensor index
* @value row number on screen
*/
std::map<std::pair<int, int>, int> sensor_screen_row;

/**
* Stores what hardware index and sensor index the free ram sensor is
*/
std::pair<int, int> free_ram_index;

/**
* The object that manages the recording of the session
*/
SessionRecorder sessionRecorder;

/**
* Updates and prints all sensors data from the computer object on the window object
* Uses the sensor_screen_row map to know what row the sensor value should be printed on
* @see printStaticHarwareInfo()
* @param computer The computer object to get the hardware info from
* @param window The Curses window to print the info on
*/
void updateAndPrintSensorData(OpenHardwareMonitor::Hardware::Computer^ computer, WINDOW* window, bool add_to_buffer = 0)
{
    //Iterate over all of the available hardware
    for (int hardware_index = 0; hardware_index < computer->Hardware->Length; hardware_index++)
    {
        //Update hardware data
        computer->Hardware[hardware_index]->Update();

        //Iterate over all available sensors
        for (int sesnor_index = 0; sesnor_index < computer->Hardware[hardware_index]->Sensors->Length; sesnor_index++) {

            //If there is not a row assigned to the current sensor do not print it
            if (sensor_screen_row.find(std::make_pair(hardware_index, sesnor_index)) == sensor_screen_row.end())
            {
                continue;
            }

            std::string value; //stores the value to print

            //if add_to_buffer or in other terms if the session is being recorded then add the values to the buffer
            if (sessionRecorder.isRecording())
            {
                sessionRecorder.record_buffer[hardware_index][sesnor_index].push(computer->Hardware[hardware_index]->Sensors[sesnor_index]->Value.Value);
            }

            //Error handling
            //if has value set it else set it to "NULL" text
            if (computer->Hardware[hardware_index]->Sensors[sesnor_index]->Value.HasValue) {
                value = toString(computer->Hardware[hardware_index]->Sensors[sesnor_index]->Value.Value);

            }
            else 
            {
                value = "NULL";
            }

            //print data
            mvwprintw(window, sensor_screen_row[std::make_pair(hardware_index, sesnor_index)], 50, value.c_str());
        }
        //if add_to_buffer or in other terms if the session is being recorded then add the values to the buffer
        if (sessionRecorder.isRecording())
        {
            //check if we exceeded the maximum buffer size, if yes then flush the buffer
            double free_memory = computer->Hardware[free_ram_index.first]->Sensors[free_ram_index.second]->Value.Value;
                
            //if the size of the buffer exceeds the 5% of the current available memory then flush it
            if (sizeof(sessionRecorder.record_buffer) >= (free_memory * 1024LL * 1024 * 1024) * 0.05)
            {
                sessionRecorder.flush_buffer();
            }
        }
    }
}

/**
* Prints the physical disk info from the PhysicalDisks std::map from the storageInformation parameter
* @param window The curses window to print the information on
* @param name The physical disk name
* @param current_display_row The current current row we are printing on in the curses window object
* @param storageInformation A StorageInformation object to get the info of the physical disk from
*/
void PrintPhysicalDiskInfo(WINDOW* window, const std::wstring name, int &current_display_row, StorageInformation& storageInformation)
{
    //Print name
    mvwprintw(window, current_display_row, 15, "Device ID");
    
    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.PhysicalDisks[name].DeviceID.begin(), storageInformation.PhysicalDisks[name].DeviceID.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Bus Type");
    
    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.PhysicalDisks[name].BusType.begin(), storageInformation.PhysicalDisks[name].BusType.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Media Type");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.PhysicalDisks[name].MediaType.begin(), storageInformation.PhysicalDisks[name].MediaType.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Part Number");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.PhysicalDisks[name].partNumber.begin(), storageInformation.PhysicalDisks[name].partNumber.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Health Status");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.PhysicalDisks[name].HealthStatus.begin(), storageInformation.PhysicalDisks[name].HealthStatus.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Size");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.PhysicalDisks[name].Size).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Allocated Size");
    
    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.PhysicalDisks[name].AllocatedSize).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Logical Sector Size");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.PhysicalDisks[name].LogicalSectorSize).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Physical Sector Size");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.PhysicalDisks[name].PhysicalSectorSize).c_str());
    current_display_row++;
}

void PrintNetworkAdapterInfo(WINDOW* window, const int index, int& current_display_row, NetworkInformation& networkInformation)
{
    //Print name
    mvwprintw(window, current_display_row, 15, "Adapter Type");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].AdapterType.begin(), networkInformation.Adapters[index].AdapterType.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Availability");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].Availability.begin(), networkInformation.Adapters[index].Availability.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Caption");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].Caption.begin(), networkInformation.Adapters[index].Caption.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Description");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].Description.begin(), networkInformation.Adapters[index].Description.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Device ID");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].DeviceID.begin(), networkInformation.Adapters[index].DeviceID.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "GUID");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].GUID.begin(), networkInformation.Adapters[index].GUID.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Index");

    //print value
    mvwprintw(window, current_display_row, 50, toString(networkInformation.Adapters[index].Index).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Install Date");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].InstallDate.begin(), networkInformation.Adapters[index].InstallDate.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Installed");

    //print value
    mvwprintw(window, current_display_row, 50, toString((int)networkInformation.Adapters[index].Installed).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Interface Index");

    //print value
    mvwprintw(window, current_display_row, 50, toString(networkInformation.Adapters[index].InterfaceIndex).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "MAC Address");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].MACAddress.begin(), networkInformation.Adapters[index].MACAddress.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Manufacturer");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].Manufacturer.begin(), networkInformation.Adapters[index].Manufacturer.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Max Number Controlled");

    //print value
    mvwprintw(window, current_display_row, 50, toString(networkInformation.Adapters[index].MaxNumberControlled).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Max Speed");

    //print value
    mvwprintw(window, current_display_row, 50, toString((unsigned long long)networkInformation.Adapters[index].MaxSpeed).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Net Connection ID");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].NetConnectionID.begin(), networkInformation.Adapters[index].NetConnectionID.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Net Connection Status");

    //print value
    mvwprintw(window, current_display_row, 50, toString((int)networkInformation.Adapters[index].NetConnectionStatus).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Net Enabled");

    //print value
    mvwprintw(window, current_display_row, 50, toString((int)networkInformation.Adapters[index].NetEnabled).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Permanent Address");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].PermanentAddress.begin(), networkInformation.Adapters[index].PermanentAddress.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Physical Adapter");

    //print value
    mvwprintw(window, current_display_row, 50, toString((int)networkInformation.Adapters[index].PhysicalAdapter).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "PNP Device ID");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].PNPDeviceID.begin(), networkInformation.Adapters[index].PNPDeviceID.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Power Management Supported");

    //print value
    mvwprintw(window, current_display_row, 50, toString((int)networkInformation.Adapters[index].PowerManagementSupported).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Product Name");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].ProductName.begin(), networkInformation.Adapters[index].ProductName.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Service Name");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].ServiceName.begin(), networkInformation.Adapters[index].ServiceName.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Net Enabled");

    //print value
    mvwprintw(window, current_display_row, 50, toString((unsigned long long)networkInformation.Adapters[index].Speed).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Status");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].Status.begin(), networkInformation.Adapters[index].Status.end()).c_str());
    current_display_row++;


    //Print name
    mvwprintw(window, current_display_row, 15, "Time Of Last Reset");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(networkInformation.Adapters[index].TimeOfLastReset.begin(), networkInformation.Adapters[index].TimeOfLastReset.end()).c_str());
    current_display_row++;
}

/**
* Prints the drive info from the Drives std::map from the storageInformation parameter
* @param window The curses window to print the information on
* @param name The drive name
* @param current_display_row The current current row we are printing on in the curses window object
* @param storageInformation A StorageInformation object to get the info of the drive from
*/
void PrintDriveInfo(WINDOW* window, const wchar_t DriveLetter, int& current_display_row, StorageInformation& storageInformation)
{
    //Print name
    mvwprintw(window, current_display_row, 15, "Bytes Per Sector");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.Drives[DriveLetter].BytesPerSector).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "Sectors Per Track");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.Drives[DriveLetter].SectorsPerTrack).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "Tracks Per Cylinder");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.Drives[DriveLetter].TracksPerCylinder).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "Volume Serial Number");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.Drives[DriveLetter].VolumeSerialNumber).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "Cylinders Quad Part");

    //print value
    mvwprintw(window, current_display_row, 50, toString(storageInformation.Drives[DriveLetter].Cylinders_QuadPart).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "VolumeType");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.Drives[DriveLetter].VolumeType.begin(), storageInformation.Drives[DriveLetter].VolumeType.end()).c_str());
    current_display_row++;

    //Print name
    mvwprintw(window, current_display_row, 15, "Volume Name");

    //print value
    mvwprintw(window, current_display_row, 50, std::string(storageInformation.Drives[DriveLetter].VolumeName.begin(), storageInformation.Drives[DriveLetter].VolumeName.end()).c_str());
    current_display_row++;
}

/**
* Prints all available sensors of a given hardware
* @param computer The computer object to get the hardware info from
* @param window The curses window to print the info on
* @param hardware_index The index of the hardware to print the sesnors of
* @param current_display_row The current current row we are printing on in the curses window object
*/
void PrintHardwareSensors(OpenHardwareMonitor::Hardware::Computer^ computer, WINDOW* window, unsigned int hardware_index, int& current_display_row)
{
    //Iterate over all available sensors
    for (int sensor_index = 0; sensor_index < computer->Hardware[hardware_index]->Sensors->Length; sensor_index++, current_display_row++) {

        //convert string and print it
        std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->Name);
        mvwprintw(window, current_display_row, 15, std::string(SensorName.begin(), SensorName.end()).c_str());

        if (SensorName == "Available Memory")
        {
            free_ram_index = { hardware_index, sensor_index };
        }

        //convert string and print it
        std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->SensorType.ToString());
        mvwprintw(window, current_display_row, 35, SensorType.c_str());

        //save the position of this sensor to be used in updating the sensor value in printing the dynamic sensor data
        sensor_screen_row[std::make_pair(hardware_index, sensor_index)] = current_display_row;
    }
}

/**
* Prints the Hardware name and sensor name and data type once as they do not need to be updated
* @see updateAndPrintSensorData()
* @param computer The computer object to get the hardware info from
* @param window The Curses window to print the info on
* @return The number of rows taken to print all of the info
*/
int printStaticHarwareInfo(OpenHardwareMonitor::Hardware::Computer^ computer, WINDOW* window, StorageInformation& storageInformation, NetworkInformation& networkInformation)
{
    int current_display_row = 0; //keeps track of what row we are displaying on

    //resize the hardware count in the session record buffer
    sessionRecorder.record_buffer.resize(computer->Hardware->Length);

    //stores the index of OpenHardwareMonitor storage devices with its name as the key and index as the value
    std::map<std::string, int> storageDevices;

    //iterate over all available hardware
    for (int hardware_index = 0; hardware_index < computer->Hardware->Length; hardware_index++, current_display_row += 2)
    {
        //resize the sensor count of the current hardware of session record buffer
        sessionRecorder.record_buffer[hardware_index].resize(computer->Hardware[hardware_index]->Sensors->Length);

        //convert hardware name and store it
        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Name);

        //If device is a storage device then store its index and continue to be used when printing the physical disks
        if (computer->Hardware[hardware_index]->HardwareType == OpenHardwareMonitor::Hardware::HardwareType::HDD)
        {
            //store its index
            storageDevices[HardwareName] = hardware_index;

            //cancel out the blank lines that gets printed between different hardware because we did not print any hardware
            current_display_row -= 2;

            continue;
        }
        
        //print hardware name
        mvwprintw(window, current_display_row, 0, HardwareName.c_str());

        current_display_row++; //go to the next row

        //Print all available sensors of the device
        PrintHardwareSensors(computer, window, hardware_index, current_display_row);
    }

    //Print category name
    mvwprintw(window, current_display_row, 0, "Storage Devices");
    current_display_row+=2;

    //Print all physcial disks info
    for (auto& physicalDisk : storageInformation.PhysicalDisks)
    {
        //Print disk name
        mvwprintw(window, current_display_row, 5, std::string(physicalDisk.first.begin(), physicalDisk.first.end()).c_str());
        current_display_row++;

        //If current disk is recognized by the OpenHardwareMonitor::Computer object
        if (storageDevices.find(std::string(physicalDisk.second.FriendlyName.begin(), physicalDisk.second.FriendlyName.end())) != storageDevices.end())
        {
            //Print all available sensors of the device
            PrintHardwareSensors(computer, window, storageDevices[std::string(physicalDisk.second.FriendlyName.begin(), physicalDisk.second.FriendlyName.end())], current_display_row);
        }
        
        //Print all static info of the disk
        PrintPhysicalDiskInfo(window, physicalDisk.first, current_display_row, storageInformation);

        current_display_row+=2;
    }

    //Print category name
    mvwprintw(window, current_display_row, 0, "Drives");
    current_display_row += 2;

    //Print all physcial disks info
    for (auto& Drive : storageInformation.Drives)
    {
        //Print drive letter

        //convert character to std::wstring
        std::wstring DriveLetterWstr;
        DriveLetterWstr.push_back(Drive.first);
        DriveLetterWstr.push_back(L':');

        //print it
        mvwprintw(window, current_display_row, 5, std::string(DriveLetterWstr.begin(), DriveLetterWstr.end()).c_str());
        current_display_row++;

        //Print all static info of the disk
        PrintDriveInfo(window, Drive.first, current_display_row, storageInformation);

        current_display_row += 2;
    }

    //Print category name
    mvwprintw(window, current_display_row, 0, "Network adapters");
    current_display_row += 2;

    for (int adapter_index = 0; adapter_index < networkInformation.Adapters.size(); adapter_index++)
    {
        //print name
        mvwprintw(window, current_display_row, 5, std::string(networkInformation.Adapters[adapter_index].Name.begin(), networkInformation.Adapters[adapter_index].Name.end()).c_str());
        current_display_row++;
    
        //print all static info of the adapter
        PrintNetworkAdapterInfo(window, adapter_index, current_display_row, networkInformation);

        current_display_row += 2;
    }

    return current_display_row;
}

/**
* Prints the guide menu to the curses screen
* @param window A curses window to print the info on
*/
void printGuide(WINDOW* window)
{
    //print the title
    mvwprintw(window, 0, 20, "Guide");

    //store the menu options
    std::string options[2] = {
        "r -> Toggles session recording",
        "Mouse Scroll -> Scrolls through the data"
    };

    //print menu options
    for (int i = 0; i < 2; i++)
    {
        //i + 2 to leave a blank line between from the title
        mvwprintw(window, i + 2, 0, options[i].c_str());
    }

    //print the final buffer
    prefresh(window, 0, 0, 0, 151, getmaxy(stdscr) - 1, getmaxx(stdscr) - 1);
}

int main()
{
    //set locale for curses
    setlocale(LC_ALL, "");

    //init curses screen
    initscr();

    //resize terminal
    resize_term(60, 200);

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
    WINDOW* pad = newpad(1000, 150);

    //enable keypad keys input
    keypad(pad, TRUE);

    //set timeout period for the wgetch() function and therefor limit the update rate of the program
    wtimeout(pad, 5);

    //get max rows and cols of the terminal to be used later in displaying the updating info
    int mxrows = 0, mxcols = 0;
    getmaxyx(stdscr, mxrows, mxcols);

    //Clear the waiting text from the display to start displaying the data
    clear();

    //Initialize static storage information object
    StorageInformation storageInfo = StorageInformation();

    //Initialize static netwrok information object
    NetworkInformation networkInfo = NetworkInformation();
    
    //display the static information that does not get updated by time
    int totalRows = printStaticHarwareInfo(computer, pad, storageInfo, networkInfo);

    //display guide
    WINDOW* guidePad = newpad(60, 50);
    printGuide(guidePad);

    //keeps track of what row of the pad we are on
    int mypadpos = 0;
    
    //keeps track of the last time we polled the data to be used in limiting the poll rate
    auto prev_time = std::chrono::high_resolution_clock::now();

    //makrs if it is our time polling the data to avoid waiting for the poll rate limit the first time
    bool first_poll = 1;

    //the delay for the poll rate of the data in milliseconds
    int poll_delay = 1000;
    
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
        prefresh(pad, mypadpos, 0, 0, 0, mxrows - 1, mxcols - guidePad->_maxx - 1);
        
        //mouse event to be used to determine what mouse button was pressed
        MEVENT event;

        //get input form user
        char ch = wgetch(pad);

        //check if input is a mouse input
        switch (ch)
        {
        case 27:
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
            break;

        case 'r':
            //toggle recording
            sessionRecorder.toggleRecording(computer, storageInfo, networkInfo);

            //display/hide recording text to inform user
            if (sessionRecorder.isRecording())
            {
                mvwprintw(guidePad, 59, 18, "Session is being recorded.");
            }
            else
            {
                mvwprintw(guidePad, 59, 18, "                          ");
            }

            //update the pad to show the text
            prefresh(guidePad, 0, 0, 0, 151, mxrows - 1, mxcols - 1);
            break;

        case 'f':
            sessionRecorder.flush_buffer();
            break;
        default:
            break;
        }
    }

    //end curses window
    endwin();

    return 0;
}