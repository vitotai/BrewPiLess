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
#include "RotaryEncoder.h"

#include "Pins.h"
#include <limits.h>
#include "Ticks.h"
#include "Display.h"
#include "Brewpi.h"
#include "TempControl.h"

#if ESP32
#include "rom/gpio.h"
#endif

#if ButtonViaPCF8574
#include <pcf8574_esp.h>
#endif

RotaryEncoder rotaryEncoder;

int16_t RotaryEncoder::maximum;
int16_t RotaryEncoder::minimum;
volatile int16_t RotaryEncoder::steps;
volatile bool RotaryEncoder::pushFlag;

#if ESP32
static volatile bool resetBackLite;
#endif


#if !defined(ESP8266) && !defined(ESP32)
#if BREWPI_STATIC_CONFIG!=BREWPI_SHIELD_DIY
	#if rotarySwitchPin != 7
		#error Review interrupt vectors when not using pin 7 for menu push
	#endif
	#if rotaryAPin != 8
		#error Review interrupt vectors when not using pin 8 for menu right
	#endif
	#if rotaryBPin != 9
		#error Review interrupt vectors when not using pin 9 for menu left
	#endif
#else
	#if rotarySwitchPin != 0
	#error Review interrupt vectors when not using pin 0 for menu push
	#endif
	#if rotaryAPin != 2
	#error Review interrupt vectors when not using pin 2 for menu right
	#endif
	#if rotaryBPin != 1
	#error Review interrupt vectors when not using pin 1 for menu left
	#endif
#endif

#if BREWPI_ROTARY_ENCODER
#if BREWPI_BOARD!=BREWPI_BOARD_LEONARDO && BREWPI_BOARD!=BREWPI_BOARD_STANDARD
	#error Rotary encoder code is not compatible with boards other than leonardo or uno yet.
#endif
#endif
#endif //#ifndef ESP8266


#if BREWPI_BUTTONS || ButtonViaPCF8574

#define BUTTON_INTERRUPT 1

#if ButtonViaPCF8574
PCF8574 pcf8574(PCF8574_ADDRESS,PIN_SDA, PIN_SCL);
#endif

// *************************
//*  Button timeing setting
// *************************
#define ButtonPressedDetectMinTime 10 // in ms
#define ButtonLongPressedDetectMinTime 1000 // in ms
#define ButtonContinuousPressedDetectMinTime 1000 // in ms
#define ButtonContinuousPressedTrigerTime 150 // in ms
#define ButtonFatFingerTolerance 400  // in ms

static unsigned long _buttonChangeTime;
static unsigned long _continuousPressedDectedTime;
static unsigned long _oneFigerUp;

static boolean _continuousPressedDetected;
static boolean _longPressed;

static uint8_t _testButtunStatus;
static uint8_t _buttonPressed;

#define ButtonUpMask    0x01
#define ButtonDownMask  (0x01 << 1)
//#define ButtonSetMask (0x01 << 2)

#define btnIsUpPressed (_buttonPressed == ButtonUpMask)
#define btnIsDownPressed (_buttonPressed == ButtonDownMask)
#define btnIsSetPressed (_buttonPressed == (ButtonUpMask| ButtonDownMask))

#define btnIsUpContinuousPressed (_buttonPressed == (ButtonUpMask<<4))
#define btnIsDownContinuousPressed (_buttonPressed == (ButtonDownMask<<4))


static void btnInit(void){
	_testButtunStatus=0;
	_buttonChangeTime=0;
	_continuousPressedDectedTime=0;
	_continuousPressedDetected=false;
	_longPressed=0;
	_buttonPressed=0;
	_oneFigerUp=0;
}
#if SerialDebug
#define BtnDebugPrintf(...)  //Serial.printf(__VA_ARGS__)
#else
#define BtnDebugPrintf(...)  
#endif

#if BUTTON_INTERRUPT
// avoid using digitalRead, supposedly quicker.
static unsigned char buttonStatus=0;

IRAM_ATTR static boolean btnDetect(void)
{
	uint32_t currentTimeInMS=millis();

	if(buttonStatus==0)
	{
		if(_testButtunStatus ==0) return false;

		unsigned long duration=currentTimeInMS - _buttonChangeTime;

    	BtnDebugPrintf("pressed:%d,%d for %ld\n",_testButtunStatus,buttonStatus,duration);

		if(duration > ButtonPressedDetectMinTime)
		{
			if(duration > ButtonLongPressedDetectMinTime) _longPressed=true;
			else _longPressed =false;
			_buttonPressed = _testButtunStatus;

			_testButtunStatus =0;
			_continuousPressedDetected = false;

			BtnDebugPrintf("presse %d long?%d\n",_buttonPressed,_longPressed);

			return true;
		}

    	BtnDebugPrintf("Not Pressed");

		_testButtunStatus =0;
		_continuousPressedDetected = false;

		return false;
	}

	// current button status is not ZERO
	if(buttonStatus == _testButtunStatus) // pressing persists
	{
		if(_continuousPressedDetected )
		{
			//if duration exceeds a trigger point
			if( (currentTimeInMS - _continuousPressedDectedTime) > ButtonContinuousPressedTrigerTime)
			{
				_continuousPressedDectedTime=currentTimeInMS;

				BtnDebugPrintf("con-pre:%d @%ld\n",_buttonPressed,currentTimeInMS);

				return true;
			}
		}
		else
		{
			unsigned long duration=currentTimeInMS - _buttonChangeTime;

			if(duration > ButtonContinuousPressedDetectMinTime)
			{
				_continuousPressedDetected=true;
				_continuousPressedDectedTime=currentTimeInMS;
				// fir the first event
				_buttonPressed = buttonStatus << 4; // user upper 4bits for long pressed

				BtnDebugPrintf("Continue:%d start %ld\n",_buttonPressed,currentTimeInMS);

				return true;
			}
		}
	}
	else // if(buttonStatus == _testButtunStatus)
	{
		// for TWO buttons event, it is very hard to press and depress
		// two buttons at exactly the same time.
		// so if new status is contains in OLD status.
		// it might be the short period when two fingers are leaving, but one is detected
		// first before the other
		// the case might be like  01/10 -> 11 -> 01/10
		//  just handle the depressing case: 11-> 01/10
		if((_testButtunStatus & buttonStatus)
			&&  (_testButtunStatus > buttonStatus))
		{
			if(_oneFigerUp ==0)
			{
				_oneFigerUp = currentTimeInMS;
				// skip this time
				return false;
			}
			else
			{
				// one fat finger is dected
				if( (currentTimeInMS -_oneFigerUp) < ButtonFatFingerTolerance)
				{
					return false;
				}
			}

	    	BtnDebugPrintf("Failed fatfinger\n");

		}
		// first detect, note time to check if presist for a duration.
		_testButtunStatus = buttonStatus;
		_buttonChangeTime = currentTimeInMS;
		_oneFigerUp = 0;

		BtnDebugPrintf("Attempt:%d\n",buttonStatus);
	}

	return false;
}
static bool _buttonStatusChanged=false;

IRAM_ATTR static void processbuttons(){
	if(btnDetect()){
		_buttonStatusChanged = true;
	}
}
IRAM_ATTR static void isr_upChanged(void) {
	if (digitalRead(UpButtonPin) == 0){
		buttonStatus |= ButtonUpMask;
	}else{
		buttonStatus &= 0xFF ^ ButtonUpMask;
	}
	processbuttons();
}

IRAM_ATTR static void isr_downChanged(void) {
	if (digitalRead(DownButtonPin) == 0){
		buttonStatus |= ButtonDownMask;
	}else{
		buttonStatus &= 0xFF ^ ButtonDownMask;
	}
	processbuttons();
}

static boolean btnReadButtons(void){
	processbuttons(); // to process continuous pressing

	//noInterrupts();
	if(_buttonStatusChanged){
		//  Interrupt happens just before read and write.
		//  that would result in loss of button pressed event.
		//  however, disable/enable interrupt seems to make the system unstable.
		// it's better to lose information instead of instaibliity.
		_buttonStatusChanged = false;
		//interrupts();
		return true;
	}
	//interrupts();
	return false;
}

#else //#if BUTTON_INTERRUPT
static boolean btnReadButtons(void)
{
	uint32_t currentTimeInMS=millis();

  	unsigned char buttons=0;

	#if ButtonViaPCF8574
	uint8_t p;
	
	p =pcf8574.read8();
	BtnDebugPrintf("pcfread:%d\n",p);

	if(!(p & UpButtonBitMask))	buttons |= ButtonUpMask;
	if(!(p & DownButtonBitMask))	buttons |= ButtonDownMask;

	#else

  	if (digitalRead(UpButtonPin) == 0)
  	{
  		buttons |= ButtonUpMask;
  	}
  	if (digitalRead(DownButtonPin) == 0)
  	{
  		buttons |= ButtonDownMask;
  	}
	#endif

	if(buttons==0)
	{
		if(_testButtunStatus ==0) return false;

		unsigned long duration=currentTimeInMS - _buttonChangeTime;

    	BtnDebugPrintf("pressed:%d,%d for %ld\n",_testButtunStatus,buttons,duration);

		if(duration > ButtonPressedDetectMinTime)
		{
			if(duration > ButtonLongPressedDetectMinTime) _longPressed=true;
			else _longPressed =false;
			_buttonPressed = _testButtunStatus;

			_testButtunStatus =0;
			_continuousPressedDetected = false;

			BtnDebugPrintf("presse %d long?%d\n",_buttonPressed,_longPressed);

			return true;
		}

    	BtnDebugPrintf("Not Pressed");

		_testButtunStatus =0;
		_continuousPressedDetected = false;

		return false;
	}

	// current button status is not ZERO
	if(buttons == _testButtunStatus) // pressing persists
	{
		if(_continuousPressedDetected )
		{
			//if duration exceeds a trigger point
			if( (currentTimeInMS - _continuousPressedDectedTime) > ButtonContinuousPressedTrigerTime)
			{
				_continuousPressedDectedTime=currentTimeInMS;

				BtnDebugPrintf("con-pre:%d @%ld\n",_buttonPressed,currentTimeInMS);

				return true;
			}
		}
		else
		{
			unsigned long duration=currentTimeInMS - _buttonChangeTime;

			if(duration > ButtonContinuousPressedDetectMinTime)
			{
				_continuousPressedDetected=true;
				_continuousPressedDectedTime=currentTimeInMS;
				// fir the first event
				_buttonPressed = buttons << 4; // user upper 4bits for long pressed

				BtnDebugPrintf("Continue:%d start %ld\n",_buttonPressed,currentTimeInMS);

				return true;
			}
		}
	}
	else // if(buttons == _testButtunStatus)
	{
		// for TWO buttons event, it is very hard to press and depress
		// two buttons at exactly the same time.
		// so if new status is contains in OLD status.
		// it might be the short period when two fingers are leaving, but one is detected
		// first before the other
		// the case might be like  01/10 -> 11 -> 01/10
		//  just handle the depressing case: 11-> 01/10
		if((_testButtunStatus & buttons)
			&&  (_testButtunStatus > buttons))
		{
			if(_oneFigerUp ==0)
			{
				_oneFigerUp = currentTimeInMS;
				// skip this time
				return false;
			}
			else
			{
				// one fat finger is dected
				if( (currentTimeInMS -_oneFigerUp) < ButtonFatFingerTolerance)
				{
					return false;
				}
			}

	    	BtnDebugPrintf("Failed fatfinger\n");

		}
		// first detect, note time to check if presist for a duration.
		_testButtunStatus = buttons;
		_buttonChangeTime = currentTimeInMS;
		_oneFigerUp = 0;

		BtnDebugPrintf("Attempt:%d\n",buttons);
	}

	return false;
}

#endif //#if BUTTON_INTERRUPT


void RotaryEncoder::init(void){

#if ButtonViaPCF8574

#else



// BREWPI_BUTTONS
	pinMode(UpButtonPin, INPUT_PULLUP);
	pinMode(DownButtonPin, INPUT_PULLUP);

#if BUTTON_INTERRUPT
	attachInterrupt(UpButtonPin, isr_upChanged, CHANGE);
	attachInterrupt(DownButtonPin, isr_downChanged, CHANGE);
#endif

#endif

	btnInit();
}

void RotaryEncoder::setRange(int16_t start, int16_t minVal, int16_t maxVal){
		// this part cannot be interrupted
		// Multiply by two to convert to half steps
		steps = start;
		minimum = minVal;
		maximum = maxVal; // +1 to make sure that one step is still two half steps at overflow
}

void RotaryEncoder::setPushed(void){
	pushFlag = true;
}

void RotaryEncoder::process(void){
	// check button status
	if(btnReadButtons()){
		display.resetBacklightTimer();

		if(btnIsSetPressed){
			DBG_PRINTF("SET\n");
			setPushed();
		}else if(btnIsUpPressed){
			DBG_PRINTF("UP\n");
			if(steps < maximum) steps++;
		}else if(btnIsDownPressed){
			DBG_PRINTF("DOWN\n");
			if(steps > minimum)  steps--;
		}else if(btnIsUpContinuousPressed){
			DBG_PRINTF("UP+\n");
			steps+=2;
			if(steps > maximum) steps = maximum;
		}else if(btnIsDownContinuousPressed){
			DBG_PRINTF("DOWN+\n");
			steps-=2;
			if(steps < minimum) steps = minimum;
		}
	}
}
bool RotaryEncoder::pushed(void){
	process();
	return pushFlag;
}
bool RotaryEncoder::changed(void){
	process();
	// returns one if the value changed since the last call of changed.
	static int16_t prevValue = 0;
	int16_t r = read();
	if(r != prevValue){
		prevValue = r;
		return 1;
	}
	if(pushFlag == true){
		return 1;
	}
	return 0;
}

int16_t RotaryEncoder::read(void){
	return steps;
}

#else //#if BREWPI_BUTTONS
// Implementation based on work of Ben Buxton:

/* Rotary encoder handler for arduino. v1.1
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 * A typical mechanical rotary encoder emits a two bit gray code
 * on 3 output pins. Every step in the output (often accompanied
 * by a physical 'click') generates a specific sequence of output
 * codes on the pins.
 *
 * There are 3 pins used for the rotary encoding - one common and
 * two 'bit' pins.
 *
 * The following is the typical sequence of code on the output when
 * moving from one step to the next:
 *
 *   Position   Bit1   Bit2
 *   ----------------------
 *     Step1     0      0
 *      1/4      1      0
 *      1/2      1      1
 *      3/4      0      1
 *     Step2     0      0
 *
 * From this table, we can see that when moving from one 'click' to
 * the next, there are 4 changes in the output code.
 *
 * - From an initial 0 - 0, Bit1 goes high, Bit0 stays low.
 * - Then both bits are high, halfway through the step.
 * - Then Bit1 goes low, but Bit2 stays high.
 * - Finally at the end of the step, both bits return to 0.
 *
 * Detecting the direction is easy - the table simply goes in the other
 * direction (read up instead of down).
 *
 * To decode this, we use a simple state machine. Every time the output
 * code changes, it follows state, until finally a full steps worth of
 * code is received (in the correct order). At the final 0-0, it returns
 * a value indicating a step in one direction or the other.
 *
 * It's also possible to use 'half-step' mode. This just emits an event
 * at both the 0-0 and 1-1 positions. This might be useful for some
 * encoders where you want to detect all positions.
 *
 * If an invalid state happens (for example we go from '0-1' straight
 * to '1-0'), the state machine resets to the start until 0-0 and the
 * next valid codes occur.
 *
 * The biggest advantage of using a state machine over other algorithms
 * is that this has inherent debounce built in. Other algorithms emit spurious
 * output with switch bounce, but this one will simply flip between
 * sub-states until the bounce settles, then continue along the state
 * machine.
 * A side effect of debounce is that fast rotations can cause steps to
 * be skipped. By not requiring debounce, fast rotations can be accurately
 * measured.
 * Another advantage is the ability to properly handle bad state, such
 * as due to EMI, etc.
 * It is also a lot simpler than others - a static state table and less
 * than 10 lines of logic.
 */

/*
 * The below state table has, for each state (row), the new state
 * to set based on the next encoder output. From left to right in,
 * the table, the encoder outputs are 00, 01, 10, 11, and the value
 * in that position is the new state to set.
 */

#define R_START 0x0
// #define HALF_STEP

// Use the half-step state table (emits a code at 00 and 11)
#define HS_R_CCW_BEGIN 0x1
#define HS_R_CW_BEGIN 0x2
#define HS_R_START_M 0x3
#define HS_R_CW_BEGIN_M 0x4
#define HS_R_CCW_BEGIN_M 0x5
#if  ESP32
// this table is accessed in ISR, needed to stay in RAM
uint8_t hs_ttable[7][4] = {
#else
const uint8_t PROGMEM hs_ttable[7][4] = {
#endif
	// R_START (00)
	{HS_R_START_M,            HS_R_CW_BEGIN,     HS_R_CCW_BEGIN,  R_START},
	// HS_R_CCW_BEGIN
	{HS_R_START_M | DIR_CCW, R_START,        HS_R_CCW_BEGIN,  R_START},
	// HS_R_CW_BEGIN
	{HS_R_START_M | DIR_CW,  HS_R_CW_BEGIN,     R_START,      R_START},
	// HS_R_START_M (11)
	{HS_R_START_M,            HS_R_CCW_BEGIN_M,  HS_R_CW_BEGIN_M, R_START},
	// HS_R_CW_BEGIN_M
	{HS_R_START_M,            HS_R_START_M,      HS_R_CW_BEGIN_M, R_START | DIR_CW},
	// HS_R_CCW_BEGIN_M
	{HS_R_START_M,            HS_R_CCW_BEGIN_M,  HS_R_START_M,    R_START | DIR_CCW},
	{R_START, R_START, R_START, R_START}
};

// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6
#if  ESP32
// this table is accessed in ISR, needed to stay in RAM
uint8_t ttable[7][4] = {
#else
const uint8_t PROGMEM ttable[7][4] = {
#endif
	// R_START
	{R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
	// R_CW_FINAL
	{R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
	// R_CW_BEGIN
	{R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
	// R_CW_NEXT
	{R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
	// R_CCW_BEGIN
	{R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
	// R_CCW_FINAL
	{R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
	// R_CCW_NEXT
	{R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

#if BREWPI_ROTARY_ENCODER

#ifdef ESP8266
#error "ESP8266 doesn't support Rotary encoder"
#endif

#ifdef ESP32

#define GPIO_READ(pin) (((pin)>31)? (gpio_input_get_high() & (1<<(pin-32))):(gpio_input_get() & (1<<pin)))

static void IRAM_ATTR isr_rotary(void) { 
	rotaryEncoder.process(); 
}

#define MINIMUM_PUSH_GAP 200000

static void IRAM_ATTR isr_push(void) {
	// using software debouncing.
	static uint32_t pushed_time=0;
	uint32_t now= micros();
	if(! GPIO_READ(rotarySwitchPin) && (now - pushed_time) > MINIMUM_PUSH_GAP ){
		pushed_time = now;
		rotaryEncoder.setPushed();
	}
}

#else //#ifdef ESP32
#include "util/atomic.h"
#include "FastDigitalPin.h"


#if BREWPI_STATIC_CONFIG==BREWPI_SHIELD_DIY
ISR(INT2_vect) {
	rotaryEncoder.setPushed();
}
ISR(INT3_vect) {
	rotaryEncoder.process();
}
ISR(INT1_vect) {
	rotaryEncoder.process();
}
#elif BREWPI_BOARD == BREWPI_BOARD_LEONARDO
ISR(INT6_vect){
	rotaryEncoder.setPushed();
}
ISR(PCINT0_vect){
	rotaryEncoder.process();
}
#elif BREWPI_BOARD == BREWPI_BOARD_STANDARD
ISR(PCINT2_vect){
	if(!bitRead(PIND,7)){
		// high to low transition
		rotaryEncoder.setPushed();
	}
}
ISR(PCINT0_vect){
	rotaryEncoder.process();
}
#else
	#error board/processor not supported by rotary encoder code. Disable or fix the rotary encoder.
#endif
#endif //#ifdef ESP8266


void IRAM_ATTR RotaryEncoder::process(void){

	static uint8_t state=R_START;
	// Grab state of input pins.

	#if ESP32
	/* uint8_t currPinA = !digitalRead(rotaryAPin);
	 uint8_t currPinB = !digitalRead(rotaryBPin); */

//	uint32_t input=gpio_input_get();

	uint8_t currPinA = ! GPIO_READ(rotaryAPin);
	uint8_t currPinB = ! GPIO_READ(rotaryBPin);

	#else // #ifdef ESP32
	#if BREWPI_STATIC_CONFIG == BREWPI_SHIELD_DIY
	uint8_t currPinA = !bitRead(PIND,2);
	uint8_t currPinB = !bitRead(PIND,3);
	#elif BREWPI_BOARD == BREWPI_BOARD_LEONARDO
	uint8_t currPinA = !bitRead(PINB,4);
	uint8_t currPinB = !bitRead(PINB,5);
	#elif BREWPI_BOARD == BREWPI_BOARD_STANDARD
	uint8_t currPinA = !bitRead(PINB,0);
	uint8_t currPinB = !bitRead(PINB,1);
	#endif
	#endif //#ifdef ESP32

	unsigned char pinstate = (currPinB << 1) | currPinA;

	// Determine new state from the pins and state table.
	if(tempControl.cc.rotaryHalfSteps){
		#if ESP32
		state = hs_ttable[state & 0xf][pinstate];
		#else
		state = pgm_read_byte(&(hs_ttable[state & 0xf][pinstate]));
		#endif
	}
	else{
		#if ESP32
		state = ttable[state & 0xf][pinstate];
		#else
		state = pgm_read_byte(&(ttable[state & 0xf][pinstate]));
		#endif
	}

	// Get emit bits, ie the generated event.

	uint8_t dir = state & 0x30;

	if(dir){
		int16_t s = steps;	// steps is volatile - save a copy here to avoid multiple fetches
		s = (dir==DIR_CW) ? s+1 : s-1;
		if (s > maximum)
			s = minimum;
		else if (s < minimum)
			s = maximum;
		steps = s;
		// this goes too deep, and needed to put in ICACHE display.resetBacklightTimer();
		#if ESP32
		resetBackLite = true;
		#endif
	}
}
#endif  // BREWPI_ROTARY_ENCODER

#if ESP8266
//#define IRAM_ATTR 
#endif

void IRAM_ATTR RotaryEncoder::setPushed(void){
	pushFlag = true;
	
	// this goes too deep, and needed to put in ICACHE, also it is processed in outer loop 
	//display.resetBacklightTimer();
		#if ESP32
		resetBackLite = true;
		#endif
}

#define BREWPI_INPUT_PULLUP (USE_INTERNAL_PULL_UP_RESISTORS ? INPUT_PULLUP : INPUT)

void RotaryEncoder::init(void){
#if BREWPI_ROTARY_ENCODER

#if ESP32
	fastPinMode(rotaryAPin, BREWPI_INPUT_PULLUP);
	fastPinMode(rotaryBPin, BREWPI_INPUT_PULLUP);
	fastPinMode(rotarySwitchPin, BREWPI_INPUT_PULLUP);
	attachInterrupt(rotaryAPin, isr_rotary, CHANGE);
	attachInterrupt(rotaryBPin, isr_rotary, CHANGE);
	attachInterrupt(rotarySwitchPin, isr_push, FALLING);
#else //#ifdef ESP32
	#define BREWPI_INPUT_PULLUP (USE_INTERNAL_PULL_UP_RESISTORS ? INPUT_PULLUP : INPUT)
	fastPinMode(rotaryAPin, BREWPI_INPUT_PULLUP);
	fastPinMode(rotaryBPin, BREWPI_INPUT_PULLUP);
	fastPinMode(rotarySwitchPin, BREWPI_INPUT_PULLUP);

	#if BREWPI_STATIC_CONFIG==BREWPI_SHIELD_DIY
		EICRA |= (1<<ISC21) | (1<<ISC10) | (1<<ISC30);; // any logical change for encoder pins, falling edge for switch
		EIMSK |= (1<<INT2) | (1<<INT1) | (1<<INT3); // enable interrupts for each pin
	#elif BREWPI_BOARD == BREWPI_BOARD_LEONARDO
		// falling edge interrupt for switch on INT6
		EICRB |= (1<<ISC61) | (0<<ISC60);
		// enable interrupt for INT6
		EIMSK |= (1<<INT6);
		// enable pin change interrupts
		PCICR |= (1<<PCIE0);
		// enable pin change interrupt on Arduino pin 8 and 9
		PCMSK0 |= (1<<PCINT5) | (1<<PCINT4);
	#elif BREWPI_BOARD == BREWPI_BOARD_STANDARD
		// enable PCINT0 (PCINT0 and PCINT1 pin) and PCINT2 vector (PCINT23 pin)
		PCICR |= (1<<PCIE2) | (1<<PCIE0);
		// enable mask bits for PCINT0 and PCINT1
		PCMSK0 |= (1<<PCINT0) | (1<<PCINT1);
		// enable mask bit for PCINT23
		PCMSK2 |= (1<<PCINT23);
	#endif
#endif // #ifdef ESP32
#endif	//#if BREWPI_ROTARY_ENCODER

}


void RotaryEncoder::setRange(int16_t start, int16_t minVal, int16_t maxVal){
#if BREWPI_ROTARY_ENCODER
#ifdef ESP32
	noInterrupts();
#else
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
#endif
		// this part cannot be interrupted
		// Multiply by two to convert to half steps
		steps = start;
		minimum = minVal;
		maximum = maxVal; // +1 to make sure that one step is still two half steps at overflow
#ifdef ESP32
	interrupts();
#else
	}
#endif
#endif
}

bool RotaryEncoder::changed(void){
	// returns one if the value changed since the last call of changed.
	static int16_t prevValue = 0;
	int16_t r = read();
	if(r != prevValue){
		prevValue = r;
		return 1;
	}
	if(pushFlag == true){
		return 1;
	}
	return 0;
}

int16_t RotaryEncoder::read(void){
#if BREWPI_ROTARY_ENCODER
#ifdef ESP32
	return steps;
#else
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return steps;
	}
#endif
#endif
	return 0;
}
#if ESP32
bool RotaryEncoder::pushed(void){
		if(resetBackLite){
			resetBackLite = false;
			display.resetBacklightTimer();
		}
		return pushFlag;
}
#endif

#endif // #if BREWPI_BUTTONS