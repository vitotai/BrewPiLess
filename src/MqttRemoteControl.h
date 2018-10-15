
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include "Config.h"

#define InvalidMode 'x'
#define ModeOff 'o'
#define ModeFridgeConst 'f'
#define ModeBeerConst 'b'
#define ModeBeerProfile 'p'

#define MaxSettingLength 31

#define ReconnectTimer 5000

#define CapStateOn 1
#define CapStateOff 0
#define CapStateUnknown 2

#define PtcRemoteControlRange 3

class MqttRemoteControl{
protected:
    WiFiClient _espClient;
    PubSubClient _client;

    uint32_t _connectTime;

    bool _enabled;
    char _lvMode;
    char _lvSetting[MaxSettingLength+1];

    char* _serverAddress;
    uint16_t _serverPort;
    char* _username;
    char* _password;

    char* _modePath;
    char* _setTempPath;
    
    #if EanbleParasiteTempControl
    char* _ptcPath;
    #endif

    #if Auto_CAP
    char* _capPath;
    #endif

    void _onConnect(void);
    void _onDisconnect(void);
    void _onMessage(char* topic, uint8_t* payload, size_t len);

    void _onModeChange(char* payload,size_t len);
    void _onSettingChange(char* payload, size_t len);

    void _runModeCommand(void);
    void _runSettingCommand(void);

#if EanbleParasiteTempControl
    void _onPtcChange(char* payload,size_t len);
#endif

#if Auto_CAP
    void _onCapChange(char* payload,size_t len);
#endif

public:
    MqttRemoteControl();
    bool begin();
    bool disconnect();

    bool loop();
};

extern MqttRemoteControl mqttRemoteControl;