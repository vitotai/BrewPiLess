
#include "BleListener.h"
#include <string>
#if SupportBleHydrometer
#define RescanTimeout 5000
#define MaxRetryCount 10

BleScanner bleScanner;


void BleScanner::onResult(NimBLEAdvertisedDevice* advertisedDevice) {

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
#if 0
    if(advertisedDevice->getServiceDataCount()>0){
        NimBLEAddress address = advertisedDevice->getAddress();

    // service data
        for(int si=0;si<advertisedDevice->getServiceDataCount();si++){
        NimBLEUUID uuid =advertisedDevice->getServiceDataUUID(si);
        std::string strSvrData=advertisedDevice->getServiceData(uuid);
        if(strSvrData.length()>0){

            std::string devName= advertisedDevice->getName();
            DBG_PRINTF("  Dev: %s, %d ",devName.empty()? devName.c_str():"unknown",advertisedDevice->getRSSI());
            DBG_PRINTF("\t  %s ",address.toString().c_str());   
            DBG_PRINTF(" UUID:%s ",uuid.toString().c_str());
            for(int si=0;si<advertisedDevice->getServiceDataCount();si++){
                NimBLEUUID suuid=advertisedDevice->getServiceDataUUID(si);
                DBG_PRINTF("\n\t %d: UUID:%s :\n",si,suuid.toString().c_str());
            }
            DBG_PRINTF("\t\t");
            uint8_t rawdata[100];
            strSvrData.copy((char *)rawdata, strSvrData.length(), 0);
            for(int i=0;i< strSvrData.length();i++){
                if(i>0) DBG_PRINT(",");
                DBG_PRINTF("0x%x",rawdata[i]);
            }
            DBG_PRINTF("\n");
        }
    }
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

void BleScanner::begin(void) {
  BLEDevice::init("");
  _pBLEScan = BLEDevice::getScan(); //create new scan
  _setupAsyncScan();
//  _pBLEScan->setDuplicateFilter(false); // keep reporting, so the device/pill seen will be reported again.
}

void BleScanner::_setupAsyncScan(void){
  _pBLEScan->setAdvertisedDeviceCallbacks(this);
  _pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  _pBLEScan->setInterval(100); // in msecs
  _pBLEScan->setWindow(99);  // less or equal setInterval value

  _pBLEScan->setMaxResults(0); // don't cache anything

}

BLEScanResults BleScanner::scan(uint32_t scanTime){
    if(_pBLEScan->isScanning()){
        // currently running scanning
        _pBLEScan->stop();
    }    
    _pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    _pBLEScan->setMaxResults(32); 

    BLEScanResults foundDevices = _pBLEScan->start(scanTime, false);
    Serial.print("Devices found: ");
    Serial.println(foundDevices.getCount());
    Serial.println("Scan done!");
   return foundDevices;
}

void BleScanner::clearScanData(void){
}

void BleScanner::_startScan(void) {
    // put your main code here, to run repeatedly:
    _pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
//    _scanning=true;
    _pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
    _pBLEScan->setMaxResults(0); // not cached.
    _lastScanTime=millis();
    if(!_pBLEScan->start(0, NULL,false)){
        //Serial.printf("Error starting scan\n");
//        _scanning=false;
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

void BleScanner::startListen(BleDeviceListener* listener){
    _bleDeviceListeners.push_back(listener);
    if(!_enabled){
        _enabled=true;
    }else{
    }
}

void BleScanner::stopListen(BleDeviceListener* listener) {
    _bleDeviceListeners.remove(listener);
    if(_bleDeviceListeners.size() ==0){
        _enabled=false;
        _pBLEScan->stop();
        //_scanning=false;
    }
}

void BleScanner::loop(void) {
    if(!_enabled || _pBLEScan->isScanning() ) return;
    // else, finish searching
    if(_enabled && (!_pBLEScan->isScanning() && ((millis() - _lastScanTime) > RescanTimeout ))){
        _startScan();
    }
}

//***********************************************************************
// BleDeviceListener

void BleDeviceListener::startListen(void){
    if(_listening) return;
    _listening=true;
    bleScanner.startListen(this);
}
void BleDeviceListener::stopListen(void){
    if(!_listening) return;
    _listening=false;
    bleScanner.stopListen(this);
}
#endif