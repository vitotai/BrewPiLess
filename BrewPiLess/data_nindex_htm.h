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
<script>/*<![CDATA[*/var Q=function(a){return document.querySelector(a)};var BrewChart=function(b){this.cid=b;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=null};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.prototype.setCelius=function(a){this.celius=a;this.ylabel(STR.ChartLabel+"("+(a?"°C":"°F")+")")};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(h){var f=h.getHours();var e=h.getMinutes();var i=h.getSeconds();function g(a){return(a>9)?a:("0"+a)}return h.toLocaleDateString()+" "+g(f)+":"+g(e)+":"+g(i)};BrewChart.prototype.showLegend=function(f,e){var g=new Date(f);Q(".beer-chart-legend-time").innerHTML=this.formatDate(g);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,4));var h=parseInt(this.state[e]);if(!isNaN(h)){Q(".chart-legend-row.state .legend-label").innerHTML=STATES[h].text}};BrewChart.prototype.hideLegend=function(){var a=document.querySelectorAll(".legend-value");a.forEach(function(b){b.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML="Date/Time";Q(".chart-legend-row.state .legend-label").innerHTML="state"};BrewChart.prototype.tempFormat=function(d){var c=parseFloat(d);if(isNaN(c)){return"--"}var b=this.celius?"°C":"°F";return parseFloat(c).toFixed(2)+b};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4]};BrewChart.prototype.toggleLine=function(b){this.shownlist[b]=!this.shownlist[b];if(this.shownlist[b]){Q("."+b+" .toggle").style.backgroundColor=Q(".chart-legend-row."+b).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,true)}else{Q("."+b+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,false)}};BrewChart.prototype.createChart=function(){var a=this;a.initLegend();a.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true};var b=document.createElement("div");b.className="hide";document.body.appendChild(b);a.chart=new Dygraph(document.getElementById(a.cid),a.data,{labels:BrewChart.Labels,colors:BrewChart.Colors,axisLabelFontSize:12,animatedZooms:true,gridLineColor:"#ccc",gridLineWidth:"0.1px",labelsDiv:b,labelsDivStyles:{display:"none"},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(c){a.tempFormat(c)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(g,d,f,c){a.showLegend(d,c)},unhighlightCallback:function(c){a.hideLegend()}})};BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","rgb(153,0,153)"];BrewChart.Labels=["Time","beerSet","beerTemp","fridgeTemp","fridgeSet","roomTemp"];BrewChart.prototype.addMode=function(a){var b=String.fromCharCode(a);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:b.toUpperCase(),text:BrewChart.Mode[b],attachAtBottom:true})};BrewChart.testData=function(b){if(b[0]!=255){return false}var c=b[1]&7;if(c>5){return false}return{sensor:c,f:b[1]&16}};BrewChart.prototype.addResume=function(b){this.ctime+=b;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:"R",text:"Resume",attachAtBottom:true})};BrewChart.prototype.process=function(u){var x=this;for(var k=0;k<u.length;){var s=u[k++];var r=u[k++];if(s==255){x.celius=(r&16)?false:true;var j=u[k++];j=j*256+u[k++];x.interval=j;x.starttime=(u[k]<<24)+(u[k+1]<<16)+(u[k+2]<<8)+u[k+3];x.ctime=x.starttime;k+=4;x.data=[];x.anno=[];x.state=[];x.cstate=0}else{if(s==244){x.addMode(r)}else{if(s==241){x.cstate=r}else{if(s==247){var o=u[k++];var n=u[k++];var w=(o&127)*256+n;x.beerSet=(w==32767)?null:(w/100)}else{if(s==254){x.addResume(r)}else{if(s<128){if(x.lidx==0){var q=new Date(this.ctime*1000);var m=x.beerSet;x.dataset=[q,m];x.incTime()}var l=s*256+r;if(l==32767||l>12000){l=NaN}else{l=l/100}x.dataset.push(l);if(++x.lidx>=x.numLine){x.lidx=0;x.data.push(x.dataset);x.state.push(x.cstate)}}}}}}}}if(typeof x.chart=="undefined"){x.createChart()}else{x.chart.updateOptions({file:x.data})}x.chart.setAnnotations(x.anno)};var BChart={offset:0,url:"chart.php",toggle:function(b){this.chart.toggleLine(b)},reqdata:function(){var d=this;var a="offset="+d.offset;var e=new XMLHttpRequest();e.open("GET",d.url+"?"+a);e.responseType="arraybuffer";e.onload=function(c){if(this.status==404){console.log("Not logging");return}var b=new Uint8Array(this.response);if(b.length==0){console.log("zero content");if(d.timer){clearInterval(d.timer)}d.timer=null;setTimeout(function(){d.reqdata()},3000);return}d.chart.process(b);d.offset+=b.length;if(d.timer==null){d.settimer()}};e.onerror=function(){alert("unknown error!")};e.send()},settimer:function(){var a=this;a.timer=setInterval(function(){a.reqdata()},a.chart.interval*1000)},init:function(b){this.chart=new BrewChart(b)},timer:null,pull:function(){},setCelius:function(b){this.chart.setCelius(b)},setYLabel:function(b){this.chart.ylabel(b)},start:function(){if(this.running){return}this.running=true;this.offset=0;this.reqdata()},stop:function(){if(!this.running){return}this.running=false;if(this.timer){clearInterval(this.timer);this.timer=null}}};function setLcdText(e,c){var f=document.getElementById(e);f.innerHTML=c}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function init(){var a=true;BWF.init({error:function(b){console.log("error");communicationError()},handlers:{L:function(b){a=true;for(var c=0;c<4;c++){setLcdText("lcd-line-"+c,b[c])}}}});setInterval(function(){if(!a){controllerError()}BWF.send("l");a=false},5000);BChart.init("div_g");BChart.start()};/*]]>*/</script>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:965px;height:410px;padding:5px 5px 5px 5px}.hide{display:none}#frame{border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;width:975px;height:520px}#topbar{width:965px;height:108px;background-color:#5c9ccc;padding:5px 5px 5px 5px}#menu{float:right}button{float:right}#banner{font-size:18pt;float:left;color:white;font-family:fantasy;margin-top:16px;margin-left:16px}button{width:200px;margin-top:5px}</style>
</head>
<body onload=init()>
<div id="frame">
<div id="topbar">
<div id="lcd" class="lcddisplay"><span class="lcd-text">
<span class="lcd-line" id="lcd-line-0">Live LCD waiting</span>
<span class="lcd-line" id="lcd-line-1">for update from</span>
<span class="lcd-line" id="lcd-line-2">script...</span>
<span class="lcd-line" id="lcd-line-3"></span></p><p>
</div>
<div id="banner">BrewPiLess
</div>
<div id="menu">
<button onclick="window.open('/control.htm')">Temperature Management</button><br>
<button onclick="window.open('/log')">Data Logging</button><br>
<button onclick="window.open('/setup.htm')">Device Setup</button><br>
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
</body>
</html>
)END";


