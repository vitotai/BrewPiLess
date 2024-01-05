#include "BleTiltListener.h"
#include <string>
#if SupportTiltHydrometer


#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

TiltListener tiltListener;

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


bool TiltListener::_parseTiltInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,TiltHydrometerInfo& tiltInfo){


    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    #if 0
    // printout data
    DBG_PRINTF("  Dev: %s, %d ",(advertisedDevice->haveName())? advertisedDevice->getName().c_str():"NONAME",advertisedDevice->getRSSI());
    DBG_PRINTF("\t  %s :",(advertisedDevice->haveServiceData())? advertisedDevice->getServiceDataUUID(0).toString().c_str():"NO ServiceData");   
    DBG_PRINTF(" Man len: %d ", strManufacturerData.length());

    std::string strServiceData = advertisedDevice->getServiceData();
    DBG_PRINTF(" Ser len: %d \n", strServiceData.length());
    if(strServiceData.length() >0){
        DBG_PRINTF("\t\t");
        uint8_t cServiceData[100];
        strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);
        for(int i=0;i< strServiceData.length();i++){
            if(i>0) DBG_PRINT(",");
            DBG_PRINTF("0x%x",cServiceData[i]);
        }
        DBG_PRINTF("\n");
    }

    #endif
    
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
        DBG_PRINTF("unknown TILT UUID\n");
        return false;
    }
    tiltInfo.color= color;
    tiltInfo.gravity =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMinor());
    tiltInfo.temperature =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMajor());
    tiltInfo.rssi = advertisedDevice->getRSSI();

    DBG_PRINTF("TILT- color:%d temp:%d sg:%d, rssi:%d\n",color, tiltInfo.temperature, tiltInfo.gravity,tiltInfo.rssi); 
    return true;
}


void TiltListener::listen(TiltColor color,TiltDataHandler onData){
    _dataAvailableHandler=onData;
    _targetColor = color;
    startListen();
}

void TiltListener::scan(void (*scanCompleteHandler)(std::vector<BleHydrometerDevice*>)){
    _scanCompleteHandler = scanCompleteHandler;
    requestScan();
}


BleHydrometerDevice* TiltListener::identifyDevice(NimBLEAdvertisedDevice* device){
    if(_parseTiltInfoFromAdvertise(device,_tiltInfo)){
        if(_tiltInfo.color == _targetColor){
            if(_dataAvailableHandler) _dataAvailableHandler(_tiltInfo);
        }
        return &_tiltInfo; // dangerous. But I don't like freqent allocation and free memory
    }else{
        return NULL;
    }
}

void TiltListener::scanDone(std::vector<BleHydrometerDevice*> foundDevices){
    _scanCompleteHandler(foundDevices);
}


#endif