/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "TempImageRegistry.h"

#include <stdlib.h>
#include <string.h>

#include <vector>

#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <OS.h>
#include <Path.h>
#include <String.h>

#include "ExternalProcess.h"


static bool
IsTeamAlive(team_id team)
{
	team_info info;
	return get_team_info(team, &info) == B_OK;
}


// Riconosce "image-7242.bfs" / "image-7242.img" / "mount-7242" ed estrae
// 7242. Restituisce false se name non inizia con prefix o se la parte dopo
// il prefix non e' un numero valido.
static bool
ExtractPid(const BString& name, const char* prefix, team_id& pid)
{
	if (!name.StartsWith(prefix))
		return false;

	BString rest;
	name.CopyInto(rest, strlen(prefix), name.Length() - strlen(prefix));

	int32 dot = rest.FindFirst('.');
	BString pidPart = dot >= 0 ? BString(rest.String(), dot) : rest;
	if (pidPart.Length() == 0)
		return false;

	pid = atoi(pidPart.String());
	return pid > 0;
}


static void
ForceUnmount(const BPath& mountPointPath)
{
	std::vector<BString> argv;
	argv.push_back("unmount");
	argv.push_back("-f");
	argv.push_back(mountPointPath.Path());

	ExternalProcess process;
	process.Run(argv); // ignora l'esito: puo' anche non essere montato
}


void
TempImageRegistry::CleanOrphans()
{
	BPath cacheDir;
	if (find_directory(B_USER_CACHE_DIRECTORY, &cacheDir) != B_OK)
		return;
	cacheDir.Append("FricoBurner");

	BDirectory dir(cacheDir.Path());
	if (dir.InitCheck() != B_OK)
		return; // prima esecuzione, cartella non ancora creata: niente da fare

	BEntry entry;
	while (dir.GetNextEntry(&entry) == B_OK) {
		char name[B_FILE_NAME_LENGTH];
		if (entry.GetName(name) != B_OK)
			continue;
		BString nameStr(name);

		team_id pid = -1;
		bool isMountDir = false;

		if (ExtractPid(nameStr, "image-", pid)) {
			isMountDir = false;
		} else if (ExtractPid(nameStr, "mount-", pid)) {
			isMountDir = true;
		} else {
			continue; // non e' roba nostra, non toccarla
		}

		if (IsTeamAlive(pid))
			continue; // un'altra istanza di FricoBurner e' ancora in esecuzione

		BPath entryPath;
		if (entry.GetPath(&entryPath) != B_OK)
			continue;

		if (isMountDir)
			ForceUnmount(entryPath);

		BEntry(entryPath.Path()).Remove();
	}
}
