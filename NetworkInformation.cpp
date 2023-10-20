#include "NetworkInformation.h"
#include <msclr\marshal_cppstd.h> //Needed to convert between System::String and std:string

void NetworkInformation::initAdapters()
{
#pragma comment(lib, "wbemuuid.lib")

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);

    //Error checking
    if (FAILED(hres)) {
        //Failed to initialize COM library. Error code:hres
        return;
    }

    //Initialize the COM interface pointer
    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(
        CLSID_WbemLocator,          // CLSID of the object
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

    //Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = nullptr;

    BSTR nmespace = SysAllocString(L"ROOT\\CIMV2");

    hres = pLoc->ConnectServer(
        nmespace, // Namespace
        nullptr,                               // User
        nullptr,                               // Password
        0,                                     // Locale
        NULL,                                  // Security flags
        0,                                     // Authority
        0,                                     // Context
        &pSvc                                  // IWbemServices pointer
    );

    //Error checking
    if (FAILED(hres)) {
        //Could not connect to WMI namespace. Error code: hres
        pLoc->Release();
        CoUninitialize();
        return;
    }

    //Set security levels on the proxy
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

    //Execute a query to retrieve network adapter information
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        _bstr_t("WQL"),
        _bstr_t("SELECT * FROM Win32_NetworkAdapter"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    //Error checking
    if (FAILED(hres)) {
        //Query for network adapter data failed. Error code: hres
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
    }

    // Retrieve and print network adapter information
    IWbemClassObject* pclsObj;
    ULONG uReturn;

    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        Adapters.push_back(NetworkAdapter());

        VARIANT vtProp;

        //AdapterType
        pclsObj->Get(L"AdapterType", 0, &vtProp, 0, 0);
        Adapters.rbegin()->AdapterType = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //Availability
        pclsObj->Get(L"Availability", 0, &vtProp, 0, 0);
        switch (vtProp.uintVal)
        {
        case 1:
            Adapters.rbegin()->AdapterType = L"Other";
            break;
        case 2:
            Adapters.rbegin()->AdapterType = L"Unknown";
            break;
        case 3:
            Adapters.rbegin()->AdapterType = L"Running/Full Power";
            break;
        case 4:
            Adapters.rbegin()->AdapterType = L"Warning";
            break;
        case 5:
            Adapters.rbegin()->AdapterType = L"In Test";
            break;
        case 6:
            Adapters.rbegin()->AdapterType = L"Not Applicable";
            break;
        case 7:
            Adapters.rbegin()->AdapterType = L"Power Off";
            break;
        case 8:
            Adapters.rbegin()->AdapterType = L"Off Line";
            break;
        case 9:
            Adapters.rbegin()->AdapterType = L"Off Duty";
            break;
        case 10:
            Adapters.rbegin()->AdapterType = L"Degraded";
            break;
        case 11:
            Adapters.rbegin()->AdapterType = L"Not Installed";
            break;
        case 12:
            Adapters.rbegin()->AdapterType = L"Install Error";
            break;
        case 13:
            Adapters.rbegin()->AdapterType = L"Power Save - Unknown";
            break;
        case 14:
            Adapters.rbegin()->AdapterType = L"Power Save - Low Power Mode";
            break;
        case 15:
            Adapters.rbegin()->AdapterType = L"Power Save - Standby";
            break;
        case 16:
            Adapters.rbegin()->AdapterType = L"Power Cycle";
            break;
        case 17:
            Adapters.rbegin()->AdapterType = L"Power Save - Warning";
            break;
        case 18:
            Adapters.rbegin()->AdapterType = L"Paused";
            break;
        case 19:
            Adapters.rbegin()->AdapterType = L"Not Ready";
            break;
        case 20:
            Adapters.rbegin()->AdapterType = L"Not Configured";
            break;
        case 21:
            Adapters.rbegin()->AdapterType = L"Quiesced";
            break;

        default:
            break;
        }
        Adapters.rbegin()->Availability = vtProp.uintVal;
        VariantClear(&vtProp);

        //Caption
        pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Caption = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //Description
        pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Description = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //DeviceID
        pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
        Adapters.rbegin()->DeviceID = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //GUID
        pclsObj->Get(L"GUID", 0, &vtProp, 0, 0);
        Adapters.rbegin()->GUID = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //Index
        pclsObj->Get(L"Index", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Index = vtProp.uintVal;
        VariantClear(&vtProp);

        //InstallDate
        pclsObj->Get(L"InstallDate", 0, &vtProp, 0, 0);
        Adapters.rbegin()->InstallDate = msclr::interop::marshal_as<std::wstring>(vtProp.date.ToString());
        VariantClear(&vtProp);

        //Installed
        pclsObj->Get(L"Installed", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Installed = vtProp.boolVal;
        VariantClear(&vtProp);

        //InterfaceIndex
        pclsObj->Get(L"InterfaceIndex", 0, &vtProp, 0, 0);
        Adapters.rbegin()->InterfaceIndex = vtProp.uintVal;
        VariantClear(&vtProp);

        //MACAddress
        pclsObj->Get(L"MACAddress", 0, &vtProp, 0, 0);
        Adapters.rbegin()->MACAddress = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //Manufacturer
        pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Manufacturer = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //MaxNumberControlled
        pclsObj->Get(L"MaxNumberControlled", 0, &vtProp, 0, 0);
        Adapters.rbegin()->MaxNumberControlled = vtProp.uintVal;
        VariantClear(&vtProp);

        //MaxSpeed
        pclsObj->Get(L"MaxSpeed", 0, &vtProp, 0, 0);
        Adapters.rbegin()->MaxSpeed = vtProp.ullVal;
        VariantClear(&vtProp);

        //Name
        pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Name = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //NetConnectionID
        pclsObj->Get(L"NetConnectionID", 0, &vtProp, 0, 0);
        Adapters.rbegin()->NetConnectionID = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //NetConnectionStatus
        pclsObj->Get(L"NetConnectionStatus", 0, &vtProp, 0, 0);
        Adapters.rbegin()->NetConnectionStatus = vtProp.uiVal;
        VariantClear(&vtProp);

        //NetEnabled
        pclsObj->Get(L"NetEnabled", 0, &vtProp, 0, 0);
        Adapters.rbegin()->NetEnabled = vtProp.boolVal;
        VariantClear(&vtProp);

        //PermanentAddress
        pclsObj->Get(L"PermanentAddress", 0, &vtProp, 0, 0);
        Adapters.rbegin()->PermanentAddress = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //PhysicalAdapter
        pclsObj->Get(L"PhysicalAdapter", 0, &vtProp, 0, 0);
        Adapters.rbegin()->PhysicalAdapter = vtProp.boolVal;
        VariantClear(&vtProp);

        //PNPDeviceID
        pclsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0);
        Adapters.rbegin()->PNPDeviceID = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //PowerManagementSupported
        pclsObj->Get(L"PowerManagementSupported", 0, &vtProp, 0, 0);
        Adapters.rbegin()->PowerManagementSupported = vtProp.boolVal;
        VariantClear(&vtProp);

        //ProductName
        pclsObj->Get(L"ProductName", 0, &vtProp, 0, 0);
        Adapters.rbegin()->ProductName = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //ServiceName
        pclsObj->Get(L"ServiceName", 0, &vtProp, 0, 0);
        Adapters.rbegin()->ServiceName = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //Speed
        pclsObj->Get(L"Speed", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Speed = vtProp.ullVal;
        VariantClear(&vtProp);

        //Status
        pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
        Adapters.rbegin()->Status = std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
        VariantClear(&vtProp);

        //TimeOfLastReset
        pclsObj->Get(L"TimeOfLastReset", 0, &vtProp, 0, 0);
        Adapters.rbegin()->TimeOfLastReset = msclr::interop::marshal_as<std::wstring>(vtProp.date.ToString());
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}
