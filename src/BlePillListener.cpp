#include "BlePillListener.h"
#include <string>
#if SupportPillHydrometer


#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

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
ex:
0x52,0x41,0x50,0x54,0x2,0x0,0x0,0x0,0x0,0x0,0x0, 0x94,0x2b, 0x43,0x2e,0xed,0x7,0x40,0x37,0x0,0x95,0x2,0x2,0x64,0x0
                                                   tt   tt    gg   gg   gg  gg   xx  xx   yy  yy   zz zz    bb  bb
*/
#define DataLength 25

const uint8_t PillIdentifier[]={0x52,0x41,0x50,0x54};
#define BigEndianU16(a0,a1) ( ((a0)<<8) | (a1))


bool _parsePillInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,PillHydrometerInfo& info){


    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    
    if(!advertisedDevice->haveManufacturerData()) return false;
    size_t macDataLength =strManufacturerData.length();
    
    if (macDataLength < sizeof(PillIdentifier)) return false;
    if(macDataLength < DataLength) return false;

    uint8_t cManufacturerData[100];
    strManufacturerData.copy((char *)cManufacturerData, macDataLength > 100 ? 100:macDataLength, 0);

    if(memcmp(cManufacturerData,PillIdentifier,sizeof(PillIdentifier))!=0) return false;
    
    uint8_t *ptr = cManufacturerData+sizeof(PillIdentifier);
    if(*ptr == 0x01 || *ptr == 0x02){
    } else{
        // unknown version
        return false;
    }
    // get mac address anyway
    info.macAddress=advertisedDevice->getAddress();
    info.rssi = advertisedDevice->getRSSI();

    ptr += 7; // skip version, mac addr or gravity velocity
    // temperature in Kelvin, multiplied by 128
    uint16_t temp = BigEndianU16(*ptr,*(ptr+1));
    // convert to C
    info.temperature = (float)temp / 128.0 - 273.15;
    ptr += 2; 

    // gravity in float, big endian
    uint32_t raw_data;
    uint8_t *bytes= ptr;
   #if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    // Little-endian system, reverse the byte order
    raw_data = ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[1] << 8) | bytes[0];
    #else
    // Big-endian system, no need to reverse the byte order
    raw_data = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
    #endif
    memcpy(& info.gravity, &raw_data,sizeof(float));
    ptr +=4;

    // keep acc value as is

    info.accX =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    info.accY =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    info.accZ =BigEndianU16(*ptr,*(ptr+1));
    ptr +=2;

    // battery state-of-charge, percentage * 256,
    info.battery= ((float)BigEndianU16(*ptr,*(ptr+1))) /256.0;
    
    #if SerialDebug
    DBG_PRINTF("\t RAW:");
    for(int i=0;i< strManufacturerData.length();i++){
        if(i>0) DBG_PRINT(",");
        DBG_PRINTF("0x%x",cManufacturerData[i]);
    }
    DBG_PRINTF("\n");

    DBG_PRINT(" SG:");
    DBG_PRINT(info.gravity,4);
    DBG_PRINT(" Temp:");
    DBG_PRINT(info.temperature);
    DBG_PRINT(" Battery:");
    DBG_PRINT(info.battery);
    DBG_PRINT("%\n");

    #endif

    return true;
}


void PillListener::listen(PillDataHandler onData){
    _dataAvailableHandler=onData;
    startListen();
}

bool PillListener::identifyDevice(NimBLEAdvertisedDevice* device){
    if(_parsePillInfoFromAdvertise(device,_info)){
        if(_mac == _info.macAddress){
            if(_dataAvailableHandler) _dataAvailableHandler(&_info);
            return true;
        }
    }
    return NULL;
}

PillScanner pillScanner;

BleHydrometerDevice* PillScanner::checkDevice(NimBLEAdvertisedDevice* device){
    PillHydrometerInfo info;
    if(_parsePillInfoFromAdvertise(device,info)){
        return info.duplicate(); 
    }else{
        return NULL;
    }
}


#endif