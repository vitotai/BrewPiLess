
#include "BleListener.h"
#include <string>
#if SupportBLEScanner

#define  BLEScanTime  2 //In seconds

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

BleListener bleListener;

void BleListener::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
}

void BleListener::begin(void) {
  BLEDevice::init("");
  _pBLEScan = BLEDevice::getScan(); //create new scan
  _pBLEScan->setAdvertisedDeviceCallbacks(this);
  _pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  _pBLEScan->setInterval(100);
  _pBLEScan->setWindow(99);  // less or equal setInterval value
}

void BleListener::scanComplete(NimBLEScanResults& result){
    DBG_PRINTF("BLE found:%d\n",result.getCount());
    if(_bleDeviceListener){
        for(auto it = result.begin(); it != result.end(); ++it) {
            _bleDeviceListener->identifyDevice(*it);
        }
    }

    if (_bleDeviceListener) {
        _bleDeviceScanner->scanComplete(result);
        _bleDeviceListener=NULL;
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
    for(auto it = result.begin(); it != result.end(); ++it) {
        BleHydrometerDevice* dev=getDevice(*it);
        if(dev){
            _scannedDevices.push_back(dev);
        }
    }
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