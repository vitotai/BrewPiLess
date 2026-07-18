/*
 * Copyright 2013 Matthew McGowan
 * Copyright 2013 BrewPi/Elco Jacobs.
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
#include "OneWire.h"

#include "DS2413.h"



/*
	* Read all values at once, both current state and sensed values. The read performs data-integrity checks.
	* Returns a negative result if the device cannot be read successfully within the given number of tries.
	* The lower 4-bits are the values as described under PIO ACCESSS READ [F5h] in the ds2413 datasheet:
	* b0: PIOA state
	* b1: PIOA output latch state
	* b2: PIOB state
	* b3: PIOB output latch state
	*/
byte DS2413::accessRead(uint8_t maxTries) /* const */
{
	#define ACCESS_READ 0xF5

	oneWire->reset();
	oneWire->select(address);
	oneWire->write(ACCESS_READ);

	bool success = false;
	uint8_t data;
	do
	{
		data = oneWire->read();
		success = (data>>4)==(!data&0xF);
		data &= 0xF;
	} while (!success && maxTries-->0);

	oneWire->reset();
	return success ? data : data|0x80;
}

/*
	* Writes the state of all PIOs in one operation.
	* /param b pio data - PIOA is bit 0 (lsb), PIOB is bit 1
	* /param maxTries the maximum number of attempts before giving up.
	* /return true on success
	*/
bool DS2413::accessWrite(uint8_t b, uint8_t maxTries)
{
	#define ACCESS_WRITE 0x5A
	#define ACK_SUCCESS 0xAA
	#define ACK_ERROR 0xFF

	b |= 0xFC;   		/* Upper 6 bits should be set to 1's */
	uint8_t ack = 0;
	do
	{
		oneWire->reset();
		oneWire->select(address);
		oneWire->write(ACCESS_WRITE);
		oneWire->write(b);

		/* data is sent again, inverted to guard against transmission errors */
		oneWire->write(~b);
		/* Acknowledgement byte, 0xAA for success, 0xFF for failure. */
		ack = oneWire->read();

		if (ack==ACK_SUCCESS)
			oneWire->read();		// status byte sent after ack

		//out.print("tries "); out.print(maxTries); out.print(" ack ");out.print(ack, HEX);out.print(" newValues ");out.print(newSettings, HEX);
		//out.println();
	} while (ack!=ACK_SUCCESS && maxTries-->0);

	oneWire->reset();
	return ack==ACK_SUCCESS;
}
