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
var Q=function(d){return document.querySelector(d)};var BrewChart=function(a){this.cid=a;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=null};BrewChart.prototype.setCelius=function(c){this.celius=c;this.ylabel(STR.ChartLabel+'('+(c?"°C":"°F")+')')};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(d){var a=d.getHours();var b=d.getMinutes();var c=d.getSeconds();function T(x){return(x>9)?x:("0"+x)}return d.toLocaleDateString()+" "+T(a)+":"+T(b)+":"+T(c)};BrewChart.prototype.showLegend=function(a,b){var d=new Date(a);Q(".beer-chart-legend-time").innerHTML=this.formatDate(d);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,5));var c=parseInt(this.state[b]);if(!isNaN(c)){Q('.chart-legend-row.state .legend-label').innerHTML=STATES[c].text}};BrewChart.prototype.hideLegend=function(){var v=document.querySelectorAll(".legend-value");v.forEach(function(a){a.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q('.chart-legend-row.state .legend-label').innerHTML="state"};BrewChart.prototype.tempFormat=function(y){var v=parseFloat(y);if(isNaN(v))return"--";var a=this.celius?"°C":"°F";return parseFloat(v).toFixed(2)+a};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(a){this.shownlist[a]=!this.shownlist[a];if(this.shownlist[a]){Q("."+a+" .toggle").style.backgroundColor=Q(".chart-legend-row."+a).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,true)}else{Q("."+a+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,false)}};BrewChart.prototype.createChart=function(){var t=this;t.initLegend();t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true};var c=document.createElement("div");c.className="hide";document.body.appendChild(c);t.chart=new Dygraph(document.getElementById(t.cid),t.data,{labels:BrewChart.Labels,colors:BrewChart.Colors,axisLabelFontSize:12,animatedZooms:true,gridLineColor:'#ccc',gridLineWidth:'0.1px',labelsDiv:c,labelsDivStyles:{'display':'none'},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(y){t.tempFormat(y)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(e,x,a,b){t.showLegend(x,b)},unhighlightCallback:function(e){t.hideLegend()}})};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","rgb(153,0,153)"];BrewChart.Labels=['Time','beerSet','beerTemp','fridgeTemp','fridgeSet','roomTemp'];BrewChart.prototype.addMode=function(m){var s=String.fromCharCode(m);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:s.toUpperCase(),text:BrewChart.Mode[s],attachAtBottom:true})};BrewChart.testData=function(a){if(a[0]!=0xFF)return false;var s=a[1]&0x07;if(s>5)return false;return{sensor:s,f:a[1]&0x10}};BrewChart.prototype.addResume=function(a){this.ctime+=a;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:'R',text:'Resume',attachAtBottom:true})};BrewChart.prototype.process=function(a){var b=false;var t=this;for(var i=0;i<a.length;){var c=a[i++];var e=a[i++];if(c==0xFF){t.celius=(e&0x10)?false:true;var p=a[i++];p=p*256+a[i++];t.interval=p;t.starttime=(a[i]<<24)+(a[i+1]<<16)+(a[i+2]<<8)+a[i+3];t.ctime=t.starttime;i+=4;t.data=[];t.anno=[];t.state=[];t.cstate=0;b=true}else if(c==0xF4){t.addMode(e)}else if(c==0xF1){t.cstate=e}else if(c==0xF7){var f=a[i++];var g=a[i++];var v=(f&0x7F)*256+g;t.beerSet=(v==0x7FFF)?null:(v/100)}else if(c==0xFE){if(t.lidx){var h;for(h=t.lidx;h<t.numLine;h++)t.dataset.push(NaN);t.data.push(t.dataset)}t.lidx=0;t.addResume(e)}else if(c<128){if(t.lidx==0){var d=new Date(this.ctime*1000);var j=t.beerSet;t.dataset=[d,j];t.incTime()}var k=c*256+e;if(k==0x7FFF||k>12000){k=NaN}else{k=k/100}t.dataset.push(k);if(++t.lidx>=t.numLine){t.lidx=0;t.data.push(t.dataset);t.state.push(t.cstate)}}}if(typeof t.chart=="undefined")t.createChart();else t.chart.updateOptions({'file':t.data});t.chart.setAnnotations(t.anno);return b};var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:'C'};var ControlChart={process:function(a){var t=this;t.data=[];if(typeof a=="undefined"){t.unit="C";return}var b=new Date(a.s);t.unit=a.u;for(var i=0;i<a.t.length;i++){var c=new Date(b.getTime()+Math.round(a.t[i].d*86400)*1000);this.data.push([c,a.t[i].t])}},init:function(a,b){var t=this;t.process(b);var c=function(v){d=new Date(v);return(d.getMonth()+1)+"/"+d.getDate()};var e=function(t){return t.toFixed(1)+"&deg;"+t.unit};t.chart=new Dygraph(document.getElementById(a),t.data,{colors:['rgb(89, 184, 255)'],axisLabelFontSize:12,gridLineColor:'#ccc',gridLineWidth:'0.1px',labels:["Time","Temperature"],labelsDiv:document.getElementById(a+"-label"),legend:'always',labelsDivStyles:{'textAlign':'right'},strokeWidth:1,axes:{y:{valueFormatter:e,pixelsPerLabel:20,axisLabelWidth:35},x:{axisLabelFormatter:c,valueFormatter:c,pixelsPerLabel:30,axisLabelWidth:40}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},})},update:function(p){this.process(p);this.chart.updateOptions({'file':this.data})}};function formatDate(a){var y=a.getFullYear();var M=a.getMonth()+1;var d=a.getDate();var h=a.getHours();var m=a.getMinutes();var s=a.getSeconds();function dd(n){return(n<10)?'0'+n:n}return dd(M)+"/"+dd(d)+"/"+y+" "+dd(h)+":"+dd(m)+":"+dd(s)}var profileEditor={dirty:false,tempUnit:'C',C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(d){this.dirty=d;document.getElementById(this.C_savebtn_Id).innerHTML=(d)?"Save*":"Save"},getStartDate:function(){return this.sd},setStartDate:function(d){},startDayChange:function(){var a=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(a.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=a;this.sortrow();this.markdirty(true)}},startnow:function(){var d=new Date();document.getElementById(this.C_startday_Id).value=formatDate(d);this.sd=d;this.sortrow();this.markdirty(true)},deleteRow:function(i){var a=document.getElementById("profile_t").getElementsByTagName("tr")[i];a.parentNode.removeChild(a);this.sortrow();this.markdirty(true)},showmenu:function(e,r){var a=r.rowIndex;e.preventDefault();var b=document.createElement("div");b.className="contextmenu";b.innerHTML="Delete!";b.style.top=(e.clientY-1)+"px";b.style.left=(e.clientX-1)+"px";b.onmouseout=function(){b.parentNode.removeChild(b)};var c=this;b.onclick=function(){b.parentNode.removeChild(b);c.deleteRow(a)};var d=document.getElementsByTagName("body")[0];d.appendChild(b)},dayChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;this.sortrow();this.markdirty(true)},tempChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML))a.innerHTML=a.saved;else{this.markdirty(true);ControlChart.update(this.getProfile())}},newrow:function(d,t,a){var b=this;var c=document.createElement("tr");var f=document.createElement("td");f.className="pday";f.innerHTML=(typeof d!=="undefined")?d:0;f.contentEditable=true;f.onblur=function(){b.dayChange(this)};f.onfocus=function(){this.saved=this.innerHTML};var g=document.createElement("td");g.className="ptemp";g.innerHTML=(typeof t!=="undefined")?t:20;g.contentEditable=true;g.onblur=function(){b.tempChange(this)};g.onfocus=function(){this.saved=this.innerHTML};var h=document.createElement("td");h.className="pdaystr";h.innerHTML=(a)?a:"";c.appendChild(f);c.appendChild(g);c.appendChild(h);c.oncontextmenu=function(e){b.showmenu(e,this);return false};return c},datestr:function(a){var b=new Date(this.sd.getTime()+Math.round(a*86400)*1000);return formatDate(b)},addRow:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var r=this.newrow();r.className=(b.length%2)?"odd":"even";a.appendChild(r);this.markdirty(true)},init:function(s,d){this.sd=d;document.getElementById(this.C_startday_Id).value=formatDate(d);var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];for(var i=0;i<s.length;i++){var r=this.newrow(s[i].d,s[i].t,this.datestr(s[i].d));r.className=(i%2)?"odd":"even";a.appendChild(r)}},sortrow:function(){var e=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var f=e.getElementsByTagName("tr");var g=Array.prototype.slice.call(f);g.sort(function(a,b){var c=parseFloat(a.querySelector('td.pday').innerHTML);var d=parseFloat(b.querySelector('td.pday').innerHTML);if(isNaN(c)||isNaN(d)||(c===d)){return 0}else{return c-d}});for(var i=0;i<g.length;i++){var h=g[i];var j=h.querySelector('td.pday').innerHTML;h.querySelector('td.pdaystr').innerHTML=this.datestr(parseFloat(j));h.className=(i%2)?"odd":"even";h.parentNode.removeChild(h);e.appendChild(h)}ControlChart.update(this.getProfile())},clear:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c=b.length;for(var i=b.length-1;i>=0;i--){var d=b[i];d.parentNode.removeChild(d)}this.markdirty(true)},getProfile:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");var c=b.length;var d=0;var e=[];for(var i=0;i<b.length;i++){var f=b[i];var g=parseFloat(f.querySelector('td.pday').innerHTML);var h=parseFloat(f.querySelector('td.ptemp').innerHTML);if(isNaN(g)||isNaN(h))return false;if(h>BrewPiSetting.maxDegree||h<BrewPiSetting.minDegree)return false;if(i==0){if(g!=0)return false}else{if(d==g)return false}d=g;e.push({d:g,t:h})}var s=this.sd.toISOString();return{s:s,u:this.tempUnit,t:e}},initProfile:function(p){var a=new Date(p.s);this.tempUnit=p.u;profileEditor.init(p.t,a)},setTempUnit:function(u){if(u==this.tempUnit)return;this.tempUnit=u;var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var b=a.getElementsByTagName("tr");for(var i=0;i<b.length;i++){var c=b[i].querySelector('td.ptemp');var d=parseFloat(c.innerHTML);c.innerHTML=(u=='C')?F2C(d):C2F(d)}}};var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(m){var d=document.getElementById(m+"-m");var a=document.getElementById(m+"-m").className.replace(/\snav-selected/,'');d.className=a;document.getElementById(m+"-s").style.display="none"},select:function(m){document.getElementById(m+"-m").className+=' nav-selected';document.getElementById(m+"-s").style.display="block"},init:function(){var b=this;if(b.initiated)return;b.initiated=true;for(var i=0;i<4;i++){var m=b.modes[i];document.getElementById(m+"-s").style.display="none";document.getElementById(m+"-m").onclick=function(){var a=this.id.replace(/-m/,'');b.dselect(b.cmode);b.select(a);b.cmode=a}}b.cmode="profile";b.select(b.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var v=document.getElementById(this.cmode+"-t").value;if(v==''||isNaN(v)||(v>BrewPiSetting.maxDegree||v<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+v);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+v+"}");BWF.send("j{mode:b, beerSet:"+v+"}")}else{console.log("j{mode:f, fridgeSet:"+v+"}");BWF.send("j{mode:f, fridgeSet:"+v+"}")}}else if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}};function saveprofile(){console.log("save");var r=profileEditor.getProfile();if(r===false){alert("invalid value. check again");return}var a=JSON.stringify(r);console.log("result="+a);BWF.save(BWF.BrewProfile,a,function(){profileEditor.markdirty(false);alert("Done.")},function(e){alert("save failed:"+e)})}function C2F(c){return Math.round((c*1.8+32)*10)/10}function F2C(f){return Math.round((f-32)/1.8*10)/10}function updateTempUnit(u){var a=document.getElementsByClassName("t_unit");for(var i=0;i<a.length;i++){a[i].innerHTML=u}}function openDlgLoading(){document.getElementById('dlg_loading').style.display="block"}function closeDlgLoading(){document.getElementById('dlg_loading').style.display="none"}function onload(){modekeeper.init();openDlgLoading();function complete(){var t;var b=true;function initComp(){if(!b)return;b=false;clearTimeout(t);closeDlgLoading()}invoke({url:"/tcc",m:"GET",fail:function(a){console.log("error connect to BrwePiLess!");initComp()},success:function(d){var s=JSON.parse(d);var a={valid:true,minDegree:s.tempSetMin,maxDegree:s.tempSetMax,tempUnit:s.tempFormat};if(a.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(a.tempUnit);profileEditor.setTempUnit(a.tempUnit)}BrewPiSetting=a;initComp()}})}BWF.load(BWF.BrewProfile,function(d){var p=JSON.parse(d);updateTempUnit(p.u);BrewPiSetting.tempUnit=p.u;profileEditor.initProfile(p);ControlChart.init("tc_chart",p);complete()},function(e){profileEditor.init([],new Date());ControlChart.init("tc_chart");complete()})}function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}function getLogName(){s_ajax({url:"loglist.php",m:"GET",success:function(d){var r=JSON.parse(d);if(r.rec){Q("#recording").innerHTML=r.log}else{Q("#recording").innerHTML=""}},fail:function(d){alert("failed:"+e)}})}var BChart={offset:0,url:'chart.php',toggle:function(a){this.chart.toggleLine(a)},reqdata:function(){var t=this;var c='offset='+t.offset;if(typeof t.startOff!="undefined"&&t.startOff!==null)c=c+"&index="+t.startOff;var d=new XMLHttpRequest();d.open('GET',t.url+'?'+c);d.responseType='arraybuffer';d.onload=function(e){if(this.status==404){console.log("Error getting log data");return}var a=new Uint8Array(this.response);if(a.length==0){console.log("zero content");if(t.timer)clearInterval(t.timer);t.timer=null;setTimeout(function(){t.reqdata()},3000);return}var b=t.chart.process(a);if(b){t.offset=a.length;t.startOff=d.getResponseHeader("LogOffset");getLogName();console.log("new chart, offset="+t.startOff)}else t.offset+=a.length;if(t.timer==null)t.settimer()};d.onerror=function(){console.log("error getting data.");setTimeout(function(){t.reqdata()},10000)};d.send()},settimer:function(){var t=this;t.timer=setInterval(function(){t.reqdata()},t.chart.interval*1000)},init:function(a){this.chart=new BrewChart(a)},timer:null,start:function(){if(this.running)return;this.running=true;this.offset=0;this.reqdata()},reqnow:function(){var t=this;if(t.timer)clearInterval(t.timer);t.timer=null;t.reqdata()}};function setLcdText(a,b){var d=document.getElementById(a);d.innerHTML=b}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function init(){var b=true;BWF.init({error:function(e){console.log("error");communicationError()},handlers:{L:function(a){b=true;for(var i=0;i<4;i++)setLcdText("lcd-line-"+i,a[i])},V:function(c){console.log("forced reload chart");BChart.reqnow()}}});setInterval(function(){if(!b)controllerError();BWF.send("l");b=false},5000);BWF.send("l");BChart.init("div_g");BChart.start();onload()}
</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}.frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px}#top-frame{height:520px}#bottom-frame{height:380px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}button{float:right}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}#recording{color:lightblue;font-size:18px}button{width:200px;margin-top:5px}// control .corner-top{border-top-right-radius:5px;border-top-left-radius:5px}.corner-bottom{border-bottom-right-radius:5px;border-bottom-left-radius:5px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:3.5em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:965px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}button{color:#1d5987;background:#dfeffc;font-weight:bold;border-top-right-radius:5px;border-top-left-radius:5px;border-bottom-right-radius:5px;border-bottom-left-radius:5px}#addbutton{float:right;width:36%;margin:5px 1% 5px 1%}.contextmenu{display:block;position:absolute;background:#D00;color:#fff;font-weight:bold;padding:.5em}#header{width:965px}#clearbtn{width:48%}#savebtn{width:48%}.modal{display:none;position:fixed;z-index:1;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:20px;border:1px solid #888;width:80%}#modekeeper-apply{margin-top:0}#profile-edit{float:left;margin:6px;width:420px}#tc_chart{width:430px;height:280px;float:right;margin:6px}</style>
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
<div id="banner">BrewPiLess v1.2
<div id="recording">
</div>
</div>
<div id="menu">
<button onclick="window.open('/log')">Data Logging</button>
<br>
<button onclick="window.open('/setup.htm')">Device Setup</button>
<br>
<button onclick="window.open('/config')">System Config</button>
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
<div class="chart-legend-row state">
<div class="legend-label">state</div>
<br>
</div>
</div>
</div>
</div>
<div class="frame" id="bottom-frame">
<div class="navbar corner-top corner-bottom" id="header">
<div id="set-mode-text">Set temperature mode:</div>
<div class="navitems">
<span class="navitem corner-top" id="profile-m">Beer profile</span>
<span class="navitem corner-top" id="beer-m">Beer Const.</span>
<span class="navitem corner-top" id="fridge-m">Fridge Const.</span>
<span class="navitem corner-top" id="off-m">Off</span>
<button id="modekeeper-apply" class="corner-top corner-bottom" onclick="modekeeper.apply()">Apply</button>
</div>
</div>
<div id="containter">
<div id="profile-s" class="detail">
<div id="profile-edit">
<div>
<div><span>Start Date:</span><input type="text" size="40" id="startdate" onchange="profileEditor.startDayChange()">
<button id="setnow" onclick=profileEditor.startnow()>Now</button></div>
<button id="addbutton" onclick="profileEditor.addRow()">Add</button>
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
<p>Loading setting from BrewPi controller..</p>
</div>
</div>
</div>
</body>
</html>
)END";





















































































