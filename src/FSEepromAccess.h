/*
* Copyright 2019 Vito Tai
*
* This is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Config.h"
#include "EepromTypes.h"
#include "EepromStructs.h"
#include "EepromFormat.h"
#ifndef FSEepromAccess_H
#define FSEepromAccess_H
#if defined(ESP32)
#if UseLittleFS
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif
#endif


#define File_ControlSettings "/eeprom_control_setting"
#define File_ControlConstant "/eeprom_control_constant"
#define File_DeviceDefinition "/eeprom_device_definition"

class FSEepromAccess
{
    static bool exists(const char* file){
        return FileSystem.exists(file);
    }
    static void remove(const char* file){
        if(exists(file)) FileSystem.remove(file);
    }
    static bool readFromFile(const char* filename,char* target,size_t size){
        if(!exists(filename)) return false;

		File file = FileSystem.open(filename, "r");
		if (file) {
			file.readBytes(target, size);
			file.close();
			return true;
		}
        DBG_PRINTF("read %s error:%u\n",filename,size);
        return false;
    }
    static bool writeToFile(const char* filename,const uint8_t* source,size_t size){
		File file = FileSystem.open(filename, "w");
		if (file) {
			file.write(source, size);
			file.close();
			return true;
		} else {
             DBG_PRINTF("read %s error:%u\n",filename,size);
			return false;
		}
	}

    static DeviceConfig devices[MAX_DEVICE_SLOT];
    
    
    static void loadDeviceDefinition(){
        if(!readFromFile(File_DeviceDefinition,(char*)&devices,sizeof(devices)))
            memset((char*)&devices,0,sizeof(devices));
    }
public:
    
    static void begin(){
        loadDeviceDefinition();
    }
    
    static bool hasSettings(void){
        return exists(File_ControlSettings) && exists(File_ControlConstant);
        // if hasSettings() returns false, 
        //   ControlConstant and ControlSettings will be set to ZERO.
    }

    static void initializeSetting(){
    }

    static void zapData(){
        DBG_PRINTF("Remove EEPROM files.\n");
        remove(File_ControlSettings);
        remove(File_ControlConstant);
        remove(File_DeviceDefinition);
        memset((void*) devices,0,sizeof(devices));
    }

	static void readControlSettings(ControlSettings& target, eptr_t offset, uint16_t size) {
        if(!readFromFile(File_ControlSettings,(char*)&target,size))
            memset(&target,0,size);
	}

	static void writeControlSettings(eptr_t target, ControlSettings& source, uint16_t size) {
        writeToFile(File_ControlSettings,(const uint8_t*)&source,size);
        DBG_PRINTF("Write Control Settings:%u\n",size);
	}

	static void readControlConstants(ControlConstants& target, eptr_t offset, uint16_t size) {
        if(!readFromFile(File_ControlConstant,(char*)&target,size))
            memset(&target,0,size);
	}

	static void writeControlConstants(eptr_t target, ControlConstants& source, uint16_t size) {
        writeToFile(File_ControlConstant,(const uint8_t*) &source, size);
        DBG_PRINTF("Write Control constants:%u\n",size);
	}

	static void readDeviceDefinition(DeviceConfig& target, uint8_t deviceIndex, uint16_t size) {
        memcpy((void*)&target,(void*) &devices[deviceIndex],size);
	}

	static void writeDeviceDefinition(uint8_t deviceIndex, const DeviceConfig& source, uint16_t size) {
	    memcpy((void*) &devices[deviceIndex],(void*)&source,size);
        DBG_PRINTF("write device:%d size:%u\n",deviceIndex,size);
        saveDeviceDefinition();
	}
    static void saveDeviceDefinition(){
        writeToFile(File_DeviceDefinition, (const uint8_t*) devices,sizeof(devices));
    }
};
#endif