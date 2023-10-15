#pragma once
#include <fstream>

/**
* Manages the recording and saving of the given data
*/
class SessionRecorder
{
private:
	/**
	* file output stream used to store the session data
	*/
	std::ofstream session_record_stream;

	/**
	* stores the name of the current output file stream
	*/
	std::string session_record_stream_file_name;


public:
    
    /**
    * deconstructor
    */
    ~SessionRecorder()
    {
        close_stream();
    }
    
    /**
    * creates a csv file with it's name as the current date and time initializes the session_record_stream stream
    * @return The name of the file created without the extension
    */
    void init_stream();

    /**
    * Flushes the contents of session_record_buffer to the session_record_stream stream
    */
    void flush_buffer();
    
    /**
    * closes the output stream and renames the file to add the final timestamp
    */
    void close_stream();
};

