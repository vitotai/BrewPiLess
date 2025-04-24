#if SupportBTHomeSensor

#include "BleRaptThermometer.h"
#include <string>
#define ONEWIRE_TEMP_SENSOR_PRECISION 4
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

bool _parseAdvertisedData(const NimBLEAdvertisedDevice* advertisedDevice,double &temperature,int &battery);

BleRaptThermometerLisener::BleRaptThermometerLisener(uint8_t mac[6]){
    memcpy(_macAddress,mac,6); 
}

BleRaptThermometerLisener::~BleRaptThermometerLisener(){ 
    //DBG_PRINTF("*** destructor of BleRaptThermometerLisener ***\n");
    stopListen(); 
}
bool BleRaptThermometerLisener::onDeviceFound(const NimBLEAdvertisedDevice* device){

    int battery;
    double temp;

    if( memcmp(device->getAddress().getBase()->val,_macAddress,6)==0 && _parseAdvertisedData(device,temp,battery)){
        _temp = temp;
        _battery = battery;
        _rssi = device->getRSSI();
        //DBG_PRINTF("Rapt T- tempx10:%d bat:%d, rssi:%d, last seen:%ds\n",(int)(_temp*10), battery,_rssi,(millis()-_lastUpdate)/1000); 
        _lastUpdate= millis();
    }
    return false;
}

bool BleRaptThermometerLisener::isReporting(void){
    if(millis() - _lastUpdate > RAPT_THERMOMETER_TIMEOUT) return false;
    return true;
}

// BleRaptThermometer

// static function
bool BleRaptThermometer::isRaptThermemoter(NimBLEAdvertisedDevice* device){
    int battery;
    double temp;
    if(_parseAdvertisedData(device,temp,battery)){
        return true;
    }
    return false;
}

BleRaptThermometer::BleRaptThermometer(uint8_t mac[6],fixed4_4 cal):_bleListener(mac){
    //DBG_PRINTF("*** Constructor of BleRaptThermometer ***\n");
	const uint8_t shift = TEMP_FIXED_POINT_BITS-ONEWIRE_TEMP_SENSOR_PRECISION; // difference in precision between DS18B20 format and temperature adt
	_tempOffset =constrainTemp16(temperature(cal)<<shift);
}



bool BleRaptThermometer::isConnected(void){
    return _bleListener.isReporting();
}


bool BleRaptThermometer::init(void){
    _bleListener.startListen();
    return false;
}

temperature BleRaptThermometer::read(){
    if (!isConnected()){
         return TEMP_SENSOR_DISCONNECTED;
    }
    double temp =  _bleListener.latestTemperature();
    temperature ret =  _tempOffset + doubleToTemp(temp);
    return ret;
}


// parsing funcitons

//const uint8_t UuidRaptThermometerRaw[]={0x4b,0x65,0x67,0xb7,0x22,0x31,0x49,0x77,0x85,0x26,0x25,0xb7,0x4c,0x61,0x6e,0x64};
//const NimBLEUUID UuidRaptThermometer(UuidRaptThermometerRaw,16);
const NimBLEUUID UuidRaptThermometer("4b6567b7-2231-4977-8526-25b74c616e64");

bool _parseAdvertisedData(const NimBLEAdvertisedDevice* advertisedDevice,double &temp,int &battery){

    if(!advertisedDevice->haveManufacturerData()) return false;

    std::string strManufacturerData = advertisedDevice->getManufacturerData();
    
    if (strManufacturerData.length() != 25) return false;
    
    BLEBeacon oBeacon = BLEBeacon();
    oBeacon.setData(reinterpret_cast<const uint8_t*>(strManufacturerData.data()), strManufacturerData.length());

    //DBG_PRINTF("getManufacturerId:%x\n",oBeacon.getManufacturerId());
    if(oBeacon.getManufacturerId() != 0x4C) return false;

    
    if( oBeacon.getProximityUUID() !=  UuidRaptThermometer){
        DBG_PRINTF("unknown  UUID:  %s vs %s from %s\n", oBeacon.getProximityUUID().toString().c_str(), UuidRaptThermometer.toString().c_str(), 
            advertisedDevice->getAddress().toString().c_str());
        return false;
    }

    uint16_t major =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMajor());
    uint16_t minor =(uint16_t)  ENDIAN_CHANGE_U16(oBeacon.getMinor());

    temp = ((double)major / 64.0) - 273.15;
    battery = minor >> 8;

    //DBG_PRINTF("Rapt T- temp:%d bat:%d\n", major, battery); 
    return true;
}

#endif