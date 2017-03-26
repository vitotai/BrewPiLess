#include "espconfig.h"
#if EnableGravitySchedule
const char data_nindex_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BrewPiLess reporting for duty!</title>
<meta name="apple-mobile-web-app-title" content="BrewPiLess">
<meta name="apple-mobile-web-app-capable" content="yes">
<script src="http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js"></script>
<script type="text/javascript" src="bwf.js"></script>
<script>
var Q=function(d){return document.querySelector(d)};var BrewChart=function(a){this.cid=a;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=NaN;this.auxTemp=NaN;this.gravity=NaN;this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(c){this.celius=c;this.ylabel(STR.ChartLabel+'('+(c?"°C":"°F")+')')};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(d){var a=d.getHours();var b=d.getMinutes();var c=d.getSeconds();function T(x){return(x>9)?x:("0"+x)}return d.toLocaleDateString()+" "+T(a)+":"+T(b)+":"+T(c)};BrewChart.prototype.showLegend=function(a,b){var d=new Date(a);Q(".beer-chart-legend-time").innerHTML=this.formatDate(d);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,6));var g=this.chart.getValue(b,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(isNaN(g))?"--":g.toFixed(3);var c=parseInt(this.state[b]);if(!isNaN(c)){Q('.chart-legend-row.state .legend-label').innerHTML=STATES[c].text}};BrewChart.prototype.hideLegend=function(){var v=document.querySelectorAll(".legend-value");v.forEach(function(a){a.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q('.chart-legend-row.state .legend-label').innerHTML="state"};BrewChart.prototype.tempFormat=function(y){var v=parseFloat(y);if(isNaN(v))return"--";var a=this.celius?"°C":"°F";return parseFloat(v).toFixed(2)+a};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(a){this.shownlist[a]=!this.shownlist[a];if(this.shownlist[a]){Q("."+a+" .toggle").style.backgroundColor=Q(".chart-legend-row."+a).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,true)}else{Q("."+a+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,false)}};BrewChart.prototype.createChart=function(){var t=this;t.initLegend();t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var c=document.createElement("div");c.className="hide";document.body.appendChild(c);var d={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:'Temperature',y2label:'Gravity',series:{'gravity':{axis:'y2',drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:'#ccc',gridLineWidth:'0.1px',labelsDiv:c,labelsDivStyles:{'display':'none'},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(y){return t.tempFormat(y)}},y2:{valueFormatter:function(y){return y.toFixed(3)},axisLabelFormatter:function(y){return y.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(e,x,a,b){t.showLegend(x,b)},unhighlightCallback:function(e){t.hideLegend()}};t.chart=new Dygraph(document.getElementById(t.cid),t.data,d)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=['Time','beerSet','beerTemp','fridgeTemp','fridgeSet','roomTemp','auxTemp','gravity'];BrewChart.prototype.addMode=function(m){var s=String.fromCharCode(m);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:s.toUpperCase(),text:BrewChart.Mode[s],attachAtBottom:true})};BrewChart.testData=function(a){if(a[0]!=0xFF)return false;var s=a[1]&0x07;if(s!=4||s!=6)return false;return{sensor:s,f:a[1]&0x10}};BrewChart.prototype.addResume=function(a){this.ctime+=a;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:'R',text:'Resume',attachAtBottom:true})};BrewChart.prototype.process=function(a){var b=false;var t=this;for(var i=0;i<a.length;){var c=a[i++];var e=a[i++];if(c==0xFF){t.celius=(e&0x10)?false:true;t.numLine=(e&0xF);var p=a[i++];p=p*256+a[i++];t.interval=p;t.starttime=(a[i]<<24)+(a[i+1]<<16)+(a[i+2]<<8)+a[i+3];t.ctime=t.starttime;i+=4;t.data=[];t.anno=[];t.state=[];t.cstate=0;b=true}else if(c==0xF4){t.addMode(e)}else if(c==0xF1){t.cstate=e}else if(c==0xF7){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;t.beerSet=(v==0x7FFF)?NaN:(v/100)}else if(c==0xF8){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;if(e){t.og=v/1000}else{t.gravity=(v==0x7FFF)?NaN:(v/1000);t.sg=t.gravity}}else if(c==0xF9){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;t.auxTemp=(v==0x7FFF)?NaN:(v/100)}else if(c==0xFE){if(t.lidx){var h;for(h=t.lidx;h<t.numLine;h++)t.dataset.push(NaN);t.data.push(t.dataset)}t.lidx=0;t.addResume(e)}else if(c<128){if(t.lidx==0){var d=new Date(this.ctime*1000);var j=t.beerSet;t.dataset=[d,j];t.incTime()}var k=c*256+e;if(k==0x7FFF||k>12000){k=NaN}else{if(t.lidx==t.numLine-1){k=k/1000}else{k=k/100}}t.dataset.push(k);if(++t.lidx>=t.numLine){if(t.numLine==4){t.dataset.push(t.auxTemp);t.dataset.push(t.gravity);t.gravity=NaN}t.lidx=0;t.data.push(t.dataset);t.state.push(t.cstate)}}}if(typeof t.chart=="undefined")t.createChart();else t.chart.updateOptions({'file':t.data});t.chart.setAnnotations(t.anno);return b};var profileEditor={dirty:false,tempUnit:'C',C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(d){this.dirty=d;document.getElementById(this.C_savebtn_Id).innerHTML=(d)?"Save*":"Save"},getStartDate:function(){return this.sd},setStartDate:function(d){},startDayChange:function(){var a=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(a.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=a;this.reorg();this.markdirty(true)}},startnow:function(){var d=new Date();document.getElementById(this.C_startday_Id).value=formatDate(d);this.sd=d;this.reorg();this.markdirty(true)},rowList:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];return a.getElementsByTagName("tr")},sgChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;else a.saved=parseFloat(a.innerHTML);this.markdirty(true)},dayChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;else{this.markdirty(true);this.reorg();ControlChart.update(this.chartdata(),this.tempUnit)}},tempChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;else{this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)}},initrow:function(a,c){var b=this;var d=c.c;a.type=d;var e=a.getElementsByClassName("stage-temp")[0];if(d=="r"){e.innerHTML=""}else{e.innerHTML=c.t;e.contentEditable=true;e.onblur=function(){b.tempChange(this)};e.onfocus=function(){this.saved=this.innerHTML}}var f=a.getElementsByClassName("stage-time")[0];f.innerHTML=c.d;f.contentEditable=true;f.onblur=function(){b.dayChange(this)};f.onfocus=function(){this.saved=this.innerHTML};var g=a.getElementsByClassName("stage-sg")[0];if(d=="r"){g.innerHTML=""}else{g.saved=c.g;g.innerHTML=c.g;g.contentEditable=true;g.onblur=function(){b.sgChange(this)};g.onfocus=function(){this.saved=this.innerHTML}}var h=a.getElementsByClassName("for-time")[0];var i=a.getElementsByClassName("condition")[0];var j={t:0,g:1,a:2,o:3};if(d=="r"){h.style.display="block";i.style.display="none"}else{i.value=c.c;i.selectedIndex=j[c.c];h.style.display="none";i.style.display="block"}},datestr:function(a){var b=new Date(this.sd.getTime()+Math.round(a*86400)*1000);return formatDate(b)},reorg:function(){var a=this.rowList();var b=this.sd.getTime();for(var i=0;i<a.length;i++){var c=a[i];c.className=(i%2)?"odd":"even";c.getElementsByClassName("diaplay-time")[0].innerHTML=formatDate(new Date(b));var d=this.rowTime(c);b+=Math.round(d*86400)*1000}},chartdata:function(){var a=this.rowList();if(a.length==0)return[];var b=this.sd.getTime();var c=a[0];var d=this.rowTemp(c);var e=[];e.push([new Date(b),d]);for(var i=0;i<a.length;i++){var c=a[i];var f;if(c.type=="r"){f=this.rowTemp(a[i+1])}else{f=this.rowTemp(c)}b+=Math.round(this.rowTime(c)*86400)*1000;e.push([new Date(b),f])}return e},addRow:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c;if(b.length==0){var d=(this.tempUnit=='C')?20:68;c={c:'t',t:d,d:1,g:1.01}}else{var e=b[b.length-1];var f=this.row.cloneNode(true);this.initrow(f,{c:"r",d:1});a.appendChild(f);c={c:'t',t:this.rowTemp(e),d:1,g:this.rowSg(e)}}var f=this.row.cloneNode(true);this.initrow(f,c);a.appendChild(f);this.reorg();this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)},delRow:function(){var a=this.rowList();if(a.length==0)return;var b=a[a.length-1];if(a.length>1){var c=a[a.length-2];c.parentNode.removeChild(c)}b.parentNode.removeChild(b);this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)},rowTemp:function(a){return parseFloat(a.getElementsByClassName("stage-temp")[0].innerHTML)},rowCondition:function(a){return a.getElementsByClassName("condition")[0].value},rowTime:function(a){return parseFloat(a.getElementsByClassName("stage-time")[0].innerHTML)},rowSg:function(a){return a.getElementsByClassName("stage-sg")[0].saved},initable:function(s,d){this.sd=d;document.getElementById(this.C_startday_Id).value=formatDate(d);var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];this.row=a.getElementsByTagName("tr")[0];a.removeChild(this.row);for(var i=0;i<s.length;i++){var b=this.row.cloneNode(true);this.initrow(b,s[i]);a.appendChild(b)}this.reorg()},clear:function(){var a=this.rowList();var b=a.length;for(var i=a.length-1;i>=0;i--){var c=a[i];c.parentNode.removeChild(c)}this.markdirty(true)},getProfile:function(){var a=this.rowList();var b=0;var c=[];for(var i=0;i<a.length;i++){var d=a[i];var e=this.rowTime(d);if(isNaN(e))return false;if(d.type=="r"){c.push({c:"r",d:e})}else{var f=this.rowTemp(d);var g=this.rowSg(d);if(isNaN(f)||isNaN(g))return false;if(f>BrewPiSetting.maxDegree||f<BrewPiSetting.minDegree)return false;c.push({c:this.rowCondition(d),d:e,t:f,g:g})}}var s=this.sd.toISOString();var h={s:s,v:2,u:this.tempUnit,t:c};console.log(h);return h},initProfile:function(p){if(typeof p!="undefined"){var a=new Date(p.s);this.tempUnit=p.u;profileEditor.initable(p.t,a)}else{profileEditor.initable([],new Date())}},setTempUnit:function(u){if(u==this.tempUnit)return;this.tempUnit=u;var a=this.rowList();for(var i=0;i<a.length;i++){var b=a[i].querySelector('td.ptemp');var c=parseFloat(b.innerHTML);b.innerHTML=(u=='C')?F2C(c):C2F(c)}}};var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:'C'};var ControlChart={unit:"C",init:function(a,b,c){var t=this;t.data=b;t.unit=c;var e=function(v){d=new Date(v);return(d.getMonth()+1)+"/"+d.getDate()};var f=function(v){return v.toFixed(1)+"&deg;"+t.unit};t.chart=new Dygraph(document.getElementById(a),t.data,{colors:['rgb(89, 184, 255)'],axisLabelFontSize:12,gridLineColor:'#ccc',gridLineWidth:'0.1px',labels:["Time","Temperature"],labelsDiv:document.getElementById(a+"-label"),legend:'always',labelsDivStyles:{'textAlign':'right'},strokeWidth:1,axes:{y:{valueFormatter:f,pixelsPerLabel:20,axisLabelWidth:35},x:{axisLabelFormatter:e,valueFormatter:e,pixelsPerLabel:30,axisLabelWidth:40}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},})},update:function(a,b){if(a.length==0)return;this.data=a;this.unit=b;this.chart.updateOptions({'file':this.data})}};function formatDate(a){var y=a.getFullYear();var M=a.getMonth()+1;var d=a.getDate();var h=a.getHours();var m=a.getMinutes();function dd(n){return(n<10)?'0'+n:n}return dd(M)+"/"+dd(d)+" "+dd(h)+":"+dd(m)}var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(m){var d=document.getElementById(m+"-m");var a=document.getElementById(m+"-m").className.replace(/\snav-selected/,'');d.className=a;document.getElementById(m+"-s").style.display="none"},select:function(m){document.getElementById(m+"-m").className+=' nav-selected';document.getElementById(m+"-s").style.display="block"},init:function(){var b=this;if(b.initiated)return;b.initiated=true;for(var i=0;i<4;i++){var m=b.modes[i];document.getElementById(m+"-s").style.display="none";document.getElementById(m+"-m").onclick=function(){var a=this.id.replace(/-m/,'');b.dselect(b.cmode);b.select(a);b.cmode=a}}b.cmode="profile";b.select(b.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var v=document.getElementById(this.cmode+"-t").value;if(v==''||isNaN(v)||(v>BrewPiSetting.maxDegree||v<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+v);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+v+"}");BWF.send("j{mode:b, beerSet:"+v+"}")}else{console.log("j{mode:f, fridgeSet:"+v+"}");BWF.send("j{mode:f, fridgeSet:"+v+"}")}}else if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}};function saveprofile(){console.log("save");var r=profileEditor.getProfile();if(r===false){alert("invalid value. check again");return}var a=JSON.stringify(r);console.log("result="+a);BWF.save(BWF.BrewProfile,a,function(){profileEditor.markdirty(false);alert("Done.")},function(e){alert("save failed:"+e)})}function C2F(c){return Math.round((c*1.8+32)*10)/10}function F2C(f){return Math.round((f-32)/1.8*10)/10}function updateTempUnit(u){var a=document.getElementsByClassName("t_unit");for(var i=0;i<a.length;i++){a[i].innerHTML=u}}function openDlgLoading(){document.getElementById('dlg_loading').style.display="block"}function closeDlgLoading(){document.getElementById('dlg_loading').style.display="none"}function onload(c){modekeeper.init();openDlgLoading();function complete(){var b=true;function initComp(){if(!b)return;b=false;closeDlgLoading();c()}invoke({url:"/tcc",m:"GET",fail:function(a){console.log("error connect to BrwePiLess!");initComp()},success:function(d){var s=JSON.parse(d);var a={valid:true,minDegree:s.tempSetMin,maxDegree:s.tempSetMax,tempUnit:s.tempFormat};if(a.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(a.tempUnit);profileEditor.setTempUnit(a.tempUnit)}BrewPiSetting=a;initComp()}})}BWF.load(BWF.BrewProfile,function(d){var p=JSON.parse(d);updateTempUnit(p.u);BrewPiSetting.tempUnit=p.u;profileEditor.initProfile(p);ControlChart.init("tc_chart",profileEditor.chartdata(),p.u);complete()},function(e){profileEditor.initProfile();ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);complete()})}function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}function getLogName(){s_ajax({url:"loglist.php",m:"GET",success:function(d){var r=JSON.parse(d);if(r.rec){Q("#recording").innerHTML=r.log}else{Q("#recording").innerHTML=""}},fail:function(d){alert("failed:"+d)}})}var BChart={offset:0,url:'chart.php',toggle:function(a){this.chart.toggleLine(a)},reqdata:function(){var t=this;var c='offset='+t.offset;if(typeof t.startOff!="undefined"&&t.startOff!==null)c=c+"&index="+t.startOff;var d=new XMLHttpRequest();d.open('GET',t.url+'?'+c);d.timeout=5000;d.ontimeout=function(e){console.error("Timeout!!")};d.responseType='arraybuffer';d.onload=function(e){if(this.status==404){console.log("Error getting log data");return}var a=new Uint8Array(this.response);if(a.length==0){console.log("zero content");if(t.timer)clearInterval(t.timer);t.timer=null;setTimeout(function(){t.reqdata()},3000);return}var b=t.chart.process(a);if(b){t.offset=a.length;t.startOff=d.getResponseHeader("LogOffset");getLogName();console.log("new chart, offset="+t.startOff)}else t.offset+=a.length;if(!isNaN(t.chart.og)){updateOriginGravity(t.chart.og);t.chart.og=NaN}if(!isNaN(t.chart.sg)){updateGravity(t.chart.sg);t.chart.sg=NaN}if(t.timer==null)t.settimer()};d.onerror=function(){console.log("error getting data.");setTimeout(function(){t.reqdata()},10000)};d.send()},settimer:function(){var t=this;t.timer=setInterval(function(){t.reqdata()},t.chart.interval*1000)},init:function(a){this.chart=new BrewChart(a)},timer:null,start:function(){if(this.running)return;this.running=true;this.offset=0;this.reqdata()},reqnow:function(){var t=this;if(t.timer)clearInterval(t.timer);t.timer=null;t.reqdata()}};function setLcdText(a,b){var d=document.getElementById(a);d.innerHTML=b}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function gravityDevice(a){if(typeof a["name"]=="undefined")return;if(a.name.startsWith("iSpindel")){if(typeof window.iSpindel=="undefined"){window.iSpindel=true;Q("#iSpindel-pane").style.display="block"}Q("#iSpindel-name").innerHTML=a.name;if(typeof a["battery"]!="undefined")Q("#iSpindel-battery").innerHTML=a.battery;var b;if(typeof a["lu"]!="undefined")b=new Date(a.lu*1000);else b=new Date();Q("#iSpindel-last").innerHTML=formatDate(b);if(typeof a["gravityP"]!="undefined")updateGravity(BrewMath.pla2sg(a["gravityP"]))}}function init(){BChart.init("div_g");var b=true;onload(function(){BWF.init({onconnect:function(){BWF.send("l");setInterval(function(){if(!b)controllerError();BWF.send("l");b=false},5000)},error:function(e){console.log("error");communicationError()},handlers:{L:function(a){b=true;for(var i=0;i<4;i++)setLcdText("lcd-line-"+i,a[i])},V:function(c){console.log("forced reload chart");BChart.reqnow()},G:function(c){gravityDevice(c)}}});BChart.start()})}var BrewMath={abv:function(a,b){return((76.08*(a-b)/(1.775-a))*(b/0.794)).toFixed(1)},att:function(a,b){return Math.round((a-b)/(a-1)*100)},sg2pla:function(a){return-616.868+1111.14*a-630.272*a*a+135.997*a*a*a},pla2sg:function(a){return 1+(a/(258.6-((a/258.2)*227.1)))}};function updateGravity(a){window.sg=a;Q("#gravity-sg").innerHTML=a.toFixed(3);if(typeof window.og!="undefined"){Q("#gravity-att").innerHTML=BrewMath.att(window.og,a);Q("#gravity-abv").innerHTML=BrewMath.abv(window.og,a)}}function updateOriginGravity(a){if(typeof window.og!="undefined"&&window.og==a)return;window.og=a;Q("#gravity-og").innerHTML=a.toFixed(3);if(typeof window.sg!="undefined")updateGravity(window.sg)}function showgravitydlg(a){Q('#dlg_addgravity .msg').innerHTML=a;Q('#dlg_addgravity').style.display="block"}function dismissgravity(){Q('#dlg_addgravity').style.display="none"}function inputgravity(){dismissgravity();openDlgLoading();var a=parseFloat(Q("#dlg_addgravity input").value);if(window.isog)updateOriginGravity(a);else updateGravity(a);var b={name:"webjs",gravity:a};if(window.isog)b.og=1;s_ajax({url:"gravity",m:"POST",mime:"application/json",data:JSON.stringify(b),success:function(d){closeDlgLoading()},fail:function(d){alert("failed:"+d);closeDlgLoading()}})}function gravity(){window.isog=false;showgravitydlg("Add gravity Record:")}function origingravity(){window.isog=true;showgravitydlg("Set Original Gravity:")}
</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}.frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px}#top-frame{height:520px}#bottom-frame{height:380px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}#recording{color:lightblue;font-size:18px}#top-frame button{float:right;width:200px;margin-top:3px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:3.8em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em;border-top-right-radius:5px;border-top-left-radius:5px}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:965px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}#bottom-frame button{color:#1d5987;background:#dfeffc;font-weight:bold;border-radius:6px;margin:4px}#addbutton{float:right;width:120px}#header{width:968px}#clearbtn{width:18%}#savebtn{width:25%;float:right}#delbtn{width:18%}.modal{display:none;position:fixed;z-index:100;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:10px;border:1px solid #888;width:320px;height:100px;border-radius:8px}#dlg_addgravity button{float:right;width:56px;margin-top:2px}#modekeeper-apply{margin-top:0}#profile-edit{float:left;margin:6px;width:480px}#tc_chart{width:430px;height:280px;float:right;margin:6px}#gravity-pane table,#iSpindel-pane table{width:100px;float:right;border:1px solid white;border-radius:5px;margin-right:10px;color:cyan;font-size:12px}#iSpindel-pane{display:none}</style>
</head>
<body onload=init()>
<div class="frame" id="top-frame">
<div id="topbar">
<div id="lcd" class="lcddisplay"><span class="lcd-text">
<span class="lcd-line" id="lcd-line-0">Live LCD waiting</span>
<span class="lcd-line" id="lcd-line-1">for update from</span>
<span class="lcd-line" id="lcd-line-2">script...</span>
<span class="lcd-line" id="lcd-line-3"></span></p><p>
</div>
<div id="banner">BrewPiLess v2.0
<div id="recording"></div>
</div>
<div id="menu">
<button onclick="window.open('/log')">Data Logging</button>
<br>
<button onclick="window.open('/setup.htm')">Device Setup</button>
<br>
<button onclick="window.open('/config')">System Config</button>
<br>
<button onclick="window.open('/gdc')">Gravity Sensor</button>
</div>
<div id="gravity-pane">
<table>
<tr><th>OG:</th><td> <span id="gravity-og" onclick="origingravity()">--</span><td></tr>
<tr><th>SG:</th><td> <span id="gravity-sg" onclick="gravity()">--</span><td></tr>
<tr><th>ATT:</th><td> <span id="gravity-att"> --</span>%<td></tr>
<tr><th>ABV:</th><td> <span id="gravity-abv"> --</span>%<td></tr>
</table>
</div>
<div id="iSpindel-pane">
<table>
<tr><th colspan="2"><span id="iSpindel-name"></span></th></tr>
<tr><th>Battery</th><td> <span id="iSpindel-battery">--</span><td></tr>
<tr><th colspan="2">Last Update</th></tr>
<tr><td colspan="2" align="center"> <span id="iSpindel-last">--</span><td></tr>
</table>
</div>
</div>
<div id="chart-container">
<div id="div_g"></div>
<div id="chart-legend" class="chart-legend">
<div class="chart-legend-row time">
<div class="beer-chart-legend-time">Date/Time</div>
</div>
<div class="chart-legend-row beerTemp">
<div class="toggle beerTemp" onclick="BChart.toggle('beerTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('beerTemp')">Beer Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row beerSet">
<div class="toggle beerSet" onclick="BChart.toggle('beerSet')"></div>
<div class="legend-label" onclick="BChart.toggle('beerSet')">Beer Set</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row fridgeTemp">
<div class="toggle beerSet" onclick="BChart.toggle('fridgeTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('fridgeTemp')">Fridge Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row fridgeSet">
<div class="toggle beerSet" onclick="BChart.toggle('fridgeSet')"></div>
<div class="legend-label" onclick="BChart.toggle('fridgeSet')">Fridge Set</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row roomTemp">
<div class="toggle beerSet" onclick="BChart.toggle('roomTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('roomTemp')">Room Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row auxTemp">
<div class="toggle gravity" onclick="BChart.toggle('auxTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('auxTemp')">Aux Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row gravity">
<div class="toggle gravity" onclick="BChart.toggle('gravity')"></div>
<div class="legend-label" onclick="BChart.toggle('gravity')">Gravity</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row state">
<div class="legend-label">state</div>
<br>
</div>
</div>
</div>
</div>
<div class="frame" id="bottom-frame">
<div class="navbar" id="header">
<div id="set-mode-text">Set temperature mode:</div>
<div class="navitems">
<span class="navitem" id="profile-m">Beer profile</span>
<span class="navitem" id="beer-m">Beer Const.</span>
<span class="navitem" id="fridge-m">Fridge Const.</span>
<span class="navitem" id="off-m">Off</span>
<button id="modekeeper-apply" onclick="modekeeper.apply()">Apply</button>
</div>
</div>
<div id="containter">
<div id="profile-s" class="detail">
<div id="profile-edit">
<div>
<div><span>Start Date:</span><input type="text" size="16" id="startdate" onchange="profileEditor.startDayChange()">
<button id="setnow" onclick=profileEditor.startnow()>Now</button>
<button id="addbutton" onclick="profileEditor.addRow()">Add</button></div>
</div>
<table class="profileTable" id="profile_t">
<thead><tr><th>Temp(&deg;<span class="t_unit">C</span>)</th><th>Until</th><th>Days</th><th>Gravity</th><th>Start</th></tr></thead>
<tbody>
<tr>
<td class="stage-temp">19</td><td>
<div class="for-time">Ramp</div>
<div class="condition-con">
<select class="condition"><option value="t">Time&gt;</option><option value="g">SG&lt;</option>
<option value="a">Time&gt; & SG&lt;</option><option value="o">Time&gt; OR SG&lt;</option></select>
</div>
</td><td class="stage-time">7</td><td class="stage-sg">1.01</td><td class="diaplay-time"></td></tr>
</tbody>
</table>
<div><button id="delbtn" onclick="profileEditor.delRow()">Delete</button><button id="clearbtn" onclick="profileEditor.clear()">Clear</button><button id="savebtn" onclick="saveprofile()">Save</button></div>
</div>
<div id="tc_chart"></div>
</div>
<div id="beer-s" class="detail">
Set Beer temp:
<input type="text" size="6" id="beer-t"></input>&deg;<span class="t_unit">C</span>
</div>
<div id="fridge-s" class="detail">
Set Fridge temp:
<input type="text" size="6" id="fridge-t"></input>&deg;<span class="t_unit">C</span>
</div>
<div id="off-s" class="detail">Turning temperature controll Off.</div>
</div>
<div id="dlg_loading" class="modal">
<div class="modal-content">
<p>Communicating with BrewPiLess controller..</p>
</div>
</div>
<div id="dlg_addgravity" class="modal">
<div class="modal-content">
<p><span class="msg"></span><input type="text" size="6" value="1.0">
<button onclick="dismissgravity()">Cancel</button>
<button onclick="inputgravity()">OK</button>
</p>
</div>
</div>
</div>
</body>
</html>
)END";

#else
const char data_nindex_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BrewPiLess reporting for duty!</title>
<meta name="apple-mobile-web-app-title" content="BrewPiLess">
<meta name="apple-mobile-web-app-capable" content="yes">
<script src="http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js"></script>
<script type="text/javascript" src="bwf.js"></script>
<script>
var Q=function(d){return document.querySelector(d)};var BrewChart=function(a){this.cid=a;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=NaN;this.auxTemp=NaN;this.gravity=NaN;this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(c){this.celius=c;this.ylabel(STR.ChartLabel+'('+(c?"°C":"°F")+')')};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(d){var a=d.getHours();var b=d.getMinutes();var c=d.getSeconds();function T(x){return(x>9)?x:("0"+x)}return d.toLocaleDateString()+" "+T(a)+":"+T(b)+":"+T(c)};BrewChart.prototype.showLegend=function(a,b){var d=new Date(a);Q(".beer-chart-legend-time").innerHTML=this.formatDate(d);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,6));var g=this.chart.getValue(b,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(isNaN(g))?"--":g.toFixed(3);var c=parseInt(this.state[b]);if(!isNaN(c)){Q('.chart-legend-row.state .legend-label').innerHTML=STATES[c].text}};BrewChart.prototype.hideLegend=function(){var v=document.querySelectorAll(".legend-value");v.forEach(function(a){a.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q('.chart-legend-row.state .legend-label').innerHTML="state"};BrewChart.prototype.tempFormat=function(y){var v=parseFloat(y);if(isNaN(v))return"--";var a=this.celius?"°C":"°F";return parseFloat(v).toFixed(2)+a};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(a){this.shownlist[a]=!this.shownlist[a];if(this.shownlist[a]){Q("."+a+" .toggle").style.backgroundColor=Q(".chart-legend-row."+a).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,true)}else{Q("."+a+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,false)}};BrewChart.prototype.createChart=function(){var t=this;t.initLegend();t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var c=document.createElement("div");c.className="hide";document.body.appendChild(c);var d={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:'Temperature',y2label:'Gravity',series:{'gravity':{axis:'y2',drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:'#ccc',gridLineWidth:'0.1px',labelsDiv:c,labelsDivStyles:{'display':'none'},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(y){return t.tempFormat(y)}},y2:{valueFormatter:function(y){return y.toFixed(3)},axisLabelFormatter:function(y){return y.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(e,x,a,b){t.showLegend(x,b)},unhighlightCallback:function(e){t.hideLegend()}};t.chart=new Dygraph(document.getElementById(t.cid),t.data,d)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=['Time','beerSet','beerTemp','fridgeTemp','fridgeSet','roomTemp','auxTemp','gravity'];BrewChart.prototype.addMode=function(m){var s=String.fromCharCode(m);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:s.toUpperCase(),text:BrewChart.Mode[s],attachAtBottom:true})};BrewChart.testData=function(a){if(a[0]!=0xFF)return false;var s=a[1]&0x07;if(s!=4&&s!=6)return false;return{sensor:s,f:a[1]&0x10}};BrewChart.prototype.addResume=function(a){this.ctime+=a;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:'R',text:'Resume',attachAtBottom:true})};BrewChart.prototype.process=function(a){var b=false;var t=this;for(var i=0;i<a.length;){var c=a[i++];var e=a[i++];if(c==0xFF){t.celius=(e&0x10)?false:true;t.numLine=(e&0xF);var p=a[i++];p=p*256+a[i++];t.interval=p;t.starttime=(a[i]<<24)+(a[i+1]<<16)+(a[i+2]<<8)+a[i+3];t.ctime=t.starttime;i+=4;t.data=[];t.anno=[];t.state=[];t.cstate=0;b=true}else if(c==0xF4){t.addMode(e)}else if(c==0xF1){t.cstate=e}else if(c==0xF7){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;t.beerSet=(v==0x7FFF)?NaN:(v/100)}else if(c==0xF8){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;if(e){t.og=v/1000}else{t.gravity=(v==0x7FFF)?NaN:(v/1000);t.sg=t.gravity}}else if(c==0xF9){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;t.auxTemp=(v==0x7FFF)?NaN:(v/100)}else if(c==0xFE){if(t.lidx){var h;for(h=t.lidx;h<t.numLine;h++)t.dataset.push(NaN);t.data.push(t.dataset)}t.lidx=0;t.addResume(e)}else if(c<128){if(t.lidx==0){var d=new Date(this.ctime*1000);var j=t.beerSet;t.dataset=[d,j];t.incTime()}var k=c*256+e;if(k==0x7FFF||k>12000){k=NaN}else{if(t.lidx==t.numLine-1){k=k/1000}else{k=k/100}}t.dataset.push(k);if(++t.lidx>=t.numLine){if(t.numLine==4){t.dataset.push(t.auxTemp);t.dataset.push(t.gravity);t.gravity=NaN}t.lidx=0;t.data.push(t.dataset);t.state.push(t.cstate)}}}if(typeof t.chart=="undefined")t.createChart();else t.chart.updateOptions({'file':t.data});t.chart.setAnnotations(t.anno);return b};var profileEditor={dirty:false,tempUnit:'C',C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(d){this.dirty=d;document.getElementById(this.C_savebtn_Id).innerHTML=(d)?"Save*":"Save";if(d)ControlChart.update(this.chartdata(),this.tempUnit)},getStartDate:function(){return this.sd},setStartDate:function(d){},startDayChange:function(){var a=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(a.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=a;this.sortrow();this.markdirty(true)}},startnow:function(){var d=new Date();document.getElementById(this.C_startday_Id).value=formatDate(d);this.sd=d;this.sortrow();this.markdirty(true)},deleteRow:function(i){var a=document.getElementById("profile_t").getElementsByTagName("tr")[i];a.parentNode.removeChild(a);this.sortrow();this.markdirty(true)},showmenu:function(e,r){var a=r.rowIndex;e.preventDefault();var b=document.createElement("div");b.className="contextmenu";b.innerHTML="Delete!";b.style.top=(e.clientY-1)+"px";b.style.left=(e.clientX-1)+"px";b.onmouseout=function(){b.parentNode.removeChild(b)};var c=this;b.onclick=function(){b.parentNode.removeChild(b);c.deleteRow(a)};var d=document.getElementsByTagName("body")[0];d.appendChild(b)},dayChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;this.sortrow();this.markdirty(true)},tempChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;else this.markdirty(true)},newrow:function(d,t,a){var b=this;var c=document.createElement("tr");var f=document.createElement("td");f.className="pday";f.innerHTML=(typeof d!=="undefined")?d:0;f.contentEditable=true;f.onblur=function(){b.dayChange(this)};f.onfocus=function(){this.saved=this.innerHTML};var g=document.createElement("td");g.className="ptemp";g.innerHTML=(typeof t!=="undefined")?t:20;g.contentEditable=true;g.onblur=function(){b.tempChange(this)};g.onfocus=function(){this.saved=this.innerHTML};var h=document.createElement("td");h.className="pdaystr";h.innerHTML=(a)?a:"";c.appendChild(f);c.appendChild(g);c.appendChild(h);c.oncontextmenu=function(e){b.showmenu(e,this);return false};return c},datestr:function(a){var b=new Date(this.sd.getTime()+Math.round(a*86400)*1000);return formatDate(b)},addRow:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var r=this.newrow();r.className=(b.length%2)?"odd":"even";a.appendChild(r);this.markdirty(true)},init:function(s,d){this.sd=d;document.getElementById(this.C_startday_Id).value=formatDate(d);var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];for(var i=0;i<s.length;i++){var r=this.newrow(s[i].d,s[i].t,this.datestr(s[i].d));r.className=(i%2)?"odd":"even";a.appendChild(r)}},sortrow:function(){var e=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var f=e.getElementsByTagName("tr");var g=Array.prototype.slice.call(f);g.sort(function(a,b){var c=parseFloat(a.querySelector('td.pday').innerHTML);var d=parseFloat(b.querySelector('td.pday').innerHTML);if(isNaN(c)||isNaN(d)||(c===d)){return 0}else{return c-d}});for(var i=0;i<g.length;i++){var h=g[i];var j=h.querySelector('td.pday').innerHTML;h.querySelector('td.pdaystr').innerHTML=this.datestr(parseFloat(j));h.className=(i%2)?"odd":"even";h.parentNode.removeChild(h);e.appendChild(h)}},clear:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c=b.length;for(var i=b.length-1;i>=0;i--){var d=b[i];d.parentNode.removeChild(d)}this.markdirty(true)},getProfile:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c=b.length;var d=0;var e=[];for(var i=0;i<b.length;i++){var f=b[i];var g=parseFloat(f.querySelector('td.pday').innerHTML);var h=parseFloat(f.querySelector('td.ptemp').innerHTML);if(isNaN(g)||isNaN(h))return false;if(h>BrewPiSetting.maxDegree||h<BrewPiSetting.minDegree)return false;if(i==0){if(g!=0)return false}else{if(d==g)return false}d=g;e.push({d:g,t:h})}var s=this.sd.toISOString();return{s:s,u:this.tempUnit,t:e}},chartdata:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c=b.length;var d=0;var e=[];for(var i=0;i<b.length;i++){var f=b[i];var g=parseFloat(f.querySelector('td.pday').innerHTML);var h=parseFloat(f.querySelector('td.ptemp').innerHTML);if(isNaN(g)||isNaN(h))return false;if(h>BrewPiSetting.maxDegree||h<BrewPiSetting.minDegree)return false;if(i==0){if(g!=0)return false}else{if(d==g)return false}d=g;var j=new Date(this.sd.getTime()+g*86400000);e.push([j,h])}return e},initProfile:function(p){var a=new Date(p.s);this.tempUnit=p.u;profileEditor.init(p.t,a)},setTempUnit:function(u){if(u==this.tempUnit)return;this.tempUnit=u;var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");for(var i=0;i<b.length;i++){var c=b[i].querySelector('td.ptemp');var d=parseFloat(c.innerHTML);c.innerHTML=(u=='C')?F2C(d):C2F(d)}}};var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:'C'};var ControlChart={unit:"C",init:function(a,b,c){var t=this;t.data=(b===false)?[]:b;t.unit=c;var e=function(v){d=new Date(v);return(d.getMonth()+1)+"/"+d.getDate()};var f=function(v){return v.toFixed(1)+"&deg;"+t.unit};t.chart=new Dygraph(document.getElementById(a),t.data,{colors:['rgb(89, 184, 255)'],axisLabelFontSize:12,gridLineColor:'#ccc',gridLineWidth:'0.1px',labels:["Time","Temperature"],labelsDiv:document.getElementById(a+"-label"),legend:'always',labelsDivStyles:{'textAlign':'right'},strokeWidth:1,axes:{y:{valueFormatter:f,pixelsPerLabel:20,axisLabelWidth:35},x:{axisLabelFormatter:e,valueFormatter:e,pixelsPerLabel:30,axisLabelWidth:40}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},})},update:function(a,b){if(a===false)return;if(a.length==0)return;this.data=a;this.unit=b;this.chart.updateOptions({'file':this.data})}};function formatDate(a){var y=a.getFullYear();var M=a.getMonth()+1;var d=a.getDate();var h=a.getHours();var m=a.getMinutes();function dd(n){return(n<10)?'0'+n:n}return dd(M)+"/"+dd(d)+" "+dd(h)+":"+dd(m)}var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(m){var d=document.getElementById(m+"-m");var a=document.getElementById(m+"-m").className.replace(/\snav-selected/,'');d.className=a;document.getElementById(m+"-s").style.display="none"},select:function(m){document.getElementById(m+"-m").className+=' nav-selected';document.getElementById(m+"-s").style.display="block"},init:function(){var b=this;if(b.initiated)return;b.initiated=true;for(var i=0;i<4;i++){var m=b.modes[i];document.getElementById(m+"-s").style.display="none";document.getElementById(m+"-m").onclick=function(){var a=this.id.replace(/-m/,'');b.dselect(b.cmode);b.select(a);b.cmode=a}}b.cmode="profile";b.select(b.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var v=document.getElementById(this.cmode+"-t").value;if(v==''||isNaN(v)||(v>BrewPiSetting.maxDegree||v<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+v);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+v+"}");BWF.send("j{mode:b, beerSet:"+v+"}")}else{console.log("j{mode:f, fridgeSet:"+v+"}");BWF.send("j{mode:f, fridgeSet:"+v+"}")}}else if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}};function saveprofile(){console.log("save");var r=profileEditor.getProfile();if(r===false){alert("invalid value. check again");return}var a=JSON.stringify(r);console.log("result="+a);BWF.save(BWF.BrewProfile,a,function(){profileEditor.markdirty(false);alert("Done.")},function(e){alert("save failed:"+e)})}function C2F(c){return Math.round((c*1.8+32)*10)/10}function F2C(f){return Math.round((f-32)/1.8*10)/10}function updateTempUnit(u){var a=document.getElementsByClassName("t_unit");for(var i=0;i<a.length;i++){a[i].innerHTML=u}}function openDlgLoading(){document.getElementById('dlg_loading').style.display="block"}function closeDlgLoading(){document.getElementById('dlg_loading').style.display="none"}function onload(c){modekeeper.init();openDlgLoading();function complete(){var b=true;function initComp(){if(!b)return;b=false;closeDlgLoading();c()}invoke({url:"/tcc",m:"GET",fail:function(a){console.log("error connect to BrwePiLess!");initComp()},success:function(d){var s=JSON.parse(d);var a={valid:true,minDegree:s.tempSetMin,maxDegree:s.tempSetMax,tempUnit:s.tempFormat};if(a.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(a.tempUnit);profileEditor.setTempUnit(a.tempUnit)}BrewPiSetting=a;initComp()}})}BWF.load(BWF.BrewProfile,function(d){var p=JSON.parse(d);updateTempUnit(p.u);BrewPiSetting.tempUnit=p.u;profileEditor.initProfile(p);ControlChart.init("tc_chart",profileEditor.chartdata(),p.u);complete()},function(e){profileEditor.initProfile();ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);complete()})}function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}function getLogName(){s_ajax({url:"loglist.php",m:"GET",success:function(d){var r=JSON.parse(d);if(r.rec){Q("#recording").innerHTML=r.log}else{Q("#recording").innerHTML=""}},fail:function(d){alert("failed:"+d)}})}var BChart={offset:0,url:'chart.php',toggle:function(a){this.chart.toggleLine(a)},reqdata:function(){var t=this;var c='offset='+t.offset;if(typeof t.startOff!="undefined"&&t.startOff!==null)c=c+"&index="+t.startOff;var d=new XMLHttpRequest();d.open('GET',t.url+'?'+c);d.timeout=5000;d.ontimeout=function(e){console.error("Timeout!!")};d.responseType='arraybuffer';d.onload=function(e){if(this.status==404){console.log("Error getting log data");return}var a=new Uint8Array(this.response);if(a.length==0){console.log("zero content");if(t.timer)clearInterval(t.timer);t.timer=null;setTimeout(function(){t.reqdata()},3000);return}var b=t.chart.process(a);if(b){t.offset=a.length;t.startOff=d.getResponseHeader("LogOffset");getLogName();console.log("new chart, offset="+t.startOff)}else t.offset+=a.length;if(!isNaN(t.chart.og)){updateOriginGravity(t.chart.og);t.chart.og=NaN}if(!isNaN(t.chart.sg)){updateGravity(t.chart.sg);t.chart.sg=NaN}if(t.timer==null)t.settimer()};d.onerror=function(){console.log("error getting data.");setTimeout(function(){t.reqdata()},10000)};d.send()},settimer:function(){var t=this;t.timer=setInterval(function(){t.reqdata()},t.chart.interval*1000)},init:function(a){this.chart=new BrewChart(a)},timer:null,start:function(){if(this.running)return;this.running=true;this.offset=0;this.reqdata()},reqnow:function(){var t=this;if(t.timer)clearInterval(t.timer);t.timer=null;t.reqdata()}};function setLcdText(a,b){var d=document.getElementById(a);d.innerHTML=b}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function gravityDevice(a){if(typeof a["name"]=="undefined")return;if(a.name.startsWith("iSpindel")){if(typeof window.iSpindel=="undefined"){window.iSpindel=true;Q("#iSpindel-pane").style.display="block"}Q("#iSpindel-name").innerHTML=a.name;if(typeof a["battery"]!="undefined")Q("#iSpindel-battery").innerHTML=a.battery;var b;if(typeof a["lu"]!="undefined")b=new Date(a.lu*1000);else b=new Date();Q("#iSpindel-last").innerHTML=formatDate(b);if(typeof a["gravityP"]!="undefined")updateGravity(BrewMath.pla2sg(a["gravityP"]))}}function init(){BChart.init("div_g");var b=true;onload(function(){BWF.init({onconnect:function(){BWF.send("l");setInterval(function(){if(!b)controllerError();BWF.send("l");b=false},5000)},error:function(e){console.log("error");communicationError()},handlers:{L:function(a){b=true;for(var i=0;i<4;i++)setLcdText("lcd-line-"+i,a[i])},V:function(c){console.log("forced reload chart");BChart.reqnow()},G:function(c){gravityDevice(c)}}});BChart.start()})}var BrewMath={abv:function(a,b){return((76.08*(a-b)/(1.775-a))*(b/0.794)).toFixed(1)},att:function(a,b){return Math.round((a-b)/(a-1)*100)},sg2pla:function(a){return-616.868+1111.14*a-630.272*a*a+135.997*a*a*a},pla2sg:function(a){return 1+(a/(258.6-((a/258.2)*227.1)))}};function updateGravity(a){window.sg=a;Q("#gravity-sg").innerHTML=a.toFixed(3);if(typeof window.og!="undefined"){Q("#gravity-att").innerHTML=BrewMath.att(window.og,a);Q("#gravity-abv").innerHTML=BrewMath.abv(window.og,a)}}function updateOriginGravity(a){if(typeof window.og!="undefined"&&window.og==a)return;window.og=a;Q("#gravity-og").innerHTML=a.toFixed(3);if(typeof window.sg!="undefined")updateGravity(window.sg)}function showgravitydlg(a){Q('#dlg_addgravity .msg').innerHTML=a;Q('#dlg_addgravity').style.display="block"}function dismissgravity(){Q('#dlg_addgravity').style.display="none"}function inputgravity(){dismissgravity();openDlgLoading();var a=parseFloat(Q("#dlg_addgravity input").value);if(window.isog)updateOriginGravity(a);else updateGravity(a);var b={name:"webjs",gravity:a};if(window.isog)b.og=1;s_ajax({url:"gravity",m:"POST",mime:"application/json",data:JSON.stringify(b),success:function(d){closeDlgLoading()},fail:function(d){alert("failed:"+d);closeDlgLoading()}})}function gravity(){window.isog=false;showgravitydlg("Add gravity Record:")}function origingravity(){window.isog=true;showgravitydlg("Set Original Gravity:")}
</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}.frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px}#top-frame{height:520px}#bottom-frame{height:380px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}.contextmenu{display:block;position:absolute;background:#D00;color:#fff;font-weight:bold;padding:.5em}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}#recording{color:lightblue;font-size:18px}#top-frame button{float:right;width:200px;margin-top:3px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:4em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em;border-top-right-radius:5px;border-top-left-radius:5px}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:965px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}#bottom-frame button{color:#1d5987;background:#dfeffc;font-weight:bold;border-radius:6px;margin:4px}#addbutton{float:right;width:120px}#header{width:968px}#clearbtn{width:18%}#savebtn{width:25%;float:right}#delbtn{width:18%}.modal{display:none;position:fixed;z-index:100;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:10px;border:1px solid #888;width:320px;height:100px;border-radius:8px}#dlg_addgravity button{float:right;width:56px;margin-top:2px}#modekeeper-apply{margin-top:0}#profile-edit{float:left;margin:6px;width:480px}#tc_chart{width:430px;height:280px;float:right;margin:6px}#gravity-pane table,#iSpindel-pane table{width:100px;float:right;border:1px solid white;border-radius:5px;margin-right:10px;color:cyan;font-size:12px}#iSpindel-pane{display:none}</style>
</head>
<body onload=init()>
<div class="frame" id="top-frame">
<div id="topbar">
<div id="lcd" class="lcddisplay"><span class="lcd-text">
<span class="lcd-line" id="lcd-line-0">Live LCD waiting</span>
<span class="lcd-line" id="lcd-line-1">for update from</span>
<span class="lcd-line" id="lcd-line-2">script...</span>
<span class="lcd-line" id="lcd-line-3"></span></p><p>
</div>
<div id="banner">BrewPiLess v1.6
<div id="recording"></div>
</div>
<div id="menu">
<button onclick="window.open('/log')">Data Logging</button>
<br>
<button onclick="window.open('/setup.htm')">Device Setup</button>
<br>
<button onclick="window.open('/config')">System Config</button>
<br>
<button onclick="window.open('/gdc')">Gravity Sensor</button>
</div>
<div id="gravity-pane">
<table>
<tr><th>OG:</th><td> <span id="gravity-og" onclick="origingravity()">--</span><td></tr>
<tr><th>SG:</th><td> <span id="gravity-sg" onclick="gravity()">--</span><td></tr>
<tr><th>ATT:</th><td> <span id="gravity-att"> --</span>%<td></tr>
<tr><th>ABV:</th><td> <span id="gravity-abv"> --</span>%<td></tr>
</table>
</div>
<div id="iSpindel-pane">
<table>
<tr><th colspan="2"><span id="iSpindel-name"></span></th></tr>
<tr><th>Battery</th><td> <span id="iSpindel-battery">--</span><td></tr>
<tr><th colspan="2">Last Update</th></tr>
<tr><td colspan="2" align="center"> <span id="iSpindel-last">--</span><td></tr>
</table>
</div>
</div>
<div id="chart-container">
<div id="div_g"></div>
<div id="chart-legend" class="chart-legend">
<div class="chart-legend-row time">
<div class="beer-chart-legend-time">Date/Time</div>
</div>
<div class="chart-legend-row beerTemp">
<div class="toggle beerTemp" onclick="BChart.toggle('beerTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('beerTemp')">Beer Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row beerSet">
<div class="toggle beerSet" onclick="BChart.toggle('beerSet')"></div>
<div class="legend-label" onclick="BChart.toggle('beerSet')">Beer Set</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row fridgeTemp">
<div class="toggle beerSet" onclick="BChart.toggle('fridgeTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('fridgeTemp')">Fridge Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row fridgeSet">
<div class="toggle beerSet" onclick="BChart.toggle('fridgeSet')"></div>
<div class="legend-label" onclick="BChart.toggle('fridgeSet')">Fridge Set</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row roomTemp">
<div class="toggle beerSet" onclick="BChart.toggle('roomTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('roomTemp')">Room Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row auxTemp">
<div class="toggle gravity" onclick="BChart.toggle('auxTemp')"></div>
<div class="legend-label" onclick="BChart.toggle('auxTemp')">Aux Temp</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row gravity">
<div class="toggle gravity" onclick="BChart.toggle('gravity')"></div>
<div class="legend-label" onclick="BChart.toggle('gravity')">Gravity</div>
<div class="legend-value">--</div>
<br>
</div>
<div class="chart-legend-row state">
<div class="legend-label">state</div>
<br>
</div>
</div>
</div>
</div>
<div class="frame" id="bottom-frame">
<div class="navbar" id="header">
<div id="set-mode-text">Set temperature mode:</div>
<div class="navitems">
<span class="navitem" id="profile-m">Beer profile</span>
<span class="navitem" id="beer-m">Beer Const.</span>
<span class="navitem" id="fridge-m">Fridge Const.</span>
<span class="navitem" id="off-m">Off</span>
<button id="modekeeper-apply" onclick="modekeeper.apply()">Apply</button>
</div>
</div>
<div id="containter">
<div id="profile-s" class="detail">
<div id="profile-edit">
<div>
<div><span>Start Date:</span><input type="text" size="16" id="startdate" onchange="profileEditor.startDayChange()">
<button id="setnow" onclick=profileEditor.startnow()>Now</button>
<button id="addbutton" onclick="profileEditor.addRow()">Add</button></div>
</div>
<table class="profileTable" id="profile_t">
<thead><tr><th>Day</th><th>Temperature(&deg;<span class="t_unit">C</span>)</th><th>Date and Time</th></tr></thead>
<tbody></tbody>
</table>
<div><button id="clearbtn" onclick="profileEditor.clear()">Clear</button><button id="savebtn" onclick="saveprofile()">Save</button></div>
</div>
<div id="tc_chart"></div>
</div>
<div id="beer-s" class="detail">
Set Beer temp:
<input type="text" size="6" id="beer-t"></input>&deg;<span class="t_unit">C</span>
</div>
<div id="fridge-s" class="detail">
Set Fridge temp:
<input type="text" size="6" id="fridge-t"></input>&deg;<span class="t_unit">C</span>
</div>
<div id="off-s" class="detail">Turning temperature controll Off.</div>
</div>
<div id="dlg_loading" class="modal">
<div class="modal-content">
<p>Communicating with BrewPiLess controller..</p>
</div>
</div>
<div id="dlg_addgravity" class="modal">
<div class="modal-content">
<p><span class="msg"></span><input type="text" size="6" value="1.0">
<button onclick="dismissgravity()">Cancel</button>
<button onclick="inputgravity()">OK</button>
</p>
</div>
</div>
</div>
</body>
</html>
)END";

#endif



