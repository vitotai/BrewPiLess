#if SupportBTHomeSensor

#include "BleBTHomeListener.h"
#include <string>
#include "TimeKeeper.h"

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

/*
https://bthome.io/format/
*/

static const  NimBLEUUID BTHomeServiceUUID((uint16_t)0xFCD2);

typedef struct _BTHomeDataObject{
    uint8_t objectId;
    uint8_t length;
} BTHomeDataObject;

#define ID_Temperature_2p 0x02
#define ID_Temperature_1p 0x45
#define ID_Humidity_2B 0x03
#define ID_Humidity_1B 0x2E

static const BTHomeDataObject BTHomeDataObjectMap[]={
    {0x00, 1}, //"packet ID",
    {0x01, 1}, //"battery", "%",
    {0x02, 2}, // "temperature","°C",factor=0.01,
    {0x03, 2}, // "humidity", "%",factor=0.01,
    {0x04, 3}, //"pressure", "mbar", factor=0.01,
    {0x05, 3}, //"illuminance", "lx", factor=0.01,
    {0x06, 2}, //"weight", "kg", factor=0.01,
    {0x07, 2}, //"weight", "lbs", factor=0.01,
    {0x08, 2},  //"dewpoint", "°C", "signed_integer", factor=0.01,
    {0x09, 1}, //"count"
    {0x0A, 3}, // "energy", "kWh", factor=0.001,
    {0x0B, 3}, //"power","W", factor=0.01,
    {0x0C, 2}, //"voltage", "V", factor=0.001,
    {0x0D, 2}, //"pm2.5", "µg/m³"
    {0x0E, 2}, //"pm10","µg/m³"
    {0x0F, 1}, //"binary",generic boolean
    {0x10, 1}, //"switch",
    {0x11, 1}, //"opening",
    {0x12, 2}, //"co2","ppm",
    {0x13, 2}, //"tvoc", "µg/m³" 
    {0x14, 2}, //"moisture", "%", factor=0.01,
    {0x15, 1}, //"battery low",
    {0x16, 1}, //"battery charging",
    {0x17, 1}, //"carbon monoxide",
    {0x18, 1}, //"cold",
    {0x19, 1}, //"connectivity",
    {0x1A, 1}, //"door",
    {0x1B, 1}, //"garage door",
    {0x1C, 1}, //"gas detected",
    {0x1D, 1}, //"heat",
    {0x1E, 1}, //"light",
    {0x1F, 1}, //"lock",
    {0x20, 1}, //"moisture detected",
    {0x21, 1}, //"motion",
    {0x22, 1}, //"moving",
    {0x23, 1}, //"occupancy",
    {0x24, 1}, //"plug",
    {0x25, 1}, //"presence",
    {0x26, 1}, //"problem",
    {0x27, 1}, //"running",
    {0x28, 1}, //"safety",
    {0x29, 1}, //"smoke",
    {0x2A, 1}, //"sound",
    {0x2B, 1}, //"tamper",
    {0x2C, 1}, //"vibration",
    {0x2D, 1}, //"window",
    {0x2E, 1}, //"humidity", "%",
    {0x2F, 1},//"moisture", "%",
    {0x3A, 1}, //"button",
    {0x3C, 2}, //"dimmer"
    {0x3D, 2}, //"count",
    {0x3E, 4}, //"count",
    {0x3F, 2}, //"rotation", "°", "signed_integer", factor=0.1,
    {0x40, 2}, //"distance mm", "mm"
    {0x41, 2}, //"distance", "m", factor=0.1
    {0x42, 3}, //"duration", "s", factor=0.001,
    {0x43, 2}, //"current", "A", factor=0.001,
    {0x44, 2}, //"speed", "m/s", factor=0.01,
    {0x45, 2}, //"temperature", "°C", "signed_integer",factor=0.1,
    {0x46, 1}, //"uv index", factor=0.1,
    {0x47, 2}, //"volume", "L", factor=0.1,
    {0x48, 2}, //"volume mL", "mL",
    {0x49, 2}, //"volume flow rate", "m3/h", factor=0.001,
    {0x4A, 2}, //"voltage", "V", factor=0.1,
    {0x4B, 3}, //"gas", "m3", factor=0.001,
    {0x4C, 4}, //"gas", "m3", factor=0.001,
    {0x4D, 4}, //"energy", "kWh", factor=0.001,
    {0x4E, 4}, //"volume", "L", factor=0.001,
    {0x4F, 4}, //"water", "L", factor=0.001,
    {0x50, 4}, //"timestamp", "timestamp",
    {0x51, 2}, //"acceleration", "m/s²", factor=0.001,
    {0x52, 2}, //"gyroscope", "°/s", factor=0.001,
    {0x53, 255} //"text","string",
};


void BTHomeEnvironmentSensor::begin(void){
    startListen();
}

void BTHomeEnvironmentSensor::stop(void){
    stopListen();
}

bool BTHomeEnvironmentSensor::onDeviceFound(NimBLEAdvertisedDevice* device){
    //device->isAdvertisingService() doesn't work?
    const uint8_t *amac=device->getAddress().getNative();
    if(memcmp(_macAddress,amac,6) ==0 ){
        if(_getData(device))
        return true;
    }
    return false;
}

static uint8_t findTagStartFrom(int sidx,uint8_t objId){
    uint8_t idx;
    DBG_PRINTF("\t findTag:%x from %d",objId,sidx);
    for(idx =sidx; idx< sizeof(BTHomeDataObjectMap)/sizeof(BTHomeDataObject); idx++){
        if(BTHomeDataObjectMap[idx].objectId == objId){
            DBG_PRINTF("\t Found len:%d\n",BTHomeDataObjectMap[idx].length);
            return idx;
        }
    }
    DBG_PRINTF("\t Not found\n");
    return 0xFF;
}

static bool parseBTHomeSensorData(NimBLEAdvertisedDevice* device, float& temperature,uint8_t& humidity){
    // haveServieceData() doesn't work as expected
    // copy to "data" doesn't include length information?
    

    std::string strSvrData=device->getServiceData(BTHomeServiceUUID);

    DBG_PRINTF("Scanned device found:%s sv data len:%d\n", device->getAddress().toString().c_str(),strSvrData.length());

    if(strSvrData.length()<=0) return false;

    bool gotData=false;

    uint8_t rawdata[128];
    uint32_t length =  (strSvrData.length()>128)? 128: strSvrData.length(); // for safety?
    strSvrData.copy((char *)rawdata,length, 0);
    // ONly V2, non-encried device supported for now

    DBG_PRINTF("\t flag:%x", rawdata[0]);
    
    if(rawdata[0] != 0x40) return false;
    int lastId=0;
    for(int idx=1;idx< length;idx++){
        uint8_t objId= rawdata[idx];
        // find the objectID
        int oidx = findTagStartFrom(lastId,objId);

        if(oidx == 0xFF ) break;

        if(objId == ID_Temperature_2p){
                // litle endian
                temperature =((float)( rawdata[idx +1] | (rawdata[idx +2]<<8))* 0.01);
                gotData=true;
                DBG_PRINTF("\t found temp:%d\n", (int)(temperature *100));
        }else if(objId == ID_Temperature_1p){
                temperature =( (float)(rawdata[idx +1] | (rawdata[idx +2] << 8))* 0.1);
                gotData=true;
                DBG_PRINTF("\t found temp:%d\n", (int)(temperature *100));
        }else if(objId == ID_Humidity_2B){
                humidity =(uint8_t)((float)(rawdata[idx +1] | (rawdata[idx +2]<< 8))* 0.01);
                gotData=true;
                DBG_PRINTF("\t found hum:%d\n", humidity);
        }else if(objId == ID_Humidity_1B){
                humidity = rawdata[idx +1];
                gotData=true;
                DBG_PRINTF("\t found hum:%d\n", humidity);
        }    

        idx += BTHomeDataObjectMap[oidx].length;
        lastId = oidx;
    }
    return gotData;
}

bool BTHomeEnvironmentSensor::_getData(NimBLEAdvertisedDevice* device){
    // haveServieceData() doesn't work as expected
    // copy to "data" doesn't include length information?
    if(parseBTHomeSensorData(device,_temperature,_humidity)){
        _lastUpdate = millis();
        return true;
    }
    return false;
}


bool BTHomeEnvironmentSensor::isConnected(){
    if(millis() - _lastUpdate > MaximumReportPeriod) return false;
    return true;
}
    uint8_t BTHomeEnvironmentSensor::humidity(){
        if(isConnected()) return _humidity;
        return 0xFF;
    }
    float  BTHomeEnvironmentSensor::readTemperature(){
        if(isConnected()) return _temperature;
        return -1000.0;
    }


int BTHomeEnvironmentSensor::scanForDevice(BTHomeDevicdFoundFunc foundCb){

    BLEScanResults result=bleScanner.scan(5);
    int found=0;
    for(auto it = result.begin(); it != result.end(); it++){
        float temp;
        uint8_t humidity;
        
        
        if(parseBTHomeSensorData(*it,temp,humidity)){
            found++;
            foundCb(((NimBLEAdvertisedDevice*)(*it))->getAddress().getNative());
        }
    }
    DBG_PRINTF("BTHome device found:%d\n",found);
    return found;
}
#endif