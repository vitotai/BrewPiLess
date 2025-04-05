


#include "Brewpi.h"
#include "TempSensor.h"
#include "OneWireTempSensor.h"

class WirelessTempSensor: public BasicTempSensor
{
public:
	static  WirelessTempSensor* theWirelessTempSensor;
	
	WirelessTempSensor(bool connected=false,fixed4_4 cal=0,uint32_t expiryTime=300){
        setConnected(connected);
        _expiryTime = expiryTime * 1000;
		_temperatureValid = false;
		const uint8_t shift = TEMP_FIXED_POINT_BITS-ONEWIRE_TEMP_SENSOR_PRECISION; // difference in precision between DS18B20 format and temperature adt
		//temperature i fixed7_9, calibration fixed4_4
		_calibrationOffset =constrainTemp16(temperature(cal)<<shift);
		if(! theWirelessTempSensor)theWirelessTempSensor=this;
	}
	~WirelessTempSensor(){
		theWirelessTempSensor=NULL;
    }
     
    void setExpiryTime(uint32_t period){
        _expiryTime = period  * 1000;
    }

	void setTemp(double temp){
		_temperatureValid = true;
        setValue(doubleToTemp(temp));
        _updateTime = millis();
	}

	void setConnected(bool connected)
	{
		this->_connected = connected;
	}

	bool isConnected() { return _connected; }

	bool init() {
		if(_temperatureValid){
			if((millis() - _updateTime) < _expiryTime){
				setConnected(true);
				return true;
			}
			_temperatureValid=false;
		}
		return false;
	}

	temperature read() {
		// _connected should be set only in "init()" before "read()" is called
		// so that the filters can be initialized() corrected.
		if (!isConnected()) return TEMP_SENSOR_DISCONNECTED;

        if((millis() - _updateTime) > _expiryTime){
			setConnected(false);
			_temperatureValid = false;
            return TEMP_SENSOR_DISCONNECTED;
        }else{
			setConnected(true);
		}
		temperature temp = _temperature + _calibrationOffset;
		return temp;
	}

	void setValue(temperature newTemp) {
		_temperature = newTemp;
	}

	private:
	temperature _temperature;
	temperature _calibrationOffset;
    bool _connected;
	bool _temperatureValid;
    uint32_t _expiryTime;
    uint32_t _updateTime;    
};