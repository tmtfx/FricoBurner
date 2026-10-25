/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef MEDIA_PRESET_H
#define MEDIA_PRESET_H

#include <SupportDefs.h>

// Preset di capacita' supporto ottico (SRS sezione 4.1).
enum MediaPresetKind {
	MEDIA_PRESET_CD		= 0,
	MEDIA_PRESET_DVD_R,
	MEDIA_PRESET_DVD_DL,
	MEDIA_PRESET_CUSTOM,

	MEDIA_PRESET_COUNT
};

struct MediaPreset {
	MediaPresetKind	kind;
	const char*		label;
	uint64			capacityBytes;	// 0 per MEDIA_PRESET_CUSTOM: va impostata a runtime
};

static const MediaPreset kMediaPresets[MEDIA_PRESET_COUNT] = {
	{ MEDIA_PRESET_CD,		"CD-R (700 MB)",			700ULL * 1024 * 1024 },
	{ MEDIA_PRESET_DVD_R,	"DVD-R (4.7 GB)",			4700372992ULL },
	{ MEDIA_PRESET_DVD_DL,	"DVD-R DL (8.5 GB)",		8547991552ULL },
	{ MEDIA_PRESET_CUSTOM,	"Personalizzato...",		0 }
};

#endif // MEDIA_PRESET_H
