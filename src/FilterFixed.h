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

/* This class implements an IIR low pass filter, with the following frequency response

				1  +		  2 z^-1  +					1 z^-2
H(z) = 2^-a * -------------------------------------------------
				1  + (-2 + 2^-b)z^-1  + (1-2^-b + 4* 2^-a)z^-2

 All filter coefficients are powers of two, so the filter can be efficiently implemented with bit shifts
 The DC gain is exactly 1.
 For real poles, and therefore no overshoot, use a <= 2b+4.
 To calculate the poles, you can use this wolfram alpha link:
 http://www.wolframalpha.com/input/?i=solve+%281++%2B+%28-2+%2B+2^-b%29z^-1++%2B+%281-2^-b+%2B+4*+2^-a%29z^-2%29+%3D+0+where+a+%3D+24+and+b+%3D+10
 The filter has a zero at z = -1
 For a=2b+4, it has a pole at z = (2^(b+1)-1) / 2^(b+1)
 Use this MATLAB script to visualize the filter:
	NUM_SECTIONS=1;
	a=6; b=1; FS=1;
	DEN = [1  , -2 + 2^-b  , 1-2^-b+ 4*2^-a];
	NUM = 2^(-a)*[1, 2, 1];
	F=dfilt.df2(NUM,DEN);
	H=F;
	for i=2:NUM_SECTIONS
	H=dfilt.cascade(H,F);
	end
	%H=F^NUM_SECTIONS;
	h=fvtool(H,'Fs',FS)
	zeropos = roots(NUM)
	polepos = roots(DEN)
	set(h,'FrequencyRange', 'Specify freq. vector');
	set(h,'FrequencyScale','Log')
	set(h,'FrequencyVector', logspace(-4,0,1000));

	Here are the specifications for a single stage filter, for values a=2b+4
	The delay time is the time it takes to rise to 0.5 in a step response.
	When cascaded filters are used, multiply the delay time by the number of cascades.

	a=4,	b=0,	delay time = 3
	a=6,	b=1,	delay time = 6
	a=8,	b=2,	delay time = 13
	a=10,	b=3,	delay time = 26
	a=12,	b=4,	delay time = 53
	a=14,	b=5,	delay time = 106
	a=16,	b=6,	delay time = 213

*/

#include "TemperatureFormats.h"

class FixedFilter{
	public:
		// input and output arrays
		temperature_precise xv[3];
		temperature_precise yv[3];
		uint8_t a;
		uint8_t b;

	public:
		FixedFilter() { setCoefficients(20); /* default to a b value of 2 */ }
		~FixedFilter() { }
		void init(temperature val);

		void setCoefficients(uint8_t bValue) {
			a = bValue*2+4;
			b = bValue;
		}

		temperature add(temperature val); // adds a value and returns the most recent filter output
		temperature_precise addDoublePrecision(temperature_precise val);

		temperature readOutput(void){
			return yv[0]>>16; // return 16 most significant bits of most recent output
		}

		temperature readInput(void){
			return xv[0]>>16; // return 16 most significant bits of most recent input
		}

		temperature_precise readOutputDoublePrecision(void){
			return yv[0];
		}

		temperature_precise readPrevOutputDoublePrecision(void){
			return yv[1];
		}

		temperature detectPosPeak(void); //returns positive peak or INVALID_TEMP when no peak has been found
		temperature detectNegPeak(void); //returns negative peak or INVALID_TEMP when no peak has been found

};
