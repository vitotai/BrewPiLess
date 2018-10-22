#include "MqttRemoteControl.h"
#include "BrewKeeper.h"
#include "BPLSettings.h"

#if EanbleParasiteTempControl
#include "ParasiteTempController.h"
#endif

#if Auto_CAP
#include "AutoCapContro.h"
#endif


MqttRemoteControl mqttRemoteControl;

MqttRemoteControl::MqttRemoteControl(){
    _lvMode = InvalidMode;
    _lvSetting[0] = '\0';
    _enabled =false;
    _connectTime =0;

    _client.setClient(_espClient);

}

bool MqttRemoteControl::loop(){
    if(! _enabled) return false;

    if(_client.connected()){
        _client.loop();
    }else{
        // reconnect
        uint32_t now=millis();
        if(now - _connectTime > ReconnectTimer){
            _connectTime = now;
            if(_client.connect("BrewPiLess-12",_username,_password)){
                this->_onConnect();
            }else{
                DBG_PRINTF("MQTT:connect failed\n");
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

void MqttRemoteControl::_runSettingCommand(void){
    if(_lvSetting[0] !='\0'){
        if(_lvMode == ModeBeerConst){
            DBG_PRINTF("MQTT:set beerSet:%s\n",_lvSetting);
            brewKeeper.setBeerSet(_lvSetting);
        }else if(_lvMode == ModeFridgeConst){
             DBG_PRINTF("MQTT:set fridgeSet:%s\n",_lvSetting);
            brewKeeper.setFridgeSet(_lvSetting);
        }else{
            DBG_PRINTF("MQTT: invalid mode to set:%c\n",_lvMode);
        }
    }
}

bool MqttRemoteControl::begin()
{
    MqttRemoteControlSettings *settings=theSettings.mqttRemoteControlSettings();

    _enabled = settings->enabled;
    if(_enabled){
        _serverAddress=settings->serverOffset? (char*)settings->_strings + settings->serverOffset:NULL;
        _serverPort = settings->port;

        _username = settings->usernameOffset? (char*)settings->_strings + settings->usernameOffset:NULL;
        _password = settings->passwordOffset? (char*)settings->_strings + settings->passwordOffset:NULL;

        _modePath = settings->modePathOffset? (char*)settings->_strings + settings->modePathOffset:NULL;
        _setTempPath = settings->settingTempPathOffset? (char*)settings->_strings + settings->settingTempPathOffset:NULL;
#if EanbleParasiteTempControl
        _ptcPath = settings->ptcPathOffset? (char*)settings->_strings + settings->ptcPathOffset:NULL;
#endif

#if Auto_CAP
        _capPath = settings->capControlPathOffset? (char*)settings->_strings + settings->capControlPathOffset:NULL;
#endif

        _client.setServer(_serverAddress, _serverPort);
        _client.setCallback([this](char* topic, uint8_t* payload, unsigned int len){
            this->_onMessage(topic,payload,len);
        });
    }

    return false;

}

void MqttRemoteControl::_onConnect(void){
    // subscribe
    if(_modePath){
        if(_client.subscribe(_modePath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_modePath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_modePath);
        }
    }

    if(_setTempPath){
        if(_client.subscribe(_setTempPath, 1)){
            DBG_PRINTF("MQTT:Subscribing %s\n",_setTempPath);
        }else{
            DBG_PRINTF("MQTT:Subscribing %s FAILED\n",_setTempPath);
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

    #if Auto_CAP
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

    if (WiFi.isConnected()) {
     //   mqttReconnectTimer.once(2, connectToMqtt);
    }
}


void MqttRemoteControl::_onMessage(char* topic, uint8_t* payload, size_t len) {
    DBG_PRINTF("MQTT:rcv %s\n",topic);

    if(strcmp(topic, _modePath) ==0){
        this->_onModeChange((char*)payload,len);
    }else if(strcmp(topic, _setTempPath) ==0){
        this->_onSettingChange((char*)payload,len);
    }
#if EanbleParasiteTempControl
    else if(strcmp(topic, _ptcPath) ==0){
        this->_onPtcChange((char*)payload,len);
    }
#endif 
#if Auto_CAP
    else if(strcmp(topic, _capPath) ==0){
        this->_onCapChange((char*)payload,len);
    }
#endif
}

void MqttRemoteControl::_onModeChange(char* payload,size_t len){
    // we are going to accept mode and integer.
    char mode; // o:off, f: fridgeConst, b: beerConst, p: beerProfile
    
    const char modeChars[]={ModeOff,ModeFridgeConst,ModeBeerConst,ModeBeerProfile};

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

void MqttRemoteControl::_onSettingChange(char* payload, size_t len){
    // assume it's just a simple float string.
    size_t toCopy=len;

    if(toCopy > MaxSettingLength) toCopy=MaxSettingLength;

    if(strncmp(_lvSetting,payload,toCopy) !=0){
    
        memcpy(_lvSetting,payload,toCopy);
        _lvSetting[toCopy]='\0';

        DBG_PRINTF("MQTT: setting:%s\n",_lvSetting);    
        _runSettingCommand();
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


#if Auto_CAP
void MqttRemoteControl::_onCapChange(char* payload,size_t len){
    bool mode;

    if(*payload >='0' && *payload <= '1'){
        mode = *payload != '0';
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
