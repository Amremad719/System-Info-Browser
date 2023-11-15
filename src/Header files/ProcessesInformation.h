#pragma once
#include <string>
#include <map>
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#include <pdhmsg.h>
#include <iostream>
#include <iomanip>

class ProcessesInformation
{
private: struct Process
	{
		unsigned int			ID;
        std::wstring            Path;
		std::wstring			Name;
		double					CPUUsage;
		double					DiskUsage;
		std::string		        BaseAddress;
		unsigned long long		MemoryUsage;
		unsigned long long		SizeOfImage;
		double					NetworkUsage;
		unsigned long long		PagefileUsage;
		unsigned long long		WorkingSetSize;
	};

    /**
    * A map used to store process information as a Process struct with relation to its ID
    * @see Process
    */
public: std::map<DWORD, Process> processes;
    
    /**
    * A map and a struct used to store the ProcessTimes of a process with the key being the process ID
    * to be able to get the CPU usage reading accurately
    */
private: struct ProcessTimes
    {
        int64_t last_time_;
        int64_t last_system_time_;
    };
    std::map<DWORD, ProcessTimes> lastProcessTimes;

private: unsigned int NumberOfProcessors;

    /**
    * Fetches the static info about a specific process given its ID in the processes map
    * @param processID The ID of the process to fetch the static info of
    * @see processes()
    */
private: void fetchProcessStaticInfo(const DWORD& processID);

    /**
    * Updates the dynamic info about a specific process given its ID in the processes map
    * @param processID The ID of the process to update the dynamic info of
    * @see processes()
    */
private: void updateProcessDynamicInfo(const DWORD& processID);

    /**
    * Gets the current CPU usage of the given process. Will report inaccurate information if called too quickly, 250ms wait is enough
    * @param processID The ID of the process to get the CPU usage of
    * @return The precentage of the CPU the process is using
    */
private: double getCPUUsage(const DWORD& processID);

    /**
    * Gets the memory usage of the given process
    * @param processID The ID of the process to get the memory usage of
    * @return The memory usage of the process in bytes
    */
private: unsigned long long getMemoryUsgae(const DWORD& processID);

    /**
    * Fetches the static info of all processes in the processes map based on the present keys
    * @see fetchProcessStaticInfo()
    * @see processes()
    */
private: void fetchProcessesStaticInfo();

    /**
    * Fetches the dynamic info of all processes in the processes map based on the present keys
    * @see updateProcessDynamicInfo()
    * @see processes()
    */
public: void updateProcessesDynamicInfo();

    /**
    * Fetches all Processes from the OS and stores them in the processes map
    * @see processes()
    */
public: void fetchProcesses();

    /**
    * Fetches the number of processors from the OS and stores it in NumberOfProcessors
    */
private: void fetchNumberOfProcessors();

public: ProcessesInformation() : NumberOfProcessors(0)
    {
        //Order is important

        fetchNumberOfProcessors();
        fetchProcesses();
    }
};