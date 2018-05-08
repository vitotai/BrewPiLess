


#include "Brewpi.h"
#include "TempSensor.h"

class WirelessTempSensor: public BasicTempSensor
{
public:
	static  WirelessTempSensor* theWirelessTempSensor;
	
	WirelessTempSensor(bool connected=false,fixed4_4 cal=0,uint32_t expiryTime=300){
        setConnected(connected);
        _expiryTime = expiryTime * 1000;
		calibrationOffset = cal;

		if(! theWirelessTempSensor)theWirelessTempSensor=this;
	}
	~WirelessTempSensor(){
		theWirelessTempSensor=NULL;
    }
    
    void setExpiryTime(uint32_t period){
        _expiryTime = period  * 1000;
    }

	void setTemp(double temp){
		this->_connected = true;
        setValue(doubleToTemp(temp));
        _updateTime = millis();
	}

	void setConnected(bool connected)
	{
		this->_connected = connected;
	}

	bool isConnected() { return _connected; }

	bool init() {
		return read()!=TEMP_SENSOR_DISCONNECTED;
	}

	temperature read() {
		if (!isConnected())
            return TEMP_SENSOR_DISCONNECTED;
        if((millis() - _updateTime) > _expiryTime){
			this->_connected = false;
            return TEMP_SENSOR_DISCONNECTED;
        }
		const uint8_t shift = 5; // difference in precision between DS18B20 format and temperature adt
		temperature temp = constrainTemp(temp+calibrationOffset+(C_OFFSET>>shift), ((int) MIN_TEMP)>>shift, ((int) MAX_TEMP)>>shift)<<shift;

		return temp;
	}

	void setValue(temperature newTemp) {
		_temperature = newTemp;
	}

	private:
	temperature _temperature;
	fixed4_4 calibrationOffset;
    bool _connected;
    uint32_t _expiryTime;
    uint32_t _updateTime;    
};