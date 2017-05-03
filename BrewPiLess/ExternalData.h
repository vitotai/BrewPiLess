#ifndef ExternalData_H
#define ExternalData_H
#include "BrewPiProxy.h"
#include "BrewKeeper.h"
#include "BrewLogger.h"
#include "mystrlib.h"

#define INVALID_VOLTAGE -1
#define INVALID_GRAVITY -1
#define INVALID_TEMP  -250

#define IsVoltageValid(v) ((v) > 0)
#define IsGravityValid(g) ((g) > 0)

extern BrewKeeper brewKeeper;
extern BrewLogger brewLogger;
extern BrewPiProxy brewPi;

#define ErrorNone 0
#define ErrorAuthenticateNeeded 1
#define ErrorJSONFormat 2
#define ErrorMissingField 3
#define ErrorUnknownSource 4

#define C2F(t) ((t)*1.8+32)

const char gravityconfig_html[]  PROGMEM =R"END(
<html><head><title>Gravity Device</title><meta http-equiv="content-type" content="text/html; charset=utf-8" >
<script>
function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}var Q=function(d){return document.querySelector(d)};function fill(a){for(var b in a){var c=Q("input[name="+b+"]");if(c.type=="checkbox")c.checked=a[b];else c.value=a[b]}}function save(){var b=document.getElementsByTagName("input");var c={};for(var i=0;i<b.length;i++){var d=b[i];if(d.type=="checkbox")c[d.name]=d.checked;else if(d.type=="text")c[d.name]=d.value}console.log("result="+JSON.stringify(c));s_ajax({url:window.location.href,m:"POST",mime:"text/plain",data:JSON.stringify(c),success:function(a){alert("done.")},fail:function(a){alert("failed updating data:"+a)}})}function init(){s_ajax({url:window.location.href+"?data=1",m:"GET",success:function(a){fill(JSON.parse(a))},fail:function(a){alert("failed getting data:"+a)}})}
</script>
</head><body onload=init()>
<form action="" method="post">
<table>
<tr><td>iSpindel</td><td><input type="checkbox" name="ispindel" value="1"></td></tr>
<tr><td>Calculated by BPL</td><td><input type="checkbox" name="cbpl" value="1"> </td></tr>
<tr><td>SG Calibration</td><td><input type="text" name="gc"size=4> point</td></tr>
<tr><td>Temp. Correction</td><td><input type="checkbox" name="tc" value="1"> @ <input type="text" name="ctemp" size=4>&deg;C </td></tr>
<tr><td>Coefficients</td><td><input type="text" name="a3"  size=15>*x^3 + <input type="text" name="a2" size=15>*x^2+ <input type="text" name="a1" size=15>*x + <input type="text" name="a0" size=15> </td></tr>
<tr><td>Save Change</td><td><input type="submit" name="submit" onclick="save();return false;"></input></td></tr>
</table>
</form>
</form>
</body></html>
)END";


class SimpleFilter
{
	float _y;
	float _b;
public:
	SimpleFilter(){ _b = 0.1;}
	void setInitial(float v){ _y=v;}
	void setBeta(float b) { _b = b; }

	float addData(float x){
		_y = _y + _b * (x - _y);
		return _y;
	}
};

class ExternalData
{
protected:
	float _gravity;
	float _auxTemp;
	time_t _lastUpdate;
	float  _deviceVoltage;
	float _og;
	SimpleFilter filter;
    
    bool _ispindelEnable;
    float _ispindelCalibration;
    bool _ispindelTempCal;
    float _ispindelCalibrationBaseTemp;
    float _ispindelCoefficients[4];
    char *_ispindelName;

    bool _calculateGravity;

public:
	ExternalData(void):_gravity(INVALID_GRAVITY),_auxTemp(INVALID_TEMP),_deviceVoltage(INVALID_VOLTAGE),_lastUpdate(0)
	,_ispindelEnable(false),_ispindelName(NULL),_calculateGravity(false){}
    
    bool iSpindelEnabled(void){return _ispindelEnable;}
    
    const char * html(void) { return gravityconfig_html;}

    void sseNotify(char *buf){
		
		
		char strbattery[8];
		int len=sprintFloat(strbattery,_deviceVoltage,2);
		strbattery[len]='\0';

		char strgravity[8];
		len=sprintFloat(strgravity,_gravity,3);
		strgravity[len]='\0';
		
		const char *spname=(_ispindelName)? _ispindelName:"Unknown";
		//static const char fmt[] PROGMEM ="G:{\"name\":\"%s\",\"battery\":%s,\"gravity\":%s,\"lu\":%ld}";
		//sprintf_P(buf,PSTR("G:{\"name\":\"%s\",\"battery\":%s,\"gravity\":%s,\"lu\":%ld}"),spname, strbattery,strgravity,_lastUpdate);
		sprintf(buf,"G:{\"name\":\"%s\",\"battery\":%s,\"sg\":%s,\"lu\":%ld}",spname, strbattery,strgravity,_lastUpdate);

    }
    void config(char* configdata)
    {
    	const int BUFFER_SIZE = JSON_OBJECT_SIZE(12);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(configdata);
		
		if (!root.success()
		    || !root.containsKey("ispindel")
		    || !root.containsKey("gc")
		    || !root.containsKey("tc")
		    || !root.containsKey("a3")
		    || !root.containsKey("a2")
		    || !root.containsKey("a1")
		    || !root.containsKey("a0")){
  			DBG_PRINTF("Invalid JSON config\n");
  			return;
		}
		_ispindelEnable=root["ispindel"];
		_ispindelCalibration =root["gc"];
		_ispindelCalibration = _ispindelCalibration / 1000.0;
		_ispindelTempCal = root["tc"];
		if(_ispindelTempCal){
		    _ispindelCalibrationBaseTemp=(root.containsKey("ctemp"))? root["ctemp"]:20;
		}
		_calculateGravity =(root.containsKey("cbpl"))? false:root["cbpl"];
		
		_ispindelCoefficients[0]=root["a0"];
		_ispindelCoefficients[1]=root["a1"];
		_ispindelCoefficients[2]=root["a2"];
		_ispindelCoefficients[3]=root["a3"];				
		// debug
		#if 0
		Serial.print("Coefficient:");
		for(int i=0;i<4;i++){
		    Serial.print(_ispindelCoefficients[i],10);
		    Serial.print(", ");
		}
		Serial.println(""); 
		#endif
    }
    
	void setOriginalGravity(float og){
		_og = og;
		brewLogger.addGravity(og,true);
	}
	
	void setPlato(float plato, time_t now){
		float sg=1 + (plato / (258.6 -((plato/258.2)*227.1)));
		setGravity(sg,now);
	}
	
	void setTilt(float tilt,float temp,time_t now){
	    // calculate plato
	    float sg = _ispindelCoefficients[0] 
                    +  _ispindelCoefficients[1] * tilt
                    +  _ispindelCoefficients[2] * tilt * tilt
                    +  _ispindelCoefficients[3] * tilt * tilt * tilt;
      //  Serial.print("Plato:");
	   // Serial.print(plato,2);
	    // convert to SG
	    //float sg=1 + (plato / (258.6 -((plato/258.2)*227.1)));

	    //Serial.print(" SG:");
	    //Serial.print(sg,3);

	    // temp. correction
	    if(_ispindelTempCal){
	        sg = temperatureCorrection(sg,C2F(temp),C2F(_ispindelCalibrationBaseTemp));
        	//Serial.print(" TC:");
    	    //Serial.print(sg,3);
	    }

	    // add correction
	    sg += _ispindelCalibration;
        	//Serial.print(" Corrected:");
	        //Serial.println(sg,3);

	    // update
	    setGravity(sg,now);
	}
	
	void setGravity(float sg, time_t now){
	
		if(!IsGravityValid(_gravity)) filter.setInitial(sg);
		
		_gravity = sg; 
		_lastUpdate=now;
#if EnableGravitySchedule		
		brewKeeper.updateGravity(filter.addData(sg));
#endif
		brewLogger.addGravity(_gravity,false);
	}

	float gravity(void){ return _gravity;}
	void invalidateGravity(void){  _gravity = INVALID_GRAVITY;}

	
	void setAuxTemperatureCelsius(float temp){
		char unit;
		float max,min;
		brewPi.getTemperatureSetting(&unit,&min,&max);
		if(unit == 'C'){
			_auxTemp= temp;
		}else{
			_auxTemp= temp * 1.8 +32 ;
		}
		brewLogger.addAuxTemp(_auxTemp);
	}
	
	
	float auxTemp(void){return _auxTemp; }
	void invalidateAuxTemp(void){ _auxTemp=INVALID_TEMP;}

//	void setUpdateTime(time_t update){ _lastUpdate=update;}
	time_t lastUpdate(void){return _lastUpdate;}
	void setDeviceVoltage(float vol){ _deviceVoltage = vol; }
	float deviceVoltage(void){return _deviceVoltage;}
	void invalidateDeviceVoltage(void) { _deviceVoltage= INVALID_VOLTAGE; }

	float temperatureCorrection(float sg, float t, float c){
	    
	    float nsg= sg*((1.00130346-0.000134722124*t+0.00000204052596*t*t -0.00000000232820948*t*t*t)/
	        (1.00130346-0.000134722124*c+0.00000204052596*c*c-0.00000000232820948*c*c*c));
	    return nsg;
	}
	
	bool processJSON(char data[],size_t length, bool authenticated, uint8_t& error)
	{
		const int BUFFER_SIZE = JSON_OBJECT_SIZE(8);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject((char*)data,length);
        
		
		if (!root.success() || !root.containsKey("name")){
  			DBG_PRINTF("Invalid JSON\n");
  			error = ErrorJSONFormat;
  			return false;
		}
		
		String name= root["name"];
		
		if(name.equals("webjs")){
			if(! authenticated){
			    error = ErrorAuthenticateNeeded;
    	        return false;
    	    }
	        
			if(!root.containsKey("gravity")){
  				DBG_PRINTF("No gravity\n");
  				error = ErrorMissingField;
  				return false;
  			}
			float  gravity = root["gravity"];
			if(root.containsKey("og")){
				setOriginalGravity(gravity);
			}
			else{
				setGravity(gravity,TimeKeeper.getTimeSeconds());
			}
		}else if(name.startsWith("iSpindel")){
			//{"name": "iSpindel01", "id": "XXXXX-XXXXXX", "temperature": 20.5, "angle": 89.5, "gravityP": 13.6, "battery": 3.87}
			DBG_PRINTF("%s\n",name.c_str());
			
			if(!_ispindelName){
			    _ispindelName=(char*) malloc(name.length()+1);
			    if(_ispindelName) strcpy(_ispindelName,name.c_str());
			}
			if(! root.containsKey("temperature")){
			    DBG_PRINTF("iSpindel report no temperature!\n");
			    return false;
			}
			float itemp=root["temperature"];
			setAuxTemperatureCelsius(itemp);
			
			//setPlato(root["gravityP"],TimeKeeper.getTimeSeconds());
			if(!_calculateGravity && root.containsKey("gravity"))
            	setGravity(root["gravity"], TimeKeeper.getTimeSeconds());
			else{
		    	if(! root.containsKey("angle")){
        		    DBG_PRINTF("iSpindel report no angle!\n");
			        return false;
			    }
    			setTilt(root["angle"],itemp,TimeKeeper.getTimeSeconds());
            }
            if(root.containsKey("battery"))
    		    setDeviceVoltage(root["battery"]);			
		}else{
		    error = ErrorUnknownSource;
		    return false;
		}
		return true;
	}
};

extern ExternalData externalData;

#endif