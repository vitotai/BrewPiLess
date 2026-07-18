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

typedef uint32_t ticks_millis_t;
typedef uint32_t ticks_micros_t;
typedef uint16_t ticks_seconds_t;
typedef uint8_t ticks_seconds_tiny_t;

/**
 * Ticks - interface to a millisecond timer
 *
 * With more code space, Ticks would have been a virtual base class, so all implementations can easily provide the same interface.
 * Here, the different implementations have no common (virtual) base class to save code space.
 * Instead, a typedef is used to compile-time select the implementation to use.
 * If that implementation doesn't implement the Ticks interface as expected, it will fail to compile.
 */

/**
 * A Ticks implementation that increments the millis count each time it is called.
 * This is used for testing.
 */
class MockTicks {
public:
	MockTicks(uint8_t increment) : _increment(increment), _ticks(0) { }

	ticks_millis_t millis() { return _ticks+=_increment; }
	ticks_micros_t micros() { return _ticks+=_increment; }
	ticks_seconds_t seconds() { return millis()>>10; }
	ticks_seconds_t timeSince(ticks_seconds_t timeStamp) { return timeStamp-seconds(); }
private:

	uint32_t _increment;
	uint32_t _ticks;
};

/**
 * Externally provided millis timer. The calling code takes care of advancing the timer by calling setMillis or incMillis.
 * This is used for testing and also by the simulator to provide simulated time.
 */
class ExternalTicks {
	public:
	ExternalTicks() : _ticks(0) { }

	ticks_millis_t millis() { return _ticks; }
	ticks_micros_t micros() { return _ticks*1000; }
	ticks_seconds_t seconds() { return millis()/1000; }
	ticks_seconds_t timeSince(ticks_seconds_t timeStamp);

	void setMillis(ticks_millis_t now)	{ _ticks = now; }
	void incMillis(ticks_millis_t advance)	{ _ticks += advance; }
private:
	ticks_millis_t _ticks;
};


/**
 * A delay class that does nothing.
 * In the AVR simulator, delays using millis() take a very long time. Using this class makes it possible to step through the code.
 */
class NoOpDelay {
public:
	void seconds(uint16_t seconds)	{ millis(seconds<<10); }
	void millis(uint32_t millis)	{ }
	void microseconds(uint32_t micros) { }
};

#include "TicksArduino.h"

// Determine the type of Ticks needed
// TICKS_IMPL_CONFIG is the code string passed to the constructor of the Ticks implementation

#if BREWPI_SIMULATE
/** For simulation, by the simulator - each step in the simulator advances the time by one second. */
	typedef ExternalTicks TicksImpl;
	#define TICKS_IMPL_CONFIG		// no configuration of ExternalTicks necessary

#elif BREWPI_EMULATE
/** When debugging in AVR studio (and running normal brewpi - not the simulator), use a simple MockTicks that increments 100
	millis each time it's called. */
	typedef MockTicks TicksImpl;
	#define TICKS_IMPL_CONFIG 100

#else // use regular hardware timer/delay
	typedef HardwareTicks TicksImpl;
	#define TICKS_IMPL_CONFIG
#endif	// BREWPI_EMULATE

extern TicksImpl ticks;

// Determine the type of delay required.
// For emulation, don't delay, since time in the emulator is not real time, so the delay is meaningless.
// For regular code, use the arduino delay function.

#if BREWPI_EMULATE || !defined(ARDUINO)
typedef NoOpDelay DelayImpl;		// for emulation (avr debugger), don't bother delaying, it takes ages.
#define DELAY_IMPL_CONFIG
#else
typedef HardwareDelay DelayImpl;
#define DELAY_IMPL_CONFIG
#endif

extern DelayImpl wait;
