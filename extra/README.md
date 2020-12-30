# Utitlities that might help

Log Viewers are moved to htmljs/dist/english/BPLLogViewer.htm

## backup.htm
The utilities to backup/restore settings, includes
System Configuration, Gravity Device, Remote Logging, Pressure tranducer, and MQTT.

Use filemanager(http://brewpiless.local:8008/filemanager) to upload this html file.
Go to this file http://brewpiless.local/backup.htm
"Retrieve from Controller", and then "Save..".
The downloaed file, bpl.settings.json, will be saved to your computer.

To Restore, 
Select file to "Load from", enter "username/password", and then "Restore..".

In the future, this funcion will be embedded. Before that, you need to upload this file again if the FileSystem is erased during upgrading firmware.

## brewpimon.php
A small simple script that I used to monitor whether or not my BrewPiLess is still alive.

## calibrationSG.htm
Calibration tool for iSpindel. It's essentially the same as the one on Sam's Github repository.

## iSpindel-BPL.5.12.bin
The golden version of my iSpindel.
