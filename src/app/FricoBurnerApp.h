/*
 * Copyright 2026 Pirati Del Frico. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef FRICOBURNER_APP_H
#define FRICOBURNER_APP_H

#include <Application.h>

class MainWindow;

class FricoBurnerApp : public BApplication {
public:
								FricoBurnerApp();

private:
			MainWindow*			fMainWindow;
};

#endif // FRICOBURNER_APP_H
