#include <Arduino.h>
#include <pgmspace.h>
#include "espconfig.h"

#ifndef NULL
#define NULL 0
#endif


typedef struct _EmbeddedFileMapEntry{
	const char *filename;
	const char *content;
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

#include "data_viewlog_htm.h"
const char file_viewlog_htm [] PROGMEM="/viewlog.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_js},
{file_index_htm,data_nindex_htm},
{file_lcd,data_index_htm},
{file_setup_htm,data_setup_htm},
{file_testcmd_htm,data_testcmd_htm},
{file_viewlog_htm,data_viewlog_htm}
};

const char* getEmbeddedFile(const char* filename)
{
	for(int i=0;i<sizeof(fileMaps)/sizeof(EmbeddedFileMapEntry);i++)
	{
		if(strcmp_P(filename,fileMaps[i].filename) ==0){
			return fileMaps[i].content;
		}
	}
	return NULL;
} 















































































































































































































































































































































