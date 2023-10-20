#include "StorageInformation.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <Windows.h>

void StorageInformation::InitDiskGeometry(const wchar_t DriveLetter)
{
    //Put the letter in the appropriate format for the CreateFileW function
    std::wstring rootPath;
    rootPath += L"\\\\.\\";
    rootPath.push_back(DriveLetter);
    rootPath += L":";

    //Open a handle to the drive using CreateFile
    HANDLE hDevice = CreateFileW(
        rootPath.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDevice != INVALID_HANDLE_VALUE) {
        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;

        //Get detailed information about the drive using DeviceIoControl
        if (DeviceIoControl(
            hDevice,
            IOCTL_DISK_GET_DRIVE_GEOMETRY,
            NULL,
            0,
            &diskGeometry,
            sizeof(diskGeometry),
            &bytesReturned,
            NULL
        )) {

            //copy the information to the corresponding object
            this->Drives[DriveLetter].BytesPerSector = diskGeometry.BytesPerSector;
            this->Drives[DriveLetter].SectorsPerTrack = diskGeometry.SectorsPerTrack;
            this->Drives[DriveLetter].TracksPerCylinder = diskGeometry.TracksPerCylinder;
            this->Drives[DriveLetter].Cylinders_QuadPart = diskGeometry.Cylinders.QuadPart;

            // Close the handle to the drive
            CloseHandle(hDevice);
        }
    }
}

void StorageInformation::InitDriveType(const wchar_t DriveLetter)
{
    //Put the letter in the appropriate format for the GetDriveTypeW function
    std::wstring rootPath;
    rootPath += L"\\\\.\\";
    rootPath.push_back(DriveLetter);
    rootPath += L":\\";

    //Fetch drive type
    UINT driveType = GetDriveTypeW(rootPath.c_str());

    //Assign object members based on the returned value
    if (driveType != DRIVE_UNKNOWN) 
    {
        switch (driveType) 
        {
        case DRIVE_REMOVABLE:
            this->Drives[DriveLetter].VolumeType = L"Removable Drive";
            break;

        case DRIVE_FIXED:
            this->Drives[DriveLetter].VolumeType = L"Fixed Drive";
            break;

        case DRIVE_CDROM:
            this->Drives[DriveLetter].VolumeType = L"CD-ROM Drive";
            break;

        case DRIVE_REMOTE:
            this->Drives[DriveLetter].VolumeType = L"Network Drive";
            break;

        case DRIVE_RAMDISK:
            this->Drives[DriveLetter].VolumeType = L"RAM Disk";
            break;

        default:
            this->Drives[DriveLetter].VolumeType = L"Unknown";
            break;
        }
    }
}

void StorageInformation::InitVolumeInfo(const wchar_t DriveLetter)
{
    //Put the letter in the appropriate format for the GetDriveTypeW function
    std::wstring rootPath;
    rootPath += L"\\\\.\\";
    rootPath.push_back(DriveLetter);
    rootPath += L":\\";

    //Variables to be passed to the function to store the data in
    wchar_t VolumeName[MAX_PATH];
    DWORD SerialNumber;
    DWORD maxComponentLength;
    DWORD fileSystemFlags;

    //Fetch the information from the system
    if (GetVolumeInformation(
        rootPath.c_str(),
        VolumeName,
        MAX_PATH,
        &SerialNumber,
        &maxComponentLength,
        &fileSystemFlags,
        NULL,
        0
    )) 
    {
        //Assign values to our object
        this->Drives[DriveLetter].VolumeName = VolumeName;
        this->Drives[DriveLetter].VolumeSerialNumber = SerialNumber;
    }
}

void StorageInformation::InitDrives()
{
    //Get the drives mask
    DWORD drives = GetLogicalDrives();
    DWORD mask = 1;

    for (char driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++)
    {
        //If there is a drive attached to the current letter
        if (drives & mask)
        {
            //Initalize the data based on current letter

            InitDiskGeometry(driveLetter);

            InitDriveType(driveLetter);

            InitVolumeInfo(driveLetter);
        }

        mask <<= 1;
    }
}

void StorageInformation::InitPhysicalDisks()
{
    #pragma comment(lib, "wbemuuid.lib")

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);

    //Error checking
    if (FAILED(hres)) {
        //Failed to initialize COM library. Error code:hres
        return;
    }

    // Initialize the COM interface pointer
    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(
        CLSID_WbemLocator,           // CLSID of the object
        0,                          // Reserved
        CLSCTX_INPROC_SERVER,       // Context
        IID_IWbemLocator,           // Interface ID
        (LPVOID*)&pLoc              // Interface pointer
    );

    //Error checking
    if (FAILED(hres)) {
        //Failed to create IWbemLocator object. Error code: hres
        CoUninitialize();
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = nullptr;

    BSTR nmespace = SysAllocString(L"root\\Microsoft\\Windows\\Storage");

    hres = pLoc->ConnectServer(
        nmespace, // Namespace
        nullptr,                               // User
        nullptr,                               // Password
        0,                                     // Locale
        NULL,                                  // Security flags
        0,                                     // Authority
        0,                                     // Context
        &pSvc                                 // IWbemServices pointer
    );

    //Error checking
    if (FAILED(hres)) {
        //Could not connect to WMI namespace. Error code: hres
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // IWbemServices pointer
        RPC_C_AUTHN_WINNT,           // Authentication service
        RPC_C_AUTHZ_NONE,            // Authorization service
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // Authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE, // Impersonation level
        NULL,                        // Authentication info
        EOAC_NONE                    // Additional capabilities
    );

    //Error checking
    if (FAILED(hres)) {
        //Could not set proxy blanket. Error code: hres
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    //Init parameters
    BSTR strQueryLanguage = SysAllocString(L"WQL");
    BSTR strQuery = SysAllocString(L"SELECT * FROM MSFT_PhysicalDisk");

    // Use the query language to retrieve data from the MSFT_PhysicalDisk class
    IEnumWbemClassObject* pEnumerator = nullptr;
    hres = pSvc->ExecQuery(
        strQueryLanguage,
        strQuery,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    //Error checking
    if (FAILED(hres)) {
        //Query for MSFT_PhysicalDisk failed. Error code: hres
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Retrieve data from the query
    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    //Iterate over all returned records
    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        //Break if there are no records left
        if (0 == uReturn) {
            break;
        }

        //Add new PhysicalDisk object to the PhysicalDisks std::vector
        //this->PhysicalDisks.push_back(PhysicalDisk());

        //Stores the return values
        VARIANT vtProp;

        std::wstring DiskName;

        //Get the value of the "FriendlyName" property
        hres = pclsObj->Get(L"FriendlyName", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            DiskName = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
            
            this->PhysicalDisks[DiskName].FriendlyName = DiskName;
            
            VariantClear(&vtProp);
        }

        //Get the value of the "DeviceID" property
        hres = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres)) 
        {
            this->PhysicalDisks[DiskName].DeviceID = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
            VariantClear(&vtProp);
        }

        //Get the value of the "BusType" property
        hres = pclsObj->Get(L"BusType", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            switch (vtProp.uintVal)
            {
            case 0:
                this->PhysicalDisks[DiskName].BusType = L"Unknown";
                break;
            case 1:
                this->PhysicalDisks[DiskName].BusType = L"SCSI";
                break;
            case 2:
                this->PhysicalDisks[DiskName].BusType = L"ATAPI";
                break;
            case 3:
                this->PhysicalDisks[DiskName].BusType = L"ATA";
                break;
            case 4:
                this->PhysicalDisks[DiskName].BusType = L"1394";
                break;
            case 5:
                this->PhysicalDisks[DiskName].BusType = L"SSA";
                break;
            case 6:
                this->PhysicalDisks[DiskName].BusType = L"Fibre Channel";
                break;
            case 7:
                this->PhysicalDisks[DiskName].BusType = L"USB";
                break;
            case 8:
                this->PhysicalDisks[DiskName].BusType = L"RAID";
                break;
            case 9:
                this->PhysicalDisks[DiskName].BusType = L"iSCSI";
                break;
            case 10:
                this->PhysicalDisks[DiskName].BusType = L"SAS";
                break;
            case 11:
                this->PhysicalDisks[DiskName].BusType = L"SATA";
                break;
            case 12:
                this->PhysicalDisks[DiskName].BusType = L"SD";
                break;
            case 13:
                this->PhysicalDisks[DiskName].BusType = L"MMC";
                break;
            case 15:
                this->PhysicalDisks[DiskName].BusType = L"File Backed Virtual";
                break;
            case 16:
                this->PhysicalDisks[DiskName].BusType = L"Storage Spaces";
                break;
            case 17:
                this->PhysicalDisks[DiskName].BusType = L"NVMe";
                break;
            default:
                break;
            }
            VariantClear(&vtProp);
        }

        //Get the value of the "MediaType" property
        hres = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            switch (vtProp.uintVal)
            {
            case 0:
                this->PhysicalDisks[DiskName].MediaType = L"Unspecified";
                break;
            case 3:
                this->PhysicalDisks[DiskName].MediaType = L"HDD";
                break;
            case 4:
                this->PhysicalDisks[DiskName].MediaType = L"SSD";
                break;
            case 5:
                this->PhysicalDisks[DiskName].MediaType = L"SCM";
                break;
            default:
                break;
            }
            VariantClear(&vtProp);
        }

        //Get the value of the "partNumber" property
        hres = pclsObj->Get(L"partNumber", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].partNumber = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
            VariantClear(&vtProp);
        }

        //Get the value of the "HealthStatus" property
        hres = pclsObj->Get(L"HealthStatus", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            switch (vtProp.uintVal)
            {
            case 0:
                this->PhysicalDisks[DiskName].HealthStatus = L"Healthy";
                break;
            case 1:
                this->PhysicalDisks[DiskName].HealthStatus = L"Warning";
                break;
            case 2:
                this->PhysicalDisks[DiskName].HealthStatus = L"Unhealthy";
                break;
            case 5:
                this->PhysicalDisks[DiskName].HealthStatus = L"Unknown";
                break;
            default:
                break;
            }
            VariantClear(&vtProp);
        }

        //Get the value of the "Usage" property
        hres = pclsObj->Get(L"Usage", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            switch (vtProp.uintVal)
            {
            case 0:
                this->PhysicalDisks[DiskName].Usage = L"Unknown";
                break;
            case 1:
                this->PhysicalDisks[DiskName].Usage = L"Auto-Select";
                break;
            case 2:
                this->PhysicalDisks[DiskName].Usage = L"Manual-Select";
                break;
            case 3:
                this->PhysicalDisks[DiskName].Usage = L"Hot Spare";
                break;
            case 4:
                this->PhysicalDisks[DiskName].Usage = L"Retired";
                break;
            case 5:
                this->PhysicalDisks[DiskName].Usage = L"Journal";
                break;
            default:
                break;
            }
            VariantClear(&vtProp);
        }

        //Get the value of the "PhysicalSectorSize" property
        hres = pclsObj->Get(L"PhysicalSectorSize", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].PhysicalSectorSize = vtProp.ullVal;
            VariantClear(&vtProp);
        }

        //Get the value of the "LogicalSectorSize" property
        hres = pclsObj->Get(L"LogicalSectorSize", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].LogicalSectorSize = vtProp.ullVal;
            VariantClear(&vtProp);
        }

        //Get the value of the "AllocatedSize" property
        hres = pclsObj->Get(L"AllocatedSize", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].AllocatedSize = vtProp.ullVal;
            VariantClear(&vtProp);
        }

        //Get the value of the "SpindleSpeed" property
        hres = pclsObj->Get(L"SpindleSpeed", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].SpindleSpeed = vtProp.uintVal;
            VariantClear(&vtProp);
        }

        //Get the value of the "Size" property
        hres = pclsObj->Get(L"Size", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            this->PhysicalDisks[DiskName].Size = vtProp.ullVal;
            VariantClear(&vtProp);
        }

        pclsObj->Release();
    }

    //Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}