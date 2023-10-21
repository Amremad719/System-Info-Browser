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
        std::wstring                Availability;
        std::wstring                Caption;
        std::wstring                Description;
        std::wstring                DeviceID;
        std::wstring                GUID;
        int                         Index;
        std::wstring                InstallDate;
        bool                        Installed;
        int                         InterfaceIndex;
        std::wstring                MACAddress;
        std::wstring                Manufacturer;
        int                         MaxNumberControlled;
        long long                   MaxSpeed;
        std::wstring                Name;
        std::wstring                NetConnectionID;
        short                       NetConnectionStatus;
        bool                        NetEnabled;
        std::vector<std::wstring>   NetworkAddresses;
        std::wstring                PermanentAddress;
        bool                        PhysicalAdapter;
        std::wstring                PNPDeviceID;
        std::vector<short>          PowerManagementCapabilities;
        bool                        PowerManagementSupported;
        std::wstring                ProductName;
        std::wstring                ServiceName;
        long long                   Speed;
        std::wstring                Status;
        std::wstring                TimeOfLastReset;
    };

    void initAdapters();

public:
    std::vector<NetworkAdapter> Adapters;

    NetworkInformation()
    {
        initAdapters();
    }
};

