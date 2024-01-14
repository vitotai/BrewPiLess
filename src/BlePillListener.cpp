#include "BlePillListener.h"
#include <string>
#include "TimeKeeper.h"
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

union FloatRaw{
    float fval;
    uint32_t ival;
};


bool _parsePillInfoFromAdvertise(NimBLEAdvertisedDevice* advertisedDevice,PillHydrometerInfo& info){


    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    
    if(!advertisedDevice->haveManufacturerData()) return false;
    size_t macDataLength =strManufacturerData.length();
    
    if (macDataLength < sizeof(PillIdentifier)) return false;
    if(macDataLength < DataLength) return false;

    uint8_t cManufacturerData[100];
    strManufacturerData.copy((char *)cManufacturerData, macDataLength > 100 ? 100:macDataLength, 0);

    // RAPT
    if(memcmp(cManufacturerData,PillIdentifier,4)!=0) return false;
    
    uint8_t *ptr = cManufacturerData+4;
    if(*ptr == 0x01 || *ptr == 0x02){
         // v1: 0x01 mm mm mm mm mm mm
         // v2: 0x02 0x00 cc vv vv vv vv
    } else{
        // unknown version
        return false;
    }
    ptr += 7; // skip version, mac addr or gravity velocity

    // temperature in Kelvin, multiplied by 128
    uint16_t temp = BigEndianU16(*ptr,*(ptr+1));
    // convert to C
    info.temperature = (float)temp / 128.0 - 273.15;
    ptr += 2; 

    // gravity in float, big endian
    uint8_t *bytes= ptr;
    FloatRaw gravity;
    //FloatRaw gravity2;
    gravity.ival = (bytes[0] << 24) | (bytes[1] << 16)  | (bytes[2] << 8)  | (bytes[3]);
    //gravity2.ival = (bytes[3] << 24) | (bytes[2] << 16)  | (bytes[1] << 8)  | (bytes[0]);
    // it's x1000
    info.gravity=gravity.fval/1000.0;
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

    // get mac address anyway
    info.macAddress=advertisedDevice->getAddress();
    info.rssi = advertisedDevice->getRSSI();


    #if 0
    DBG_PRINTF("\t RAW:");
    for(int i=0;i< strManufacturerData.length();i++){
        if(i>0) DBG_PRINT(",");
        DBG_PRINTF("0x%x",cManufacturerData[i]);
    }
    DBG_PRINTF("\n");

    DBG_PRINT(" SG:");
    DBG_PRINT(info.gravity);
//    DBG_PRINT(" SG2:");
//    DBG_PRINT(gravity2.fval);
    DBG_PRINT(" Temp:");
    DBG_PRINT(info.temperature);
    DBG_PRINTF(" Acc:%X,%X,%X ",info.accX,info.accY,info.accZ);
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

bool PillListener::onDeviceFound(NimBLEAdvertisedDevice* device){
    
    const uint8_t *amac=device->getAddress().getNative();
//    DBG_PRINTF("Target:%x:%x:%x:%x:%x:%x rcv: %x:%x:%x:%x:%x:%x\n",_macAddress[0],_macAddress[1],_macAddress[2],_macAddress[3],_macAddress[4],_macAddress[5],amac[0],amac[1],amac[2],amac[3],amac[4],amac[5]);
        //if(_mac == device->getAddress()){ 
        //  There might be tow issues here
        //  first, we don't know if the address is public  or not
        //  second, constructor NimBleAddress(uint8_t[6]) will reverse the order of the bytes
        //  however, getNative() return internal byte array.???

    if(memcmp(_macAddress,amac,6) ==0 
        && _parsePillInfoFromAdvertise(device,_info)){
            DBG_PRINTF("Pill found @%ld\n",TimeKeeper.getTimeSeconds());
            if(_dataAvailableHandler) _dataAvailableHandler(&_info);
            return true;        
    }
    return NULL;
}

PillScanner pillScanner;

void PillScanner::scan(PillDataHandler onData){
    _dataAvailableHandler=onData;
    startListen();
}

void PillScanner::stopScan(void){
    stopListen();
}

bool PillScanner::onDeviceFound(NimBLEAdvertisedDevice* device){
    PillHydrometerInfo info;
    if(_parsePillInfoFromAdvertise(device,info)){
        DBG_PRINTF("Scan device found!!!\n");
        if(_dataAvailableHandler) _dataAvailableHandler(&info);
        return true;
    }
    return false;
}
#endif