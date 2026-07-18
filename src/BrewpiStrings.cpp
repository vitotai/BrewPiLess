/*
 * Copyright 2013 BrewPi/Elco Jacobs.
 * Copyright 2013 Matthew McGowan
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

#include "Brewpi.h"
#include "BrewpiStrings.h"

// some useful strings
const char STR_FMT_S_RAM[] PROGMEM = "%s"; // RAM string
const char STR_FMT_S_PROGMEM[] PROGMEM = "%S"; // PROGMEM string
const char STR_FMT_D[] PROGMEM = "%d";
const char STR_FMT_U[] PROGMEM = "%u";
const char STR_6SPACES[] PROGMEM = "      ";



#if !indexOf_inline
int8_t indexOf(const char* s, char c)
{
	char c2;
	int8_t idx = -1;
	while ((c2=s[++idx]))
	{
		if (c==c2)
		return idx;
	}
	return -1;
}
#endif
