#include <iostream>
#include <windows.h> //Needed for the Sleep() function
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string

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

    //Iterate over all of the available hardware
    for (int i = 0; i < computer->Hardware->Length; i++)
    {
        //Update hardware data
        computer->Hardware[i]->Update();

        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Name);
        std::cout << HardwareName << '\n';

        //Iterate over all available sensors
        for (int j = 0; j < computer->Hardware[i]->Sensors->Length; j++){

            std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->Name);
            std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[i]->Sensors[j]->SensorType.ToString());

            std::cout << '\t' << SensorName << ' ' << SensorType << ' ';

            //Check if there is a value te avoid printing null
            if (computer->Hardware[i]->Sensors[j]->Value.HasValue){
                std::cout << computer->Hardware[i]->Sensors[j]->Value.Value;
            }
            else {
                std::cout << "NULL";
            }

            std::cout << '\n';
        }
        std::cout << '\n';
    }

    system("pause");

    return 0;
}