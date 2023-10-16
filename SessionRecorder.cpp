#include "SessionRecorder.h"
#include <tuple>
#include <algorithm>
#include <iomanip> //Needed for setprecision()
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string

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

bool SessionRecorder::isRecording()
{
    return this->recording_active;
}

void SessionRecorder::startRecording(OpenHardwareMonitor::Hardware::Computer^ computer)
{
    if (this->recording_active) return;

    if (!this->record_stream.is_open())
    {
        this->init_stream();
    }

    this->recording_active = 1;

    printColumnHeaders(computer);
}

void SessionRecorder::stopRecording()
{
    if (!this->recording_active) return;

    if (this->record_stream.is_open())
    {
        this->close_stream();
    }

    initRecordingVariables();
}

void SessionRecorder::toggleRecording(OpenHardwareMonitor::Hardware::Computer^ computer)
{
    if (this->recording_active)
    {
        this->stopRecording();
    }
    else
    {
        this->startRecording(computer);
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
    this->record_stream << std::setprecision(9);
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