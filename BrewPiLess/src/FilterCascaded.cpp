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

#include "Brewpi.h"
#include "FilterFixed.h"
#include "FilterCascaded.h"
#include <stdlib.h>
#include <limits.h>
#include "TemperatureFormats.h"

CascadedFilter::CascadedFilter() {
	for(uint8_t i=0; i<NUM_SECTIONS; i++){
		sections[i].setCoefficients(2); // default to a b value of 2
	}
}

void CascadedFilter::setCoefficients(uint8_t bValue){
	for(uint8_t i=0; i<NUM_SECTIONS; i++){
		sections[i].setCoefficients(bValue);
	}
}

temperature CascadedFilter::add(temperature val){
	temperature_precise valDoublePrecision = tempRegularToPrecise(val);
	valDoublePrecision = addDoublePrecision(valDoublePrecision);
	// return output, shifted back to single precision
	return tempPreciseToRegular(valDoublePrecision);
}

temperature_precise CascadedFilter::addDoublePrecision(temperature_precise val){
	temperature_precise input = val;
	// input is input for next section, which is the output of the previous section
	for(uint8_t i=0; i<NUM_SECTIONS; i++){
		input = sections[i].addDoublePrecision(input);
	}
	return input;
}


temperature CascadedFilter::readInput(void){
	return sections[0].readInput(); // return input of first section
}

temperature_precise CascadedFilter::readOutputDoublePrecision(void){
	return sections[NUM_SECTIONS-1].readOutputDoublePrecision(); // return output of last section
}

temperature_precise CascadedFilter::readPrevOutputDoublePrecision(void){
	return sections[NUM_SECTIONS-1].readPrevOutputDoublePrecision(); // return previous output of last section
}

void CascadedFilter::init(temperature val){
	for(uint8_t i=0; i<NUM_SECTIONS; i++){
		sections[i].init(val);
	}
}
