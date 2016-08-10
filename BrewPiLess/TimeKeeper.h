#ifndef TimeKeeper_H
#define TimeKeeper_H

class TimeKeeperClass
{
public:
	TimeKeeperClass(void):_referenceSeconds(0),_referenceSystemTime(0){}
	void begin(char* server1,char* server2,char* server3);
	time_t getTimeSeconds(void); // get Epoch time
	const char *getDateTimeStr(void);

private:
	time_t _referenceSeconds;
	time_t _referenceSystemTime;
};

extern TimeKeeperClass TimeKeeper;

#endif

