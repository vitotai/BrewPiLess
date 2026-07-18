/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
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


#if BREWPI_MENU

#include <inttypes.h>
#include "TemperatureFormats.h"

enum menuPages{
	MENU_TOP,
	MENU_PROFILE_SETTING,
	MENU_BEER_SETTING,
	MENU_FRIDGE_SETTING,
	MENU_PROFILE
};

class Menu{
	public:
	Menu(){};
	static void pickSettingToChange(void);
	static void pickMode(void);
	static void pickBeerSetting(void);
	static void pickFridgeSetting(void);
	static void initRotaryWithTemp(temperature oldSetting);

	~Menu(){};
	private:
	static void pickSettingToChangeLoop();
};

extern Menu menu;

#endif
