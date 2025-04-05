/*
 * Copyright 2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later v7ersion.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Brewpi.h"
#include "SettingsManager.h"
#include "TempControl.h"
#include "PiLink.h"
#include "TempSensorExternal.h"

void SettingsManager::loadSettings()
{
	logDebug("loading settings");


	if (!eepromManager.applySettings())
	{
		tempControl.loadDefaultSettings();
		tempControl.loadDefaultConstants();

		deviceManager.setupUnconfiguredDevices();

		logWarning(WARNING_START_IN_SAFE_MODE);
	}

	#if (BREWPI_SIMULATE)
	{
		logDebug("Setting up simulator devices.");

		// temp sensors are special in the simulator - make sure they are set up even if not
		// configured in the eeprom
		DeviceConfig cfg;
		clear((uint8_t*)&cfg, sizeof(cfg));
		cfg.deviceHardware = DEVICE_HARDWARE_ONEWIRE_TEMP;
		cfg.chamber = 1;
		cfg.deviceFunction = DEVICE_CHAMBER_ROOM_TEMP;
		deviceManager.uninstallDevice(cfg);
		deviceManager.installDevice(cfg);

		cfg.deviceFunction = DEVICE_CHAMBER_TEMP;
		deviceManager.uninstallDevice(cfg);
		deviceManager.installDevice(cfg);

		cfg.beer = 1;
		cfg.deviceFunction = DEVICE_BEER_TEMP;
		deviceManager.uninstallDevice(cfg);
		deviceManager.installDevice(cfg);

	}
	#endif

}

SettingsManager settingsManager;
