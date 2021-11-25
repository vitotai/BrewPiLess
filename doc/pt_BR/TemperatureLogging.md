* **The log won’t start automatically**, you have to start it at log setting page.
* The gravity information is stored in the log, so if you are using gravity-based schedule, you have to start local log, **manually**.
* When logging is not started, BrewPiLess still logs the data and keep that latest 3 to 6 hours of data. The data will not be saved to file system, though.
* The temperatures are logged every minute.
* A 30 day log will take around 350k bytes. That might imply that 3M space can record around 6 month data. However, there is no guarantee of the robustness of SPIFFS.
* Changing temperature unit when logging will result in wrong data interpretation.
* A maximum of 10 logs is allowed. The logs will **not** be deleted automatically. Manual deleting is necessary.
* Off-line viewer is available. You can download the log and view it from your computer. Download the file "BPLLogViewer.htm" in the "extra" subfolder. Save it anywhere in your computer. Open it using a web browser.
* The loggging format changed after v1.2.7/v2.0/v2.4. Use BPLLogViewerV2.htm to view the new logs and BPLLogViewer.htm for old logs.

You can view, share, and crop the logs. Check http://vito.tw/?p=821

