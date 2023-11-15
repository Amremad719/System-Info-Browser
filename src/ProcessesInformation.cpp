#include "ProcessesInformation.h"
#include "GlobalFunctions.h"
#include <msclr\marshal_cppstd.h>

void ProcessesInformation::fetchProcessStaticInfo(const DWORD& processID)
{
    //Get the process handle
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    //Return if the handle is not correct
    if (hProcess == NULL) return;
    
    // Get process name
    TCHAR szProcessName[MAX_PATH];
    if (GetModuleBaseName(hProcess, NULL, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) 
    {
        this->processes[processID].Name = szProcessName;
    }

    // Get process base address
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) 
    {
        this->processes[processID].BaseAddress = toHex(hMods[0]);
    }

    // Get module information
    MODULEINFO moduleInfo;
    if (GetModuleInformation(hProcess, hMods[0], &moduleInfo, sizeof(moduleInfo))) 
    {
        this->processes[processID].SizeOfImage = moduleInfo.SizeOfImage;
    }

    // Get process memory information
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) 
    {
        this->processes[processID].WorkingSetSize = pmc.WorkingSetSize;
        this->processes[processID].PagefileUsage = pmc.PagefileUsage;
    }

    // Get process path
    wchar_t processPath[MAX_PATH];
    DWORD pathSize = sizeof(processPath) / sizeof(processPath[0]);
    if (QueryFullProcessImageName(hProcess, 0, processPath, &pathSize) != 0) 
    {
        this->processes[processID].Path = processPath;
    }

    CloseHandle(hProcess);
}

void ProcessesInformation::updateProcessDynamicInfo(const DWORD& processID)
{
    this->processes[processID].CPUUsage = getCPUUsage(processID);

    this->processes[processID].MemoryUsage = getMemoryUsgae(processID);
}

double ProcessesInformation::getCPUUsage(const DWORD& processID)
{
    //used to convert FILETIME to a LARGE_INTEGER
    auto file_time_2_utc = [](const FILETIME* ftime) -> uint64_t
    {
        LARGE_INTEGER li;

        li.LowPart = ftime->dwLowDateTime;
        li.HighPart = ftime->dwHighDateTime;
        return li.QuadPart;
    };

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    int64_t system_time;
    int64_t time;
    int64_t system_time_delta;
    int64_t time_delta;

    double cpu = -1;

    //return if thre number of processors equals zero because the reading will be wrong
    if (this->NumberOfProcessors == 0)
    {
        return 0;
    }

    GetSystemTimeAsFileTime(&now);

    //get process handle
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processID);
    if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
    {
        // can not find the process
        
        //Close the handle
        CloseHandle(hProcess);

        return -1;
    }

    //convert times
    system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / this->NumberOfProcessors;
    time = file_time_2_utc(&now);

    //If the itme is zero then the process is basically not using anything so we can return 0
    //to avoid being stuck in a recursive loop
    if (system_time == 0 || time == 0)
    {
        //Close the handle
        CloseHandle(hProcess);

        return 0;
    }

    //If no prior times were saved then store the current times and recurse the function to get the new times
    if ((this->lastProcessTimes[processID].last_system_time_ == 0) || (this->lastProcessTimes[processID].last_time_ == 0))
    {
        this->lastProcessTimes[processID].last_system_time_ = system_time;
        this->lastProcessTimes[processID].last_time_ = time;

        //Close the handle
        CloseHandle(hProcess);

        return this->getCPUUsage(processID);
    }

    //get times delta
    system_time_delta = system_time - this->lastProcessTimes[processID].last_system_time_;
    time_delta = time - this->lastProcessTimes[processID].last_time_;

    //if delta is zero i.e no time has passed then we call again to get a reading
    if (time_delta == 0)
    {
        //Close the handle
        CloseHandle(hProcess);
        return this->getCPUUsage(processID);
    }

    //calculate usage
    cpu = ((system_time_delta * 100.0 + time_delta / 2.0) / time_delta);

    //update previous times
    this->lastProcessTimes[processID].last_system_time_ = system_time;
    this->lastProcessTimes[processID].last_time_ = time;

    //Close the handle
    CloseHandle(hProcess);

    return cpu;
}

unsigned long long ProcessesInformation::getMemoryUsgae(const DWORD& processID)
{
    //prcess handle
    HANDLE hProcess;

    //structure that receives information about the memory usage of the process
    PROCESS_MEMORY_COUNTERS pmc;

    //obtain process handle
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    //return if invalid handle or could not get information
    if (NULL == hProcess || !GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        return -1;
    }

    CloseHandle(hProcess);

    return pmc.WorkingSetSize;
}

void ProcessesInformation::fetchProcessesStaticInfo()
{
    for (std::pair<const DWORD, Process>& process : this->processes)
    {
        fetchProcessStaticInfo(process.first);
    }
}

void ProcessesInformation::updateProcessesDynamicInfo()
{
    for (std::pair<const DWORD, Process>& process : this->processes)
    {
        updateProcessDynamicInfo(process.first);
    }
}

void ProcessesInformation::fetchProcesses()
{
    DWORD aProcesses[1024], cbNeeded;

    //If fails to enumarate then return
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return;
    }

    //Divide by byte size to get the count of items
    cbNeeded /= sizeof(DWORD);

    //Iterate over all elements in aProcesses
    for (unsigned int i = 0; i < cbNeeded; i++)
    {
        //If not null then add to the processes map
        if (aProcesses[i] != NULL) 
        {
            //Assign process ID
            this->processes[aProcesses[i]].ID = aProcesses[i];
        }
    }

    //fetch static info
    fetchProcessesStaticInfo();
}

void ProcessesInformation::fetchNumberOfProcessors()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    this->NumberOfProcessors = info.dwNumberOfProcessors;
}
