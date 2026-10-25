/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "ExternalProcessLineReader.h"


void
ExternalProcessLineCallback(const char* chunk, size_t length, void* cookie)
{
	LineReaderContext* ctx = static_cast<LineReaderContext*>(cookie);
	if (ctx->reporter == NULL)
		return;

	ctx->buffer.append(chunk, length);

	size_t pos;
	while ((pos = ctx->buffer.find_first_of("\r\n")) != std::string::npos) {
		std::string line = ctx->buffer.substr(0, pos);
		ctx->buffer.erase(0, pos + 1);
		if (!line.empty())
			ctx->reporter->OnProcessLine(line.c_str());
	}
}
