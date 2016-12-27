const char data_viewlog_htm[] PROGMEM =
R"END(
<html>
<head>
<title>BrewPiLess Log Viewer</title>
<script src="http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js"></script>
<script>/*<![CDATA[*/var Q=function(d){return document.querySelector(d)};var BrewChart=function(a){this.cid=a;this.ctime=0;this.interval=60;this.numLine=4;this.lidx=0;this.celius=true;this.beerSet=null};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.prototype.setCelius=function(c){this.celius=c;this.ylabel(STR.ChartLabel+'('+(c?"°C":"°F")+')')};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(d){var a=d.getHours();var b=d.getMinutes();var c=d.getSeconds();function T(x){return(x>9)?x:("0"+x)}return d.toLocaleDateString()+" "+T(a)+":"+T(b)+":"+T(c)};BrewChart.prototype.showLegend=function(a,b){var d=new Date(a);Q(".beer-chart-legend-time").innerHTML=this.formatDate(d);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,4));var c=parseInt(this.state[b]);if(!isNaN(c)){Q('.chart-legend-row.state .legend-label').innerHTML=STATES[c].text}};BrewChart.prototype.hideLegend=function(){var v=document.querySelectorAll(".legend-value");v.forEach(function(a){a.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML="Date/Time";Q('.chart-legend-row.state .legend-label').innerHTML="state"};BrewChart.prototype.tempFormat=function(y){var v=parseFloat(y);if(isNaN(v))return"--";var a=this.celius?"°C":"°F";return parseFloat(v).toFixed(2)+a};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4]};BrewChart.prototype.toggleLine=function(a){this.shownlist[a]=!this.shownlist[a];if(this.shownlist[a]){Q("."+a+" .toggle").style.backgroundColor=Q(".chart-legend-row."+a).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,true)}else{Q("."+a+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,false)}};BrewChart.prototype.createChart=function(){var t=this;t.initLegend();t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true};var c=document.createElement("div");c.className="hide";document.body.appendChild(c);t.chart=new Dygraph(document.getElementById(t.cid),t.data,{labels:BrewChart.Labels,colors:BrewChart.Colors,axisLabelFontSize:12,animatedZooms:true,gridLineColor:'#ccc',gridLineWidth:'0.1px',labelsDiv:c,labelsDivStyles:{'display':'none'},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(y){t.tempFormat(y)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(e,x,a,b){t.showLegend(x,b)},unhighlightCallback:function(e){t.hideLegend()}})};BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","rgb(153,0,153)"];BrewChart.Labels=['Time','beerSet','beerTemp','fridgeTemp','fridgeSet','roomTemp'];BrewChart.prototype.addMode=function(m){var s=String.fromCharCode(m);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:s.toUpperCase(),text:BrewChart.Mode[s],attachAtBottom:true})};BrewChart.testData=function(a){if(a[0]!=0xFF)return false;var s=a[1]&0x07;if(s>5)return false;return{sensor:s,f:a[1]&0x10}};BrewChart.prototype.addResume=function(a){this.ctime+=a;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:'R',text:'Resume',attachAtBottom:true})};BrewChart.prototype.process=function(a){var t=this;for(var i=0;i<a.length;){var b=a[i++];var c=a[i++];if(b==0xFF){t.celius=(c&0x10)?false:true;var p=a[i++];p=p*256+a[i++];t.interval=p;t.starttime=(a[i]<<24)+(a[i+1]<<16)+(a[i+2]<<8)+a[i+3];t.ctime=t.starttime;i+=4;t.data=[];t.anno=[];t.state=[];t.cstate=0}else if(b==0xF4){t.addMode(c)}else if(b==0xF1){t.cstate=c}else if(b==0xF7){var e=a[i++];var f=a[i++];var v=(e&0x7F)*256+f;t.beerSet=(v==0x7FFF)?null:(v/100)}else if(b==0xFE){if(t.lidx){var i;for(i=t.lidx;i<t.numLine;i++)t.dataset.push(NaN);t.data.push(t.dataset)}t.lidx=0;t.addResume(c)}else if(b<128){if(t.lidx==0){var d=new Date(this.ctime*1000);var g=t.beerSet;t.dataset=[d,g];t.incTime()}var h=b*256+c;if(h==0x7FFF||h>12000){h=NaN}else{h=h/100}t.dataset.push(h);if(++t.lidx>=t.numLine){t.lidx=0;t.data.push(t.dataset);t.state.push(t.cstate)}}}if(typeof t.chart=="undefined")t.createChart();else t.chart.updateOptions({'file':t.data});t.chart.setAnnotations(t.anno)};var BChart={url:'loglist.php',toggle:function(a){this.chart.toggleLine(a)},reqdata:function(b){var t=this;var c='dl='+b;var d=new XMLHttpRequest();d.open('GET',t.url+'?'+c);d.responseType='arraybuffer';d.onload=function(e){var a=new Uint8Array(this.response);if(a.length==0){console.log("zero content");return}t.chart.process(a)};d.onerror=function(){alert("unknown error!")};d.send()},init:function(a){this.chart=new BrewChart(a)}};var qs=(function(a){if(a=="")return{};var b={};for(var i=0;i<a.length;++i){var p=a[i].split('=',2);if(p.length==1)b[p[0]]="";else b[p[0]]=decodeURIComponent(p[1].replace(/\+/g," "))}return b})(window.location.search.substr(1).split('&'));function loaded(){BChart.init("div_g");BChart.reqdata(qs["dl"])}/*]]>*/</script>
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
<div class="chart-legend-row state">
<div class="legend-label">state</div>
<br>
</div>
</div>
</div>
</body>
</html>
)END";


