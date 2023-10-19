#pragma once
#include <string>
#include <map>
#include <vector>

/**
* Gets and stores all available static information about system drives and physical disks
*/
class StorageInformation
{

	/**
	* Stores information about a system drive
	*/
	struct Drive
	{
		unsigned long BytesPerSector;
		unsigned long Cylinders_QuadPart;
		unsigned long VolumeSerialNumber;

		std::wstring VolumeType;
		std::wstring VolumeName;
	};

	/**
	* Stores information about a physical storage device
	*/
	struct PhysicalDisk
	{
		unsigned int SpindleSpeed;

		unsigned long long Size;
		unsigned long long AllocatedSize;
		unsigned long long LogicalSectorSize;
		unsigned long long PhysicalSectorSize;

		std::wstring Usage;
		std::wstring BusType;
		std::wstring DeviceID;
		std::wstring MediaType;
		std::wstring partNumber;
		std::wstring HealthStatus;
		std::wstring FriendlyName;
	};

	/**
	* Initializes BytesPerSector and Cylinders_QuadPart members in the Drive struct for the given DriveLetter in the Drives std::map
	* @param DriveLetter The drive letter to initialize in the Drives std::map
	*/
	void InitDiskGeometry(const wchar_t DriveLetter);

	/**
	* Initializes VolumeType member in the Drive struct for the given DriveLetter in the Drives std::map
	* @param DriveLetter The drive letter to initialize in the Drives std::map
	*/
	void InitDriveType(const wchar_t DriveLetter);

	/**
	* Initializes VolumeName and VolumeSerialNumber in the Drive struct for the given DriveLetter in the Drives std::map
	* @param DriveLetter The drive letter to initialize in the Drives std::map
	*/
	void InitVolumeInfo(const wchar_t DriveLetter);

	/**
	* Initializes the Drive struct for all available drives in the Drives std::map
	* @param DriveLetter The drive letter to initialize in the Drives std::map
	*/
	void InitDrives();

	/**
	* Initializes the PhysicalDisks std::vector with all available physical disks
	*/
	void InitPhysicalDisks();

public:

	/**
	* A std::map storing all available drives as a Drive object with the key being the drive letter
	*/
	std::map<wchar_t, Drive> Drives;

	/**
	* A std::vector of all available physical disks as a PhysicalDrive object
	*/
	std::map<std::wstring, PhysicalDisk> PhysicalDisks;
	//std::vector<PhysicalDisk> PhysicalDisks;

	StorageInformation()
	{
		InitDrives();
		InitPhysicalDisks();
	}
};