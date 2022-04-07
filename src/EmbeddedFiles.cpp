#include <Arduino.h>
#include <pgmspace.h>
#include "Config.h"

#ifndef NULL
#define NULL 0
#endif
#define Language english
#define STRINGIFY(str)  #str
#define PASTER(lo,file)   STRINGIFY(lo ## file )
#define EVALUATOR(l,x)  PASTER(l,x)

#define STRINGIFY(str)  #str
#define PASTER(lo,file)   STRINGIFY(lo ## file )
#define EVALUATOR(l,x)  PASTER(l,x)

#define ClassicIndexHtmFile EVALUATOR(WebPageLanguage,_c_index_htm.h)
#define ClassicSetupHtmFile EVALUATOR(WebPageLanguage,_c_setup_htm.h)
#define ClassicLogHtmFile EVALUATOR(WebPageLanguage,_c_log_htm.h)
#define ClassicGravityHtmFile EVALUATOR(WebPageLanguage,_c_gdc_htm.h)
#define ClassicConfigHtmFile EVALUATOR(WebPageLanguage,_c_config_htm.h)


#define IndexHtmFile EVALUATOR(WebPageLanguage,_index_htm.h)
#define ControlHtmFile EVALUATOR(WebPageLanguage,_control_htm.h)
#define SetupHtmFile EVALUATOR(WebPageLanguage,_setup_htm.h)
#define LogHtmFile EVALUATOR(WebPageLanguage,_log_htm.h)

#if SupportTiltHydrometer
#define GravityHtmFile EVALUATOR(WebPageLanguage,_gdc_e32_htm.h)
#else
#define GravityHtmFile EVALUATOR(WebPageLanguage,_gdc_htm.h)
#endif

#define ConfigHtmFile EVALUATOR(WebPageLanguage,_config_htm.h)
#define PressureHtmFile EVALUATOR(WebPageLanguage,_pressure_htm.h)

#define BackupHtmFile EVALUATOR(WebPageLanguage,_backup_htm.h)


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

#include "bwf_js.h"
const char file_bwf_js [] PROGMEM="/bwf.js";

#include "testcmd_htm.h"
const char file_testcmd_htm [] PROGMEM="/testcmd.htm";

#include "dygraph_js.h"


const char file_lcd_htm [] PROGMEM="/lcd";

#if FrontEnd == TomsFrontEnd
#include "lcd_htm.h"

#include IndexHtmFile
#include ControlHtmFile
#include SetupHtmFile
#include LogHtmFile
#include GravityHtmFile
#include ConfigHtmFile
#include PressureHtmFile
#include BackupHtmFile

const char file_index_htm [] PROGMEM="/index.htm";
const char file_dygraph_js [] PROGMEM="/dygraph.min.js";
const char file_control_htm [] PROGMEM="/control.htm";
const char file_setup_htm [] PROGMEM="/setup.htm";
const char file_logconfig [] PROGMEM="/logging.htm";
const char file_gravitydevice [] PROGMEM="/gravity.htm";
const char file_config [] PROGMEM="/config.htm";
const char file_pressure [] PROGMEM="/pressure.htm";
const char file_backup [] PROGMEM="/backup.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_min_js_gz,sizeof(data_bwf_min_js_gz),true},
{file_index_htm,data_index_htm_gz,sizeof(data_index_htm_gz),true},
{file_dygraph_js,dygraph_min_js_gz,sizeof(dygraph_min_js_gz),true},
{file_control_htm,control_htm_gz,sizeof(control_htm_gz),true},
{file_setup_htm,setup_htm_gz,sizeof(setup_htm_gz),true},
{file_logconfig,logging_htm_gz,sizeof(logging_htm_gz),true},
{file_gravitydevice,gravity_htm_gz,sizeof(gravity_htm_gz),true},
{file_config,config_htm_gz,sizeof(config_htm_gz),true},
{file_pressure,pressure_htm_gz,sizeof(pressure_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false},
{file_lcd_htm,lcd_htm_gz,sizeof(lcd_htm_gz),true},
{file_backup,backup_htm_gz,sizeof(backup_htm_gz),true}
};

#else
#error "classic frontend is deprecated!"
#include "data/c_lcd_htm.h"

#include ClassicIndexHtmFile
#include ClassicSetupHtmFile
#include ClassicLogHtmFile
#include ClassicGravityHtmFile
#include ClassicConfigHtmFile

const char file_index_htm [] PROGMEM="/index.htm";
const char file_setup_htm [] PROGMEM="/setup.htm";
const char file_logconfig [] PROGMEM="/log.htm";
const char file_gravitydevice [] PROGMEM="/gdc.htm";
const char file_config [] PROGMEM="/config.htm";


EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_min_js_gz,sizeof(data_bwf_min_js_gz),true},
{file_index_htm,data_c_index_htm_gz,sizeof(data_c_index_htm_gz),true},
{file_setup_htm,data_c_setup_htm_gz,sizeof(data_c_setup_htm_gz),true},
{file_logconfig,data_c_log_htm_gz,sizeof(data_c_log_htm_gz),true},
{file_gravitydevice,data_c_gdc_htm_gz,sizeof(data_c_gdc_htm_gz),true},
{file_config,data_c_config_htm_gz,sizeof(data_c_config_htm_gz),true},
{file_testcmd_htm,(const uint8_t *)data_testcmd_htm,0,false},
{file_lcd_htm,lcd_htm_gz,sizeof(lcd_htm_gz),true}
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