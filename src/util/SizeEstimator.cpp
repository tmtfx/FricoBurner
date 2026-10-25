/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "SizeEstimator.h"


SizeEstimate
SizeEstimator::Estimate(uint64 dataBytes, uint64 capacityBytes)
{
	SizeEstimate result;
	result.dataBytes = dataBytes;

	const uint64 kSixteenMB = 16ULL * 1024 * 1024;

	uint64 multiplied = (uint64)((double)dataBytes * 1.08);
	uint64 withMargin = dataBytes + kSixteenMB;

	result.totalBytes = multiplied > withMargin ? multiplied : withMargin;
	result.exceedsCapacity = capacityBytes > 0 && result.totalBytes > capacityBytes;

	return result;
}
