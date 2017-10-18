


#include "Brewpi.h"
#include "TempSensor.h"

class WirelessTempSensor: public BasicTempSensor
{
public:
	static  WirelessTempSensor* theWirelessTempSensor;
	
	WirelessTempSensor(bool connected=false,uint32_t expiryTime=300){
        setConnected(connected);
        _expiryTime = expiryTime * 1000;

		if(! theWirelessTempSensor)theWirelessTempSensor=this;
	}
	~WirelessTempSensor(){
		theWirelessTempSensor=NULL;
    }
    
    void setExpiryTime(uint32_t period){
        _expiryTime = period  * 1000;
    }

	void setTemp(float temp){
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
		return _temperature;
	}

	void setValue(temperature newTemp) {
		_temperature = newTemp;
	}

	private:
	temperature _temperature;
    bool _connected;
    uint32_t _expiryTime;
    uint32_t _updateTime;    
};