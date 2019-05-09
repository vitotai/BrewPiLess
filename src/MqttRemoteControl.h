
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <AsyncMqttClient.h>

#include "Config.h"

#define InvalidMode 'x'
#define ModeOff 'o'
#define ModeFridgeConst 'f'
#define ModeBeerConst 'b'
#define ModeBeerProfile 'p'

#define MaxSettingLength 31

#define MaximumMqttConnectNumber 5
#define ReconnectTimer 10000
#define ReconnectTimerLong 600000

#define CapStateOn 1
#define CapStateOff 0
#define CapStateUnknown 2

#define PtcRemoteControlRange 3


class MqttRemoteControl{
protected:
    AsyncMqttClient _client;
    uint32_t _connectTime;
    uint16_t _connectAttempt;

    bool _enabled;
    bool _reconnecting;
    char _lvMode;
    char _lvBeerSet[MaxSettingLength+1];
    char _lvFridgeSet[MaxSettingLength+1];

    char* _serverAddress;
    uint16_t _serverPort;
    char* _username;
    char* _password;

    char* _modePath;
    char* _beerSetPath;
    char* _fridgeSetPath;
    
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

    void _onSettingTempChange(bool isBeerSet,char* payload, size_t len);

    void _runModeCommand(void);

#if EanbleParasiteTempControl
    void _onPtcChange(char* payload,size_t len);
#endif

#if Auto_CAP
    void _onCapChange(char* payload,size_t len);
#endif
    void _loadConfig();
public:
    MqttRemoteControl();
    bool begin();
    void reset();

    bool disconnect();

    bool loop();
};

extern MqttRemoteControl mqttRemoteControl;