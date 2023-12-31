#include "SessionRecorder.h"
#include <tuple>
#include <algorithm>
#include <iomanip> //Needed for setprecision()
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string
#include "GlobalFunctions.h"

std::string getCurrentDateAndTimeInValidFormat()
{
    //fetch current time and date from system
    System::DateTime dateTime = System::DateTime::Now;

    //convert to a std::string
    std::string dateTimeString = msclr::interop::marshal_as<std::string>(dateTime.ToString());

    //replace invalid file naming characters to valid ones
    std::replace(dateTimeString.begin(), dateTimeString.end(), ':', '-');
    std::replace(dateTimeString.begin(), dateTimeString.end(), '/', '-');

    return dateTimeString;
}

void SessionRecorder::initBuffer(OpenHardwareMonitor::Hardware::Computer^ computer)
{
    //resize the hardware count in the session record buffer
    this->record_buffer.resize(computer->Hardware->Length);

    //iterate over all available hardware
    for (int hardware_index = 0; hardware_index < computer->Hardware->Length; hardware_index++)
    {
        //resize the sensor count of the current hardware of session record buffer
        this->record_buffer[hardware_index].resize(computer->Hardware[hardware_index]->Sensors->Length);
    }
}

void SessionRecorder::printColumnHeaders(OpenHardwareMonitor::Hardware::Computer^ computer)
{
    if (this->column_headers_printed) return;

    //iterate over all available hardware
    for (int hardware_index = 0; hardware_index < computer->Hardware->Length; hardware_index++)
    {
        std::string HardwareName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Name);

        for (int sensor_index = 0; sensor_index < computer->Hardware[hardware_index]->Sensors->Length; sensor_index++) {

            //convert string and print it
            std::string SensorName = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->Name);

            //convert string and print it
            std::string SensorType = msclr::interop::marshal_as<std::string>(computer->Hardware[hardware_index]->Sensors[sensor_index]->SensorType.ToString());

            //construct final header
            std::string columnHeader = HardwareName + "." + SensorName + "." + SensorType;

            //print column header to stream
            this->record_stream << columnHeader;

            //if not the final sensor print a comma to seperate the values
            if (!(hardware_index == computer->Hardware->Length - 1 && sensor_index == computer->Hardware[hardware_index]->Sensors->Length - 1))
            {
                this->record_stream << ',';
            }
        }
    }
    this->record_stream << '\n';

    this->column_headers_printed = 1;
}

void SessionRecorder::printStaticStorageInfo(StorageInformation& storageInformation)
{
    //if no stream open the return
    if (!record_stream.is_open()) return;

    //Print physical disks info

    record_stream << "==== Physical Disks info ====\n";

    for (auto& PhysicalDisk : storageInformation.PhysicalDisks)
    {
        record_stream << std::string(PhysicalDisk.second.MediaType.begin(), PhysicalDisk.second.MediaType.end()) << "====>\n";

        record_stream << "Name," << std::string(PhysicalDisk.second.FriendlyName.begin(), PhysicalDisk.second.FriendlyName.end()) << '\n';

        record_stream << "Bus Type," << std::string(PhysicalDisk.second.BusType.begin(), PhysicalDisk.second.BusType.end()) << '\n';

        record_stream << "Health Status," << std::string(PhysicalDisk.second.HealthStatus.begin(), PhysicalDisk.second.HealthStatus.end()) << '\n';

        record_stream << "Device ID," << std::string(PhysicalDisk.second.DeviceID.begin(), PhysicalDisk.second.DeviceID.end()) << '\n';

        record_stream << "Usage," << std::string(PhysicalDisk.second.Usage.begin(), PhysicalDisk.second.Usage.end()) << '\n';

        record_stream << "Part Number," << std::string(PhysicalDisk.second.partNumber.begin(), PhysicalDisk.second.partNumber.end()) << '\n';

        record_stream << "Physical Sector Size," << PhysicalDisk.second.PhysicalSectorSize << '\n';

        record_stream << "Logical Sector Size," << PhysicalDisk.second.LogicalSectorSize << '\n';

        record_stream << "Allocated Size," << PhysicalDisk.second.AllocatedSize << '\n';

        record_stream << "Size," << PhysicalDisk.second.Size << '\n';
    }

    //print drives info

    record_stream << "==== Drives info ====\n";

    for (auto& Drive : storageInformation.Drives)
    {
        record_stream << char(Drive.first - (L'a' - 'a')) << "====>\n";

        record_stream << "Volume Name," << std::string(Drive.second.VolumeName.begin(), Drive.second.VolumeName.end()) << '\n';

        record_stream << "Volume Type," << std::string(Drive.second.VolumeType.begin(), Drive.second.VolumeType.end()) << '\n';

        record_stream << "Bytes Per Sector," << Drive.second.BytesPerSector << '\n';

        record_stream << "Sectors Per Track," << Drive.second.SectorsPerTrack << '\n';

        record_stream << "Tracks Per Cylinder," << Drive.second.TracksPerCylinder << '\n';

        record_stream << "Volume Serial Number," << Drive.second.VolumeSerialNumber << '\n';

        record_stream << "Cylinders Quad Part," << Drive.second.Cylinders_QuadPart << '\n';
    }
}

void SessionRecorder::printStaticNetworkInfo(NetworkInformation& networkInformation)
{
    //if no stream open the return
    if (!record_stream.is_open()) return;

    //Print physical disks info

    record_stream << "==== Network adapters ====\n";

    for (auto& Adapter : networkInformation.Adapters)
    {
        record_stream << std::string(Adapter.Name.begin(), Adapter.Name.end()) << "====>\n";

        record_stream << "Adapter Type," << std::string(Adapter.AdapterType.begin(), Adapter.AdapterType.end()) << '\n';

        record_stream << "Availability," << std::string(Adapter.Availability.begin(), Adapter.Availability.end()) << '\n';

        record_stream << "Caption," << std::string(Adapter.Caption.begin(), Adapter.Caption.end()) << '\n';

        record_stream << "Description," << std::string(Adapter.Description.begin(), Adapter.Description.end()) << '\n';

        record_stream << "Device ID," << std::string(Adapter.DeviceID.begin(), Adapter.DeviceID.end()) << '\n';

        record_stream << "GUID," << std::string(Adapter.GUID.begin(), Adapter.GUID.end()) << '\n';

        record_stream << "Index," << Adapter.Index << '\n';

        record_stream << "Install Date," << std::string(Adapter.InstallDate.begin(), Adapter.InstallDate.end()) << '\n';

        record_stream << "Installed," << Adapter.Installed << '\n';

        record_stream << "Interface Index," << Adapter.InterfaceIndex << '\n';

        record_stream << "MAC Address," << std::string(Adapter.MACAddress.begin(), Adapter.MACAddress.end()) << '\n';

        record_stream << "Manufacturer," << std::string(Adapter.Manufacturer.begin(), Adapter.Manufacturer.end()) << '\n';

        record_stream << "Max Number Controlled," << Adapter.MaxNumberControlled << '\n';

        record_stream << "Max Speed," << Adapter.MaxSpeed << '\n';

        record_stream << "Net Connection ID," << std::string(Adapter.NetConnectionID.begin(), Adapter.NetConnectionID.end()) << '\n';

        record_stream << "Net Connection Status," << Adapter.NetConnectionStatus << '\n';

        record_stream << "Net Enabled," << Adapter.NetEnabled << '\n';

        record_stream << "Permanent Address," << std::string(Adapter.PermanentAddress.begin(), Adapter.PermanentAddress.end()) << '\n';

        record_stream << "Physical Adapter," << Adapter.PhysicalAdapter << '\n';

        record_stream << "PNP Device ID," << std::string(Adapter.PNPDeviceID.begin(), Adapter.PNPDeviceID.end()) << '\n';

        record_stream << "Power Management Supported," << Adapter.PowerManagementSupported << '\n';

        record_stream << "Product Name," << std::string(Adapter.ProductName.begin(), Adapter.ProductName.end()) << '\n';

        record_stream << "Service Name," << std::string(Adapter.ServiceName.begin(), Adapter.ServiceName.end()) << '\n';

        record_stream << "Speed," << Adapter.Speed << '\n';

        record_stream << "Status," << std::string(Adapter.Status.begin(), Adapter.Status.end()) << '\n';

        record_stream << "Time Of Last Reset," << std::string(Adapter.TimeOfLastReset.begin(), Adapter.TimeOfLastReset.end()) << '\n';
    }
}

void SessionRecorder::printStaticInfo(StorageInformation& storageInformation, NetworkInformation& networkInformation)
{
    this->printStaticStorageInfo(storageInformation);

    this->printStaticNetworkInfo(networkInformation);

    //mark the beginning of the dynamic data
    this->record_stream << "==== Dynamic data ====\n";
}

bool SessionRecorder::isRecording()
{
    return this->recording_active;
}

void SessionRecorder::startRecording(OpenHardwareMonitor::Hardware::Computer^ computer, StorageInformation& storageInformation, NetworkInformation& networkInformation)
{
    //if recording is already active then return
    if (this->recording_active) return;

    //initialize variables
    initRecordingVariables();

    //if the stream is not open initialize it
    if (!this->record_stream.is_open())
    {
        this->init_stream();
    }

    //mark that the recording is active
    this->recording_active = 1;

    //print all static information in the file
    printStaticInfo(storageInformation, networkInformation);

    //print the column headers for the dynamic data
    printColumnHeaders(computer);
}

void SessionRecorder::stopRecording()
{
    //return if recording is not active
    if (!this->recording_active) return;

    //close the stream if it is open
    if (this->record_stream.is_open())
    {
        this->close_stream();
    }
}

void SessionRecorder::toggleRecording(OpenHardwareMonitor::Hardware::Computer^ computer, StorageInformation& storageInformation, NetworkInformation& networkInformation)
{
    //if recording is active then stop recording else if it is not stop the recording
    if (this->recording_active)
    {
        this->stopRecording();
    }
    else
    {
        this->startRecording(computer, storageInformation, networkInformation);
    }
}

void SessionRecorder::init_stream()
{
    //make direcotory
    std::ignore = _wmkdir(L"Recordings");

    //get current dat and time
    std::string fileName = "Recordings\\" + getCurrentDateAndTimeInValidFormat();

    //add the file extension
    fileName += ".csv";

    //create the file and open the stream
    this->record_stream.open(fileName);

    //store the file name without the extension
    this->record_stream_file_name = fileName.substr(0, fileName.size() - 4);

    //set the precision of the decimal output
    this->record_stream << std::fixed << std::setprecision(4);
}

void SessionRecorder::flush_buffer()
{
    //if no output file is open to write to create one
    if (!record_stream.is_open())
    {
        this->init_stream();
    }

    //marks if the buffer is empty or not
    bool buffer_not_empty = 1;

    do
    {
        buffer_not_empty = 0;

        std::vector<float> values;

        //iterate over all available sensors
        for (int hardware_index = 0; hardware_index < record_buffer.size(); hardware_index++)
        {
            for (int sensor_index = 0; sensor_index < record_buffer[hardware_index].size(); sensor_index++)
            {
                //if the queue is not empty
                if (!record_buffer[hardware_index][sensor_index].empty())
                {
                    //mark the buffer as not empty
                    buffer_not_empty = 1;

                    //write value to the stream
                    values.push_back(record_buffer[hardware_index][sensor_index].front());

                    //pop the value from the queue
                    record_buffer[hardware_index][sensor_index].pop();

                }
            }
        }

        //if there are values then print them
        if (values.size())
        {
            //iterate over the values
            for (int value_index = 0; value_index < values.size(); value_index++)
            {
                //print the value to the stream
                this->record_stream << values[value_index];

                //if not the final sensor print a comma to seperate the values
                if (value_index != values.size() - 1)
                {
                    this->record_stream << ',';
                }
            }
        }

        //end the row if anything was printed
        if (buffer_not_empty)
        {
            this->record_stream << '\n';
        }

    } while (buffer_not_empty);
}

void SessionRecorder::close_stream()
{
    //if no stream to close then return
    if (!record_stream.is_open()) return;

    //flush the remaining contents of the buffer before closing
    this->flush_buffer();

    //stores the new file name to replace the old one
    std::string newFileName = record_stream_file_name;

    //get the current time to add to the name
    System::DateTime dateTime = System::DateTime::Now;

    //convert current time to std::string
    std::string currentTime = msclr::interop::marshal_as<std::string>(dateTime.ToLongTimeString());

    //replace invalid file naming character to valid ones
    std::replace(currentTime.begin(), currentTime.end(), ':', '-');

    //add a dash and the file extension
    newFileName += " - " + currentTime + ".csv";

    //close the stream to create the file if not created
    record_stream.close();

    //rename the file to the new name
    std::ignore = rename((record_stream_file_name + ".csv").c_str(), newFileName.c_str());
}

void SessionRecorder::initRecordingVariables()
{
    this->recording_active = 0;

    this->column_headers_printed = 0;
}