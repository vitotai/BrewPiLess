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
<script>/*<![CDATA[*/function s_ajax(a){var e=new XMLHttpRequest();e.onreadystatechange=function(){if(e.readyState==4){if(e.status==200){a.success(e.responseText)}else{e.onerror(e.status)}}};e.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{e.onerror(-1)}},e.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};e.open(a.m,a.url,true);if(typeof a.data!="undefined"){e.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");e.send(a.data)}else{e.send()}}var Q=function(a){return document.querySelector(a)};var BrewChart=function(b){this.cid=b;this.ctime=0;this.interval=60;this.numLine=7;this.lidx=0;this.celius=true;this.clearData()};BrewChart.prototype.clearData=function(){this.laststat=[NaN,NaN,NaN,NaN,NaN,NaN,NaN];this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(a){this.celius=a;this.ylabel(STR.ChartLabel+"("+(a?"°C":"°F")+")")};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(h){var f=h.getHours();var e=h.getMinutes();var i=h.getSeconds();function g(a){return(a>9)?a:("0"+a)}return h.toLocaleDateString()+" "+g(f)+":"+g(e)+":"+g(i)};BrewChart.prototype.showLegend=function(f,e){var i=new Date(f);Q(".beer-chart-legend-time").innerHTML=this.formatDate(i);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,6));var h=this.chart.getValue(e,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(!h||isNaN(h))?"--":h.toFixed(3);var j=parseInt(this.state[e]);if(!isNaN(j)){Q(".chart-legend-row.state .legend-label").innerHTML=STATES[j].text}};BrewChart.prototype.hideLegend=function(){var a=document.querySelectorAll(".legend-value");a.forEach(function(b){b.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q(".chart-legend-row.state .legend-label").innerHTML="state"};BrewChart.prototype.tempFormat=function(e){var c=parseFloat(e);if(isNaN(c)){return"--"}var b=this.celius?"°C":"°F";return parseFloat(c).toFixed(2)+b};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(b){this.shownlist[b]=!this.shownlist[b];if(this.shownlist[b]){Q("."+b+" .toggle").style.backgroundColor=Q(".chart-legend-row."+b).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,true)}else{Q("."+b+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,false)}};BrewChart.prototype.createChart=function(){var a=this;a.initLegend();a.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var e=document.createElement("div");e.className="hide";document.body.appendChild(e);var b={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:"Temperature",y2label:"Gravity",series:{gravity:{axis:"y2",drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:"#ccc",gridLineWidth:"0.1px",labelsDiv:e,labelsDivStyles:{display:"none"},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(c){return a.tempFormat(c)}},y2:{valueFormatter:function(c){return c.toFixed(3)},axisLabelFormatter:function(c){return c.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(h,f,g,c){a.showLegend(f,c)},unhighlightCallback:function(c){a.hideLegend()}};a.chart=new Dygraph(document.getElementById(a.cid),a.data,b)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=["Time","beerSet","beerTemp","fridgeTemp","fridgeSet","roomTemp","auxTemp","gravity"];BrewChart.prototype.addMode=function(a){var b=String.fromCharCode(a);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:b.toUpperCase(),text:BrewChart.Mode[b],attachAtBottom:true})};BrewChart.testData=function(b){if(b[0]!=255){return false}var c=b[1]&7;if(c!=5){return false}return{sensor:c,f:b[1]&16}};BrewChart.prototype.addResume=function(b){this.ctime+=b;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:"R",text:"Resume",attachAtBottom:true})};BrewChart.prototype.process=function(x){var w=false;var z=this;for(var m=0;m<x.length;){var u=x[m++];var r=x[m++];if(u==255){if((r&15)!=5){alert("log version mismatched!");return}z.celius=(r&16)?false:true;var k=x[m++];k=k*256+x[m++];z.interval=k;z.starttime=(x[m]<<24)+(x[m+1]<<16)+(x[m+2]<<8)+x[m+3];z.ctime=z.starttime;m+=4;z.data=[];z.anno=[];z.state=[];z.cstate=0;this.clearData();w=true}else{if(u==244){z.addMode(r)}else{if(u==241){z.cstate=r}else{if(u==254){if(z.lidx){var q;for(q=z.lidx;q<z.numLine;q++){z.dataset.push(NaN)}z.data.push(z.dataset)}z.lidx=0;z.addResume(r)}else{if(u==248){var o=x[m++];var n=x[m++];var y=(o&127)*256+n;z.og=y/1000}else{if(u==240){z.changes=r;z.lidx=0;var s=new Date(this.ctime*1000);z.incTime();z.dataset=[s];z.processRecord()}else{if(u<128){var l=u*256+r;if(z.lidx==z.numLine-1){l=(l==32767)?NaN:l/1000;z.sg=l}else{l=(l==32767)?NaN:l/100}if(z.lidx<z.numLine){if(typeof z.dataset!="undefined"){z.dataset.push(l);z.laststat[z.lidx]=(z.lidx>=z.numLine-2)?null:l;z.lidx++;z.processRecord()}else{console.log("Error: missing tag.")}}else{console.log("Error: data overlap?")}}}}}}}}}if(typeof z.chart=="undefined"){z.createChart()}else{z.chart.updateOptions({file:z.data})}z.chart.setAnnotations(z.anno);return w};BrewChart.prototype.processRecord=function(){var a=this;while((((1<<a.lidx)&a.changes)==0)&&a.lidx<a.numLine){a.dataset.push(a.laststat[a.lidx]);a.lidx++}if(a.lidx>=a.numLine){a.data.push(a.dataset);a.state.push(a.cstate)}};var profileEditor={dirty:false,tempUnit:"C",C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(a){this.dirty=a;document.getElementById(this.C_savebtn_Id).innerHTML=(a)?"Save*":"Save"},getStartDate:function(){return this.sd},setStartDate:function(a){},startDayChange:function(){var b=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(b.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=b;this.reorg();this.markdirty(true)}},startnow:function(){var a=new Date();document.getElementById(this.C_startday_Id).value=formatDate(a);this.sd=a;this.reorg();this.markdirty(true)},rowList:function(){var b=document.getElementById("profile_t").getElementsByTagName("tbody")[0];return b.getElementsByTagName("tr")},sgChange:function(b){if(b.innerHTML==""||isNaN(b.innerHTML)){b.innerHTML=b.saved}else{b.saved=parseFloat(b.innerHTML)}this.markdirty(true)},dayChange:function(b){if(b.innerHTML==""||isNaN(b.innerHTML)){b.innerHTML=b.saved}else{this.markdirty(true);this.reorg();ControlChart.update(this.chartdata(),this.tempUnit)}},tempChange:function(b){if(b.innerHTML==""||isNaN(b.innerHTML)){b.innerHTML=b.saved}else{this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)}},initrow:function(t,r){var s=this;var q=r.c;t.type=q;var p=t.getElementsByClassName("stage-temp")[0];if(q=="r"){p.innerHTML=""}else{p.innerHTML=r.t;p.contentEditable=true;p.onblur=function(){s.tempChange(this)};p.onfocus=function(){this.saved=this.innerHTML}}var o=t.getElementsByClassName("stage-time")[0];o.innerHTML=r.d;o.contentEditable=true;o.onblur=function(){s.dayChange(this)};o.onfocus=function(){this.saved=this.innerHTML};var n=t.getElementsByClassName("stage-sg")[0];if(q=="r"){n.innerHTML=""}else{n.saved=r.g;n.innerHTML=r.g;n.contentEditable=true;n.onblur=function(){s.sgChange(this)};n.onfocus=function(){this.saved=this.innerHTML}}var m=t.getElementsByClassName("for-time")[0];var l=t.getElementsByClassName("condition")[0];var k={t:0,g:1,a:2,o:3};if(q=="r"){m.style.display="block";l.style.display="none"}else{l.value=r.c;l.selectedIndex=k[r.c];m.style.display="none";l.style.display="block"}},datestr:function(e){var c=new Date(this.sd.getTime()+Math.round(e*86400)*1000);return formatDate(c)},reorg:function(){var f=this.rowList();var e=this.sd.getTime();for(var g=0;g<f.length;g++){var j=f[g];j.className=(g%2)?"odd":"even";j.getElementsByClassName("diaplay-time")[0].innerHTML=formatDate(new Date(e));var h=this.rowTime(j);e+=Math.round(h*86400)*1000}},chartdata:function(){var h=this.rowList();if(h.length==0){return[]}var g=this.sd.getTime();var n=h[0];var m=this.rowTemp(n);var l=[];l.push([new Date(g),m]);for(var j=0;j<h.length;j++){var n=h[j];var k;if(n.type=="r"){k=this.rowTemp(h[j+1])}else{k=this.rowTemp(n)}g+=Math.round(this.rowTime(n)*86400)*1000;l.push([new Date(g),k])}return l},addRow:function(){var h=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var g=h.getElementsByTagName("tr");var l;if(g.length==0){var k=(this.tempUnit=="C")?20:68;l={c:"t",t:k,d:1,g:1.01}}else{var j=g[g.length-1];var i=this.row.cloneNode(true);this.initrow(i,{c:"r",d:1});h.appendChild(i);l={c:"t",t:this.rowTemp(j),d:1,g:this.rowSg(j)}}var i=this.row.cloneNode(true);this.initrow(i,l);h.appendChild(i);this.reorg();this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)},delRow:function(){var f=this.rowList();if(f.length==0){return}var e=f[f.length-1];if(f.length>1){var g=f[f.length-2];g.parentNode.removeChild(g)}e.parentNode.removeChild(e);this.markdirty(true);ControlChart.update(this.chartdata(),this.tempUnit)},rowTemp:function(b){return parseFloat(b.getElementsByClassName("stage-temp")[0].innerHTML)},rowCondition:function(b){return b.getElementsByClassName("condition")[0].value},rowTime:function(b){return parseFloat(b.getElementsByClassName("stage-time")[0].innerHTML)},rowSg:function(b){return b.getElementsByClassName("stage-sg")[0].saved},renderRows:function(a){var h=document.getElementById("profile_t").getElementsByTagName("tbody")[0];for(var b=0;b<a.length;b++){var i=this.row.cloneNode(true);this.initrow(i,a[b]);h.appendChild(i)}this.reorg()},initable:function(g,f){this.sd=f;document.getElementById(this.C_startday_Id).value=formatDate(f);var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];this.row=a.getElementsByTagName("tr")[0];a.removeChild(this.row);this.renderRows(g)},clear:function(){var f=this.rowList();var e=f.length;for(var g=f.length-1;g>=0;g--){var h=f[g];h.parentNode.removeChild(h)}this.markdirty(true)},getProfile:function(){var r=this.rowList();var q=0;var p=[];for(var j=0;j<r.length;j++){var o=r[j];var n=this.rowTime(o);if(isNaN(n)){return false}if(o.type=="r"){p.push({c:"r",d:n})}else{var m=this.rowTemp(o);var l=this.rowSg(o);if(isNaN(m)||isNaN(l)){return false}if(m>BrewPiSetting.maxDegree||m<BrewPiSetting.minDegree){return false}p.push({c:this.rowCondition(o),d:n,t:m,g:l})}}var t=this.sd.toISOString();var k={s:t,v:2,u:this.tempUnit,t:p};console.log(k);return k},loadProfile:function(b){this.sd=new Date(b.s);this.tempUnit=b.u;this.clear();this.renderRows(b.t);ControlChart.update(this.chartdata(),this.tempUnit)},initProfile:function(c){if(typeof c!="undefined"){var b=new Date(c.s);this.tempUnit=c.u;profileEditor.initable(c.t,b)}else{profileEditor.initable([],new Date())}},setTempUnit:function(g){if(g==this.tempUnit){return}this.tempUnit=g;var f=this.rowList();for(var h=0;h<f.length;h++){var e=f[h].querySelector("td.ptemp");var j=parseFloat(e.innerHTML);e.innerHTML=(g=="C")?F2C(j):C2F(j)}}};var PL={pl_path:"P",url_list:"/list",url_save:"/fputs",url_del:"/rm",url_load:"pl.php?ld=",div:"#profile-list-pane",shown:false,initialized:false,plist:[],path:function(b){return"/"+this.pl_path+"/"+b},depath:function(b){return b.substring(this.pl_path.length+1)},rm:function(b){var a=this;var g="path="+a.path(a.plist[b]);s_ajax({url:a.url_del,m:"DELETE",data:g,success:function(c){a.plist.splice(b,1);a.list(a.plist)},fail:function(c){alert("failed:"+c)}})},load:function(b){var a=this;var g=a.path(a.plist[b]);s_ajax({url:g,m:"GET",success:function(c){var e=JSON.parse(c);profileEditor.loadProfile(e)},fail:function(c){alert("failed:"+c)}})},list:function(g){var f=this;var j=Q(f.div).querySelector("tbody");var k;while(k=j.querySelector("tr:nth-of-type(1)")){j.removeChild(k)}var c=f.row;g.forEach(function(b,a){var e=c.cloneNode(true);e.querySelector(".profile-name").innerHTML=b;e.querySelector(".profile-name").onclick=function(h){h.preventDefault();f.load(a);return false};e.querySelector(".rmbutton").onclick=function(){f.rm(a)};j.appendChild(e)})},append:function(a){if(!this.initialized){return}this.plist.push(a);this.list(this.plist)},init:function(){var b=this;b.initialized=true;b.row=Q(b.div).querySelector("tr:nth-of-type(1)");b.row.parentNode.removeChild(b.row);s_ajax({url:b.url_list,m:"POST",data:"dir="+b.path(""),success:function(e){b.plist=[];var a=JSON.parse(e);a.forEach(function(c){if(c.type=="file"){b.plist.push(b.depath(c.name))}});b.list(b.plist)},fail:function(a){alert("failed:"+a)}})},toggle:function(){if(!this.initialized){this.init()}this.shown=!this.shown;if(this.shown){Q(this.div).style.left="0px"}else{Q(this.div).style.left="-300px"}},saveas:function(){Q("#dlg_saveas").style.display="block"},cancelSave:function(){Q("#dlg_saveas").style.display="none"},doSave:function(){var h=Q("#dlg_saveas input").value;if(h==""){return}if(h.match(/[\W]/g)){return}var a=profileEditor.getProfile();if(a===false){alert("invalid value. check again");return}var b=this;var i="path="+b.path(h)+"&content="+encodeURIComponent(JSON.stringify(a));var b=this;s_ajax({url:b.url_save,m:"POST",data:i,success:function(c){b.append(h);b.cancelSave()},fail:function(c){alert("failed:"+c)}})}};var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:"C"};function formatDate(e){var f=e.getHours();var c=e.getMinutes();function b(a){return(a<10)?"0"+a:a}return e.toLocaleDateString()+" "+b(f)+":"+b(c)}var ControlChart={unit:"C",init:function(a,n,m){var i=this;i.data=n;i.unit=m;var l=function(b){d=new Date(b);return formatDate(d)};var k=function(c){d=new Date(c);var f=d.getYear()+1900;var b=new RegExp("[^\d]?"+f+"[^\d]?");var e=d.toLocaleDateString();return e.replace(b,"")};var j=function(b){return b.toFixed(1)+"&deg;"+i.unit};i.chart=new Dygraph(document.getElementById(a),i.data,{colors:["rgb(89, 184, 255)"],axisLabelFontSize:12,gridLineColor:"#ccc",gridLineWidth:"0.1px",labels:["Time","Temperature"],labelsDiv:document.getElementById(a+"-label"),legend:"always",labelsDivStyles:{textAlign:"right"},strokeWidth:1,axes:{y:{valueFormatter:j,pixelsPerLabel:20,axisLabelWidth:35},x:{axisLabelFormatter:k,valueFormatter:l,pixelsPerLabel:30,axisLabelWidth:40}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},})},update:function(e,c){if(e.length==0){return}this.data=e;this.unit=c;this.chart.updateOptions({file:this.data})}};var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(b){var e=document.getElementById(b+"-m");var c=document.getElementById(b+"-m").className.replace(/\snav-selected/,"");e.className=c;document.getElementById(b+"-s").style.display="none"},select:function(a){document.getElementById(a+"-m").className+=" nav-selected";document.getElementById(a+"-s").style.display="block"},init:function(){var c=this;if(c.initiated){return}c.initiated=true;for(var e=0;e<4;e++){var a=c.modes[e];document.getElementById(a+"-s").style.display="none";document.getElementById(a+"-m").onclick=function(){var b=this.id.replace(/-m/,"");c.dselect(c.cmode);c.select(b);c.cmode=b}}c.cmode="profile";c.select(c.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var a=document.getElementById(this.cmode+"-t").value;if(a==""||isNaN(a)||(a>BrewPiSetting.maxDegree||a<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+a);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+a+"}");BWF.send("j{mode:b, beerSet:"+a+"}")}else{console.log("j{mode:f, fridgeSet:"+a+"}");BWF.send("j{mode:f, fridgeSet:"+a+"}")}}else{if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}}};function saveprofile(){console.log("save");var c=profileEditor.getProfile();if(c===false){alert("invalid value. check again");return}var b=JSON.stringify(c);console.log("result="+b);BWF.save(BWF.BrewProfile,b,function(){profileEditor.markdirty(false);alert("Done.")},function(a){alert("save failed:"+a)})}function C2F(a){return Math.round((a*1.8+32)*10)/10}function F2C(a){return Math.round((a-32)/1.8*10)/10}function updateTempUnit(c){var b=document.getElementsByClassName("t_unit");for(var e=0;e<b.length;e++){b[e].innerHTML=c}}function openDlgLoading(){document.getElementById("dlg_loading").style.display="block"}function closeDlgLoading(){document.getElementById("dlg_loading").style.display="none"}function onload(b){modekeeper.init();openDlgLoading();function a(){var c=true;function e(){if(!c){return}c=false;closeDlgLoading();b()}invoke({url:"/tcc",m:"GET",fail:function(f){console.log("error connect to BrwePiLess!");e()},success:function(h){var g=JSON.parse(h);var f={valid:true,minDegree:g.tempSetMin,maxDegree:g.tempSetMax,tempUnit:g.tempFormat};if(f.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(f.tempUnit);profileEditor.setTempUnit(f.tempUnit)}BrewPiSetting=f;e()}})}BWF.load(BWF.BrewProfile,function(e){var c=JSON.parse(e);updateTempUnit(c.u);BrewPiSetting.tempUnit=c.u;profileEditor.initProfile(c);ControlChart.init("tc_chart",profileEditor.chartdata(),c.u);a()},function(c){profileEditor.initProfile();ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);a()})}function getLogName(){s_ajax({url:"loglist.php",m:"GET",success:function(b){var a=JSON.parse(b);if(a.rec){Q("#recording").innerHTML=a.log}else{Q("#recording").innerHTML=""}},fail:function(a){alert("failed:"+a)}})}var BChart={offset:0,url:"chart.php",toggle:function(b){this.chart.toggleLine(b)},reqdata:function(){var a=this;var e="offset="+a.offset;if(typeof a.startOff!="undefined"&&a.startOff!==null){e=e+"&index="+a.startOff}var b=new XMLHttpRequest();b.open("GET",a.url+"?"+e);b.timeout=5000;b.ontimeout=function(c){console.error("Timeout!!")};b.responseType="arraybuffer";b.onload=function(g){if(this.status==404){console.log("Error getting log data");return}var f=new Uint8Array(this.response);if(f.length==0){console.log("zero content");if(a.timer){clearInterval(a.timer)}a.timer=null;setTimeout(function(){a.reqdata()},3000);return}var c=a.chart.process(f);if(c){a.offset=f.length;a.startOff=b.getResponseHeader("LogOffset");getLogName();console.log("new chart, offset="+a.startOff)}else{a.offset+=f.length}if(!isNaN(a.chart.og)){updateOriginGravity(a.chart.og);a.chart.og=NaN}if(a.chart.sg&&!isNaN(a.chart.sg)){updateGravity(a.chart.sg);a.chart.sg=NaN}if(a.timer==null){a.settimer()}};b.onerror=function(){console.log("error getting data.");setTimeout(function(){a.reqdata()},10000)};b.send()},settimer:function(){var a=this;a.timer=setInterval(function(){a.reqdata()},a.chart.interval*1000)},init:function(b){this.chart=new BrewChart(b)},timer:null,start:function(){if(this.running){return}this.running=true;this.offset=0;this.reqdata()},reqnow:function(){var a=this;if(a.timer){clearInterval(a.timer)}a.timer=null;a.reqdata()}};function setLcdText(e,c){var f=document.getElementById(e);f.innerHTML=c}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function gravityDevice(e){if(typeof e.name=="undefined"){return}if(e.name.startsWith("iSpindel")){if(typeof window.iSpindel=="undefined"){window.iSpindel=true;Q("#iSpindel-pane").style.display="block"}Q("#iSpindel-name").innerHTML=e.name;if(typeof e.battery!="undefined"){Q("#iSpindel-battery").innerHTML=e.battery}var c;if(typeof e.lu!="undefined"){c=new Date(e.lu*1000)}else{c=new Date()}Q("#iSpindel-last").innerHTML=formatDate(c);if(typeof e.sg!="undefined"){updateGravity(e.sg)}}}function init(){BChart.init("div_g");var a=true;onload(function(){BWF.init({onconnect:function(){BWF.send("l");setInterval(function(){if(!a){controllerError()}BWF.send("l");a=false},5000)},error:function(b){console.log("error");communicationError()},handlers:{L:function(b){a=true;for(var c=0;c<4;c++){setLcdText("lcd-line-"+c,b[c])}},V:function(b){console.log("forced reload chart");BChart.reqnow()},G:function(b){gravityDevice(b)}}});BChart.start()})}var BrewMath={abv:function(e,c){return((76.08*(e-c)/(1.775-e))*(c/0.794)).toFixed(1)},att:function(e,c){return Math.round((e-c)/(e-1)*100)},sg2pla:function(b){return -616.868+1111.14*b-630.272*b*b+135.997*b*b*b},pla2sg:function(b){return 1+(b/(258.6-((b/258.2)*227.1)))}};function updateGravity(b){window.sg=b;Q("#gravity-sg").innerHTML=b.toFixed(3);if(typeof window.og!="undefined"){Q("#gravity-att").innerHTML=BrewMath.att(window.og,b);Q("#gravity-abv").innerHTML=BrewMath.abv(window.og,b)}}function updateOriginGravity(b){if(typeof window.og!="undefined"&&window.og==b){return}window.og=b;Q("#gravity-og").innerHTML=b.toFixed(3);if(typeof window.sg!="undefined"){updateGravity(window.sg)}}function showgravitydlg(b){Q("#dlg_addgravity .msg").innerHTML=b;Q("#dlg_addgravity").style.display="block"}function dismissgravity(){Q("#dlg_addgravity").style.display="none"}function inputgravity(){dismissgravity();openDlgLoading();var e=parseFloat(Q("#dlg_addgravity input").value);if(window.isog){updateOriginGravity(e)}else{updateGravity(e)}var c={name:"webjs",gravity:e};if(window.isog){c.og=1}s_ajax({url:"gravity",m:"POST",mime:"application/json",data:JSON.stringify(c),success:function(a){closeDlgLoading()},fail:function(a){alert("failed:"+a);closeDlgLoading()}})}function gravity(){window.isog=false;showgravitydlg("Add gravity Record:")}function origingravity(){window.isog=true;showgravitydlg("Set Original Gravity:")};/*]]>*/</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}.frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px}#top-frame{height:520px}#bottom-frame{height:380px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}#recording{color:lightblue;font-size:18px}#top-frame button{float:right;width:200px;margin-top:3px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:4em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em;border-top-right-radius:5px;border-top-left-radius:5px}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:965px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}#bottom-frame button{color:#1d5987;background:#dfeffc;font-weight:bold;border-radius:6px;margin:4px}#addbutton{float:right;width:120px}#header{width:968px}#clearbtn{width:18%}#savebtn{width:25%;float:right}#delbtn{width:18%}.modal{display:none;position:fixed;z-index:100;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:10px;border:1px solid #888;width:330px;height:100px;border-radius:8px}#dlg_addgravity button{float:right;width:56px;margin-top:2px}#modekeeper-apply{margin-top:0}#profile-edit{float:left;margin:6px;width:480px;position:relative}#tc_chart{width:430px;height:280px;float:right;margin:6px}#gravity-pane table,#iSpindel-pane table{width:100px;float:right;border:1px solid white;border-radius:5px;margin-right:10px;color:cyan;font-size:12px}#iSpindel-pane{display:none}#profile-list-pane{position:absolute;background-color:#fafaf0;border-style:outset;border:solid 3px;height:100%;width:200px;overflow:auto;left:-300px;top:0;overflow:auto}#profile-list-pane td{white-space:nowrap;overflow:hidden}#profile-list-pane .rmbutton{color:red}#profile-list-pane a,#profile-list-pane u{text-decoration:none}</style>
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
<div id="profile-list-pane">
<table>
<tr><td><button class="rmbutton">X</button><a href="#" class="profile-name"></a></td></tr>
</table>
</div>
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
<div><button id="delbtn" onclick="profileEditor.delRow()">Delete</button>
<button id="clearbtn" onclick="profileEditor.clear()">Clear</button>
<button id="loadbtn" onclick="PL.toggle()">...</button>
<button id="saveasbtn" onclick="PL.saveas()">Save As</button>
<button id="savebtn" onclick="saveprofile()">Save</button></div>
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
<div id="dlg_saveas" class="modal">
<div class="modal-content">
<span class="msg">Save Profile As</span>
<br><input type="text" size="32">
<br>*No special letters and space allowed.
<button onclick="PL.cancelSave()">Cancel</button>
<button onclick="PL.doSave()">OK</button>
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
<script>/*<![CDATA[*/var Q=function(a){return document.querySelector(a)};var BrewChart=function(b){this.cid=b;this.ctime=0;this.interval=60;this.numLine=7;this.lidx=0;this.celius=true;this.clearData()};BrewChart.prototype.clearData=function(){this.laststat=[NaN,NaN,NaN,NaN,NaN,NaN,NaN];this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(a){this.celius=a;this.ylabel(STR.ChartLabel+"("+(a?"°C":"°F")+")")};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(h){var f=h.getHours();var e=h.getMinutes();var i=h.getSeconds();function g(a){return(a>9)?a:("0"+a)}return h.toLocaleDateString()+" "+g(f)+":"+g(e)+":"+g(i)};BrewChart.prototype.showLegend=function(f,e){var i=new Date(f);Q(".beer-chart-legend-time").innerHTML=this.formatDate(i);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,6));var h=this.chart.getValue(e,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(!h||isNaN(h))?"--":h.toFixed(3);var j=parseInt(this.state[e]);if(!isNaN(j)){Q(".chart-legend-row.state .legend-label").innerHTML=STATES[j].text}};BrewChart.prototype.hideLegend=function(){var a=document.querySelectorAll(".legend-value");a.forEach(function(b){b.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q(".chart-legend-row.state .legend-label").innerHTML="state"};BrewChart.prototype.tempFormat=function(e){var c=parseFloat(e);if(isNaN(c)){return"--"}var b=this.celius?"°C":"°F";return parseFloat(c).toFixed(2)+b};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(b){this.shownlist[b]=!this.shownlist[b];if(this.shownlist[b]){Q("."+b+" .toggle").style.backgroundColor=Q(".chart-legend-row."+b).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,true)}else{Q("."+b+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,false)}};BrewChart.prototype.createChart=function(){var a=this;a.initLegend();a.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var e=document.createElement("div");e.className="hide";document.body.appendChild(e);var b={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:"Temperature",y2label:"Gravity",series:{gravity:{axis:"y2",drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:"#ccc",gridLineWidth:"0.1px",labelsDiv:e,labelsDivStyles:{display:"none"},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(c){return a.tempFormat(c)}},y2:{valueFormatter:function(c){return c.toFixed(3)},axisLabelFormatter:function(c){return c.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(h,f,g,c){a.showLegend(f,c)},unhighlightCallback:function(c){a.hideLegend()}};a.chart=new Dygraph(document.getElementById(a.cid),a.data,b)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=["Time","beerSet","beerTemp","fridgeTemp","fridgeSet","roomTemp","auxTemp","gravity"];BrewChart.prototype.addMode=function(a){var b=String.fromCharCode(a);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:b.toUpperCase(),text:BrewChart.Mode[b],attachAtBottom:true})};BrewChart.testData=function(b){if(b[0]!=255){return false}var c=b[1]&7;if(c!=5){return false}return{sensor:c,f:b[1]&16}};BrewChart.prototype.addResume=function(b){this.ctime+=b;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:"R",text:"Resume",attachAtBottom:true})};BrewChart.prototype.process=function(x){var w=false;var z=this;for(var m=0;m<x.length;){var u=x[m++];var r=x[m++];if(u==255){if((r&15)!=5){alert("log version mismatched!");return}z.celius=(r&16)?false:true;var k=x[m++];k=k*256+x[m++];z.interval=k;z.starttime=(x[m]<<24)+(x[m+1]<<16)+(x[m+2]<<8)+x[m+3];z.ctime=z.starttime;m+=4;z.data=[];z.anno=[];z.state=[];z.cstate=0;this.clearData();w=true}else{if(u==244){z.addMode(r)}else{if(u==241){z.cstate=r}else{if(u==254){if(z.lidx){var q;for(q=z.lidx;q<z.numLine;q++){z.dataset.push(NaN)}z.data.push(z.dataset)}z.lidx=0;z.addResume(r)}else{if(u==248){var o=x[m++];var n=x[m++];var y=(o&127)*256+n;z.og=y/1000}else{if(u==240){z.changes=r;z.lidx=0;var s=new Date(this.ctime*1000);z.incTime();z.dataset=[s];z.processRecord()}else{if(u<128){var l=u*256+r;if(z.lidx==z.numLine-1){l=(l==32767)?NaN:l/1000;z.sg=l}else{l=(l==32767)?NaN:l/100}if(z.lidx<z.numLine){if(typeof z.dataset!="undefined"){z.dataset.push(l);z.laststat[z.lidx]=(z.lidx>=z.numLine-2)?null:l;z.lidx++;z.processRecord()}else{console.log("Error: missing tag.")}}else{console.log("Error: data overlap?")}}}}}}}}}if(typeof z.chart=="undefined"){z.createChart()}else{z.chart.updateOptions({file:z.data})}z.chart.setAnnotations(z.anno);return w};BrewChart.prototype.processRecord=function(){var a=this;while((((1<<a.lidx)&a.changes)==0)&&a.lidx<a.numLine){a.dataset.push(a.laststat[a.lidx]);a.lidx++}if(a.lidx>=a.numLine){a.data.push(a.dataset);a.state.push(a.cstate)}};var profileEditor={dirty:false,tempUnit:"C",C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(a){this.dirty=a;document.getElementById(this.C_savebtn_Id).innerHTML=(a)?"Save*":"Save";if(a){ControlChart.update(this.chartdata(),this.tempUnit)}},getStartDate:function(){return this.sd},setStartDate:function(a){},startDayChange:function(){var b=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(b.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=b;this.sortrow();this.markdirty(true)}},startnow:function(){var a=new Date();document.getElementById(this.C_startday_Id).value=formatDate(a);this.sd=a;this.sortrow();this.markdirty(true)},deleteRow:function(c){var b=document.getElementById("profile_t").getElementsByTagName("tr")[c];b.parentNode.removeChild(b);this.sortrow();this.markdirty(true)},showmenu:function(i,h){var g=h.rowIndex;i.preventDefault();var f=document.createElement("div");f.className="contextmenu";f.innerHTML="Delete!";f.style.top=(i.clientY-1)+"px";f.style.left=(i.clientX-1)+"px";f.onmouseout=function(){f.parentNode.removeChild(f)};var k=this;f.onclick=function(){f.parentNode.removeChild(f);k.deleteRow(g)};var j=document.getElementsByTagName("body")[0];j.appendChild(f)},dayChange:function(b){if(b.innerHTML==""||isNaN(b.innerHTML)){b.innerHTML=b.saved}this.sortrow();this.markdirty(true)},tempChange:function(b){if(b.innerHTML==""||isNaN(b.innerHTML)){b.innerHTML=b.saved}else{this.markdirty(true)}},newrow:function(n,j,i){var e=this;var o=document.createElement("tr");var m=document.createElement("td");m.className="pday";m.innerHTML=(typeof n!=="undefined")?n:0;m.contentEditable=true;m.onblur=function(){e.dayChange(this)};m.onfocus=function(){this.saved=this.innerHTML};var l=document.createElement("td");l.className="ptemp";l.innerHTML=(typeof j!=="undefined")?j:20;l.contentEditable=true;l.onblur=function(){e.tempChange(this)};l.onfocus=function(){this.saved=this.innerHTML};var k=document.createElement("td");k.className="pdaystr";k.innerHTML=(i)?i:"";o.appendChild(m);o.appendChild(l);o.appendChild(k);o.oncontextmenu=function(a){e.showmenu(a,this);return false};return o},datestr:function(e){var c=new Date(this.sd.getTime()+Math.round(e*86400)*1000);return formatDate(c)},addRow:function(){var e=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var c=e.getElementsByTagName("tr");var f=this.newrow();f.className=(c.length%2)?"odd":"even";e.appendChild(f);this.markdirty(true)},init:function(e,g){this.sd=g;document.getElementById(this.C_startday_Id).value=formatDate(g);var b=document.getElementById("profile_t").getElementsByTagName("tbody")[0];for(var c=0;c<e.length;c++){var f=this.newrow(e[c].d,e[c].t,this.datestr(e[c].d));f.className=(c%2)?"odd":"even";b.appendChild(f)}},sortrow:function(){var m=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var l=m.getElementsByTagName("tr");var k=Array.prototype.slice.call(l);k.sort(function(f,e){var h=parseFloat(f.querySelector("td.pday").innerHTML);var g=parseFloat(e.querySelector("td.pday").innerHTML);if(isNaN(h)||isNaN(g)||(h===g)){return 0}else{return h-g}});for(var b=0;b<k.length;b++){var c=k[b];var a=c.querySelector("td.pday").innerHTML;c.querySelector("td.pdaystr").innerHTML=this.datestr(parseFloat(a));c.className=(b%2)?"odd":"even";c.parentNode.removeChild(c);m.appendChild(c)}},clear:function(){var f=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var e=f.getElementsByTagName("tr");var j=e.length;for(var g=e.length-1;g>=0;g--){var h=e[g];h.parentNode.removeChild(h)}this.markdirty(true)},getProfile:function(){var r=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var q=r.getElementsByTagName("tr");var p=q.length;var o=0;var n=[];for(var j=0;j<q.length;j++){var m=q[j];var l=parseFloat(m.querySelector("td.pday").innerHTML);var k=parseFloat(m.querySelector("td.ptemp").innerHTML);if(isNaN(l)||isNaN(k)){return false}if(k>BrewPiSetting.maxDegree||k<BrewPiSetting.minDegree){return false}if(j==0){if(l!=0){return false}}else{if(o==l){return false}}o=l;n.push({d:l,t:k})}var t=this.sd.toISOString();return{s:t,u:this.tempUnit,t:n}},chartdata:function(){var t=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var s=t.getElementsByTagName("tr");var r=s.length;var q=0;var p=[];for(var l=0;l<s.length;l++){var o=s[l];var n=parseFloat(o.querySelector("td.pday").innerHTML);var m=parseFloat(o.querySelector("td.ptemp").innerHTML);if(isNaN(n)||isNaN(m)){return false}if(m>BrewPiSetting.maxDegree||m<BrewPiSetting.minDegree){return false}if(l==0){if(n!=0){return false}}else{if(q==n){return false}}q=n;var k=new Date(this.sd.getTime()+n*86400000);p.push([k,m])}return p},initProfile:function(c){if(typeof c!="undefined"){var b=new Date(c.s);this.tempUnit=c.u;profileEditor.init(c.t,b)}else{profileEditor.init([],new Date())}},setTempUnit:function(g){if(g==this.tempUnit){return}this.tempUnit=g;var f=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var e=f.getElementsByTagName("tr");for(var h=0;h<e.length;h++){var k=e[h].querySelector("td.ptemp");var j=parseFloat(k.innerHTML);k.innerHTML=(g=="C")?F2C(j):C2F(j)}}};var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:"C"};function formatDate(e){var f=e.getHours();var c=e.getMinutes();function b(a){return(a<10)?"0"+a:a}return e.toLocaleDateString()+" "+b(f)+":"+b(c)}var ControlChart={unit:"C",init:function(a,n,m){var i=this;i.data=(n===false)?[]:n;i.unit=m;var l=function(b){d=new Date(b);return formatDate(d)};var k=function(c){d=new Date(c);var f=d.getYear()+1900;var b=new RegExp("[^\d]?"+f+"[^\d]?");var e=d.toLocaleDateString();return e.replace(b,"")};var j=function(b){return b.toFixed(1)+"&deg;"+i.unit};i.chart=new Dygraph(document.getElementById(a),i.data,{colors:["rgb(89, 184, 255)"],axisLabelFontSize:12,gridLineColor:"#ccc",gridLineWidth:"0.1px",labels:["Time","Temperature"],labelsDiv:document.getElementById(a+"-label"),legend:"always",labelsDivStyles:{textAlign:"right"},strokeWidth:1,axes:{y:{valueFormatter:j,pixelsPerLabel:20,axisLabelWidth:35},x:{axisLabelFormatter:k,valueFormatter:l,pixelsPerLabel:30,axisLabelWidth:40}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},})},update:function(e,c){if(e===false){return}if(e.length==0){return}this.data=e;this.unit=c;this.chart.updateOptions({file:this.data})}};var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(b){var e=document.getElementById(b+"-m");var c=document.getElementById(b+"-m").className.replace(/\snav-selected/,"");e.className=c;document.getElementById(b+"-s").style.display="none"},select:function(a){document.getElementById(a+"-m").className+=" nav-selected";document.getElementById(a+"-s").style.display="block"},init:function(){var c=this;if(c.initiated){return}c.initiated=true;for(var e=0;e<4;e++){var a=c.modes[e];document.getElementById(a+"-s").style.display="none";document.getElementById(a+"-m").onclick=function(){var b=this.id.replace(/-m/,"");c.dselect(c.cmode);c.select(b);c.cmode=b}}c.cmode="profile";c.select(c.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var a=document.getElementById(this.cmode+"-t").value;if(a==""||isNaN(a)||(a>BrewPiSetting.maxDegree||a<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+a);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+a+"}");BWF.send("j{mode:b, beerSet:"+a+"}")}else{console.log("j{mode:f, fridgeSet:"+a+"}");BWF.send("j{mode:f, fridgeSet:"+a+"}")}}else{if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}}};function saveprofile(){console.log("save");var c=profileEditor.getProfile();if(c===false){alert("invalid value. check again");return}var b=JSON.stringify(c);console.log("result="+b);BWF.save(BWF.BrewProfile,b,function(){profileEditor.markdirty(false);alert("Done.")},function(a){alert("save failed:"+a)})}function C2F(a){return Math.round((a*1.8+32)*10)/10}function F2C(a){return Math.round((a-32)/1.8*10)/10}function updateTempUnit(c){var b=document.getElementsByClassName("t_unit");for(var e=0;e<b.length;e++){b[e].innerHTML=c}}function openDlgLoading(){document.getElementById("dlg_loading").style.display="block"}function closeDlgLoading(){document.getElementById("dlg_loading").style.display="none"}function onload(b){modekeeper.init();openDlgLoading();function a(){var c=true;function e(){if(!c){return}c=false;closeDlgLoading();b()}invoke({url:"/tcc",m:"GET",fail:function(f){console.log("error connect to BrwePiLess!");e()},success:function(h){var g=JSON.parse(h);var f={valid:true,minDegree:g.tempSetMin,maxDegree:g.tempSetMax,tempUnit:g.tempFormat};if(f.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(f.tempUnit);profileEditor.setTempUnit(f.tempUnit)}BrewPiSetting=f;e()}})}BWF.load(BWF.BrewProfile,function(e){var c=JSON.parse(e);updateTempUnit(c.u);BrewPiSetting.tempUnit=c.u;profileEditor.initProfile(c);ControlChart.init("tc_chart",profileEditor.chartdata(),c.u);a()},function(c){profileEditor.initProfile();ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);a()})}function s_ajax(a){var e=new XMLHttpRequest();e.onreadystatechange=function(){if(e.readyState==4){if(e.status==200){a.success(e.responseText)}else{e.onerror(e.status)}}};e.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{e.onerror(-1)}},e.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};e.open(a.m,a.url,true);if(typeof a.data!="undefined"){e.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");e.send(a.data)}else{e.send()}}function getLogName(){s_ajax({url:"loglist.php",m:"GET",success:function(b){var a=JSON.parse(b);if(a.rec){Q("#recording").innerHTML=a.log}else{Q("#recording").innerHTML=""}},fail:function(a){alert("failed:"+a)}})}var BChart={offset:0,url:"chart.php",toggle:function(b){this.chart.toggleLine(b)},reqdata:function(){var a=this;var e="offset="+a.offset;if(typeof a.startOff!="undefined"&&a.startOff!==null){e=e+"&index="+a.startOff}var b=new XMLHttpRequest();b.open("GET",a.url+"?"+e);b.timeout=5000;b.ontimeout=function(c){console.error("Timeout!!")};b.responseType="arraybuffer";b.onload=function(g){if(this.status==404){console.log("Error getting log data");return}var f=new Uint8Array(this.response);if(f.length==0){console.log("zero content");if(a.timer){clearInterval(a.timer)}a.timer=null;setTimeout(function(){a.reqdata()},3000);return}var c=a.chart.process(f);if(c){a.offset=f.length;a.startOff=b.getResponseHeader("LogOffset");getLogName();console.log("new chart, offset="+a.startOff)}else{a.offset+=f.length}if(!isNaN(a.chart.og)){updateOriginGravity(a.chart.og);a.chart.og=NaN}if(!isNaN(a.chart.sg)){updateGravity(a.chart.sg);a.chart.sg=NaN}if(a.timer==null){a.settimer()}};b.onerror=function(){console.log("error getting data.");setTimeout(function(){a.reqdata()},10000)};b.send()},settimer:function(){var a=this;a.timer=setInterval(function(){a.reqdata()},a.chart.interval*1000)},init:function(b){this.chart=new BrewChart(b)},timer:null,start:function(){if(this.running){return}this.running=true;this.offset=0;this.reqdata()},reqnow:function(){var a=this;if(a.timer){clearInterval(a.timer)}a.timer=null;a.reqdata()}};function setLcdText(e,c){var f=document.getElementById(e);f.innerHTML=c}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function gravityDevice(e){if(typeof e.name=="undefined"){return}if(e.name.startsWith("iSpindel")){if(typeof window.iSpindel=="undefined"){window.iSpindel=true;Q("#iSpindel-pane").style.display="block"}Q("#iSpindel-name").innerHTML=e.name;if(typeof e.battery!="undefined"){Q("#iSpindel-battery").innerHTML=e.battery}var c;if(typeof e.lu!="undefined"){c=new Date(e.lu*1000)}else{c=new Date()}Q("#iSpindel-last").innerHTML=formatDate(c);if(typeof e.sg!="undefined"){updateGravity(e.sg)}}}function init(){BChart.init("div_g");var a=true;onload(function(){BWF.init({onconnect:function(){BWF.send("l");setInterval(function(){if(!a){controllerError()}BWF.send("l");a=false},8000)},error:function(b){console.log("error");communicationError()},handlers:{L:function(b){a=true;for(var c=0;c<4;c++){setLcdText("lcd-line-"+c,b[c])}},V:function(b){console.log("forced reload chart");BChart.reqnow()},G:function(b){gravityDevice(b)}}});BChart.start()})}var BrewMath={abv:function(e,c){return((76.08*(e-c)/(1.775-e))*(c/0.794)).toFixed(1)},att:function(e,c){return Math.round((e-c)/(e-1)*100)},sg2pla:function(b){return -616.868+1111.14*b-630.272*b*b+135.997*b*b*b},pla2sg:function(b){return 1+(b/(258.6-((b/258.2)*227.1)))}};function updateGravity(b){window.sg=b;Q("#gravity-sg").innerHTML=b.toFixed(3);if(typeof window.og!="undefined"){Q("#gravity-att").innerHTML=BrewMath.att(window.og,b);Q("#gravity-abv").innerHTML=BrewMath.abv(window.og,b)}}function updateOriginGravity(b){if(typeof window.og!="undefined"&&window.og==b){return}window.og=b;Q("#gravity-og").innerHTML=b.toFixed(3);if(typeof window.sg!="undefined"){updateGravity(window.sg)}}function showgravitydlg(b){Q("#dlg_addgravity .msg").innerHTML=b;Q("#dlg_addgravity").style.display="block"}function dismissgravity(){Q("#dlg_addgravity").style.display="none"}function inputgravity(){dismissgravity();openDlgLoading();var e=parseFloat(Q("#dlg_addgravity input").value);if(window.isog){updateOriginGravity(e)}else{updateGravity(e)}var c={name:"webjs",gravity:e};if(window.isog){c.og=1}s_ajax({url:"gravity",m:"POST",mime:"application/json",data:JSON.stringify(c),success:function(a){closeDlgLoading()},fail:function(a){alert("failed:"+a);closeDlgLoading()}})}function gravity(){window.isog=false;showgravitydlg("Add gravity Record:")}function origingravity(){window.isog=true;showgravitydlg("Set Original Gravity:")};/*]]>*/</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}.frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px}#top-frame{height:520px}#bottom-frame{height:380px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}.contextmenu{display:block;position:absolute;background:#D00;color:#fff;font-weight:bold;padding:.5em}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}#recording{color:lightblue;font-size:18px}#top-frame button{float:right;width:200px;margin-top:3px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:4em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em;border-top-right-radius:5px;border-top-left-radius:5px}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:965px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}#bottom-frame button{color:#1d5987;background:#dfeffc;font-weight:bold;border-radius:6px;margin:4px}#addbutton{float:right;width:120px}#header{width:968px}#clearbtn{width:18%}#savebtn{width:25%;float:right}#delbtn{width:18%}.modal{display:none;position:fixed;z-index:100;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:10px;border:1px solid #888;width:330px;height:100px;border-radius:8px}#dlg_addgravity button{float:right;width:56px;margin-top:2px}#modekeeper-apply{margin-top:0}#profile-edit{float:left;margin:6px;width:480px}#tc_chart{width:430px;height:280px;float:right;margin:6px}#gravity-pane table,#iSpindel-pane table{width:100px;float:right;border:1px solid white;border-radius:5px;margin-right:10px;color:cyan;font-size:12px}#iSpindel-pane{display:none}</style>
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
<div id="banner">BrewPiLess v1.2.7
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