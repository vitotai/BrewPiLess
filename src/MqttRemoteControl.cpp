#include "MqttRemoteControl.h"
#include "BrewKeeper.h"
#include "BPLSettings.h"
#include "DataLogger.h"
#include "LogFormatter.h"
#include "TemperatureFormats.h"
#include "BrewPiProxy.h"
#include "mystrlib.h"
#include "ExternalData.h"
#if SupportPressureTransducer
#include "PressureMonitor.h"
#endif

#if EnableHumidityControlSupport
#include "HumidityControl.h"
#endif

extern BrewPiProxy brewpi;

#if SupportMqttRemoteControl

#if EanbleParasiteTempControl
#include "ParasiteTempController.h"
#endif

#if AUTO_CAP
#include "AutoCapControl.h"
#endif


MqttRemoteControl mqttRemoteControl;

MqttRemoteControl::MqttRemoteControl(){
    _lvMode = InvalidMode;
    _lvBeerSet[0] = '\0';
    _lvFridgeSet[0] = '\0';
    _mode = MqttModeOff;
    _connectTime =0;
    _lastReportTime=0;
    _publishing = false;
    
    _client.onConnect([this](bool){
        this->_onConnect();
    });
    _client.onDisconnect([this](AsyncMqttClientDisconnectReason reason){
        DBG_PRINTF("\n***MQTT:disc:%d\n",(int)reason);
        this->_onDisconnect();
    });
    _client.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
        this->_onMessage(topic,(uint8_t*)payload,len);
    });

    _client.onPublish([this](uint16_t pid){
        this->_onPublish(pid);
    });
}
#define BUFFERSIZE 512

void MqttRemoteControl::_onPublish(uint16_t pid){
    if(_publishing){
        if(pid == _lastPacketId){ // finish last packet
            if(_mode == MqttModeLogging) // logging only
                // problematic _client.disconnect();
                _reconnecting = true;
                //DBG_PRINTF("disconnect on last packet id:%d\n",pid);
        }
    }
}

uint16_t MqttRemoteControl::_publish(const char* key,float value,int precision){
    DBG_PRINTF("Publish %s\n",key);

    // somwhow need to be optimized
    char topic[256];
    int baselength=strlen(_reportBasePath);
    strncpy(topic,_reportBasePath,baselength);
    topic[baselength]='/';
    strcpy(topic + baselength +1, key);

    char data[64];
    int len= sprintFloat(data,value,precision);
    return _client.publish(topic,DefaultLogginQoS,true,data,len);
}

uint16_t MqttRemoteControl::_publish(const char* key,char value){
    DBG_PRINTF("Publish %s\n",key);

    char topic[256];
    int len=strlen(_reportBasePath);
    strncpy(topic,_reportBasePath,len);
    topic[len]='/';
    strcpy(topic + len +1, key);

    char data[4];
    data[0]=value;
    data[1]='\0';
    uint16_t packetid=_client.publish(topic,DefaultLogginQoS,true,data,1);
    if(packetid ==0){
        // error
        _client.disconnect();
    }
    return packetid;
}
uint16_t MqttRemoteControl::_publish(const char* key,const char* value){
    DBG_PRINTF("Publish %s\n",key);

    // somwhow need to be optimized
    char topic[256];
    int baselength=strlen(_reportBasePath);
    strncpy(topic,_reportBasePath,baselength);
    topic[baselength]='/';
    strcpy(topic + baselength +1, key);

    return _client.publish(topic,DefaultLogginQoS,true,value);
}

void MqttRemoteControl::_reportData(void){
    _lastReportTime = millis();

    char data[BUFFERSIZE];

    uint16_t lastID=0;
    _publishing = false; // avoid race codition

    if(_reportFormat == MqttReportJson){

        int len = nonNullJson(data,BUFFERSIZE);
        lastID=_client.publish(_reportBasePath,DefaultLogginQoS,true,data,len);
        DBG_PRINTF("Publish Json:%s\n",data);
    }else if(_reportFormat == MqttReportIndividual){
    	
        uint8_t state, mode;
	    float beerSet,fridgeSet;
	    float beerTemp,fridgeTemp,roomTemp;
        state = brewPi.getState();
        mode = brewPi.getMode();
        beerTemp = brewPi.getBeerTemp();
        beerSet = brewPi.getBeerSet();
        fridgeTemp = brewPi.getFridgeTemp();
        fridgeSet = brewPi.getFridgeSet();
        roomTemp = brewPi.getRoomTemp();
        
        lastID=_publish(KeyState, (char)('0'+state));

	    if(IS_FLOAT_TEMP_VALID(beerTemp)) lastID=_publish(KeyBeerTemp, beerTemp,1);
	    if(IS_FLOAT_TEMP_VALID(beerSet)) lastID=_publish(KeyBeerSet, beerSet,1);
	    if(IS_FLOAT_TEMP_VALID(fridgeTemp)) lastID=_publish(KeyFridgeTemp,fridgeTemp,1);
	    if(IS_FLOAT_TEMP_VALID(fridgeSet)) lastID=_publish(KeyFridgeSet, fridgeSet,1);
	    if(IS_FLOAT_TEMP_VALID(roomTemp)) lastID=_publish(KeyRoomTemp, roomTemp,1);

        lastID=_publish(KeyMode,(char)mode);
    	#if SupportPressureTransducer
	        if(PressureMonitor.isCurrentPsiValid()) lastID=_publish(KeyPressure,PressureMonitor.currentPsi(),1);
	    #endif
    	float sg=externalData.gravity();
	    if(IsGravityValid(sg)){
		    lastID=_publish(KeyGravity, sg,5);
		    lastID=_publish(KeyPlato, externalData.plato(),1);
	    }

    	#if EnableHumidityControlSupport
	    if(humidityControl.isHumidityValid())  lastID=_publish(KeyFridgeHumidity,humidityControl.humidity());
	    if(humidityControl.isRoomSensorInstalled()){
            uint8_t rh=humidityControl.roomHumidity();
            if(rh <= 100) lastID=_publish(KeyRoomHumidity,,rh);
        }
	    #endif


    	// iSpindel data
	    float vol=externalData.deviceVoltage();
	    if(IsVoltageValid(vol)) lastID=_publish(KeyVoltage, vol,2);
		
        float at=externalData.auxTemp();
		if(IS_FLOAT_TEMP_VALID(at)) lastID=_publish(KeyAuxTemp, at,1);
		    
        float tilt=externalData.tiltValue();
		if(tilt>0) lastID=_publish(KeyTilt,tilt,2);
            
        int16_t rssi=externalData.rssi();
    	if(IsRssiValid(rssi)) lastID=_publish(KeyWirelessHydrometerRssi,(float)rssi,0);

    	const char *hname=externalData.getDeviceName();
	    if(hname) lastID=_publish(KeyWirelessHydrometerName, hname);

    }
    _lastPacketId = lastID;
    _publishing=true;
     DBG_PRINTF("Mqtt last packet id:%d\n",lastID);
}

bool MqttRemoteControl::loop(){
    if(_reconnecting){
        DBG_PRINTF("MQTT:reconnecting..\n");
        if(_client.connected()){
            _client.disconnect();
        }
        // load
        if(_reloadConfig){
            _loadConfig();
            _reloadConfig = false;
        }
        // reconnect aagin in next loop, if necessary
        _reconnecting =false;
    }

    if(_mode == MqttModeOff) return false;

    if(_mode == MqttModeLogging){
        //logging only, not necessary to keep connected
        if( millis() - _lastReportTime  < _reportPeriod) return false;
    }
    // Control and/or logging mode
    if(! _client.connected()){
        // reconnect
        uint32_t now=millis();

        if(( (_connectAttempt < MaximumMqttConnectNumber) && (now - _connectTime > ReconnectTimer))
            || (now - _connectTime > ReconnectTimerLong)
            ){
            DBG_PRINTF("MQTT:reconnect..\n");
            
            _connectTime = now;
            if(WiFi.status() == WL_CONNECTED) _client.connect();
            else{
                DBG_PRINTF("MQTT:no WiFi\n");                
            }
        }
    }else{
        // connected
        if(_mode == MqttModeBothControlLoggging || _mode== MqttModeLogging){
            if( millis() - _lastReportTime  > _reportPeriod){
                _reportData();
            }
        }
    }
    return true;
}

void MqttRemoteControl::_runModeCommand(void){
    if(_lvMode == InvalidMode){
        DBG_PRINTF("MQTT:mode not set\n");
    }
    brewKeeper.setModeFromRemote(_lvMode);
}

void MqttRemoteControl::_loadConfig()
{
    MqttRemoteControlSettings *settings=theSettings.mqttRemoteControlSettings();

    _mode = settings->mode;
    
    if(_mode == MqttModeOff) return;
    _serverPort = settings->port;

    _serverAddress=settings->serverOffset? (char*)settings->_strings + settings->serverOffset:NULL;

    _username = settings->usernameOffset? (char*)settings->_strings + settings->usernameOffset:NULL;
    _password = settings->passwordOffset? (char*)settings->_strings + settings->passwordOffset:NULL;


    #if SerialDebug
        DBG_PRINTF("MQTT load config, mode:%d\n",_mode);
        if(_serverAddress) DBG_PRINTF("server:%s\n",_serverAddress);
        if(_username) DBG_PRINTF("username:%s\n",_username);
        if(_password) DBG_PRINTF("_password:%s\n",_password);
    #endif

    if(_mode == MqttModeLogging || _mode == MqttModeBothControlLoggging){

        DBG_PRINTF("_reportPeriod:%d\n",settings->reportPeriod);
        DBG_PRINTF("_reportFormat:%d\n",settings->reportFormat);
        DBG_PRINTF("reportBasePathOffset:%d\n",settings->reportBasePathOffset);
        
        _reportPeriod = settings->reportPeriod * 1000;
        _reportFormat = settings->reportFormat;
        _reportBasePath =settings->reportBasePathOffset ? (char*)settings->_strings + settings->reportBasePathOffset:NULL;

        if(_reportPeriod ==0 || _reportBasePath == NULL){
            DBG_PRINTF("Invalid period %d or path %s\n",_reportPeriod, _reportBasePath);
            _mode = (_mode == MqttModeBothControlLoggging)? MqttModeControl:MqttModeOff;
        }
    }

    if(_mode == MqttModeControl || _mode == MqttModeBothControlLoggging){

        _modePath = settings->modePathOffset? (char*)settings->_strings + settings->modePathOffset:NULL;
        _beerSetPath = settings->beerSetPathOffset? (char*)settings->_strings + settings->beerSetPathOffset:NULL;
        _fridgeSetPath = settings->fridgeSetPathOffset? (char*)settings->_strings + settings->fridgeSetPathOffset:NULL;
        
#if EanbleParasiteTempControl
        _ptcPath = settings->ptcPathOffset? (char*)settings->_strings + settings->ptcPathOffset:NULL;
#endif

#if AUTO_CAP
        _capPath = settings->capControlPathOffset? (char*)settings->_strings + settings->capControlPathOffset:NULL;
#endif


        #if SerialDebug
        if(_modePath) DBG_PRINTF("_modePath:%s\n",_modePath);
        if(_beerSetPath) DBG_PRINTF("_setTempPath:%s\n",_beerSetPath);
        if(_fridgeSetPath) DBG_PRINTF("_setTempPath:%s\n",_fridgeSetPath);

        #if EanbleParasiteTempControl

        if(_ptcPath) DBG_PRINTF("_ptcPath:%s\n",_ptcPath);
        #endif

        #if AUTO_CAP
        if(_capPath) DBG_PRINTF("_capPath:%s\n",_capPath);
        #endif        
        #endif
    }

    _client.setServer(_serverAddress, _serverPort);
    _client.setCredentials(_username,_password);

}

bool MqttRemoteControl::begin()
{
    _loadConfig();
    _connectAttempt=0;
    _reloadConfig=false;
    _reconnecting=false;
    return false;
}

void MqttRemoteControl::reset()
{
    _connectAttempt=0;
    _reconnecting = true;
    _reloadConfig=true;
}


void MqttRemoteControl::_onConnect(void){
    _connectAttempt =0;
    DBG_PRINTF("MQTT:connected..\n");

    if(_mode == MqttModeLogging) return;
    // subscribe
    if(_modePath){
        if(_client.subscribe(_modePath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_modePath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_modePath);
        }
    }

    if(_beerSetPath){
        if(_client.subscribe(_beerSetPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_beerSetPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_beerSetPath);
        }
    }

    if(_fridgeSetPath){
        if(_client.subscribe(_fridgeSetPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_fridgeSetPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_fridgeSetPath);
        }
    }


    #if EanbleParasiteTempControl
    if(_ptcPath){
        if(_client.subscribe(_ptcPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_ptcPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_ptcPath);
        }
    }
    #endif

    #if AUTO_CAP
    if(_capPath){
        if(_client.subscribe(_capPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_capPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_capPath);
        }
    }    
    #endif

}

void MqttRemoteControl::_onDisconnect(void){
     DBG_PRINTF("\nMQTT:Disconnected.\n");
    _connectAttempt ++;
}


void MqttRemoteControl::_onMessage(char* topic, uint8_t* payload, size_t len) {
    DBG_PRINTF("MQTT:rcv %s\n",topic);

    if(strcmp(topic, _modePath) ==0){
        this->_onModeChange((char*)payload,len);
    }else if(strcmp(topic, _beerSetPath) ==0){
        this->_onSettingTempChange(true,(char*)payload,len);
    }else if(strcmp(topic, _fridgeSetPath) ==0){
        this->_onSettingTempChange(false,(char*)payload,len);
    }
#if EanbleParasiteTempControl
    else if(strcmp(topic, _ptcPath) ==0){
        this->_onPtcChange((char*)payload,len);
    }
#endif 
#if AUTO_CAP
    else if(strcmp(topic, _capPath) ==0){
        this->_onCapChange((char*)payload,len);
    }
#endif
}

void MqttRemoteControl::_onModeChange(char* payload,size_t len){
    // we are going to accept mode and integer.
    char mode; // o:off, f: fridgeConst, b: beerConst, p: beerProfile
    
    const char modeChars[]={ModeOff,ModeFridgeConst,ModeBeerConst,ModeBeerProfile};
 
    #if SerialDebug
    DBG_PRINTF("MQTT:mode path value:");
    for(size_t i=0;i<len;i++)
        DBG_PRINTF("%c",payload[i]);
    DBG_PRINTF("\n");
    #endif

    if(*payload >='0' && *payload <= '3'){
        mode = modeChars[*payload - '0'];
    }else{
        // char. check if it is valid
        size_t i;
        for(i=0;i< sizeof(modeChars);i++){
            if(modeChars[i] == *payload) break;
        }
        if(i>= sizeof(modeChars)){
            // error.
            DBG_PRINTF("MQTT:error mode command:%c\n",*payload);
            return;
        }
        mode = *payload;
    }
     DBG_PRINTF("MQTT:Mode command:%c\n",mode);
    if(_lvMode != mode){
        _lvMode=mode;
        _runModeCommand();
    }
}

void MqttRemoteControl::_onSettingTempChange(bool isBeerSet,char* payload, size_t len){
    // assume it's just a simple float string.
    size_t toCopy=len;
    char *settingPtr=isBeerSet? _lvBeerSet:_lvFridgeSet;

    if(toCopy > MaxSettingLength) toCopy=MaxSettingLength;

    if(strncmp(settingPtr,payload,toCopy) !=0){
    
        memcpy(settingPtr,payload,toCopy);
        settingPtr[toCopy]='\0';

        DBG_PRINTF("MQTT:tempset :%s\n",settingPtr);
        if(isBeerSet)
            brewKeeper.setBeerSet(settingPtr);
        else
            brewKeeper.setFridgeSet(settingPtr);
        
        dataLogger.reportNow();
    }else{
        DBG_PRINTF("MQTT:Setting not changed\n");
    }
}

#if EanbleParasiteTempControl
void MqttRemoteControl::_onPtcChange(char* payload, size_t len){
    char buffer[32];
    size_t toCopy=len;
    if(toCopy > 31) toCopy=31;
    memcpy(buffer,payload,toCopy);
    buffer[toCopy]='\0';

    float temp= atof(buffer);
    
    parasiteTempController.setTemperatureRange(temp, temp + PtcRemoteControlRange);
}

#endif


#if AUTO_CAP
void MqttRemoteControl::_onCapChange(char* payload,size_t len){
    bool mode;

    if(*payload >='0' && *payload <= '9'){
        // number
        mode = *payload != '0';
        uint8_t psi =(uint8_t) atoi(payload);
        if(psi > 0)
            PressureMonitor.setTargetPsi(psi);
    }else{
        // char. check if it is valid
        if(strncmp(payload,"ON",2) ==0 || strncmp(payload,"on",2) ==0){
            mode = true;
        }else {
            mode =false;
        }
    }

    autoCapControl.capManualSet(mode);
}
#endif

#endif
