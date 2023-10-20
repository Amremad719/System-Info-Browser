#pragma once
#include <fstream>
#include <queue>
#include "StorageInformation.h"
#include "NetworkInformation.h"

/**
* Manages the recording and saving of the given data
*/
class SessionRecorder
{
private:
	/**
	* file output stream used to store the session data
	*/
	std::ofstream record_stream;

	/**
	* stores the name of the current output file stream
	*/
	std::string record_stream_file_name;

    /**
    * Marks the state of the recording now
    */
    bool recording_active = 0;

    /**
    * Marks if the column headers have been printed
    */
    bool column_headers_printed = 0;

    /**
    * creates a csv file with it's name as the current date and time initializes the session_record_stream stream
    * @return The name of the file created without the extension
    */
    void init_stream();

    /**
    * closes the output stream and renames the file to add the final timestamp
    */
    void close_stream();

    /**
    * Initializes members that need to be reset when the recording state is changed
    */
    void initRecordingVariables();

    /**
    * Initializes the sizes of the recording bufffer
    */
    void initBuffer(OpenHardwareMonitor::Hardware::Computer^ computer);

    /**
    * Prints the names of the sensors to the stream as column headers
    */
    void printColumnHeaders(OpenHardwareMonitor::Hardware::Computer^ computer);

    /**
    * Print all static storage information at the beginning of the file
    */
    void printStaticStorageInfo(StorageInformation& storageInformation);

    /**
    * Print all static network information at the beginning of the file
    */
    void printStaticNetworkInfo(NetworkInformation& networkInformation);

public:
    /**
    * Buffer for session recording
    * first dimension is hardware index
    * the second dimension is sensor index
    * third dimension is the data
    */
    std::vector<std::vector<std::queue<float>>> record_buffer;
    
    /**
    * deconstructor
    */
    ~SessionRecorder()
    {
        close_stream();
    }

    /**
    * Constructor initializes the stream if called
    */
    SessionRecorder()
    {
        //init_stream();
    }
    
    /**
    * Getter for the session_active bool
    * @return If the session is currently being recorded
    */
    bool isRecording();

    /**
    * Starts the recording of current session
    */
    void startRecording(OpenHardwareMonitor::Hardware::Computer^ computer, StorageInformation& storageInformation);

    /**
    * Stops the recording of the current session
    */
    void stopRecording();

    /**
    * Toggles the recording of the current session
    */
    void toggleRecording(OpenHardwareMonitor::Hardware::Computer^ computer, StorageInformation& storageInformation);

    /**
    * Flushes the contents of session_record_buffer to the session_record_stream stream
    */
    void flush_buffer();
};