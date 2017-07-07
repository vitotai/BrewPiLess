/*
 * Buzzer.cpp
 *
 * Copyright 2012-2013 BrewPi.
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
#include "Ticks.h"
#include "Pins.h"
#include "Buzzer.h"

// TODO - Implement

#if BREWPI_BUZZER

#ifndef ESP8266_ONE
#include <util/delay.h>
#include "FastDigitalPin.h"

#if (alarmPin != 3)
	#error "Check PWM settings when you want to use a different pin for the alarm"
#endif
#endif

#if BREWPI_BOARD == BREWPI_BOARD_LEONARDO
	#define BUZZER_TIMER_REG TCCR0A
	#define BUZZER_TIMER_REG_FLAG COM0B1
	#define BUZZER_TIMER_REG_INIT (void);
#elif BREWPI_BOARD == BREWPI_BOARD_STANDARD
	#define BUZZER_TIMER_REG TCCR2A
	#define BUZZER_TIMER_REG_FLAG COM2B1
#elif BREWPI_BOARD == BREWPI_BOARD_MEGA
	#define BUZZER_TIMER_REG TCCR3A
	#define BUZZER_TIMER_REG_FLAG COM3C1
#endif

#ifdef ESP8266_ONE
#define BEEP_ON() digitalWrite(BuzzPin,1);
#define BEEP_OFF()  digitalWrite(BuzzPin,0);

#else
#define BEEP_ON() bitSet(BUZZER_TIMER_REG,BUZZER_TIMER_REG_FLAG);
#define BEEP_OFF() bitClear(BUZZER_TIMER_REG,BUZZER_TIMER_REG_FLAG);
#endif

void Buzzer::init(void){
#ifdef ESP8266_ONE
	pinMode(BuzzPin,OUTPUT);
#else
	// set up square wave PWM for buzzer
	fastPinMode(alarmPin,OUTPUT);
#endif

#if BREWPI_BOARD == BREWPI_BOARD_LEONARDO
	// Arduino Leonardo, no further setup needed - timer already active
#elif BREWPI_BOARD == BREWPI_BOARD_STANDARD
	// Arduino UNO, buzzer is on OC2B
	TCCR2A = (1<<COM2B1) | (1<<WGM20);
	TCCR2B = (1<<WGM22) | (1<<CS21) | (1<<CS20); // prescaler = 32
	OCR2A = 125; // timer top. This value adjusts the frequency.
	OCR2B = 62;
#elif BREWPI_BOARD == BREWPI_BOARD_MEGA
	TCCR3A = (1<<COM3C1) | (1<<WGM30);
	TCCR3C = (1<<WGM32) | (1<<CS31) | (1<<CS30);  // prescaler = 32
	OCR3A = 125;  // timer top. This value adjusts the frequency.
	OCR3C = 62;
	TIMSK3 |= (1 << OCIE3C);
#endif
}

void Buzzer::setActive(bool active)
{
	if (active!=this->isActive()) {
		ValueActuator::setActive(active);
		if (active) {
			BEEP_ON();
		}
		else {
			BEEP_OFF();
		}
	}
}

void Buzzer::beep(uint8_t numBeeps, uint16_t duration){
	for(uint8_t beepCount = 0; beepCount<numBeeps; beepCount++){
		BEEP_ON();
		wait.millis(duration);
		BEEP_OFF();
		if(beepCount < numBeeps - 1){
			wait.millis(duration); // not the last beep
		}
	}
}

Buzzer buzzer;

#endif
