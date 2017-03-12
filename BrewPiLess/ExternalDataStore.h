#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
#define IsGravityValid(g) ((g) > 0)

class ExternalDataStore
{
protected:
	float _gravity;
	float _auxTemp;
	time_t _lastUpdate;
	float  _deviceVoltage;

public:
	ExternalDataStore(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),_deviceVoltage(INVALID_VOLTAGE),_lastUpdate(0){}

	void setPlato(float plato){ _gravity=1 + (plato / (258.6 -((plato/258.2)*227.1)));}
	void setGravity(float sg){ _gravity = sg; }
	float gravity(void){ return _gravity;}
	void invalidateGravity(void){  _gravity = INVALID_GRAVITY;}

	void setAuxTemperature(float temp){_auxTemp= temp;}
	float auxTemp(void){return _auxTemp; }
	void invalidateAuxTemp(void){ _auxTemp=INVALID_TEMP;}

	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(void){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	float deviceVoltage(void){return _deviceVoltage;}
	void invalidateDeviceVoltage(void) { _deviceVoltage= INVALID_VOLTAGE; }
};

extern ExternalDataStore externalDataStore;


















































