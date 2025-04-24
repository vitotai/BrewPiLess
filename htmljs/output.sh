#!/bin/sh

OUTDIR="cheader"
DISTDIR="dist"

if [ ! -d $OUTDIR ]; then
    echo "$OUTDIR not found!"
    mkdir "$OUTDIR"
fi
rm $OUTDIR/*.h

htmlfiles=(index_s.htm.gz control_s.htm.gz config.htm.gz setup.htm.gz logging.htm.gz gravity.htm.gz gravity_e32.htm.gz pressure.htm.gz backup.htm.gz)

variables=(data_index_htm_gz control_htm_gz config_htm_gz setup_htm_gz logging_htm_gz gravity_htm_gz gravity_htm_gz pressure_htm_gz backup_htm_gz)

outfiles=(index_htm control_htm config_htm setup_htm log_htm gdc_htm gdc_e32_htm pressure_htm backup_htm)

languages=(norwegian english spanish portuguese-br slovak chinese italian)


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
