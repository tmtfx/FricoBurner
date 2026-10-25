/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "BackendCapabilities.h"

#include <regex>
#include <sstream>
#include <string>

#include <stdlib.h>
#include <unistd.h>

#include "../util/ExternalProcess.h"


static bool
ToolExists(const char* name)
{
	const char* pathEnv = getenv("PATH");
	if (pathEnv == NULL)
		return false;

	BString pathCopy(pathEnv);
	int32 start = 0;
	while (start <= pathCopy.Length()) {
		int32 colon = pathCopy.FindFirst(':', start);
		int32 end = colon < 0 ? pathCopy.Length() : colon;

		BString dir;
		pathCopy.CopyInto(dir, start, end - start);
		start = end + 1;

		if (dir.Length() == 0)
			continue;

		BString candidate(dir);
		candidate << "/" << name;
		if (access(candidate.String(), X_OK) == 0)
			return true;

		if (colon < 0)
			break;
	}
	return false;
}


// Analizza l'output di "cdrecord -scanbus", che elenca ogni indirizzo
// SCSI/ATAPI del bus, inclusi gli slot vuoti ("*") e i dischi non ottici
// (es. l'SSD di sistema, elencato come tipo "Disk"). Si tiene solo cio' che
// non e' uno slot vuoto e non e' di tipo "Disk".
static std::vector<OpticalDevice>
ScanOpticalDevices()
{
	std::vector<OpticalDevice> devices;

	std::vector<BString> argv;
	argv.push_back("cdrecord");
	argv.push_back("-scanbus");

	ExternalProcess process;
	ExternalProcessResult result = process.Run(argv);

	std::string output(result.output.String());
	std::regex lineRegex(
		R"(^\s*(\d+),(\d+),(\d+)\s+\d+\)\s+(.*)$)");
	std::regex deviceRegex(
		R"(^'([^']*)'\s*'([^']*)'\s*'[^']*'\s*(\S.*)$)");

	std::istringstream stream(output);
	std::string line;
	while (std::getline(stream, line)) {
		std::smatch lineMatch;
		if (!std::regex_match(line, lineMatch, lineRegex))
			continue;

		std::string busIdLun = lineMatch[1].str() + "," + lineMatch[2].str()
			+ "," + lineMatch[3].str();
		std::string remainder = lineMatch[4].str();

		if (remainder == "*")
			continue; // slot vuoto

		std::smatch deviceMatch;
		if (!std::regex_match(remainder, deviceMatch, deviceRegex))
			continue;

		std::string type = deviceMatch[3].str();
		if (type == "Disk")
			continue; // disco non ottico (es. l'SSD di sistema)

		OpticalDevice device;
		device.address = busIdLun.c_str();
		device.vendor = deviceMatch[1].str().c_str();
		device.model = deviceMatch[2].str().c_str();
		devices.push_back(device);
	}

	return devices;
}


BackendStatus
BackendCapabilities::Detect()
{
	BackendStatus status;
	status.hasMkfs = ToolExists("mkfs");
	status.hasMount = ToolExists("mount");
	status.hasUnmount = ToolExists("unmount");
	status.hasCdrecord = ToolExists("cdrecord");
	status.hasXorriso = ToolExists("xorriso");
	status.hasGrowisofs = ToolExists("growisofs");
	status.hasReadcd = ToolExists("readcd");
	status.hasDd = ToolExists("dd");

	if (status.hasCdrecord)
		status.opticalDevices = ScanOpticalDevices();

	return status;
}
