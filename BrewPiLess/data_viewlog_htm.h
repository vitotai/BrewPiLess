const char data_viewlog_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<title>BrewPiLess Log Viewer</title>
<script src="http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js"></script>
<script>/*<![CDATA[*/var Q=function(a){return document.querySelector(a)};var BrewChart=function(b){this.cid=b;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=NaN;this.auxTemp=NaN;this.gravity=NaN;this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(a){this.celius=a;this.ylabel(STR.ChartLabel+"("+(a?"°C":"°F")+")")};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(h){var f=h.getHours();var e=h.getMinutes();var i=h.getSeconds();function g(a){return(a>9)?a:("0"+a)}return h.toLocaleDateString()+" "+g(f)+":"+g(e)+":"+g(i)};BrewChart.prototype.showLegend=function(f,e){var i=new Date(f);Q(".beer-chart-legend-time").innerHTML=this.formatDate(i);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(e,6));var h=this.chart.getValue(e,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(isNaN(h))?"--":h.toFixed(3);var j=parseInt(this.state[e]);if(!isNaN(j)){Q(".chart-legend-row.state .legend-label").innerHTML=STATES[j].text}};BrewChart.prototype.hideLegend=function(){var a=document.querySelectorAll(".legend-value");a.forEach(function(b){b.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q(".chart-legend-row.state .legend-label").innerHTML="state"};BrewChart.prototype.tempFormat=function(d){var c=parseFloat(d);if(isNaN(c)){return"--"}var b=this.celius?"°C":"°F";return parseFloat(c).toFixed(2)+b};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(b){this.shownlist[b]=!this.shownlist[b];if(this.shownlist[b]){Q("."+b+" .toggle").style.backgroundColor=Q(".chart-legend-row."+b).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,true)}else{Q("."+b+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(b).column-1,false)}};BrewChart.prototype.createChart=function(){var a=this;a.initLegend();a.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var e=document.createElement("div");e.className="hide";document.body.appendChild(e);var b={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:"Temperature",y2label:"Gravity",series:{gravity:{axis:"y2",drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:"#ccc",gridLineWidth:"0.1px",labelsDiv:e,labelsDivStyles:{display:"none"},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(c){return a.tempFormat(c)}},y2:{valueFormatter:function(c){return c.toFixed(3)},axisLabelFormatter:function(c){return c.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(g,d,f,c){a.showLegend(d,c)},unhighlightCallback:function(c){a.hideLegend()}};a.chart=new Dygraph(document.getElementById(a.cid),a.data,b)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=["Time","beerSet","beerTemp","fridgeTemp","fridgeSet","roomTemp","auxTemp","gravity"];BrewChart.prototype.addMode=function(a){var b=String.fromCharCode(a);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:b.toUpperCase(),text:BrewChart.Mode[b],attachAtBottom:true})};BrewChart.testData=function(b){if(b[0]!=255){return false}var c=b[1]&7;if(c!=4||c!=6){return false}return{sensor:c,f:b[1]&16}};BrewChart.prototype.addResume=function(b){this.ctime+=b;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:"R",text:"Resume",attachAtBottom:true})};BrewChart.prototype.process=function(z){var y=false;var B=this;for(var o=0;o<z.length;){var x=z[o++];var u=z[o++];if(x==255){B.celius=(u&16)?false:true;B.numLine=(u&15);var l=z[o++];l=l*256+z[o++];B.interval=l;B.starttime=(z[o]<<24)+(z[o+1]<<16)+(z[o+2]<<8)+z[o+3];B.ctime=B.starttime;o+=4;B.data=[];B.anno=[];B.state=[];B.cstate=0;y=true}else{if(x==244){B.addMode(u)}else{if(x==241){B.cstate=u}else{if(x==247){var s=z[o++];var r=z[o++];var A=(s&127)*256+r;B.beerSet=(A==32767)?NaN:(A/100)}else{if(x==248){var s=z[o++];var r=z[o++];var A=(s&127)*256+r;if(u){B.og=A/1000}else{B.gravity=(A==32767)?NaN:(A/1000);B.sg=B.gravity}}else{if(x==249){var s=z[o++];var r=z[o++];var A=(s&127)*256+r;B.auxTemp=(A==32767)?NaN:(A/100)}else{if(x==254){if(B.lidx){var q;for(q=B.lidx;q<B.numLine;q++){B.dataset.push(NaN)}B.data.push(B.dataset)}B.lidx=0;B.addResume(u)}else{if(x<128){if(B.lidx==0){var w=new Date(this.ctime*1000);var n=B.beerSet;B.dataset=[w,n];B.incTime()}var m=x*256+u;if(m==32767||m>12000){m=NaN}else{if(B.lidx==B.numLine-1){m=m/1000}else{m=m/100}}B.dataset.push(m);if(++B.lidx>=B.numLine){if(B.numLine==4){B.dataset.push(B.auxTemp);B.dataset.push(B.gravity);B.gravity=NaN}B.lidx=0;B.data.push(B.dataset);B.state.push(B.cstate)}}}}}}}}}}if(typeof B.chart=="undefined"){B.createChart()}else{B.chart.updateOptions({file:B.data})}B.chart.setAnnotations(B.anno);return y};var BChart={url:"loglist.php",toggle:function(b){this.chart.toggleLine(b)},reqdata:function(a){var e=this;var g="dl="+a;var f=new XMLHttpRequest();f.open("GET",e.url+"?"+g);f.responseType="arraybuffer";f.onload=function(c){var b=new Uint8Array(this.response);if(b.length==0){console.log("zero content");return}e.chart.process(b)};f.onerror=function(){alert("unknown error!")};f.send()},init:function(b){this.chart=new BrewChart(b)}};var qs=(function(d){if(d==""){return{}}var c={};for(var e=0;e<d.length;++e){var f=d[e].split("=",2);if(f.length==1){c[f[0]]=""}else{c[f[0]]=decodeURIComponent(f[1].replace(/\+/g," "))}}return c})(window.location.search.substr(1).split("&"));function loaded(){BChart.init("div_g");BChart.reqdata(qs.dl)};/*]]>*/</script>
<style>.chart-legend-row .toggle{width:8px;height:8px;border-radius:5px;float:left;margin:2px 0 0 0;cursor:pointer;border:1px solid}.chart-legend{font-family:Lucida Grande,Lucida Sans,Arial,sans-serif;font-size:11px;margin:10px 0 0 0;border:solid 1px #777;border-radius:5px;float:right;width:155px}.chart-legend-row{padding:8px 5px 8px 5px}.legend-label{float:left;padding:0 5px 0 5px;cursor:pointer}.legend-value{float:right}.chart-legend-row.time{background-color:#def}#div_lb{display:none}#div_g{float:left;width:800px;height:390px}#chart-container{width:975px;height:410px;border-radius:5px;margin:5px 5px 5px 5px;border:solid 1px #304d75;padding:5px 5px 5px 5px}.hide{display:none}</style>
<body onload="loaded()">
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
</body>
</html>
)END";