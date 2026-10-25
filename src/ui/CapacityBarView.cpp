/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "CapacityBarView.h"

#include <String.h>


CapacityBarView::CapacityBarView(const char* name)
	:
	BView(name, B_WILL_DRAW),
	fUsedBytes(0),
	fCapacityBytes(1)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


void
CapacityBarView::GetPreferredSize(float* width, float* height)
{
	if (width != NULL)
		*width = 200;
	if (height != NULL)
		*height = 28;
}


void
CapacityBarView::SetValues(uint64 usedBytes, uint64 capacityBytes)
{
	fUsedBytes = usedBytes;
	fCapacityBytes = capacityBytes > 0 ? capacityBytes : 1;
	Invalidate();
}


static BString
FormatMB(uint64 bytes)
{
	BString text;
	text.SetToFormat("%.1f MB", bytes / (1024.0 * 1024.0));
	return text;
}


void
CapacityBarView::Draw(BRect updateRect)
{
	BRect bounds = Bounds();

	SetHighColor(ui_color(B_SHADOW_COLOR));
	StrokeRect(bounds);

	BRect inner = bounds.InsetByCopy(1, 1);

	double fraction = (double)fUsedBytes / (double)fCapacityBytes;
	bool overflow = fraction > 1.0;
	if (fraction > 1.0)
		fraction = 1.0;

	BRect filled = inner;
	filled.right = inner.left + inner.Width() * fraction;

	rgb_color fillColor = overflow
		? (rgb_color){ 180, 40, 40, 255 }
		: ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	SetHighColor(fillColor);
	FillRect(filled);

	BRect empty = inner;
	empty.left = filled.right;
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(empty);

	BString label;
	label << FormatMB(fUsedBytes) << " / " << FormatMB(fCapacityBytes);
	if (overflow)
		label << "  (supera la capacita' del supporto)";

	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	font_height metrics;
	GetFontHeight(&metrics);
	float baseline = bounds.top + (bounds.Height() + metrics.ascent) / 2
		- metrics.descent / 2;
	DrawString(label, BPoint(bounds.left + 6, baseline));
}
