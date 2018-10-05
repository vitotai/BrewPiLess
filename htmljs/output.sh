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

outfiles=(data_index_htm.h data_control_htm.h data_config_htm.h data_setup_htm.h data_log_htm.h data_gdc_htm.h \
 data_c_index_htm.h data_c_config_htm.h data_c_setup_htm.h data_c_gdc_htm.h data_c_log_htm.h)

languages=(english spanish)


for ((index=0; index<${#htmlfiles[@]}; index++)); do

for lang in "${languages[@]}"
do
srcdir="dist/$lang"

#   echo "[$index]: ${htmlfiles[$index]}"
   input="$srcdir/${htmlfiles[$index]}"
   output="$OUTDIR/${outfiles[$index]}"
   variable=${variables[$index]}
   #echo "input: $input output file: $output with variables $variable "
   echo "#if WebPageLanguage == $lang" >> $output
   xxd -i  "$input" >> $output 
   echo "#endif" >> $output
done
    echo "replaceing $variable"
    sed -i "s/unsigned char .\+\[\]/const unsigned char $variable\[\] PROGMEM/" $output
done
