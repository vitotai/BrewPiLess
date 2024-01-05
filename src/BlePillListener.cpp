#include "BlePillListener.h"
#include <string>
#if SupportPillHydrometer


#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

PillListener pillListener;

/*
v1 format
0x52 0x41 0x50 0x54 0x01 mm mm mm mm mm mm tt tt gg gg gg gg xx xx yy yy zz zz bb bb
	• mm mm mm mm mm mm: MAC address used for registration
	• tt tt: temperature in Kelvin, multiplied by 128, encoded as a 16-bit unsigned big-endian integer
	• gg gg gg gg: specific gravity, encoded as an IEEE 754 single-precision floating-point number, with big-endian storage
	• xx xx, yy yy, zz zz = raw accelerometer data * 16, encoded as a 16-bit signed big-endian integer
	• bb bb = battery state-of-charge, percentage * 256, encoded as a 16-bit unsigned big-endian integer
v2 format
0x52 0x41 0x50 0x54 0x02 0x00 cc vv vv vv vv tt tt gg gg gg gg xx xx yy yy zz zz bb bb
	• cc: 0x00 if gravity velocity isn't valid, 0x01 if gravity velocity is valid
	• vv vv vv vv: gravity velocity, in points per day, encoded as an IEEE 754 single-precision floating-point number, with big-endian storage. This is only valid if the cc property is 0x01.
	• tt tt: temperature in Kelvin, multiplied by 128, encoded as a 16-bit unsigned big-endian integer
	• gg gg gg gg: specific gravity, encoded as an IEEE 754 single-precision floating-point number, with big-endian storage
	• xx xx, yy yy, zz zz = raw accelerometer data * 16, encoded as a 16-bit signed big-endian integer
	• bb bb = battery state-of-charge, percentage * 256, encoded as a 16-bit unsigned big-endian integer


*/
#define DataLength 25

const uint8_t PillIdentifier[]={0x52,0x41,0x50,0x54};
#define BigEndianU16(a0,a1) ( ((a0)<<8) | (a1))

union ArrayFloat{
    uint8_t data[4];
    float   fval;
};

bool PillListener::_parsePillInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,PillHydrometerInfo& info){


    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    
    if(!advertisedDevice->haveManufacturerData()) return false;
    size_t macDataLength =strManufacturerData.length();
    
    if (macDataLength < sizeof(PillIdentifier)) return false;
    if(macDataLength < DataLength) return false;

    uint8_t cManufacturerData[100];
    strManufacturerData.copy((char *)cManufacturerData, macDataLength > 100 ? 100:macDataLength, 0);

    if(memcmp(cManufacturerData,PillIdentifier,sizeof(PillIdentifier))!=0) return false;
    
    uint8_t *ptr = cManufacturerData+sizeof(PillIdentifier);
    if(*ptr == 0x01){
        memcpy(info.mac,ptr,6);
    }else if(*ptr == 0x02){
        NimBLEAddress bleAddress= advertisedDevice->getAddress();
         memcpy(info.mac,bleAddress.getNative(),6);
    } else{
        // unknown version
        return false;
    }
    // get mac address anyway

    ptr += 7; // skip version, mac addr or gravity velocity
    // temperature in Kelvin, multiplied by 128
    uint16_t temp = BigEndianU16(*ptr,*(ptr+1));
    // convert to C
    info.temperature = (float)temp / 128.0 - 273.15;
    ptr += 2; 

    // gravity in float, big endian
    ArrayFloat gravity;
    for(int i=0;i<4;i++){
        // ESP32 is little endian
        gravity.data[3-i] = *(ptr + i);
    }
    info.gravity = gravity.fval;
    ptr +=4;

    // keep acc value as is

    info.accX =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    info.accY =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    info.accZ =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    // battery state-of-charge, percentage * 256,
    info.battery= (float)BigEndianU16(*ptr,*(ptr+1)) /256.0;

    return true;
}


void PillListener::listen(uint8_t macAddr[6],PillDataHandler onData){
    _dataAvailableHandler=onData;
    memcpy(_macAddr,macAddr,6);
    startListen();
}


void PillListener::scan(void (*scanCompleteHandler)(std::vector<BleHydrometerDevice*>)){
    _scanCompleteHandler = scanCompleteHandler;
    requestScan();
}

BleHydrometerDevice* PillListener::identifyDevice(NimBLEAdvertisedDevice* device){
    if(_parsePillInfoFromAdvertise(device,_info)){
        if(memcmp(_info.mac,_macAddr,6)==0){
            if(_dataAvailableHandler) _dataAvailableHandler(_info);
        }
        return &_info; // dangerous. But I don't like freqent allocation and free memory
    }else{
        return NULL;
    }
}

void PillListener::scanDone(std::vector<BleHydrometerDevice*> foundDevices){
    _scanCompleteHandler(foundDevices);
}
#endif