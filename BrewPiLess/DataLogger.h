#ifndef DataLogger_H
#define DataLogger_H

#define RETRY_TIME 5
#define MAX_RETRY_NUMBER 3


class DataLogger
{
public:
    DataLogger(void):_extra(NULL),_method(NULL),_url(NULL),
    _fsname(NULL),_ftname(NULL),_bsname(NULL),_btname(NULL),_enabled(false),_period(0),_lastUpdate(0),_retry(0){}

    ~DataLogger() 
    {
    	if(_bsname) free(_bsname);
    	if(_btname) free(_btname);
    	if(_fsname) free(_fsname);
    	if(_ftname) free(_ftname);

    	if(_url) free(_url);
    	if(_method) free(_method);
    	if(_extra) free(_extra);

    }

    void loadConfig(void);
    // web interface
	void updateSetting(AsyncWebServerRequest *request);
	void getSettings(AsyncWebServerRequest *request);
	
	void loop(time_t now,void(*getTemp)(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet));

protected:
	bool processJson(char* jsonstring);
	void sendData(float beerTemp,float beerSet,float fridgeTemp, float fridgeSet);

	char* _bsname;
	char* _btname;
	char* _ftname;
	char* _fsname;
	char* _url;
	char* _method;
	char* _extra;

	bool _enabled;
	time_t _period;
	time_t _lastUpdate;
	byte _retry;
};

#endif


