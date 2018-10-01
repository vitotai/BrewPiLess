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

#include "data_testcmd_htm.h"
const char file_testcmd_htm [] PROGMEM="/testcmd.htm";

#if FrontEnd == TomsFrontEnd

#include "data_index_htm.h"
const char file_index_htm [] PROGMEM="/index.htm";

#include "data_dygraph_js.h"
const char file_dygraph_js [] PROGMEM="/dygraph-combined.js";

#include "data_control_htm.h"
const char file_control_htm [] PROGMEM="/control.htm";

#include "data_setup_htm.h"
const char file_setup_htm [] PROGMEM="/setup.htm";

#include "data_log_htm.h"
const char file_logconfig [] PROGMEM="/logging.htm";

#include "data_gdc_htm.h"
const char file_gravitydevice [] PROGMEM="/gravity.htm";

#include "data_config_htm.h"
const char file_config [] PROGMEM="/config.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_index_htm,data_index_htm_gz,sizeof(data_index_htm_gz),true},
{file_dygraph_js,dygraph_combined_js_gz,sizeof(dygraph_combined_js_gz),true},
{file_control_htm,control_htm_gz,sizeof(control_htm_gz),true},
{file_setup_htm,setup_htm_gz,sizeof(setup_htm_gz),true},
{file_logconfig,logging_htm_gz,sizeof(logging_htm_gz),true},
{file_gravitydevice,gravity_htm_gz,sizeof(gravity_htm_gz),true},
{file_config,config_htm_gz,sizeof(config_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false}
};

#else
// classic front end
#include "data_bwf_js.h"
const char file_bwf_js [] PROGMEM="/bwf.js";

#include "data_c_index_htm.h"
const char file_index_htm [] PROGMEM="/index.htm";

#include "data_c_setup_htm.h"
const char file_setup_htm [] PROGMEM="/setup.htm";

#include "data_c_log_htm.h"
const char file_logconfig [] PROGMEM="/log.htm";

#include "data_c_gdc_htm.h"
const char file_gravitydevice [] PROGMEM="/gdc.htm";

#include "data_c_config_htm.h"
const char file_config [] PROGMEM="/config.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_min_js_gz,sizeof(data_bwf_min_js_gz),true},
{file_index_htm,data_c_index_htm_gz,sizeof(data_c_index_htm_gz),true},
{file_setup_htm,data_c_setup_htm_gz,sizeof(data_c_setup_htm_gz),true},
{file_logconfig,data_c_log_htm_gz,sizeof(data_c_log_htm_gz),true},
{file_gravitydevice,data_c_gdc_htm_gz,sizeof(data_c_gdc_htm_gz),true},
{file_config,data_c_config_htm_gz,sizeof(data_c_config_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false}
};

#endif

const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size)
{
	for(int i=0;i<(int)(sizeof(fileMaps)/sizeof(EmbeddedFileMapEntry));i++)
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