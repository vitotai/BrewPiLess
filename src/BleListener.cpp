
#include "BleListener.h"
#include <string>
#if SupportBleHydrometer

#define  BLEScanTime  10 //In seconds

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

BleListener bleListener;

void BleListener::onResult(NimBLEAdvertisedDevice* advertisedDevice) {

    if(_bleDeviceListener){
        
        if(_bleDeviceListener->identifyDevice(advertisedDevice)){
            //DBG_PRINTF("***OnResult:\n");
        }
    }

}

void BleListener::begin(void) {
  BLEDevice::init("");
  _pBLEScan = BLEDevice::getScan(); //create new scan
  _pBLEScan->setAdvertisedDeviceCallbacks(this);
  _pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  _pBLEScan->setInterval(100); // in msecs
  _pBLEScan->setWindow(99);  // less or equal setInterval value
}

void BleListener::scanComplete(NimBLEScanResults& result){
 #if 0
    DBG_PRINTF("BLE found:%d Time:%ld\n",result.getCount(),millis()-_lastScanTime);
    if(_bleDeviceListener){
        for(auto it = result.begin(); it != result.end(); ++it) {
           if(_bleDeviceListener->identifyDevice(*it)) break;
        }
    }
#endif
    if (_bleDeviceScanner) {
        _bleDeviceScanner->scanComplete(result);
        _bleDeviceScanner=NULL;
    }
    _scanning =false;
    _pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}

static void scanDone(BLEScanResults scanResults){
    bleListener.scanComplete(scanResults);
}

void BleListener::_startScan(void) {
    // put your main code here, to run repeatedly:
    _scanning=true;
    _lastScanTime=millis();
    if(!_pBLEScan->start(BLEScanTime, scanDone)){
        //Serial.printf("Error starting scan\n");
        _scanning=false;
    }
   //  Serial.printf("BLE scanning\n");
}

void BleListener::startListen(BleDeviceListener* listener){
    _bleDeviceListener=listener;
    _enabled=true;
}

void BleListener::stopListen(void) {
    _bleDeviceListener=NULL;
    _enabled=false;
}

void BleListener::loop(void) {
    if(_scanning) return;
    // else, finish searching
    if(_commandScan){
        DBG_PRINTF("Scanning\n");
        _commandScan=false;
        _startScan();
    }else if(_enabled
        && ((millis() - _lastScanTime) > _scanPeriod )){
        _startScan();
    }
}

void BleListener::scanForDevices(BleDeviceScanner* scanner){
    if(_commandScan) return;
    DBG_PRINTF("start scanning:%d\n",_scanning);
    _bleDeviceScanner=scanner;
    _commandScan = true;
}

void BleListener::_clearData(void){
}

//***********************************************************************
// BleDeviceListener

void BleDeviceListener::startListen(void){
    bleListener.startListen(this);
}
void BleDeviceListener::stopListen(void){
    bleListener.stopListen();
}
//***********************************************************************
// BleDeviceScanner
 
 BleDeviceScanner::BleDeviceScanner(void){
 }

void BleDeviceScanner::scanComplete(NimBLEScanResults& result){
    bool ignored;
    for(auto it = result.begin(); it != result.end(); ++it) {

        NimBLEAdvertisedDevice* advertisedDevice= *it;

        #if 0
        // printout data
        std::string strManufacturerData = advertisedDevice->getManufacturerData();
        NimBLEAddress address = advertisedDevice->getAddress();

        DBG_PRINTF("  Dev: %s, %d ",(advertisedDevice->haveName())? advertisedDevice->getName().c_str():"NONAME",advertisedDevice->getRSSI());
        DBG_PRINTF("\t  %s ",address.toString().c_str());   
        DBG_PRINTF(" Man len: %d \n", strManufacturerData.length());
        if(strManufacturerData.length() >0){
            DBG_PRINTF("\t\t");
            uint8_t cManufacturerData[100];
            strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);
            for(int i=0;i< strManufacturerData.length();i++){
                if(i>0) DBG_PRINT(",");
                DBG_PRINTF("0x%x",cManufacturerData[i]);
            }
            DBG_PRINTF("\n");
        }
        #endif
        // eliminated duplicated, I thought the library would've done it.
        ignored=false;
        for (int i = 0; i < _scannedDevices.size(); i++) {
            BleHydrometerDevice* d =_scannedDevices[i];
            if(advertisedDevice->getAddress() == d->macAddress){
                ignored=true;
                break;
            }
        }
        if(!ignored){
            BleHydrometerDevice* dev=checkDevice(advertisedDevice);
            if(dev){
                _scannedDevices.push_back(dev);
            }
        }
    } // for loop
    if(_scanResultHandler){
        _scanResultHandler(_scannedDevices);
        _scanResultHandler=NULL;
    }
    _clearResult();
}

void BleDeviceScanner::scan(BleHydrometerScanResultHandler resultHandler){
    _scanResultHandler = resultHandler;
    bleListener.scanForDevices(this);
}

void BleDeviceScanner::_clearResult(void){
    for (auto ptr : _scannedDevices) {
            delete ptr;
    }
    // Clearing the vector
    _scannedDevices.clear();
}
#endif