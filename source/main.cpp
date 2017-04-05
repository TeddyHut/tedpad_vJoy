#pragma comment(lib, "vJoyInterface.lib")
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <sstream>
#include <fstream>
#include "../vJoy/inc/public.h"
#include "../vJoy/inc/vjoyinterface.h"
#include "../tedpad/include/tedpad.h"
#include "../include/filetype_tpm.h"

int main(int argc, char *argv[]) {
	std::ifstream file("testMap.txt", std::ios_base::binary | std::ios_base::ate | std::ios_base::in);
	if (!file.is_open()) {
		std::cout << "Could not open file" << std::endl;
		exit(1);
	}
	auto fileSize = file.tellg();
	std::string fileStr(fileSize, '\0');
	file.seekg(0);
	file.read(&fileStr[0], fileSize);
	gamepad_map::CompleteMap gamepadMap = Filetype_tpm_dec::Filetype_tpm(fileStr);

	//Check that vJoy is enabled
	if (!vJoyEnabled()) {
		std::cout << "error: vJoy is disabled" << std::endl;
		exit(1);
	}
	//Make sure that the driver versiona and library version match
	if (!DriverMatch(NULL, NULL)) {
		std::cout << "error: vJoy driver does not match interface version" << std::endl;
		exit(1);
	}

	//Get the vJoy device from argv
	std::stringstream ss_vJoyDevice(argv[1]);
	unsigned int vJoyDevice = 0;
	ss_vJoyDevice >> vJoyDevice;

	//Check the vJOy device status
	VjdStat vJoyDevice_status = GetVJDStatus(vJoyDevice);
	switch (vJoyDevice_status) {
	case VjdStat::VJD_STAT_OWN:
		std::cout << "vJoy device " << vJoyDevice << " is already owned by this program" << std::endl;
		std::cout << "Continuing..." << std::endl;
		break;
	case VjdStat::VJD_STAT_FREE:
		std::cout << "vJoy device " << vJoyDevice << " is free" << std::endl;
		std::cout << "Continuing..." << std::endl;
		break;
	case VjdStat::VJD_STAT_BUSY:
		std::cout << "vJoy device " << vJoyDevice << " is already owned by another program" << std::endl;
		exit(1);
		break;
	case VjdStat::VJD_STAT_MISS:
		std::cout << "vJoy device " << vJoyDevice << " is not installed or is disabled" << std::endl;
		exit(1);
		break;
	case VjdStat::VJD_STAT_UNKN:
		std::cout << "vJoy device " << vJoyDevice << " has some unknown error" << std::endl;
		exit(1);
		break;
	default:
		break;
	}

	tedpad::Client::ScanForTimeArgs args;
	args.manageSocketService = true;
	auto scan_results = tedpad::Client::scanForTime(args);

	//Attempt to aquire the vJoy device
	if (!AcquireVJD(vJoyDevice)) {
		std::cout << "Failed to aquire vJoy device " << vJoyDevice << std::endl;
		exit(1);
	}
	std::cout << "Successfully acquired vJoy device " << vJoyDevice << std::endl;
	


	system("pause");
}
