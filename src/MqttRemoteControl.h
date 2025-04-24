#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

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

#define DefaultLogginQoS 1

class MqttRemoteControl{
protected:
    AsyncMqttClient _client;
    uint32_t _connectTime;
    uint32_t _lastReportTime;
    uint16_t _connectAttempt;

    uint8_t _mode;
    uint8_t _reportFormat;

    uint32_t _reportPeriod;

    bool _reconnecting;
    bool _reloadConfig;
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
    
    char* _reportBasePath;
    
    #if EanbleParasiteTempControl
    char* _ptcPath;
    #endif

    #if AUTO_CAP
    char* _capPath;
    #endif

    void _onConnect(void);
    void _onDisconnect(void);
    void _onMessage(char* topic, uint8_t* payload, size_t len);
    void _onPublish(uint16_t packetId);
    void _onModeChange(char* payload,size_t len);

    void _onSettingTempChange(bool isBeerSet,char* payload, size_t len);

    void _runModeCommand(void);

#if EanbleParasiteTempControl
    void _onPtcChange(char* payload,size_t len);
#endif

#if AUTO_CAP
    void _onCapChange(char* payload,size_t len);
#endif
    void _loadConfig();

    void _reportData();
    uint16_t _publish(const char* key,float value,int precision);
    uint16_t _publish(const char* key,char value);
    uint16_t _publish(const char* key,const char* value);
    uint16_t _lastPacketId;
    bool     _publishing;
public:
    MqttRemoteControl();
    bool begin();
    void reset();

    bool disconnect();

    bool loop();
};

extern MqttRemoteControl mqttRemoteControl;