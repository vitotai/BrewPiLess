#ifndef TimeKeeper_H
#define TimeKeeper_H

class TimeKeeperClass
{
public:
	TimeKeeperClass(void):_referenceEpoc(0),_referenceSystemTime(0),_ntpSynced(false){}
	void begin(char* server1,char* server2,char* server3);
	void begin(void);
	
	void updateTime(void);

	time_t getTimeSeconds(void); // get Epoch time
	time_t getLocalTimeSeconds(void);
	
	const char *getDateTimeStr(void);

	void setCurrentTime(time_t current);
	void setTimezoneOffset(int32_t offset);
	int32_t getTimezoneOffset(void);
	bool isSynchronized(void){ return _ntpSynced; }
private:
	time_t _referenceEpoc;
	time_t _referenceSystemTime;
	bool _ntpSynced;

	time_t _lastSaved;
	void saveTime(time_t t);
	time_t loadTime(void);

	time_t _queryServer(void);
};

extern TimeKeeperClass TimeKeeper;

#endif
