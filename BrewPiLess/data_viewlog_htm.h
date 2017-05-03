const char data_viewlog_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<title>BrewPiLess Log Viewer</title>
<script src="http://cdnjs.cloudflare.com/ajax/libs/dygraph/1.1.1/dygraph-combined.js"></script>
<script>/*<![CDATA[*/var Q=function(d){return document.querySelector(d)};var BrewChart=function(a){this.cid=a;this.ctime=0;this.interval=60;this.numLine=7;this.lidx=0;this.celius=true;this.clearData()};BrewChart.prototype.clearData=function(){this.laststat=[NaN,NaN,NaN,NaN,NaN,NaN,NaN];this.sg=NaN;this.og=NaN};BrewChart.prototype.setCelius=function(c){this.celius=c;this.ylabel(STR.ChartLabel+'('+(c?"°C":"°F")+')')};BrewChart.prototype.incTime=function(){this.ctime+=this.interval};BrewChart.prototype.formatDate=function(d){var a=d.getHours();var b=d.getMinutes();var c=d.getSeconds();function T(x){return(x>9)?x:("0"+x)}return d.toLocaleDateString()+" "+T(a)+":"+T(b)+":"+T(c)};BrewChart.prototype.showLegend=function(a,b){var d=new Date(a);Q(".beer-chart-legend-time").innerHTML=this.formatDate(d);Q(".chart-legend-row.beerTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,2));Q(".chart-legend-row.beerSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,1));Q(".chart-legend-row.fridgeTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,3));Q(".chart-legend-row.fridgeSet .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,4));Q(".chart-legend-row.roomTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,5));Q(".chart-legend-row.auxTemp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(b,6));var g=this.chart.getValue(b,7);Q(".chart-legend-row.gravity .legend-value").innerHTML=(!g||isNaN(g))?"--":g.toFixed(3);var c=parseInt(this.state[b]);if(!isNaN(c)){Q('.chart-legend-row.state .legend-label').innerHTML=STATES[c].text}};BrewChart.prototype.hideLegend=function(){var v=document.querySelectorAll(".legend-value");v.forEach(function(a){a.innerHTML="--"});Q(".beer-chart-legend-time").innerHTML=this.dateLabel;Q('.chart-legend-row.state .legend-label').innerHTML="state"};BrewChart.prototype.tempFormat=function(y){var v=parseFloat(y);if(isNaN(v))return"--";var a=this.celius?"°C":"°F";return parseFloat(v).toFixed(2)+a};BrewChart.prototype.initLegend=function(){Q(".chart-legend-row.beerTemp").style.color=BrewChart.Colors[1];Q(".beerTemp .toggle").style.backgroundColor=BrewChart.Colors[1];Q(".chart-legend-row.beerSet").style.color=BrewChart.Colors[0];Q(".beerSet .toggle").style.backgroundColor=BrewChart.Colors[0];Q(".chart-legend-row.fridgeTemp").style.color=BrewChart.Colors[2];Q(".fridgeTemp .toggle").style.backgroundColor=BrewChart.Colors[2];Q(".chart-legend-row.fridgeSet").style.color=BrewChart.Colors[3];Q(".fridgeSet .toggle").style.backgroundColor=BrewChart.Colors[3];Q(".chart-legend-row.roomTemp").style.color=BrewChart.Colors[4];Q(".roomTemp .toggle").style.backgroundColor=BrewChart.Colors[4];Q(".chart-legend-row.gravity").style.color=BrewChart.Colors[6];Q(".gravity .toggle").style.backgroundColor=BrewChart.Colors[6];Q(".chart-legend-row.auxTemp").style.color=BrewChart.Colors[5];Q(".auxTemp .toggle").style.backgroundColor=BrewChart.Colors[5];this.dateLabel=Q(".beer-chart-legend-time").innerHTML};BrewChart.prototype.toggleLine=function(a){this.shownlist[a]=!this.shownlist[a];if(this.shownlist[a]){Q("."+a+" .toggle").style.backgroundColor=Q(".chart-legend-row."+a).style.color;this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,true)}else{Q("."+a+" .toggle").style.backgroundColor="transparent";this.chart.setVisibility(this.chart.getPropertiesForSeries(a).column-1,false)}};BrewChart.prototype.createChart=function(){var t=this;t.initLegend();t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true};var c=document.createElement("div");c.className="hide";document.body.appendChild(c);var d={labels:BrewChart.Labels,colors:BrewChart.Colors,connectSeparatedPoints:true,ylabel:'Temperature',y2label:'Gravity',series:{'gravity':{axis:'y2',drawPoints:true,pointSize:2,highlightCircleSize:4}},axisLabelFontSize:12,animatedZooms:true,gridLineColor:'#ccc',gridLineWidth:'0.1px',labelsDiv:c,labelsDivStyles:{'display':'none'},displayAnnotations:true,strokeWidth:1,axes:{y:{valueFormatter:function(y){return t.tempFormat(y)}},y2:{valueFormatter:function(y){return y.toFixed(3)},axisLabelFormatter:function(y){return y.toFixed(3).substring(1)}}},highlightCircleSize:2,highlightSeriesOpts:{strokeWidth:1.5,strokeBorderWidth:1,highlightCircleSize:5},highlightCallback:function(e,x,a,b){t.showLegend(x,b)},unhighlightCallback:function(e){t.hideLegend()}};t.chart=new Dygraph(document.getElementById(t.cid),t.data,d)};var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};BrewChart.Colors=["rgb(240, 100, 100)","rgb(41,170,41)","rgb(89, 184, 255)","rgb(255, 161, 76)","#AAAAAA","#f5e127","rgb(153,0,153)"];BrewChart.Labels=['Time','beerSet','beerTemp','fridgeTemp','fridgeSet','roomTemp','auxTemp','gravity'];BrewChart.prototype.addMode=function(m){var s=String.fromCharCode(m);this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:s.toUpperCase(),text:BrewChart.Mode[s],attachAtBottom:true})};BrewChart.testData=function(a){if(a[0]!=0xFF)return false;var s=a[1]&0x07;if(s!=5)return false;return{sensor:s,f:a[1]&0x10}};BrewChart.prototype.addResume=function(a){this.ctime+=a;this.anno.push({series:"beerTemp",x:this.ctime*1000,shortText:'R',text:'Resume',attachAtBottom:true})};BrewChart.prototype.process=function(a){var b=false;var t=this;for(var i=0;i<a.length;){var c=a[i++];var e=a[i++];if(c==0xFF){if((e&0xF)!=5){alert("log version mismatched!");return}t.celius=(e&0x10)?false:true;var p=a[i++];p=p*256+a[i++];t.interval=p;t.starttime=(a[i]<<24)+(a[i+1]<<16)+(a[i+2]<<8)+a[i+3];t.ctime=t.starttime;i+=4;t.data=[];t.anno=[];t.state=[];t.cstate=0;this.clearData();b=true}else if(c==0xF4){t.addMode(e)}else if(c==0xF1){t.cstate=e}else if(c==0xFE){if(t.lidx){var f;for(f=t.lidx;f<t.numLine;f++)t.dataset.push(NaN);t.data.push(t.dataset)}t.lidx=0;t.addResume(e)}else if(c==0xF8){var g=a[i++];var h=a[i++];var v=(g&0x7F)*256+h;t.og=v/1000}else if(c==0xF0){t.changes=e;t.lidx=0;var d=new Date(this.ctime*1000);t.incTime();t.dataset=[d];t.processRecord()}else if(c<128){var j=c*256+e;if(t.lidx==t.numLine-1){j=(j==0x7FFF)?NaN:j/1000;t.sg=j}else{j=(j==0x7FFF)?NaN:j/100}if(t.lidx<t.numLine){if(typeof t.dataset!="undefined"){t.dataset.push(j);t.laststat[t.lidx]=(t.lidx>=t.numLine-2)?null:j;t.lidx++;t.processRecord()}else{console.log("Error: missing tag.")}}else{console.log("Error: data overlap?")}}}if(typeof t.chart=="undefined")t.createChart();else t.chart.updateOptions({'file':t.data});t.chart.setAnnotations(t.anno);return b};BrewChart.prototype.processRecord=function(){var t=this;while((((1<<t.lidx)&t.changes)==0)&&t.lidx<t.numLine){t.dataset.push(t.laststat[t.lidx]);t.lidx++}if(t.lidx>=t.numLine){t.data.push(t.dataset);t.state.push(t.cstate)}};var BChart={url:'loglist.php',toggle:function(a){this.chart.toggleLine(a)},reqdata:function(b){var t=this;var c='dl='+b;var d=new XMLHttpRequest();d.open('GET',t.url+'?'+c);d.responseType='arraybuffer';d.onload=function(e){var a=new Uint8Array(this.response);if(a.length==0){console.log("zero content");return}t.chart.process(a)};d.onerror=function(){alert("unknown error!")};d.send()},init:function(a){this.chart=new BrewChart(a)}};var qs=(function(a){if(a=="")return{};var b={};for(var i=0;i<a.length;++i){var p=a[i].split('=',2);if(p.length==1)b[p[0]]="";else b[p[0]]=decodeURIComponent(p[1].replace(/\+/g," "))}return b})(window.location.search.substr(1).split('&'));function loaded(){BChart.init("div_g");BChart.reqdata(qs["dl"])};/*]]>*/</script>
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




















































































































