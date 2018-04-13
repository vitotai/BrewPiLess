#include <ArduinoJson.h>
#include <string.h>
#include <IPAddress.h>
#include "espconfig.h"
#include "BPLSettings.h"

BPLSettings theSettings;

void BPLSettings::load(){}
void BPLSettings::save(){}
//***************************************************************
// system configuration
#define  KeyLcdBackLight "aoff"
#define  KeyPageTitle "title"
#define  KeyHostName "name"
#define  KeyPort     "port"
#define  KeyUsername  "user"
#define  KeyPassword  "pass"
#define  KeyProtect   "protect"
#define  KeyWifi      "wifi"
#define  KeyIpAddress "ip"
#define  KeyGateway   "gw"
#define  KeyNetmask    "mask"

extern IPAddress scanIP(const char *str);

SystemConfiguration* BPLSettings::systemConfiguration(){
    return &_data.syscfg;
}
    // decode json
static void stringNcopy(char *dst,const char *src,int n){
	if(strlen(src) < n){
		strcpy(dst,src);
	}else{
		strncpy(dst,src,n-1);
		dst[n-1]='\0';
	}
}

bool BPLSettings::dejsonSystemConfiguration(String json){
    char *buffer=strdup(json.c_str());

    const int BUFFER_SIZE = JSON_OBJECT_SIZE(15);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(buffer);
    
    SystemConfiguration *syscfg=&_data.syscfg;

    if (root.success()){
        stringNcopy(syscfg->titlelabel,root[KeyPageTitle],32);
        stringNcopy(syscfg->hostnetworkname,root[KeyHostName],32);
        stringNcopy(syscfg->username,root[KeyUsername],32);
        stringNcopy(syscfg->password,root[KeyPassword],32);

        syscfg->port = root[KeyPort];
        syscfg->passwordLcd = root[KeyProtect];
        syscfg->wifiMode = root[KeyWifi];
        syscfg->backlite = root[KeyLcdBackLight];

        syscfg->ip = (uint32_t) scanIP(root[KeyIpAddress]);
        syscfg->gw = (uint32_t) scanIP(root[KeyGateway]);
        syscfg->netmask = (uint32_t) scanIP(root[KeyNetmask]);
    }
    free(buffer);
}
    // encod json
String BPLSettings::jsonSystemConfiguration(void){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    SystemConfiguration *syscfg=&_data.syscfg;

    root[KeyPageTitle]=syscfg->titlelabel;
    root[KeyHostName]= syscfg->hostnetworkname;
    root[KeyUsername]= syscfg->username;
    root[KeyPassword]= syscfg->password;

    root[KeyPort]=   syscfg->port;
    root[KeyProtect]=    syscfg->passwordLcd;
    root[KeyWifi] =   syscfg->wifiMode;
    root[KeyLcdBackLight] =   syscfg->backlite;

    root[KeyIpAddress]= IPAddress(syscfg->ip).toString();
    root[KeyGateway]= IPAddress(syscfg->gw).toString();
    root[KeyNetmask]= IPAddress(syscfg->netmask).toString();

    String ret;
    root.printTo(ret);
    return ret;
}


//***************************************************************
// Gravity configuration

#define KeyEnableiSpindel "ispindel"
#define KeyTempCorrection "tc"
#define KeyCorrectionTemp "ctemp"
#define KeyCalculateGravity "cbpl"
#define KeyCoefficientA0 "a0"
#define KeyCoefficientA1 "a1"
#define KeyCoefficientA2 "a2"
#define KeyCoefficientA3 "a3"
#define KeyLPFBeta "lpc"
#define KeyStableGravityThreshold "stpt"
#define KeyNumberCalPoints "npt"


 bool BPLSettings::dejsonGravityConfig(char* json)
{
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(16);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(json);

		if (!root.success()){
  			DBG_PRINTF("Invalid JSON config\n");
  			return false;
		}
        GravityDeviceConfiguration *gdc = &_data.gdc;

		gdc->ispindelEnable=root[KeyEnableiSpindel];
		gdc->ispindelTempCal = root[KeyTempCorrection];

		if(gdc->ispindelTempCal){
		    gdc->ispindelCalibrationBaseTemp =
                (root.containsKey(KeyCorrectionTemp))? root[KeyCorrectionTemp]:20;
		}
		gdc->calculateGravity=root[KeyCalculateGravity];
		gdc->ispindelCoefficients[0]=root[KeyCoefficientA0];
		gdc->ispindelCoefficients[1]=root[KeyCoefficientA1];
		gdc->ispindelCoefficients[2]=root[KeyCoefficientA2];
		gdc->ispindelCoefficients[3]=root[KeyCoefficientA3];
        gdc->lpfBeta =root[KeyLPFBeta];
        gdc->stableThreshold=root[KeyStableGravityThreshold];
		gdc->numberCalPoints=root[KeyNumberCalPoints];
		// debug
		#if SerialDebug
		Serial.print("\nCoefficient:");
		for(int i=0;i<4;i++){
		    Serial.print(_ispindelCoefficients[i],10);
		    Serial.print(", ");
		}
		Serial.println("");
		#endif
	return true;
}

String BPLSettings::jsonGravityConfig(void){
		// save to file
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(16);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

        GravityDeviceConfiguration *gdc = &_data.gdc;

		root[KeyEnableiSpindel] = gdc->ispindelEnable;
		root[KeyTempCorrection] = gdc->ispindelTempCal;

		root[KeyCorrectionTemp] = gdc->ispindelCalibrationBaseTemp;
		root[KeyCalculateGravity] = gdc->calculateGravity;
		root[KeyLPFBeta] =gdc->lpfBeta;
		root[KeyStableGravityThreshold] = gdc->stableThreshold;

		root[KeyCoefficientA0]=gdc->ispindelCoefficients[0];
		root[KeyCoefficientA1]=gdc->ispindelCoefficients[1];
		root[KeyCoefficientA2]=gdc->ispindelCoefficients[2];
		root[KeyCoefficientA3]=gdc->ispindelCoefficients[3];
		root[KeyNumberCalPoints] = gdc->numberCalPoints;
	 String ret;
    root.printTo(ret);
    return ret;
}	