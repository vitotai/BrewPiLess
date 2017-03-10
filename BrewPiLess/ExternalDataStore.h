#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
#define IsGravityValid(g) ((g) > 0)

class ExternalDataStore
{
public:
	ExternalDataStore(void):gravity(INVALID_GRAVITY),auxTemp(INVALID_TEMP),deviceVoltage(INVALID_VOLTAGE),lastUpdate(0){}
	void invalidateGravity(void){  gravity = INVALID_GRAVITY;}
	void invalidateAuxTemp(void){ auxTemp=INVALID_TEMP;}
	void invalidateDeviceVoltage(void) { deviceVoltage= INVALID_VOLTAGE; }
	float gravity;
	float auxTemp;
	float deviceVoltage;
	time_t lastUpdate;
};

extern ExternalDataStore externalDataStore;






























