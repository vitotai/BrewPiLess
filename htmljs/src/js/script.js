/**********************************************************************
created by Vito Tai
Copyright (C) 2016 Vito Tai

This soft ware is provided as-is. Use at your own risks.
You are free to modify and distribute this software without removing
this statement.
BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/
/* chart.js */
function s_ajax(b){var c=new XMLHttpRequest();c.onreadystatechange=function(){if(c.readyState==4){if(c.status==200){b.success(c.responseText)}else{c.onerror(c.status)}}};c.ontimeout=function(){if(typeof b["timeout"]!="undefined")b.timeout();else c.onerror(-1)},c.onerror=function(a){if(typeof b["fail"]!="undefined")b.fail(a)};c.open(b.m,b.url,true);if(typeof b["data"]!="undefined"){c.setRequestHeader("Content-Type",(typeof b["mime"]!="undefined")?b["mime"]:"application/x-www-form-urlencoded");c.send(b.data)}else c.send()}

var Q=function(d){return document.querySelector(d);};
// gravity tracking
var GravityFilter={
  b:0.1,
  y:0,
  add:function(gravity){
    if(this.y ==0) this.y = gravity;
    else this.y = this.y + this.b * (gravity - this.y);
    return Math.round(this.y * 10000)/10000;
  },
  setBeta:function(beta){
    this.b = beta;
  }
};
var GravityTracker={
  NumberOfSlots:48,
  InvalidValue:0xFF,
  ridx:0,
  record:[],
  threshold:1,
  setThreshold:function(t){
    this.threshold=t;
  },
  addRecord:function(v){
    this.record[this.ridx++]=v;
    if(this.ridx >=this.NumberOfSlots) this.ridx=0;
  },
  stable:function(duration,to){
    to = (typeof to == "undefined")? this.threshold:to;
    var current = this.ridx -1;
    if(current < 0) current =this.NumberOfSlots-1;
    var previous = this.NumberOfSlots + this.ridx - duration ;
    while(previous >= this.NumberOfSlots) previous -= this.NumberOfSlots;
    return (this.record[previous] - this.record[current]) <= to;
  },
  Period:60*60,
  init:function(){
    this.curerntStart = 0;
    this.lastValue=0;
  },
  add:function(fgravity,time){
    gravity = Math.round(fgravity * 1000,1);
    var timediff = time - this.curerntStart;

    if(timediff > this.Period){
      this.addRecord(gravity);
      if(this.lastValue !=0){
        timediff -=this.Period;
        while(timediff > this.Period){
          timediff -=this.Period;
          this.addRecord(this.lastValue);
        }
      }
      this.curerntStart=time;
      this.lastValue=gravity;
    }
  }
};


function fgstate(duration)
{
  var Color={0:"red",12:"orange",24:"yellow",48:"green"};
  Q("#fgstate").style.backgroundColor=Color[duration];
}

function checkfgstate()
{
  if(GravityTracker.stable(12)){
    if(GravityTracker.stable(24)){
      if(GravityTracker.stable(48)) fgstate(48);
      else fgstate(24); // 24
    }else fgstate(12); //
  }else fgstate(0);
}
// gravity tracking

var BrewChart=function(div){
  this.cid=div;
  this.ctime=0;
  this.interval=60;
  this.numLine=7;
  this.lidx=0;
  this.celius=true;
  this.clearData();
};

BrewChart.prototype.clearData=function(){
  this.laststat=[NaN,NaN,NaN,NaN,NaN,NaN,NaN,NaN];
  this.sg=NaN;
  this.og=NaN;
};

BrewChart.prototype.setCelius=function(c){
  this.celius=c;
  this.ylabel(STR.ChartLabel +'(' + (c? "°C":"°F") +')');
};

BrewChart.prototype.incTime=function(){
  // format time, use hour and minute only.
  this.ctime += this.interval;
  //	console.log("incTime:"+ this.ctime/this.interval);
};

BrewChart.prototype.formatDate=function(d){
  var HH=d.getHours();
  var MM=d.getMinutes();
  var SS=d.getSeconds();
  function T(x){return (x>9)? x:("0"+x); }
  return d.toLocaleDateString() + " "+ T(HH) +":"+T(MM)+":"+T(SS);
};

BrewChart.prototype.showLegend=function(date,row){
  var d=new Date(date);
  Q(".beer-chart-legend-time").innerHTML = this.formatDate(d);
  Q(".chart-legend-row.beer-temp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 2));
  Q(".chart-legend-row.beer-set .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 1));
  Q(".chart-legend-row.fridge-temp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 3));
  Q(".chart-legend-row.fridge-set .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 4));
  Q(".chart-legend-row.room-temp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 5));
  Q(".chart-legend-row.aux-temp .legend-value").innerHTML=this.tempFormat(this.chart.getValue(row, 6));

  var g=this.chart.getValue(row, 7);
  Q(".chart-legend-row.gravity .legend-value").innerHTML=(!g || isNaN(g))? "--":g.toFixed(4);
  var filteredG=this.chart.getValue(row, 8);
  Q(".chart-legend-row.filtersg .legend-value").innerHTML=(!filteredG || isNaN(filteredG))? "--":filteredG.toFixed(4);

  var state = parseInt(this.state[row]);
  if ( !isNaN(state) ) {
    Q('.beer-chart-state').innerHTML=STATES[state].text;
  }
};

BrewChart.prototype.hideLegend=function(){
  var v=document.querySelectorAll(".legend-value");
  v.forEach(function(val){
    val.innerHTML = "--";
  });
  Q(".beer-chart-legend-time").innerHTML =this.dateLabel; //"Date/Time";
  Q('.beer-chart-state').innerHTML="state";
};

BrewChart.prototype.tempFormat=function(y){
  var v=parseFloat(y);
  if(isNaN(v)) return "--";
  var DEG=this.celius? "°C":"°F";
  return parseFloat(v).toFixed(2) + DEG;
};
BrewChart.prototype.initLegend=function(){
  this.dateLabel=Q(".beer-chart-legend-time").innerHTML;
};
BrewChart.prototype.toggleLine=function(line){
  this.shownlist[line] = !this.shownlist[line];
  if(this.shownlist[line]){
    this.chart.setVisibility(this.chart.getPropertiesForSeries(line).column-1, true);
  }else{
    this.chart.setVisibility(this.chart.getPropertiesForSeries(line).column-1, false);
  }
};

BrewChart.prototype.createChart=function(){
  var t=this;
  t.initLegend();
  t.shownlist={beerTemp:true,beerSet:true,fridgeSet:true,fridgeTemp:true,roomTemp:true,gravity:true,auxTemp:true,filtersg:true};

  var ldiv=document.createElement("div");
  ldiv.className="hide";
  document.body.appendChild(ldiv);
  var opt= {
    labels: BrewChart.Labels,
    colors: BrewChart.Colors,
    connectSeparatedPoints: true,
    ylabel:'Temperature',
    y2label:'Gravity',
    series:{
      'gravity':{ axis: 'y2',
      drawPoints: true,
      pointSize: 2,
      highlightCircleSize: 4
    },
    'filtersg':{ axis: 'y2',
  }
},
axisLabelFontSize:12,
animatedZooms: true,
gridLineColor:'#ccc',
gridLineWidth:'0.1px',
labelsDiv:ldiv,
labelsDivStyles:{ 'display': 'none' },
displayAnnotations:true,
//showRangeSelector: true,
strokeWidth: 1,
axes: {
  y : { valueFormatter: function(y){return t.tempFormat(y);}},
  y2: { valueFormatter: function(y){return y.toFixed(3);}, axisLabelFormatter:function(y){return y.toFixed(3).substring(1);}}
},
highlightCircleSize: 2,
highlightSeriesOpts: {
  strokeWidth: 1.5,
  strokeBorderWidth: 1,
  highlightCircleSize: 5
},
highlightCallback: function(e, x, pts, row) {
  t.showLegend(x, row);
},
unhighlightCallback: function(e) {
  t.hideLegend();
}
//underlayCallback: paintBackground,
/*                drawCallback: function(beerChart, is_initial) {
if (is_initial) {
if (t.anno.length > 0) {
t.chart.setAnnotations(t.anno);
}
}
}*/
};
t.chart = new Dygraph(document.getElementById(t.cid),t.data,opt);
};
var STATES=[{name:"IDLE",text:"Idle"},{name:"STATE_OFF",text:"Off"},{name:"DOOR_OPEN",text:"Door Open",doorOpen:true},{name:"HEATING",text:"Heating"},{name:"COOLING",text:"Cooling"},{name:"WAITING_TO_COOL",text:"Waiting to Cool",waiting:true},{name:"WAITING_TO_HEAT",text:"Waiting to Heat",waiting:true},{name:"WAITING_FOR_PEAK_DETECT",text:"Waiting for Peak",waiting:true},{name:"COOLING_MIN_TIME",text:"Cooling Min Time",extending:true},{name:"HEATING_MIN_TIME",text:"Heating Min Time",extending:true}];
BrewChart.Mode={b:"Beer Constant",f:"Fridge Constant",o:"Off",p:"Profile"};
BrewChart.Labels=['Time','beerSet', 'beerTemp','fridgeTemp','fridgeSet','roomTemp','auxTemp','gravity','filtersg'];

BrewChart.prototype.addMode=function(m){
  var s=String.fromCharCode(m);
  this.anno.push({
    series:"beerTemp",
    x:this.ctime * 1000,
    shortText:s.toUpperCase(),
    text:BrewChart.Mode[s],
    attachAtBottom: true
  });
};

BrewChart.testData=function(data){
  if(data[0] != 0xFF) return false;
  var s=data[1] & 0x07;
  if(s !=5) return false;

  return {sensor:s, f:data[1]&0x10};
};

BrewChart.prototype.addResume=function(delta){
  this.ctime += delta * 60;
  this.anno.push({
    series:"beerTemp",
    x:this.ctime * 1000,
    shortText:'R',
    text:'Resume',
    attachAtBottom: true
  });
};

BrewChart.prototype.process=function(data){
  var newchart=false;
  var t=this;
  t.filterSg=null;
  for (var i = 0; i < data.length;) {
    var d0=data[i++];
    var d1=data[i++];
    if(d0 == 0xFF){ // header.
      if( (d1 & 0xF) != 5) {
        alert("log version mismatched!");
        return;
      }
      //console.log(""+t.ctime/t.interval +" header");
      t.celius = (d1&0x10)? false:true;

      var p=data[i++];
      p =p * 256 + data[i++];
      t.interval= p;
      //
      t.starttime = (data[i] << 24) + (data[i+1] << 16) + (data[i+2] << 8) + data[i+3];
      t.ctime=t.starttime;
      i+=4;
      t.data = [];
      t.anno=[];
      t.state=[];
      t.cstate=0;
      this.clearData();
      newchart=true;
      // gravity tracking
      GravityTracker.init();
      // gravity tracking

    }else if(d0 == 0xF4){ // mode
      //console.log(""+t.ctime/t.interval +" Stage:"+d1);
      t.addMode(d1);
    }else if(d0 == 0xF1){ // state
      t.cstate = d1;
    }else if(d0 == 0xFE){ // resume
      if(t.lidx){
        var idx;
        for(idx=t.lidx;idx < t.numLine;idx++) t.dataset.push(NaN);
        t.data.push(t.dataset);
      }
      t.lidx=0;
      t.addResume(d1);
    }else if(d0 == 0xF8){
      var hh=data[i++];
      var ll=data[i++];
      var v=(hh & 0x7F) * 256 +ll;
      t.og =  v/10000;

    }else if(d0 == 0xF0){
      t.changes= d1;
      t.lidx=0;
      var d=new Date(this.ctime * 1000);
      t.incTime(); // add one time interval
      t.dataset=[d];
      t.processRecord();

    }else if(d0 < 128){   // temp.
      var tp = d0 * 256 + d1;
      if(t.lidx == t.numLine-1){
        tp =(tp == 0x7FFF)? NaN:((tp > 8000)? tp/10000:tp/1000);
        t.sg= tp;
        // gravity tracking
        if(!isNaN(tp)){
          t.filterSg=GravityFilter.add(tp);
          GravityTracker.add(t.filterSg,t.ctime);
        }
        // gravity tracking

      }else{
        tp =(tp == 0x7FFF)? NaN:tp/100;
        if(tp >= 225) tp = 225 - tp;
      }

      if(t.lidx < t.numLine){
        if(typeof t.dataset !="undefined"){
          t.dataset.push(tp);
          t.laststat[t.lidx] =(t.lidx >= t.numLine-2)? null:tp;
          t.lidx ++;
          t.processRecord();
        }else{
          console.log("Error: missing tag.");
        }
      }else{
        console.log("Error: data overlap?");
      }
    }
  }
  if(typeof t.chart == "undefined") t.createChart();
  else t.chart.updateOptions( { 'file': t.data } );
  t.chart.setAnnotations(t.anno);
  return newchart;
};
BrewChart.prototype.processRecord=function(){
  var t=this;
  while( (((1<<t.lidx) & t.changes ) ==0) && t.lidx < t.numLine){
    t.dataset.push(t.laststat[t.lidx]);
    t.lidx ++;
  }
  if(t.lidx >= t.numLine){

    if(!isNaN(t.sg)) t.dataset.push(t.filterSg);
    else t.dataset.push(null);

    t.data.push(t.dataset);
    t.state.push(t.cstate);
  }
};
/* end of chart.js */

/* profile.js */
var profileEditor={
  dirty:false,
  tempUnit:'C',
  C_startday_Id:"startdate",
  C_savebtn_Id:"savebtn",
  markdirty:function(d){
    this.dirty=d;
    document.getElementById(this.C_savebtn_Id).innerHTML=(d)? "Save*":"Save";
  },
  getStartDate:function(){return this.sd;},
  setStartDate:function(d){
  },
  startDayChange:function(){
    var nd=new Date(document.getElementById(this.C_startday_Id).value);
    if ( isNaN( nd.getTime())){
      // console.log("invalid date");
      document.getElementById(this.C_startday_Id).value=formatDate(this.sd);
    }else{
      // console.log(nd);
      this.sd=nd;
      this.reorg();
      this.markdirty(true);
    }
  },
  startnow:function(){
    var d=new Date();
    document.getElementById(this.C_startday_Id).value=formatDate(d);
    this.sd=d;
    this.reorg();
    this.markdirty(true);
  },
  rowList:function(){
    var tb=document.getElementById("profile_t").getElementsByTagName("tbody")[0];
    return tb.getElementsByTagName("tr");
  },
  sgChange:function(td){
    if(!isNaN(td.innerHTML) || td.innerHTML.match(/^[\d]+%$/) || td.innerHTML==""){
      td.saved = td.innerHTML;
      this.markdirty(true);
    }else{
      td.innerHTML = td.saved;
    }
  },
  dayChange:function(td){
    if(td.innerHTML=="" || isNaN(td.innerHTML))
    td.innerHTML = td.saved;
    else{
      this.markdirty(true);
      this.reorg();
      ControlChart.update(this.chartdata(),this.tempUnit);
    }
  },
  tempChange:function(td){
    if(td.innerHTML=="" ||  isNaN(td.innerHTML))
    td.innerHTML = td.saved;
    else{
      this.markdirty(true);
      ControlChart.update(this.chartdata(),this.tempUnit);
    }
  },
  stableChange:function(td){
    if(td.innerHTML.match(/^\s*(\d+)@(\d+)\s*$/)){
      td.saved = td.innerHTML;
      this.markdirty(true);
    }else if(! isNaN(td.innerHTML)){
      td.saved = parseInt(td.innerHTML);
      this.markdirty(true);
    }else{
      td.innerHTML = td.saved;
    }
  },
  initrow:function(tr,stage){
    var b=this;
    // temp setting
    var type=stage.c;
    tr.type=type;
    var tdTemp=tr.getElementsByClassName("stage-temp")[0];

    if(type == "r"){
      tdTemp.innerHTML="";
    }else{
      tdTemp.innerHTML=stage.t;
      tdTemp.contentEditable=true;
      tdTemp.onblur=function(){b.tempChange(this);};
      tdTemp.onfocus=function(){this.saved= this.innerHTML;};
    }
    // day setting
    var tdDay=tr.getElementsByClassName("stage-time")[0];
    tdDay.innerHTML=stage.d;
    tdDay.contentEditable=true;
    tdDay.onblur=function(){b.dayChange(this);};
    tdDay.onfocus=function(){this.saved= this.innerHTML;};

    // stable setting
    var tdStable=tr.getElementsByClassName("stage-stabletime")[0];
    // sg. only valid for hold
    var tdSG=tr.getElementsByClassName("stage-sg")[0];

    if(type == "r"){
      tdSG.innerHTML="";
      tdStable.innerHTML="";
    }else{
      tdSG.saved=stage.g;
      tdSG.innerHTML=(typeof stage.g == "undefined")? "":stage.g;
      tdSG.contentEditable=true;
      tdSG.onblur=function(){b.sgChange(this);};
      tdSG.onfocus=function(){this.saved= this.innerHTML;};
      if(typeof stage.s == "undefined") tdStable.innerHTML="";
      else tdStable.innerHTML=(typeof stage.x == "undefined")? stage.s:stage.x+"@"+stage.s;
      tdStable.contentEditable=true;
      tdStable.onblur=function(){b.stableChange(this);};
      tdStable.onfocus=function(){this.saved= this.innerHTML;};
    }


    var forTime=tr.getElementsByClassName("for-time")[0];
    // condition, only valid for hold
    var conSel=tr.getElementsByClassName("condition")[0];
    /*
    <option value="t" 0>Time</option>
    <option value="g" 1>SG</option>
    <option value="s" 2>Stable</option>
    <option value="a" 3>Time & SG</option>
    <option value="o" 4>Time OR SG</option>
    <option value="u" 5>Time OR Stable</option>
    <option value="v" 6>Time & Stable</option>
    <option value="b" 7>SG OR Stable</option>
    <option value="x" 8>SG & Stable</option>
    <option value="w" 9>ALL</option>
    <option value="e" 10>Either</option>
    */
    var condtionIndex={t:0,g:1,a:3,s:2,o:4,u:5,v:6,b:7,x:8,w:9,e:10};
    if(type == "r"){
      forTime.style.display="block";
      conSel.style.display="none";

    }else{
      conSel.value=stage.c;
      conSel.selectedIndex=condtionIndex[stage.c];

      forTime.style.display="none";
      conSel.style.display="block";
    }
  },

  datestr:function(diff){
    var dt =new Date(this.sd.getTime() + Math.round(diff*86400)*1000);
    return formatDate(dt);
  },
  reorg:function(){
    var rowlist=this.rowList();
    var utime= this.sd.getTime();
    for(var i=0;i<rowlist.length;i++){
      var row = rowlist[i];
      row.className = (i %2)? "odd":"even";
      row.getElementsByClassName("diaplay-time")[0].innerHTML=formatDate(new Date(utime));
      var time=  this.rowTime(row);
      utime += Math.round( time *86400)*1000;
    }
  },
  chartdata:function(){
    var rowlist=this.rowList();
    if(rowlist.length ==0) return [];

    var utime= this.sd.getTime();
    var row = rowlist[0];
    var start=this.rowTemp(row);

    var list=[];
    list.push([new Date(utime),start]);

    for(var i=0;i<rowlist.length;i++){
      var row = rowlist[i];
      var temp;
      if(row.type == "r"){
        temp=this.rowTemp(rowlist[i+1]);
      }else{
        temp=this.rowTemp(row);
      }
      utime += Math.round( this.rowTime(row) *86400)*1000;
      list.push([new Date(utime),temp]);
    }
    return list;
  },
  addRow:function(){
    var tb=document.getElementById("profile_t").getElementsByTagName("tbody")[0];
    var rowlist=tb.getElementsByTagName("tr");

    if(rowlist.length >= 13){
      alert("Too many steps!");
      return;
    }
    var stage;

    if(rowlist.length==0){
      var init=(this.tempUnit == 'C')? 20:68;
      stage={c:'t',t:init,d:1,g:1.01};
    }else{
      var lastRow=rowlist[rowlist.length-1];

      var tr= this.row.cloneNode(true);
      this.initrow(tr,{c:"r",d:1});
      tb.appendChild(tr);
      stage={c:'t',t:this.rowTemp(lastRow),d:1,g:""};
    }

    var tr= this.row.cloneNode(true);
    this.initrow(tr,stage);
    tb.appendChild(tr);

    this.reorg();
    this.markdirty(true);
    ControlChart.update(this.chartdata(),this.tempUnit);
  },
  delRow:function(){
    // delete last row
    var list=this.rowList();
    if(list.length == 0) return;
    var last = list[list.length -1];

    if(list.length > 1){
      var lr = list[list.length -2];
      lr.parentNode.removeChild(lr);
    }

    last.parentNode.removeChild(last);

    this.markdirty(true);
    ControlChart.update(this.chartdata(),this.tempUnit);
  },
  rowTemp:function(row){
    return parseFloat(row.getElementsByClassName("stage-temp")[0].innerHTML);
  },
  rowCondition:function(row){
    return row.getElementsByClassName("condition")[0].value;
  },
  rowTime:function(row){
    return parseFloat(row.getElementsByClassName("stage-time")[0].innerHTML);
  },
  rowSg:function(row){
    return row.getElementsByClassName("stage-sg")[0].saved;
  },
  rowSt:function(row){
    var data=row.getElementsByClassName("stage-stabletime")[0].innerHTML;
    if(typeof data != "string") return data;
    var matches=data.match(/^\s*(\d+)@(\d+)\s*$/);
    if(matches){
      return parseInt(matches[2]);
    }else{
      return parseInt(data);
    }
  },
  rowStsg:function(row){
    var data=row.getElementsByClassName("stage-stabletime")[0].innerHTML;
    if(typeof data != "string") return false;
    var matches=data.match(/^\s*(\d+)@(\d+)\s*$/);
    if(matches){
      return parseInt(matches[1]);
    }else{
      return false;
    }
  },
  renderRows: function(g) {
    var e = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
    for (var f = 0; f < g.length; f++) {
      var c = this.row.cloneNode(true);
      this.initrow(c, g[f]);
      e.appendChild(c)
    }
    this.reorg()
  },

  initable: function(c, e) {
    this.sd = e;
    document.getElementById(this.C_startday_Id).value = formatDate(e);
    var b = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
    this.row = b.getElementsByTagName("tr")[0];
    b.removeChild(this.row);
    this.renderRows(c)
  },
  clear:function(){
    var rl=this.rowList();

    var count=rl.length;
    for(var i=rl.length-1;i>=0;i--){
      var tr=rl[i];
      tr.parentNode.removeChild(tr);
    }
    this.markdirty(true);
  },
  getProfile:function(){
    var rl=this.rowList();
    var lastdate=0;
    var temps=[];
    for(var i=0;i<rl.length;i++){
      var tr=rl[i];
      var day= this.rowTime(tr);
      if(isNaN(day)) return false;

      if(tr.type == "r"){
        temps.push({c:"r",d:day});
      }else{
        var temp=  this.rowTemp(tr);
        if(isNaN(temp)) return false;
        if(temp > BrewPiSetting.maxDegree || temp < BrewPiSetting.minDegree) return false;

        /*
        <option value="t">Time</option>
        <option value="g">SG</option>
        <option value="s">Stable</option>
        <option value="a">Time & SG</option>
        <option value="o">Time OR SG</option>
        <option value="u">Time OR Stable</option>
        <option value="v">Time & Stable</option>
        <option value="b">SG OR Stable</option>
        <option value="x">SG & Stable</option>
        <option value="w">ALL</option>
        <option value="e">Either</option>
        */
        var condition=this.rowCondition(tr);
        var stage={c:condition,d:day,t:temp};

        var useSg= "gaobxwe";
        var gv= this.rowSg(tr);

        if(useSg.indexOf(condition)>=0){
          if(gv == "") return false;
          stage.g = gv;
        }
        var useStableTime="suvbxwe";
        var stv= this.rowSt(tr);
        if(useStableTime.indexOf(condition)>=0){
          if(isNaN(stv)) return false;
          stage.s = stv;
          var x =this.rowStsg(tr);
          if(x) stage.x=x;
        }

        temps.push(stage);

      }
    }
    var s=this.sd.toISOString();
    var ret= {s:s,v:2,u:this.tempUnit,t:temps};
    //console.log(ret);
    return ret;
  },
  loadProfile: function(a) {
    this.sd = new Date(a.s);
    this.tempUnit = a.u;
    this.clear();
    this.renderRows(a.t);
    ControlChart.update(this.chartdata(), this.tempUnit)
  },
  initProfile:function(p)
  {
    if(typeof p != "undefined"){
      // start date
      var sd=new Date(p.s);
      this.tempUnit = p.u;
      profileEditor.initable(p.t,sd);
    }else{
      profileEditor.initable([],new Date());
    }
  },
  setTempUnit:function(u){
    if(u == this.tempUnit) return;
    this.tempUnit = u;
    var rl=this.rowList();

    for(var i=0;i<rl.length;i++){
      var tcell=rl[i].querySelector('td.ptemp');
      var temp= parseFloat(tcell.innerHTML);
      tcell.innerHTML = (u=='C')? F2C(temp):C2F(temp);
    }
  }
};

/* end of profile.js */
/* PL: profle list */
var PL = {
  pl_path: "P",
  url_list: "/list",
  url_save: "/fputs",
  url_del: "/rm",
  url_load: "pl.php?ld=",
  div: "#profile-list-pane",
  shown: false,
  initialized: false,
  plist: [],
  path: function(a) {
    return "/" + this.pl_path + "/" + a
  },
  depath: function(a) {
    return a.substring(this.pl_path.length + 1)
  },
  rm: function(e) {
    var f = this;
    var c = "path=" + f.path(f.plist[e]);
    s_ajax({
      url: f.url_del,
      m: "DELETE",
      data: c,
      success: function(a) {
        f.plist.splice(e, 1);
        f.list(f.plist)
      },
      fail: function(a) {
        alert("failed:" + a)
      }
    })
  },
  load: function(e) {
    var f = this;
    var c = f.path(f.plist[e]);
    s_ajax({
      url: c,
      m: "GET",
      success: function(b) {
        var a = JSON.parse(b);
        profileEditor.loadProfile(a);
      },
      fail: function(a) {
        //alert("failed:" + a);
      }
    })
  },
  list: function(i) {
    var a = this;
    var h = Q(a.div).querySelector("tbody");
    var e;
    while (e = h.querySelector("tr:nth-of-type(1)")) {
      h.removeChild(e)
    }
    var b = a.row;
    i.forEach(function(f, g) {
      var c = b.cloneNode(true);
      c.querySelector(".profile-name").innerHTML = f;
      c.querySelector(".profile-name").onclick = function(j) {
        j.preventDefault();
        a.load(g);
        return false
      };
      c.querySelector(".rmbutton").onclick = function() {
        a.rm(g)
      };
      h.appendChild(c)
    })
  },
  append: function(b) {
    if (!this.initialized) {
      return
    }
    this.plist.push(b);
    this.list(this.plist)
  },
  init: function() {
    var a = this;
    a.initialized = true;
    a.row = Q(a.div).querySelector("tr:nth-of-type(1)");
    a.row.parentNode.removeChild(a.row);
    s_ajax({
      url: a.url_list,
      m: "POST",
      data: "dir=" + a.path(""),
      success: function(c) {
        a.plist = [];
        var b = JSON.parse(c);
        b.forEach(function(e) {
          if (e.type == "file") {
            a.plist.push(a.depath(e.name))
          }
        });
        a.list(a.plist)
      },
      fail: function(b) {
        alert("failed:" + b)
      }
    })
  },
  toggle: function() {
    if (!this.initialized) {
      this.init()
    }
    this.shown = !this.shown;
    if (this.shown) {
      Q(this.div).style.left = "0px"
    } else {
      Q(this.div).style.left = "-300px"
    }
  },
  saveas: function() {
    Q("#dlg_saveas").style.display = "block"
  },
  cancelSave: function() {
    Q("#dlg_saveas").style.display = "none"
  },
  doSave: function() {
    var e = Q("#dlg_saveas input").value;
    if (e == "") {
      return
    }
    if (e.match(/[\W]/g)) {
      return
    }
    var g = profileEditor.getProfile();
    if (g === false) {
      alert("invalid value. check again");
      return
    }
    var f = this;
    var c = "path=" + f.path(e) + "&content=" + encodeURIComponent(JSON.stringify(g));
    var f = this;
    s_ajax({
      url: f.url_save,
      m: "POST",
      data: c,
      success: function(a) {
        f.append(e);
        f.cancelSave()
      },
      fail: function(a) {
        alert("failed:" + a)
      }
    })
  }
};
/* end of PL*/
var BrewPiSetting={
  valid:false,
  maxDegree:30,
  minDegree:0,
  tempUnit:'C'
};
function formatDate(dt)
{
  //	var y = dt.getFullYear();
  //	var M = dt.getMonth() +1;
  //	var d = dt.getDate();
  var h = dt.getHours();
  var m = dt.getMinutes();
  //    var s = dt.getSeconds();
  function dd(n){return (n<10)? '0' + n:n;}
  //	return dd(M) + "/" + dd(d) + "/" + y +" "+ dd(h) +":"+dd(m)+":"+dd(s);
  //	return dd(M) + "/" + dd(d) +" "+ dd(h) +":"+dd(m);
  return dt.toLocaleDateString() + " "+ dd(h) +":"+dd(m);
}

var ControlChart={
  unit:"C",
  init:function(div, data,unit){
    var t=this;
    t.data=data;
    t.unit=unit;

    var dateFormatter=function(v){
      d=new Date(v);
      return formatDate(d);
    };
    var shortDateFormatter=function(v){
      d=new Date(v);
      var y= d.getYear() +1900;
      var re = new RegExp('[^\d]?'+y +'[^\d]?');
      var n=d.toLocaleDateString();
      return n.replace(re,"");
    };

    var temperatureFormatter=function(v){
      return v.toFixed(1) + "&deg;" + t.unit; }
      ;

      t.chart = new Dygraph(
        document.getElementById(div), t.data,
        {
          colors: [ 'rgb(89, 184, 255)' ],
          axisLabelFontSize:12,
          gridLineColor:'#ccc',
          gridLineWidth:'0.1px',
          labels:["Time","Temperature"],
          labelsDiv: document.getElementById(div + "-label"),
          legend: 'always',
          labelsDivStyles: { 'textAlign': 'right' },
          strokeWidth: 1,
          //        xValueParser: function(x) { return profileTable.parseDate(x); },
          //        underlayCallback: updateCurrentDateLine,
          //        "Temperature" : {},
          axes: {
            y : { valueFormatter: temperatureFormatter, pixelsPerLabel:20, axisLabelWidth: 35 },
            //            x : { axisLabelFormatter:dateFormatter, valueFormatter: dateFormatter, pixelsPerLabel: 30, axisLabelWidth:40 }
            x : { axisLabelFormatter:shortDateFormatter, valueFormatter:dateFormatter, pixelsPerLabel: 30, axisLabelWidth:40 }

          },
          highlightCircleSize: 2,
          highlightSeriesOpts: {
            strokeWidth: 1.5,
            strokeBorderWidth: 1,
            highlightCircleSize: 5
          },

        }
      );
    },
    update:function(data,unit){
      if(data.length == 0) return;
      this.data=data;
      this.unit=unit;
      this.chart.updateOptions( { 'file': this.data } );
    }
  };


  var modekeeper={
    initiated:false,
    modes:["profile","beer","fridge","off"],
    cmode:0,
    dselect:function(m){
      var d=document.getElementById(m + "-m");
      var nc=document.getElementById(m + "-m").className.replace(/\snav-selected/, '');
      d.className=nc;

      document.getElementById(m + "-s").style.display="none";
    },
    select:function(m){
      document.getElementById(m + "-m").className += ' nav-selected';
      document.getElementById(m + "-s").style.display="block";
    },
    init:function(){
      var me=this;
      if(me.initiated) return;
      me.initiated=true;
      for (var i=0;i<4;i++){
        var m=me.modes[i];
        document.getElementById(m + "-s").style.display="none";
        document.getElementById(m + "-m").onclick=function(){
          var tm=this.id.replace(/-m/,'');
          me.dselect(me.cmode);
          me.select(tm);
          me.cmode=tm;
        };
      }
      me.cmode="profile";
      me.select(me.cmode);
    },
    apply:function(){
      if(!BrewPiSetting.valid){
        alert("Not connected to controller.");
        //		return;
      }
      if((this.cmode == "beer") || (this.cmode == "fridge")){
        var v= document.getElementById(this.cmode + "-t").value;
        if(v == '' || isNaN(v) ||( v > BrewPiSetting.maxDegree || v < BrewPiSetting.minDegree)){
          alert("Invalid Temperature:"+v);
          return;
        }
        if(this.cmode == "beer"){
          //console.log("j{mode:b, beerSet:" + v+ "}");
          BWF.send("j{mode:b, beerSet:" + v+ "}");
        }else{
          console.log("j{mode:f, fridgeSet:" + v+ "}");
          BWF.send("j{mode:f, fridgeSet:" + v+ "}");
        }
      }else if(this.cmode == "off"){
        //console.log("j{mode:o}");
        BWF.send("j{mode:o}");
      } else{
        // should save first.
        if(profileEditor.dirty){
          alert("save the profile first before apply");
          return;
        }
        //console.log("j{mode:p}");
        document.getElementById('dlg_beerprofilereminder').style.display = "block";
        document.getElementById('dlg_beerprofilereminder').querySelector("button.ok").onclick=function(){
          document.getElementById('dlg_beerprofilereminder').style.display = "none";
          var gravity=parseFloat(Q("#dlg_beerprofilereminder input").value);
          updateOriginGravity(gravity);
          var data={name:"webjs",og:1,gravity:gravity};
          s_ajax({
            url:"gravity", m:"POST",mime:"application/json",
            data:JSON.stringify(data),
            success:function(d){
              BWF.send("j{mode:p}");
            },
            fail:function(d){
              alert("failed:"+d);
            }});
          };
          document.getElementById('dlg_beerprofilereminder').querySelector("button.oknog").onclick=function(){
            document.getElementById('dlg_beerprofilereminder').style.display = "none";
            BWF.send("j{mode:p}");
          };
          document.getElementById('dlg_beerprofilereminder').querySelector("button.cancel").onclick=function(){
            document.getElementById('dlg_beerprofilereminder').style.display = "none";
          };
        }
      }
    };

    function saveprofile(){
      //console.log("save");
      var r=profileEditor.getProfile();
      if(r === false){
        alert("invalid value. check again");
        return;
      }
      var json=JSON.stringify(r);
      console.log("result="+json);

      BWF.save(BWF.BrewProfile,encodeURIComponent(json),function(){
        profileEditor.markdirty(false);
        alert("Done.");
      },function(e){
        alert("save failed:"+e);
      });
    }

    function C2F(c){return Math.round((c*1.8+32)*10)/10}function F2C(f){return Math.round((f-32)/1.8*10)/10}

    function updateTempUnit(u)
    {
      var Us=document.getElementsByClassName("t_unit");
      for(var i=0;i< Us.length;i++){
        Us[i].innerHTML = u;
      }
    }

    function openDlgLoading(){document.getElementById('dlg_loading').style.display = "block";}
    function closeDlgLoading(){document.getElementById('dlg_loading').style.display = "none";}

    function onload(next){
      modekeeper.init();

      openDlgLoading();

      function complete(){
        var initial=true;
        function initComp(){
          if(!initial) return;
          initial=false;
          closeDlgLoading();
          next();
        }
        invoke({ url:"/tcc", m:"GET",
        fail:function(a){
          console.log("error connect to BrwePiLess!");
          initComp();
        },
        success:function(d){
          var s= JSON.parse(d);
          var setting={valid:true,minDegree:s.tempSetMin,maxDegree:s.tempSetMax,tempUnit:s.tempFormat};
          if(setting.tempUnit != BrewPiSetting.tempUnit){
            updateTempUnit(setting.tempUnit);
            profileEditor.setTempUnit(setting.tempUnit);
          }
          BrewPiSetting=setting;
          initComp();

        }
      });
    }

    //get current profile
    BWF.load(BWF.BrewProfile,function(d){
      try{
        var p=JSON.parse(d);
        updateTempUnit(p.u); // using profile temp before we get from controller
        BrewPiSetting.tempUnit=p.u;
        profileEditor.initProfile(p);
        ControlChart.init("tc_chart",profileEditor.chartdata(),p.u);
      }catch(err){
        console.log("error:"+err);
        profileEditor.initProfile();
        ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);
      }
      complete();
    },function(e){
      profileEditor.initProfile();
      ControlChart.init("tc_chart",profileEditor.chartdata(),profileEditor.tempUnit);
      complete();});
    }


    function getLogName()
    {
      s_ajax({
        url:"loglist.php", m:"GET",
        success:function(d){
          var r= JSON.parse(d);
          if(r.rec){
            Q("#recording").innerHTML=r.log;
          }else{
            Q("#recording").innerHTML="";
          }
        },
        fail:function(d){alert("failed:"+d);}});
      }
      var BChart={
        offset:0,
        url:'chart.php',
        toggle:function(type){
          this.chart.toggleLine(type);
        },
        reqdata:function(){
          var t=this;
          var PD='offset=' + t.offset;

          if(typeof t.startOff != "undefined" && t.startOff !== null)
          PD = PD + "&index=" + t.startOff;
          var xhr = new XMLHttpRequest();
          xhr.open('GET', t.url + '?' + PD);
          //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
          //	xhr.setRequestHeader("Content-length", PD.length);
          xhr.timeout = 5000;
          xhr.ontimeout = function(e) {
            console.error("Timeout!!")
          };
          xhr.responseType = 'arraybuffer';
          xhr.onload = function(e) {
            if(this.status == 404){
              console.log("Error getting log data");
              return;
            }
            // response is unsigned 8 bit integer
            var data = new Uint8Array(this.response);
            if(data.length ==0){
              //console.log("zero content");
              if(t.timer) clearInterval(t.timer);
              t.timer=null;
              setTimeout(function(){ t.reqdata();
              },3000);
              return;
            }
            var newchart=t.chart.process(data);
            if(newchart){
              t.offset = data.length;
              t.startOff = xhr.getResponseHeader("LogOffset");
              getLogName();
              //console.log("new chart, offset="+t.startOff);
            }else t.offset += data.length;

            if(!isNaN(t.chart.og)){
              updateOriginGravity(t.chart.og);
              t.chart.og=NaN;
            }
            if(t.chart.sg && !isNaN(t.chart.sg)){
              updateGravity(t.chart.sg);
              t.chart.sg = NaN;
              checkfgstate();
            }
            if(t.timer==null) t.settimer();
          };
          xhr.onerror=function(){
            console.log("error getting data.");
            setTimeout(function(){ t.reqdata();
            },10000);
          };
          //console.log(PD);
          xhr.send();
        },
        settimer:function(){
          var t=this;
          //console.log("start timer at "+ t.chart.interval);
          t.timer=setInterval(function(){
            t.reqdata();
          },t.chart.interval * 1000);
        },
        init:function(id){
          this.chart=new BrewChart(id);
        },
        timer:null,
        start:function(){
          if(this.running) return;
          this.running=true;
          this.offset=0;
          this.reqdata();
        },
        reqnow:function(){
          var t=this;
          if(t.timer) clearInterval(t.timer);
          t.timer=null;
          t.reqdata();
        }
      };

      function setLcdText(id,html)
      {
        var d=document.getElementById(id);
        d.innerHTML=html;
      }

      function communicationError()
      {
        setLcdText("lcd-line-0","Failed to");
        setLcdText("lcd-line-1","connect to");
        setLcdText("lcd-line-2","Server");
        setLcdText("lcd-line-3","");
      }

      function controllerError()
      {
        setLcdText("lcd-line-0","Controller not");
        setLcdText("lcd-line-1","updating data");
        setLcdText("lcd-line-2","...");
        setLcdText("lcd-line-3","");
        /*setTimeout(function(){
        console.log("reconnect");
        BWF.reconnect();
      },5000);*/
    }

    function gravityDevice(msg)
    {
      if(typeof msg["name"] == "undefined") return;
      if( msg.name.startsWith("iSpindel")){
        // iSpindel
        if(typeof window.iSpindel == "undefined"){
          window.iSpindel = true;
          Q("#ispindel-pane").style.display="block";
        }
        Q("#iSpindel-name").innerHTML = msg.name;
        if(typeof msg["battery"] != "undefined")
        Q("#iSpindel-battery").innerHTML = msg.battery;

        var lu;
        if(typeof msg["lu"] != "undefined")
        lu = new Date(msg.lu * 1000);
        else
        lu = new Date();
        Q("#iSpindel-last").innerHTML=formatDate(lu);

        if(typeof msg["sg"] != "undefined")
        updateGravity(msg["sg"]);

        if(typeof msg["angle"] != "undefined")
        console.log("iSpindel:" + JSON.stringify(msg));
      }
      if(typeof msg["lpf"] != "undefined")
      GravityFilter.setBeta(msg["lpf"]);

      if(typeof msg["stpt"] != "undefined")
      GravityTracker.setThreshold(msg["stpt"]);

    }

    function init()
    {
      BChart.init("div_g");

      var gotMsg=true;
      onload(function(){
        BWF.init({
          onconnect:function(){
            BWF.send("l");
            setInterval(function(){
              if(!gotMsg) controllerError();
              BWF.send("l");
              gotMsg=false;
            },5000);
          },
          error:function(e){
            //console.log("error");
            communicationError();
          },
          handlers:{
            L:function(lines){gotMsg=true; for(var i=0;i<4;i++) setLcdText("lcd-line-" + i,lines[i]);},
            V:function(c){ console.log("forced reload chart"); BChart.reqnow(); },
            G:function(c){ gravityDevice(c); }
          }
        });

        BChart.start();

      });

    }
    var BrewMath={
      abv:function(og,fg){return ((76.08 * (og-fg) / (1.775-og)) * (fg / 0.794)).toFixed(1);},
      att:function(og,fg){ return Math.round((og-fg)/(og -1) * 100);},
      sg2pla:function(sg){ return -616.868 + 1111.14 * sg - 630.272 * sg * sg + 135.997 * sg*sg*sg;},
      pla2sg:function(pla){ return 1 + (pla/(258.6 - ((pla/258.2) * 227.1))); }
    };

    function updateGravity(sg)
    {
      //if(typeof window.sg != "undefined") return;
      window.sg=sg;
      Q("#gravity-sg").innerHTML = sg.toFixed(3);
      if(typeof window.og != "undefined"){
        Q("#gravity-att").innerHTML = BrewMath.att(window.og,sg);
        Q("#gravity-abv").innerHTML = BrewMath.abv(window.og,sg);
      }
    }

    function updateOriginGravity(og){
      if(typeof window.og != "undefined" && window.og == og) return;
      window.og=og;
      Q("#gravity-og").innerHTML = og.toFixed(3);
      if(typeof window.sg != "undefined")
      updateGravity(window.sg);
    }

    function showgravitydlg(msg)
    {
      Q('#dlg_addgravity .msg').innerHTML=msg;
      Q('#dlg_addgravity').style.display = "block";
    }

    function dismissgravity()
    {
      Q('#dlg_addgravity').style.display = "none";
    }

    function inputgravity()
    {
      var gravity=parseFloat(Q("#dlg_addgravity input").value);

      if(gravity < 0.8 || gravity > 1.25) return;
      dismissgravity();
      openDlgLoading();

      if(window.isog) updateOriginGravity(gravity);
      else updateGravity(gravity);

      var data={name:"webjs",gravity:gravity};
      if(window.isog)
      data.og=1;
      s_ajax({
        url:"gravity", m:"POST",mime:"application/json",
        data:JSON.stringify(data),
        success:function(d){
          closeDlgLoading();
        },
        fail:function(d){alert("failed:"+d);
        closeDlgLoading();
      }});

    }
    function inputSG()
    {
      window.isog=false;
      showgravitydlg("Add gravity Record:");
    }
    function inputOG()
    {
      window.isog=true;
      showgravitydlg("Set Original Gravity:");
    }
