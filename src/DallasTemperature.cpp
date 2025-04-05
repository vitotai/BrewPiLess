// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// Version 3.7.2 modified on Dec 6, 2011 to support Arduino 1.0
// See Includes...
// Modified by Jordan Hochenbaum

// MDMA: added conditional compile for different facets of the API
// MDMA: added reset detection support so calling code knows to re-initialize the sensor

#include "Brewpi.h"
#include "DallasTemperature.h"
#include "Ticks.h"


#if ARDUINO >= 100
#include "Arduino.h"
#else
extern "C" {
#include "WConstants.h"
}
#endif

DallasTemperature::DallasTemperature(OneWire* _oneWire)
#if REQUIRESALARMS
    : _AlarmHandler(&defaultAlarmHandler)
#endif
{
    _wire = _oneWire;
#if REQUIRESINDEXEDADDRESSING
    devices = 0;
#endif
#if REQUIRESPARASITEPOWERAVAILABLE
    parasite = false;
#endif
#if !REQUIRESONLY12BITCONVERSION
    bitResolution = 9;
#endif
#if REQUIRESWAITFORCONVERSION
    waitForConversion = true;
    checkForConversion = true;
#endif
}

bool DallasTemperature::initConnection(const uint8_t* deviceAddress) {
#if REQUIRESRESETDETECTION
	ScratchPad scratchPad;

	if (!isConnected(deviceAddress, scratchPad)) {
		return false;
	}

	// assume the sensor has just been powered on. So this should only be called on initializtion, or
	// after a device was disconnected.
	if (scratchPad[HIGH_ALARM_TEMP]) {		// conditional to avoid wear on eeprom.
		scratchPad[HIGH_ALARM_TEMP] = 0;
		writeScratchPad(deviceAddress, scratchPad, true);	// save to eeprom

		// check if the write was successful (HIGH_ALARAM_TEMP == 0)
		if (!isConnected(deviceAddress, scratchPad) || !detectedReset(scratchPad))
			return false;
	}
	#if REQUIRESONLY12BITCONVERSION
		scratchPad[CONFIGURATION] = TEMP_12_BIT;
	#endif
	scratchPad[HIGH_ALARM_TEMP]=1;
	writeScratchPad(deviceAddress, scratchPad, false);	// don't save to eeprom, so that it reverts to 0 on reset
	// from this point on, if we read a scratchpad with a 0 value in HIGH_ALARM (detectedReset() returns true)
	// it means the device has reset or the previous write of the scratchpad above was unsuccessful.
	// Either way, initConnection() should be called again
#endif
	return true;
}

bool DallasTemperature::detectedReset(const uint8_t* scratchPad)
{
	#if REQUIRESRESETDETECTION
	bool reset = (scratchPad[HIGH_ALARM_TEMP]==0);
	return reset;
	#else
	return false;
	#endif
}

#if REQUIRESDEVICEENUM
// initialise the bus
void DallasTemperature::begin(void)
{
    DeviceAddress deviceAddress;

    _wire->reset_search();

    devices = 0; // Reset the number of devices when we enumerate wire devices

    while (_wire->search(deviceAddress))
    {
        if (validAddress(deviceAddress))
        {
#if REQUIRESPARASITEPOWERAVAILABLE
			if (!parasite && readPowerSupply(deviceAddress)) parasite = true;
#endif

            ScratchPad scratchPad;
            readScratchPad(deviceAddress, scratchPad);

#if REQUIRESONLY12BITCONVERSION
			setResolution(deviceAddress, 12);
#else
            //bitResolution = max(bitResolution, getResolution(deviceAddress));
			uint8_t newResolution = getResolution(deviceAddress);
			if(newResolution > bitResolution){	 // the max macro may call getResultion multiple times.
				bitResolution = newResolution;
			}
#endif
            devices++;
        }
    }
}

// returns the number of devices found on the bus
uint8_t DallasTemperature::getDeviceCount(void)
{
    return devices;
}
#endif // REQUIRESDEVICEENUM

// returns true if address is valid
bool DallasTemperature::validAddress(const uint8_t* deviceAddress)
{
    return (_wire->crc8(deviceAddress, 7) == deviceAddress[7]);
}

#if REQUIRESINDEXEDADDRESSING
// finds an address at a given index on the bus
// returns true if the device was found
bool DallasTemperature::getAddress(uint8_t* deviceAddress, uint8_t index)
{
    uint8_t depth = 0;

    _wire->reset_search();

    while (depth <= index && _wire->search(deviceAddress))
    {
        if (depth == index && validAddress(deviceAddress)) return true;
        depth++;
    }

    return false;
}
#endif

// attempt to determine if the device at the given address is connected to the bus
bool DallasTemperature::isConnected(const uint8_t* deviceAddress)
{
    ScratchPad scratchPad;
    return isConnected(deviceAddress, scratchPad) && !detectedReset(scratchPad);
}

// attempt to determine if the device at the given address is connected to the bus
// also allows for updating the read scratchpad.
bool DallasTemperature::isConnected(const uint8_t* deviceAddress, uint8_t* scratchPad)
{
    readScratchPad(deviceAddress, scratchPad);
	// Also check that device is not parasite powered, if this is disabled.
	// Thiss is to prevent sensors with a loose 5V line to be detected
	#if REQUIRESPARASITEPOWERAVAILABLE
		return (_wire->crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
	#else
		return (_wire->crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC] && !readPowerSupply(deviceAddress));
	#endif
}

void DallasTemperature::sendCommand(const uint8_t* deviceAddress, uint8_t command) {
    _wire->reset();
    _wire->select(deviceAddress);
    _wire->write(command);
}

// read device's scratch pad
void DallasTemperature::readScratchPad(const uint8_t* deviceAddress, uint8_t* scratchPad)
{
    // send the command
	sendCommand(deviceAddress, READSCRATCH);

    // TODO => collect all comments &  use simple loop
    // byte 0: temperature LSB
    // byte 1: temperature MSB
    // byte 2: high alarm temp
    // byte 3: low alarm temp
    // byte 4: DS18S20: store for crc
    //         DS18B20 & DS1822: configuration register
    // byte 5: internal use & crc
    // byte 6: DS18S20: COUNT_REMAIN
    //         DS18B20 & DS1822: store for crc
    // byte 7: DS18S20: COUNT_PER_C
    //         DS18B20 & DS1822: store for crc
    // byte 8: SCRATCHPAD_CRC
    //
#if 1			// saves 96 bytes using the loop
     for(int i=0; i<9; i++)
     {
       scratchPad[i] = _wire->read();
     }
#else

    // read the response

    // byte 0: temperature LSB
    scratchPad[TEMP_LSB] = _wire->read();

    // byte 1: temperature MSB
    scratchPad[TEMP_MSB] = _wire->read();

    // byte 2: high alarm temp
    scratchPad[HIGH_ALARM_TEMP] = _wire->read();

    // byte 3: low alarm temp
    scratchPad[LOW_ALARM_TEMP] = _wire->read();

    // byte 4:
    // DS18S20: store for crc
    // DS18B20 & DS1822: configuration register
    scratchPad[CONFIGURATION] = _wire->read();

    // byte 5:
    // internal use & crc
    scratchPad[INTERNAL_BYTE] = _wire->read();

    // byte 6:
    // DS18S20: COUNT_REMAIN
    // DS18B20 & DS1822: store for crc
    scratchPad[COUNT_REMAIN] = _wire->read();

    // byte 7:
    // DS18S20: COUNT_PER_C
    // DS18B20 & DS1822: store for crc
    scratchPad[COUNT_PER_C] = _wire->read();

    // byte 8:
    // SCTRACHPAD_CRC
    scratchPad[SCRATCHPAD_CRC] = _wire->read();
#endif
    _wire->reset();
}

// writes device's scratch pad
void DallasTemperature::writeScratchPad(const uint8_t* deviceAddress, const uint8_t* scratchPad, boolean copyToEeprom)
{
	sendCommand(deviceAddress, WRITESCRATCH);

	// DS18S20 does not use the configuration register
	uint8_t top = isDS18S20Model(deviceAddress) ? LOW_ALARM_TEMP : CONFIGURATION;
	for (uint8_t i=HIGH_ALARM_TEMP; i<=top; i++) {
		_wire->write(scratchPad[i]); // high alarm temp
	}
    _wire->reset();

	// save the newly written values to eeprom
	if (copyToEeprom)  {
	    _wire->select(deviceAddress); //<--this line was missing
		_wire->write(COPYSCRATCH, isParasitePowerMode());
		if (isParasitePowerMode())
			wait.millis(10); // 10ms delay
		_wire->reset();
	}

}

// reads the device's power requirements
bool DallasTemperature::readPowerSupply(const uint8_t* deviceAddress)
{
    bool ret = false;
    sendCommand(deviceAddress, READPOWERSUPPLY);
    if (_wire->read_bit() == 0) ret = true;
    _wire->reset();
    return ret;
}

#if REQUIRESWHOLEBUSOPS
// set resolution of all devices to 9, 10, 11, or 12 bits
// if new resolution is out of range, it is constrained.
void DallasTemperature::setResolution(uint8_t newResolution)
{
#if !REQUIRESONLY12BITCONVERSION
	if(newResolution>12){
		newResolution = 12;
	}
	else if(newResolution<9){
		newResolution = 9;
	}
	bitResolution = newResolution;
#endif

	//bitResolution = constrain(newResolution, 9, 12);
    DeviceAddress deviceAddress;
    for (uint8_t i=0; i<devices; i++)
    {
        getAddress(deviceAddress, i);
        setResolution(deviceAddress, newResolution);
    }
}
#endif // REQUIRESWHOLEBUSOPS

// set resolution of a device to 9, 10, 11, or 12 bits
// if new resolution is out of range, 9 bits is used.
bool DallasTemperature::setResolution(const uint8_t* deviceAddress, uint8_t newResolution)
{
    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad))
    {
        // DS18S20 has a fixed 9-bit resolution
        if (!isDS18S20Model(deviceAddress))
        {
			uint8_t resolution;
#if REQUIRESONLY12BITCONVERSION
			resolution = TEMP_12_BIT;
#else
            switch (newResolution)
            {
            case 12:
                resolution = TEMP_12_BIT;
                break;
            case 11:
                resolution = TEMP_11_BIT;
                break;
            case 10:
                resolution = TEMP_10_BIT;
                break;
            case 9:
            default:
                resolution = TEMP_9_BIT;
                break;
            }
#endif
			scratchPad[CONFIGURATION] = resolution;
            writeScratchPad(deviceAddress, scratchPad, false);
        }
        return true;  // new value set
    }
    return false;
}

#if !REQUIRESONLY12BITCONVERSION
// returns the global resolution
uint8_t DallasTemperature::getResolution()
{
    return bitResolution;
}
#endif

// returns the current resolution of the device, 9-12
// returns 0 if device not found
uint8_t DallasTemperature::getResolution(const uint8_t* deviceAddress)
{
    // this model has a fixed resolution of 9 bits but getTemp calculates
    // a full 12 bits resolution and we need 750ms convert time
    if (isDS18S20Model(deviceAddress)) return 12;

    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad))
    {
#if REQUIRESONLY12BITCONVERSION
		return 12;
#else
        switch (scratchPad[CONFIGURATION])
        {
        case TEMP_12_BIT:
            return 12;

        case TEMP_11_BIT:
            return 11;

        case TEMP_10_BIT:
            return 10;

        case TEMP_9_BIT:
            return 9;
        }
#endif
    }
    return 0;
}

#if REQUIRESWAITFORCONVERSION

#if 0  // moved to header for inlining
// sets the value of the waitForConversion flag
// TRUE : function requestTemperature() etc returns when conversion is ready
// FALSE: function requestTemperature() etc returns immediately (USE WITH CARE!!)
//        (1) programmer has to check if the needed delay has passed
//        (2) but the application can do meaningful things in that time
void DallasTemperature::setWaitForConversion(bool flag)
{
    waitForConversion = flag;
}
#endif

// gets the value of the waitForConversion flag
bool DallasTemperature::getWaitForConversion()
{
    return waitForConversion;
}


// sets the value of the checkForConversion flag
// TRUE : function requestTemperature() etc will 'listen' to an IC to determine whether a conversion is complete
// FALSE: function requestTemperature() etc will wait a set time (worst case scenario) for a conversion to complete
void DallasTemperature::setCheckForConversion(bool flag)
{
    checkForConversion = flag;
}

// gets the value of the waitForConversion flag
bool DallasTemperature::getCheckForConversion()
{
    return checkForConversion;
}

bool DallasTemperature::isConversionAvailable(const uint8_t* deviceAddress)
{
    // Check if the clock has been raised indicating the conversion is complete
    ScratchPad scratchPad;
    readScratchPad(deviceAddress, scratchPad);
    // MDMA: this is weak - 0 is a valid return code
	return scratchPad[0];
}
#endif

#if REQUIRESWHOLEBUSOPS
// sends command for all devices on the bus to perform a temperature conversion
void DallasTemperature::requestTemperatures()
{
    _wire->reset();
    _wire->skip();
    _wire->write(STARTCONVO, isParasitePowerMode());

#if REQUIRESWAITFORCONVERSION
    // ASYNC mode?
    if (!waitForConversion) return;
    blockTillConversionComplete(getResolution(), NULL);
#endif

}
#endif // REQUIRESWHOLEBUSOPS

// sends command for one device to perform a temperature by address
// returns FALSE if device is disconnected
// returns TRUE  otherwise
bool DallasTemperature::requestTemperaturesByAddress(const uint8_t* deviceAddress)
{
	_wire->reset();
    _wire->select(deviceAddress);
    _wire->write(STARTCONVO, isParasitePowerMode());

    // check device
    ScratchPad scratchPad;
    if (!isConnected(deviceAddress, scratchPad) || detectedReset(scratchPad)) {
		return false;
	}

    // ASYNC mode?
#if REQUIRESWAITFORCONVERSION
    if (!waitForConversion) return true;
    blockTillConversionComplete(getResolution(deviceAddress), deviceAddress);
#endif
    return true;
}

#if REQUIRESWAITFORCONVERSION
// returns number of milliseconds to wait till conversion is complete (based on IC datasheet)
int16_t DallasTemperature::millisToWaitForConversion(uint8_t bitResolution)
{
#if REQUIRESONLY12BITCONVERSION
	return 750;
#else
    switch (bitResolution)
    {
    case 9:
        return 94;
    case 10:
        return 188;
    case 11:
        return 375;
    default:
        return 750;
    }
#endif
}

// Continue to check if the IC has responded with a temperature
void DallasTemperature::blockTillConversionComplete(uint8_t bitResolution, const uint8_t* deviceAddress)
{
    int delms = millisToWaitForConversion(bitResolution);
    if (deviceAddress != NULL && checkForConversion && !isParasitePowerMode())
    {
        unsigned long timend = millis() + delms;
        while(!isConversionAvailable(deviceAddress) && (ticks.millis() < timend));
    }
    else
    {
        wait.millis(delms);
    }
}
#endif // REQUIRESWAITFORCONVERSION

#if REQUIRESINDEXEDADDRESSING
// sends command for one device to perform a temp conversion by index
bool DallasTemperature::requestTemperaturesByIndex(uint8_t deviceIndex)
{
    DeviceAddress deviceAddress;
    getAddress(deviceAddress, deviceIndex);
    return requestTemperaturesByAddress(deviceAddress);
}

#if REQUIRESTEMPCONVERSION
// Fetch temperature for device index
float DallasTemperature::getTempCByIndex(uint8_t deviceIndex)
{
    DeviceAddress deviceAddress;
    if (!getAddress(deviceAddress, deviceIndex))
        return DEVICE_DISCONNECTED_C;
    return getTempC((uint8_t*)deviceAddress);
}

// Fetch temperature for device index
float DallasTemperature::getTempFByIndex(uint8_t deviceIndex)
{
    DeviceAddress deviceAddress;
    if (!getAddress(deviceAddress, deviceIndex))
        return DEVICE_DISCONNECTED_F;
    return getTempF((uint8_t*)deviceAddress);
}
#endif // REQUIRESTEMPCONVERSION
#endif // REQUIRESINDEXEDADDRESSING

// reads scratchpad and returns the raw temperature (12bit)
int16_t DallasTemperature::calculateTemperature(const uint8_t* deviceAddress, uint8_t* scratchPad)
{
    int16_t rawTemperature = (((int16_t)scratchPad[TEMP_MSB]) << 8) | scratchPad[TEMP_LSB];

    /*  DS18S20
    Resolutions greater than 9 bits can be calculated using the data from
    the temperature, COUNT REMAIN and COUNT PER °C registers in the
    scratchpad. Note that the COUNT PER °C register is hard-wired to 16
    (10h). After reading the scratchpad, the TEMP_READ value is obtained
    by truncating the 0.5°C bit (bit 0) from the temperature data. The
    extended resolution temperature can then be calculated using the
    following equation:

                                    COUNT_PER_C - COUNT_REMAIN
    TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                            COUNT_PER_C

    Simplified to integer arithmetic for a 12 bits value:

    TEMPERATURE = ((raw & 0xFFFE) << 3) - 4 + 16 - COUNT_REMAIN

    See - http://myarduinotoy.blogspot.co.uk/2013/02/12bit-result-from-ds18s20.html
    */

    if (isDS18S20Model(deviceAddress))
        rawTemperature = ((rawTemperature & 0xFFFE) << 3) + 12 - scratchPad[COUNT_REMAIN];

    return rawTemperature;
}


// returns raw temperature in 1/16 degrees C or DEVICE_DISCONNECTED_RAW if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_RAW is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
int16_t DallasTemperature::getTempRaw(const uint8_t* deviceAddress)
{
    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad) && !detectedReset(scratchPad)) return calculateTemperature(deviceAddress, scratchPad);
    return DEVICE_DISCONNECTED;		// use a value that the sensor could not ordinarily measure
}

#if REQUIRESTEMPCONVERSION
// returns temperature in degrees C or DEVICE_DISCONNECTED_C if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_C is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
float DallasTemperature::getTempC(const uint8_t* deviceAddress)
{
    return rawToCelsius(getTemp(deviceAddress));
}

// returns temperature in degrees F or DEVICE_DISCONNECTED_F if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_F is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
float DallasTemperature::getTempF(const uint8_t* deviceAddress)
{
    return rawToFahrenheit(getTemp(deviceAddress));
}
#endif // REQUIRESTEMPCONVERSION

#if 0 && REQUIRESPARASITEPOWERAVAILABLE  // moved to header for inlining
// returns true if the bus requires parasite power
bool DallasTemperature::isParasitePowerMode(void)
{
    return parasite;
}
#endif // REQUIRESPARASITEPOWERAVAILABLE

#if REQUIRESALARMS

/*

ALARMS:

TH and TL Register Format

BIT 7 BIT 6 BIT 5 BIT 4 BIT 3 BIT 2 BIT 1 BIT 0
  S    2^6   2^5   2^4   2^3   2^2   2^1   2^0

Only bits 11 through 4 of the temperature register are used
in the TH and TL comparison since TH and TL are 8-bit
registers. If the measured temperature is lower than or equal
to TL or higher than or equal to TH, an alarm condition exists
and an alarm flag is set inside the DS18B20. This flag is
updated after every temperature measurement; therefore, if the
alarm condition goes away, the flag will be turned off after
the next temperature conversion.

*/

// sets the high alarm temperature for a device in degrees Celsius
// accepts a float, but the alarm resolution will ignore anything
// after a decimal point.  valid range is -55C - 125C
void DallasTemperature::setHighAlarmTemp(const uint8_t* deviceAddress, char celsius)
{
    // make sure the alarm temperature is within the device's range
    if (celsius > 125) celsius = 125;
    else if (celsius < -55) celsius = -55;

    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad))
    {
        scratchPad[HIGH_ALARM_TEMP] = (uint8_t)celsius;
        writeScratchPad(deviceAddress, scratchPad, true);
    }
}

// sets the low alarm temperature for a device in degrees Celsius
// accepts a float, but the alarm resolution will ignore anything
// after a decimal point.  valid range is -55C - 125C
void DallasTemperature::setLowAlarmTemp(const uint8_t* deviceAddress, char celsius)
{
    // make sure the alarm temperature is within the device's range
    if (celsius > 125) celsius = 125;
    else if (celsius < -55) celsius = -55;

    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad))
    {
        scratchPad[LOW_ALARM_TEMP] = (uint8_t)celsius;
        writeScratchPad(deviceAddress, scratchPad, true);
    }
}

// returns a char with the current high alarm temperature or
// DEVICE_DISCONNECTED for an address
char DallasTemperature::getHighAlarmTemp(const uint8_t* deviceAddress)
{
    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad)) return (char)scratchPad[HIGH_ALARM_TEMP];
    return DEVICE_DISCONNECTED_C;
}

// returns a char with the current low alarm temperature or
// DEVICE_DISCONNECTED for an address
char DallasTemperature::getLowAlarmTemp(const uint8_t* deviceAddress)
{
    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad)) return (char)scratchPad[LOW_ALARM_TEMP];
    return DEVICE_DISCONNECTED_C;
}

// resets internal variables used for the alarm search
void DallasTemperature::resetAlarmSearch()
{
    alarmSearchJunction = -1;
    alarmSearchExhausted = 0;
    for(uint8_t i = 0; i < 7; i++)
        alarmSearchAddress[i] = 0;
}

// This is a modified version of the OneWire::search method.
//
// Also added the OneWire search fix documented here:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238032295
//
// Perform an alarm search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// OneWire::address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use
// DallasTemperature::resetAlarmSearch() to start over.
bool DallasTemperature::alarmSearch(uint8_t* newAddr)
{
    uint8_t i;
    char lastJunction = -1;
    uint8_t done = 1;

    if (alarmSearchExhausted) return false;
    if (!_wire->reset()) return false;

    // send the alarm search command
    _wire->write(0xEC, 0);

    for(i = 0; i < 64; i++)
    {
        uint8_t a = _wire->read_bit( );
        uint8_t nota = _wire->read_bit( );
        uint8_t ibyte = i / 8;
        uint8_t ibit = 1 << (i & 7);

        // I don't think this should happen, this means nothing responded, but maybe if
        // something vanishes during the search it will come up.
        if (a && nota) return false;

        if (!a && !nota)
        {
            if (i == alarmSearchJunction)
            {
                // this is our time to decide differently, we went zero last time, go one.
                a = 1;
                alarmSearchJunction = lastJunction;
            }
            else if (i < alarmSearchJunction)
            {
                // take whatever we took last time, look in address
                if (alarmSearchAddress[ibyte] & ibit) a = 1;
                else
                {
                    // Only 0s count as pending junctions, we've already exhausted the 0 side of 1s
                    a = 0;
                    done = 0;
                    lastJunction = i;
                }
            }
            else
            {
                // we are blazing new tree, take the 0
                a = 0;
                alarmSearchJunction = i;
                done = 0;
            }
            // OneWire search fix
            // See: http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238032295
        }

        if (a) alarmSearchAddress[ibyte] |= ibit;
        else alarmSearchAddress[ibyte] &= ~ibit;

        _wire->write_bit(a);
    }

    if (done) alarmSearchExhausted = 1;
    for (i = 0; i < 8; i++) newAddr[i] = alarmSearchAddress[i];
    return true;
}

// returns true if device address has an alarm condition
// TODO: can this be done with only TEMP_MSB REGISTER (faster)
//       if ((char) scratchPad[TEMP_MSB] <= (char) scratchPad[LOW_ALARM_TEMP]) return true;
//       if ((char) scratchPad[TEMP_MSB] >= (char) scratchPad[HIGH_ALARM_TEMP]) return true;
bool DallasTemperature::hasAlarm(const uint8_t* deviceAddress)
{
    ScratchPad scratchPad;
    if (isConnected(deviceAddress, scratchPad))
    {
        float temp = calculateTemperature(deviceAddress, scratchPad);

        // check low alarm
        if ((char)temp <= (char)scratchPad[LOW_ALARM_TEMP]) return true;

        // check high alarm
        if ((char)temp >= (char)scratchPad[HIGH_ALARM_TEMP]) return true;
    }

    // no alarm
    return false;
}

// returns true if any device is reporting an alarm condition on the bus
bool DallasTemperature::hasAlarm(void)
{
    DeviceAddress deviceAddress;
    resetAlarmSearch();
    return alarmSearch(deviceAddress);
}

// runs the alarm handler for all devices returned by alarmSearch()
void DallasTemperature::processAlarms(void)
{
    resetAlarmSearch();
    DeviceAddress alarmAddr;

    while (alarmSearch(alarmAddr))
    {
        if (validAddress(alarmAddr))
            _AlarmHandler(alarmAddr);
    }
}

// sets the alarm handler
void DallasTemperature::setAlarmHandler(AlarmHandler *handler)
{
    _AlarmHandler = handler;
}

// The default alarm handler
void DallasTemperature::defaultAlarmHandler(const uint8_t* deviceAddress)
{
}

#endif


#if REQUIRESTEMPCONVERSION
// Convert float Celsius to Fahrenheit
float DallasTemperature::toFahrenheit(float celsius)
{
    return (celsius * 1.8) + 32;
}

// Convert float Fahrenheit to Celsius
float DallasTemperature::toCelsius(float fahrenheit)
{
    return (fahrenheit - 32) / 1.8;
}

// convert from raw to Celsius
float DallasTemperature::rawToCelsius(int16_t raw)
{
    if (raw <= DEVICE_DISCONNECTED_RAW)
        return DEVICE_DISCONNECTED_C;
    // C = RAW/16
    return (float)raw * 0.0625;
}

// convert from raw to Fahrenheit
float DallasTemperature::rawToFahrenheit(int16_t raw)
{
    if (raw <= DEVICE_DISCONNECTED_RAW)
        return DEVICE_DISCONNECTED_F;
    // C = RAW/16
    // F = (C*1.8)+32 = (RAW/16*1.8)+32 = (RAW*0.1125)+32
    return ((float)raw * 0.1125) + 32;
}
#endif // REQUIRESTEMPCONVERSION

#if REQUIRESNEW

// MnetCS - Allocates memory for DallasTemperature. Allows us to instance a new object
void* DallasTemperature::operator new(unsigned int size) // Implicit NSS obj size
{
    void * p; // void pointer
    p = malloc(size); // Allocate memory
    memset((DallasTemperature*)p,0,size); // Initialise memory

    //!!! CANT EXPLICITLY CALL CONSTRUCTOR - workaround by using an init() methodR - workaround by using an init() method
    return (DallasTemperature*) p; // Cast blank region to NSS pointer
}

// MnetCS 2009 -  Free the memory used by this instance
void DallasTemperature::operator delete(void* p)
{
    DallasTemperature* pNss =  (DallasTemperature*) p; // Cast to NSS pointer
    pNss->~DallasTemperature(); // Destruct the object

    free(p); // Free the memory
}

#endif
