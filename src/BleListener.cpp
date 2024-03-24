
#include "BleListener.h"
#include <string>
#if SupportBleHydrometer
#define RescanTimeout 5000
#define MaxRetryCount 10

BleListener bleListener;

void BleListener::onResult(NimBLEAdvertisedDevice* advertisedDevice) {

    //DBG_PRINTF("***OnResult:%s\n",advertisedDevice->getAddress().toString().c_str());
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


    if(_bleDeviceListeners.size()){  

        for (auto it=_bleDeviceListeners.begin(); it != _bleDeviceListeners.end(); ++it){
            BleDeviceListener* lisner= *it;
            if(lisner->onDeviceFound(advertisedDevice)){
                // clear duplicate cache will result in successive reports of the same devices.
                // _pBLEScan->clearDuplicateCache();
                // we might need to manually clear the cache after some period.
            }

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

  _pBLEScan->setMaxResults(0); // don't cache anything
//  _pBLEScan->setDuplicateFilter(false); // keep reporting, so the device/pill seen will be reported again.
}


void BleListener::_startScan(void) {
    // put your main code here, to run repeatedly:
    _scanning=true;
    _lastScanTime=millis();
    if(!_pBLEScan->start(0, NULL,false)){
        //Serial.printf("Error starting scan\n");
        _scanning=false;
        _retryCount++;
        if(_retryCount > MaxRetryCount){
            BLEDevice::deinit(true);
            delay(500);
            begin();
            _retryCount=0;
        }
    }
    _retryCount=0;
   //  Serial.printf("BLE scanning\n");
}

void BleListener::startListen(BleDeviceListener* listener){
    _bleDeviceListeners.push_back(listener);
    if(!_enabled){
        _enabled=true;
    }else{
        // enabled.
        if(_scanning){
            // cause crash: _pBLEScan->clearDuplicateCache();
        }
    }
}

void BleListener::stopListen(BleDeviceListener* listener) {
    _bleDeviceListeners.remove(listener);
    if(_bleDeviceListeners.size() ==0){
        _enabled=false;
        _pBLEScan->stop();
        _scanning=false;
    }
}

void BleListener::loop(void) {
    if(_scanning || !_enabled) return;
    // else, finish searching
    if(_enabled && (!_scanning && ((millis() - _lastScanTime) > RescanTimeout ))){
        _startScan();
    }
}

void BleListener::_clearData(void){
}

//***********************************************************************
// BleDeviceListener

void BleDeviceListener::startListen(void){
    if(_listening) return;
    _listening=true;
    bleListener.startListen(this);
}
void BleDeviceListener::stopListen(void){
    if(!_listening) return;
    _listening=false;
    bleListener.stopListen(this);
}
#endif