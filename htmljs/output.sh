#!/bin/sh

OUTDIR="cheader"
DISTDIR="dist"

if [ ! -d $OUTDIR ]; then
    echo "$OUTDIR not found!"
    mkdir "$OUTDIR"
fi
rm $OUTDIR/*.h

htmlfiles=(index_s.htm.gz control_s.htm.gz config.htm.gz setup.htm.gz logging.htm.gz gravity.htm.gz \
classic-index.htm.gz classic-config.htm.gz classic-setup.htm.gz classic-gdc.htm.gz classic-log.htm.gz)

variables=(data_index_htm_gz control_htm_gz config_htm_gz setup_htm_gz logging_htm_gz gravity_htm_gz \
 data_c_index_htm_gz data_c_config_htm_gz data_c_setup_htm_gz data_c_gdc_htm_gz data_c_log_htm_gz)

outfiles=(index_htm control_htm config_htm setup_htm log_htm gdc_htm \
 c_index_htm c_config_htm c_setup_htm c_gdc_htm c_log_htm)

languages=(english spanish portuguesbr)


gen_C_file()
{
lang=$1
for ((index=0; index<${#htmlfiles[@]}; index++)); do
    srcdir="dist/$lang"

#   echo "[$index]: ${htmlfiles[$index]}"
   input="$srcdir/${htmlfiles[$index]}"
   output="$OUTDIR/${lang}_${outfiles[$index]}.h"
   variable=${variables[$index]}
   #echo "input: $input output file: $output with variables $variable "
   xxd -i  "$input" > $output 
   echo "processing $output"
   sed -i "s/unsigned char .\+\[\]/const unsigned char $variable\[\] PROGMEM/" $output
done
}

for lang in "${languages[@]}"
do
gen_C_file $lang
done
