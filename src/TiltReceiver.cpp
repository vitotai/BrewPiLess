#if SupportTiltHydrometer
#include "TiltReceiver.h"


#define  BLEScanTime  5 //In seconds

TiltReceiver tiltReceiver;

void TiltReceiver::onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
}

void TiltReceiver::begin(void) {
  BLEDevice::init("");
  _pBLEScan = BLEDevice::getScan(); //create new scan
  _pBLEScan->setAdvertisedDeviceCallbacks(this);
  _pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  _pBLEScan->setInterval(100);
  _pBLEScan->setWindow(99);  // less or equal setInterval value
}

void TiltReceiver::scanComplete(void){
     _scanning =false;
}

static void scanDone(BLEScanResults scanResults){
    tiltReceiver.scanComplete();
}

void TiltReceiver::_scan(void) {
    // put your main code here, to run repeatedly:
    _scanning=true;
    if(!_pBLEScan->start(BLEScanTime, scanDone)){
        Serial.printf("Error starting scan\n");
    }
}

void TiltReceiver::listen(void) {
    _listening=true;
}

void TiltReceiver::stop(void) {
    _listening=false;
}

void TiltReceiver::loop(void) {
    if(_scanning) return;
    // else, finish searching
    if(_scanCompleteHandler){
        _scanCompleteHandler(_tilts);
        _scanCompleteHandler=NULL;
    }
    _pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    if(_listening) _scan();
}

void TiltReceiver::scan(void (*scanCompleteHandler)(TiltHydrometer*)){
    _scanCompleteHandler = scanCompleteHandler;
    _scan();
}

void TiltReceiver::_clearData(void){
    for(int i=0;i<MaxTiltNumber;i++){
        _tilts[i].valid=false;
    }
}

TiltHydrometer* TiltReceiver::getTiltBy(TiltColor color){
    return &_tilts[color];
}

#endif