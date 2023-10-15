#include "SessionRecorder.h"
#include <tuple>
#include <algorithm>
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

void SessionRecorder::init_stream()
{
    //get current dat and time
    std::string fileName = getCurrentDateAndTimeInValidFormat();

    //add the file extension
    fileName += ".csv";

    //create the file and open the stream
    session_record_stream.open(fileName);

    //store the file name without the extension
    session_record_stream_file_name = fileName.substr(0, fileName.size() - 4);
}

void SessionRecorder::flush_buffer()
{
    //if no output file is open to write to create one
    if (!session_record_stream.is_open())
    {
        this->init_stream();
    }

    //to be implemented
}

void SessionRecorder::close_stream()
{
    std::string newFileName = session_record_stream_file_name;

    System::DateTime dateTime = System::DateTime::Now;

    std::string currentTime = msclr::interop::marshal_as<std::string>(dateTime.ToLongTimeString());
    std::replace(currentTime.begin(), currentTime.end(), ':', '-');

    newFileName += " - " + currentTime + ".csv";

    session_record_stream.close();

    std::ignore = rename((session_record_stream_file_name + ".csv").c_str(), newFileName.c_str());
}