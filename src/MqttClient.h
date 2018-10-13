
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include "Config.h"

#define InvalidMode 'x'

#define ModeOff 'o'
#define ModeFridgeConst 'f'
#define ModeBeerConst 'b'
#define ModeBeerProfile 'p'

#define MaxSettingLength 31

class MqttClient{
protected:
    AsyncMqttClient _mqttClient;
    char _lvMode;
    char _lvSetting[MaxSettingLength+1];

    void _onConnect(bool sessionPresent);
    void _onDisconnect(AsyncMqttClientDisconnectReason reason);
    void _onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void _onModeChange(char* payload,size_t len);
    void _onSettingChange(char* payload, size_t len);

    void _runModeCommand(void);
    void _runSettingCommand(void);
public:
    MqttClient();
    bool connect();
    bool loop();
};

extern MqttClient mqttClient;