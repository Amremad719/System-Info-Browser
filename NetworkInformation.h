#pragma once
#include <iostream>
#include <vector>
#include <comdef.h>
#include <Wbemidl.h>


class NetworkInformation
{
    struct NetworkAdapter
    {
        std::wstring                AdapterType;
        int                         Availability;
        std::wstring                Caption;
        std::wstring                Description;
        std::wstring                DeviceID;
        std::wstring                GUID;
        int                         Index;
        std::wstring                InstallDate;
        boolean                     Installed;
        int                         InterfaceIndex;
        std::wstring                MACAddress;
        std::wstring                Manufacturer;
        int                         MaxNumberControlled;
        long long                   MaxSpeed;
        std::wstring                Name;
        std::wstring                NetConnectionID;
        short                       NetConnectionStatus;
        boolean                     NetEnabled;
        std::vector<std::wstring>   NetworkAddresses;
        std::wstring                PermanentAddress;
        boolean                     PhysicalAdapter;
        std::wstring                PNPDeviceID;
        std::vector<short>          PowerManagementCapabilities;
        boolean                     PowerManagementSupported;
        std::wstring                ProductName;
        std::wstring                ServiceName;
        long long                   Speed;
        std::wstring                Status;
        std::wstring                TimeOfLastReset;
    };

    std::vector<NetworkAdapter> Adapters;

    void initAdapters();
public:

    NetworkInformation()
    {
        initAdapters();
    }
};

