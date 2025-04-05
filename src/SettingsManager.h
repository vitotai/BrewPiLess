/*
 * Copyright 2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Brewpi.h"
#include <assert.h>
#include "EepromManager.h"
#include "DeviceManager.h"

/*
 * Manages the settings and devices for multiple carboys and multiple chambers.
 * This is the soul of brewpi.
 *
 * The manager hides the persistence of the settings, and uses the code closest to the settings to provide
 * useful defaults.
 */
class SettingsManager
{
public:
	/**
	 * Initialize settings. This attempts to read from persisted settings and apply settings from there.
	 * If that's not possible, defaults are used.
	 */
	static void loadSettings();

};

extern SettingsManager settingsManager;
