#include "BleTiltListener.h"
#include <string>
#if SupportTiltHydrometer


#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

// NimBLEUUID seems to be revsersed.
#if 0
const NimBLEUUID UUIDRed("A495BB10-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDGreen("A495BB20-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDBlack("A495BB30-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDPurple("A495BB40-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDOrange("A495BB50-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDBlue("A495BB60-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDYellow("A495BB70-C5B1-4B44-B512-1370F02D74DE");
const NimBLEUUID UUIDPink("A495BB80-C5B1-4B44-B512-1370F02D74DE");
#endif

const uint8_t UUIDRed[]={0xA4,0x95,0xBB,0x10,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDGreen[]={0xA4,0x95,0xBB,0x20,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDBlack[]={0xA4,0x95,0xBB,0x30,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDPurple[]={0xA4,0x95,0xBB,0x40,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDOrange[]={0xA4,0x95,0xBB,0x50,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDBlue[]={0xA4,0x95,0xBB,0x60,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDYellow[]={0xA4,0x95,0xBB,0x70,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};
const uint8_t UUIDPink[]={0xA4,0x95,0xBB,0x80,0xC5,0xB1,0x4B,0x44,0xB5,0x12,0x13,0x70,0xF0,0x2D,0x74,0xDE};

#define UUID_SIZE sizeof(UUIDPink)


bool _parseTiltInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,TiltHydrometerInfo& tiltInfo){


    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    
    if(!advertisedDevice->haveManufacturerData()) return false;
    if (strManufacturerData.length() != 25) return false;

    uint8_t cManufacturerData[100];
    strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

    if (cManufacturerData[0] != 0x4C || cManufacturerData[1] != 0x00 || cManufacturerData[2] != 0x02 || cManufacturerData[3] != 0x15) return false;
    
    
    BLEBeacon oBeacon = BLEBeacon();
    oBeacon.setData(strManufacturerData);
    
    uint8_t* uuid= cManufacturerData + 4;
    
    TiltColor color;

    if( memcmp(uuid,UUIDRed,UUID_SIZE) ==0 ) color = TiltColorRed;
    else if( memcmp(uuid, UUIDGreen,UUID_SIZE) ==0  ) color = TiltColorGreen;
    else if( memcmp(uuid, UUIDBlack,UUID_SIZE) ==0  ) color = TiltColorBlack;
    else if( memcmp(uuid,UUIDPurple,UUID_SIZE) ==0  ) color = TiltColorPurple;
    else if( memcmp(uuid, UUIDOrange,UUID_SIZE) ==0  ) color = TiltColorOrange;
    else if( memcmp(uuid, UUIDBlue,UUID_SIZE) ==0  ) color = TiltColorBlue;
    else if( memcmp(uuid, UUIDYellow,UUID_SIZE) ==0  ) color = TiltColorYellow;
    else if( memcmp(uuid, UUIDPink,UUID_SIZE) ==0  ) color = TiltColorPink;
    else {
        DBG_PRINTF("unknown TILT UUID:%s\n",advertisedDevice->getAddress().toString().c_str());
        return false;
    }
    tiltInfo.color= color;
    tiltInfo.gravity =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMinor());
    tiltInfo.temperature =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMajor());
    tiltInfo.rssi = advertisedDevice->getRSSI();
    tiltInfo.macAddress=advertisedDevice->getAddress();

    DBG_PRINTF("TILT- color:%d temp:%d sg:%d, rssi:%d\n",color, tiltInfo.temperature, tiltInfo.gravity,tiltInfo.rssi); 
    return true;
}


void TiltListener::listen(TiltColor color,TiltDataHandler onData){
    _dataAvailableHandler=onData;
    _targetColor = color;
    startListen();
}

bool TiltListener::onDeviceFound(NimBLEAdvertisedDevice* device){
    if(_parseTiltInfoFromAdvertise(device,_tiltInfo)){
        if(_tiltInfo.color == _targetColor){
            if(_dataAvailableHandler) _dataAvailableHandler(&_tiltInfo);
            return true;
        }
    }
    return false;
}

TiltScanner tiltScanner;


void TiltScanner::scan(TiltDataHandler onData){
    _dataAvailableHandler=onData;
    startListen();
}

void TiltScanner::stopScan(void){
    stopListen();
}

bool TiltScanner::onDeviceFound(NimBLEAdvertisedDevice* device){
    TiltHydrometerInfo info;
    if(_parseTiltInfoFromAdvertise(device,info)){
        if(_dataAvailableHandler) _dataAvailableHandler(&info);
        return true;
    }
    return false;
}

#endif