/*
 * File:   ArduinoActuator.h
 * Author: mat
 *
 * Created on 19 August 2013, 20:32
 */

#pragma once

#include "Actuator.h"

#ifndef LATCHING_RELAYS
#define LATCHING_RELAYS false
#endif

template<uint8_t pin, bool invert>
class DigitalConstantPinActuator ACTUATOR_BASE_CLASS_DECL
{
	private:
	bool active;

	public:
	DigitalConstantPinActuator() : active(false)
	{
		setActive(false);
		fastPinMode(pin, OUTPUT);
	}

	inline ACTUATOR_METHOD void setActive(bool active) {
		this->active = active;
		fastDigitalWrite(pin, active^invert ? HIGH : LOW);
	}

	bool isActive() { return active; }

};

class DigitalPinActuator ACTUATOR_BASE_CLASS_DECL
{
	private:
	bool invert;
	uint8_t pin;
	bool active;
	public:
	DigitalPinActuator(uint8_t pin, bool invert) {
		this->invert = invert;
		this->pin = pin;
		setActive(false);
		pinMode(pin, OUTPUT);
	}

	inline ACTUATOR_METHOD void setActive(bool active) {
		this->active = active;

		#if LATCHING_RELAYS // Latching relay
			if (active) {
				digitalWrite(LATCH_PIN_SET, HIGH);
				delay(10);
				digitalWrite(LATCH_PIN_SET, LOW);
			} else {
				digitalWrite(LATCH_PIN_RESET, HIGH);
				delay(10);
				digitalWrite(LATCH_PIN_RESET, LOW);
			}
		#else
			digitalWrite(pin, active^invert ? HIGH : LOW);
		#endif
	}

	bool isActive() { return active; }
};
