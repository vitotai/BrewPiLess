
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

    if(_deviceFoundCb){
        for(auto it = result.begin(); it != result.end(); ++it) {
            _deviceFoundCb(*it);
        }
    }

    if(_scanResultCb){
        _scanResultCb(result);
        _scanResultCb = NULL;
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

void BleListener::listen(ScannedDeviceHandler scannedDeviceCb){
    _deviceFoundCb = scannedDeviceCb;
    _enabled=true;
}

void BleListener::stopListen(void) {
    _enabled=false;
    _deviceFoundCb=NULL;
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

void BleListener::scanNow(ScannedResultHandler scanResultCb){
    _scanResultCb= scanResultCb;
    if(_commandScan) return;
    DBG_PRINTF("start scanning:%d\n",_scanning);
    _commandScan = true;
}

void BleListener::_clearData(void){
}

#endif