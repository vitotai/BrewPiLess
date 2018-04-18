#include <Arduino.h>
#include <pgmspace.h>
#include "Config.h"

#ifndef NULL
#define NULL 0
#endif


#if NoEmbeddedFile == true

const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size){
	return NULL;
}

#else

typedef struct _EmbeddedFileMapEntry{
	const char *filename;
	const uint8_t *content;
    unsigned int size;
	bool  gzipped;
} EmbeddedFileMapEntry;

#include "data_bwf_js.h"
const char file_bwf_js [] PROGMEM="/bwf.js";

#include "data_index_htm.h"
const char file_index_htm [] PROGMEM="/index.htm";

#include "data_setup_htm.h"
const char file_setup_htm [] PROGMEM="/setup.htm";

#include "data_testcmd_htm.h"
const char file_testcmd_htm [] PROGMEM="/testcmd.htm";

#include "data_nindex_htm.h"
const char file_lcd [] PROGMEM="/lcd";

#include "data_log_htm.h"
const char file_logconfig [] PROGMEM="/log.htm";

#include "data_gdc_htm.h"
const char file_gravitydevice [] PROGMEM="/gdc.htm";

#include "data_config_htm.h"
const char file_config [] PROGMEM="/config.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_min_js_gz,sizeof(data_bwf_min_js_gz),true},
{file_index_htm,data_nindex_htm_gz,sizeof(data_nindex_htm_gz),true},
{file_lcd,data_lcd_min_htm_gz,sizeof(data_lcd_min_htm_gz),true},
{file_setup_htm,data_setup_min_htm_gz,sizeof(data_setup_min_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false},
{file_logconfig,log_min_htm_gz,sizeof(log_min_htm_gz),true},
{file_gravitydevice,gdc_htm_gz,sizeof(gdc_htm_gz),true},
{file_config,config_htm_gz,sizeof(config_htm_gz),true}
};

const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size)
{
	for(int i=0;i<sizeof(fileMaps)/sizeof(EmbeddedFileMapEntry);i++)
	{
		if(strcmp_P(filename,fileMaps[i].filename) ==0){
		    gzip = fileMaps[i].gzipped;
		    size = fileMaps[i].size;
			return fileMaps[i].content;
		}
	}
	return NULL;
}
#endif