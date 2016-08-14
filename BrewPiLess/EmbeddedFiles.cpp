#include <pgmspace.h>
#ifndef NULL
#define NULL 0
#endif

typedef struct _EmbeddedFileMapEntry{
	const char *filename;
	const char *content;
} EmbeddedFileMapEntry;

const char data_bwf_js[] PROGMEM =
R"END(
#include "data_bwf_js.h"
)END";
const char file_bwf_js [] PROGMEM="/bwf.js";

const char data_index_htm[] PROGMEM =
R"END(
#include "data_index_html.h"
)END";
const char file_index_htm [] PROGMEM="/index.htm";

const char data_control_htm[] PROGMEM =
R"END(
#include "data_control_htm.h"
)END";

const char file_control_htm [] PROGMEM="/control.htm";

const char data_setup_htm[] PROGMEM =
R"END(
#include "data_setup_html.h"
)END";

const char file_setup_htm [] PROGMEM="/setup.htm";

const char data_testcmd_htm[] PROGMEM =
R"END(
#include "data_test_cmd_htm.h"
)END";

const char file_testcmd_htm [] PROGMEM="/testcmd.htm";

EmbeddedFileMapEntry fileMaps[]={
{file_bwf_js,data_bwf_js},
{file_index_htm,data_index_htm},
{file_control_htm,data_control_htm},
{file_setup_htm,data_setup_htm},
{file_testcmd_htm,data_testcmd_htm}
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