/*
 * File:   ArduinoActuator.h
 * Author: mat
 *
 * Created on 19 August 2013, 20:32
 */

#pragma once

#include "Actuator.h"

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
		digitalWrite(pin, active^invert ? HIGH : LOW);
	}

	bool isActive() { return active; }
};
