#ifndef GSLogger_H
#define GSLogger_H

class GSLogger
{
public:
    GSLogger(void):_scriptid(NULL),_spreadsheetid(NULL),
    				_sheetname(NULL),_passcode(NULL),_enabled(false),_period(0),_lastUpdate(0){}

    ~GSLogger() 
    {
    	if(_scriptid) free(_scriptid);
    	if(_spreadsheetid) free(_spreadsheetid);
    	if(_sheetname) free(_sheetname);
    	if(_passcode) free(_passcode);
    }

    void loadConfig(void);
    // web interface
	void updateSetting(AsyncWebServerRequest *request);
	void getSettings(AsyncWebServerRequest *request);
	
	void loop(time_t now,void(*getTemp)(float *pBeerTemp,float *pBeerSet,float *pFridgeTemp, float *pFridgeSet));

protected:
	bool processJson(char* jsonstring);
	char* _scriptid;
	char* _spreadsheetid;
	char* _sheetname;
	char* _passcode;

	bool _enabled;
	time_t _period;
	time_t _lastUpdate;
};

#endif

