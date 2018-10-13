#include "MqttClient.h"
#include "BrewKeeper.h"
#include "TimeKeeper.h"

#define _serverAddress "things.ubidots.com"
#define _serverPort 1883
#define _username "uR1H1tTRu17w3F2SllTtYqrRQKbiae"
#define _password "uR1H1tTRu17w3F2SllTtYqrRQKbiae"

#define _modePath "/v1.6/devices/bplcommander/mode/lv"
#define _setTempPath "/v1.6/devices/bplcommander/settemp/lv"


MqttClient mqttClient;

MqttClient::MqttClient(){
    _lvMode = InvalidMode;
    _lvSetting[0] = '\0';
}

void MqttClient::_runModeCommand(void){
    if(_lvMode == InvalidMode){
        DBG_PRINTF("MQTT:mode not set\n");
    }
    if(_lvMode == ModeBeerProfile){
      brewKeeper.setScheduleStartDate(TimeKeeper.getTimeSeconds());
    }
    brewKeeper.setMode(_lvMode);
}

void MqttClient::_runSettingCommand(void){
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

bool MqttClient::connect()
{

    _mqttClient.onConnect([this](bool sessionPresent){
        this->_onConnect(sessionPresent);
    });

    _mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason){
        this->_onDisconnect(reason);
    });
    
    _mqttClient.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
        this->_onMessage(topic,payload,properties,len,index,total);
    });

    _mqttClient.setServer(_serverAddress, _serverPort);
    _mqttClient.setCredentials(_username,_password);

    DBG_PRINTF("MQTT Connecting ...\n");
    _mqttClient.connect();

    //_mqttClient.onSubscribe(onMqttSubscribe);
    //_mqttClient.onUnsubscribe(onMqttUnsubscribe);
    //_mqttClient.onPublish(onMqttPublish);

}

void MqttClient::_onConnect(bool sessionPresent){
    // subscribe
    uint16_t packetIdSub = _mqttClient.subscribe(_modePath, 2);
    DBG_PRINTF("MQTT:Subscribing at QoS 2, packetId:%d\n ",packetIdSub);

    uint16_t packetIdSub2 = _mqttClient.subscribe(_setTempPath, 2);
    DBG_PRINTF("MQTT:Subscribing at QoS 2, packetId:%d\n ",packetIdSub2);

}

void MqttClient::_onDisconnect(AsyncMqttClientDisconnectReason reason){
     DBG_PRINTF("\nMQTT:Disconnected.\n");

    if (WiFi.isConnected()) {
     //   mqttReconnectTimer.once(2, connectToMqtt);
    }
}


void MqttClient::_onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  DBG_PRINTF("MQTT:rcv %s\n",topic);
  if(strcmp(topic, _modePath) ==0){
      this->_onModeChange(payload,len);
  }else if(strcmp(topic, _setTempPath) ==0){
      this->_onSettingChange(payload,len);
  }
}

void MqttClient::_onModeChange(char* payload,size_t len){
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

void MqttClient::_onSettingChange(char* payload, size_t len){
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
