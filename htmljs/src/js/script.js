/**********************************************************************
created by Vito Tai
Copyright (C) 2016 Vito Tai

This soft ware is provided as-is. Use at your own risks.
You are free to modify and distribute this software without removing
this statement.
BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

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

var STATES = [{
  name: "IDLE",
  text: "Idle"
}, {
  name: "STATE_OFF",
  text: "Off"
}, {
  name: "DOOR_OPEN",
  text: "Door Open",
  doorOpen: !0
}, {
  name: "HEATING",
  text: "Heating"
}, {
  name: "COOLING",
  text: "Cooling"
}, {
  name: "WAITING_TO_COOL",
  text: "Waiting to Cool",
  waiting: !0
}, {
  name: "WAITING_TO_HEAT",
  text: "Waiting to Heat",
  waiting: !0
}, {
  name: "WAITING_FOR_PEAK_DETECT",
  text: "Waiting for Peak",
  waiting: !0
}, {
  name: "COOLING_MIN_TIME",
  text: "Cooling Min Time",
  extending: !0
}, {
  name: "HEATING_MIN_TIME",
  text: "Heating Min Time",
  extending: !0
}];

BrewChart.Mode = {
  b:"Beer Constant",
  f:"Fridge Constant",
  o:"Off",
  p:"Profile"
};

BrewChart.Labels = [
  'Time',
  'beerSet',
  'beerTemp',
  'fridgeTemp',
  'fridgeSet',
  'roomTemp',
  'auxTemp',
  'gravity',
  'filtersg'
];

BrewChart.Colors = [
  "rgb(240, 100, 100)",
  "rgb(41,170,41)",
  "rgb(89, 184, 255)",
  "rgb(255, 161, 76)",
  "#AAAAAA",
  "#f5e127",
  "rgb(153,0,153)",
  "#000abb"
];

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

function C2F(c){return Math.round((c*1.8+32)*10)/10};
function F2C(f){return Math.round((f-32)/1.8*10)/10};

function updateTempUnit(u) {
  var Us=document.getElementsByClassName("t_unit");
  for(var i=0;i< Us.length;i++){
    Us[i].innerHTML = u;
  }
}

function onload(next) {
  function complete(){
    var initial=true;
    function initComp(){
      if(!initial) return;
      initial=false;
      // closeDlgLoading();
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
        // profileEditor.setTempUnit(setting.tempUnit);
      }
      BrewPiSetting=setting;
      initComp();

    }
  });
}
}

function getLogName() {
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
    fail:function(d){alert("failed:"+d);}
  });
};

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

function communicationError() {
  Q('.error').innerHTML="Failed to connect to server.";
}

function controllerError() {
  Q('.error').innerHTML="Controller not updating data.";
};

function gravityDevice(msg) {
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
};

function displayrssi(x){
  var strength=[-1000,-90,-80,-70,-67];
  var bar=4;
  for(;bar>=0;bar--){
    if(strength[bar] < x ) break;
  }
  var bars=document.getElementsByClassName("rssi-bar");
  for(var i=0;i< bars.length;i++){
    bars[i].style.backgroundColor=(i < bar)? "#888" : "#DDD";
  }
  Q("#rssi").title=(x>0) ? "?": Math.min(Math.max(2 * (x + 100), 0), 100) + "%";
};

function init() {
  BChart.init("div_g");

  var gotMsg=true;

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
      communicationError();
    },
    handlers:{
      L: function(t) {
        e = !0;
        processLcdText(t);
      },
      V:function(c){
        if(typeof c["rssi"] != "undefined"){
          displayrssi(c["rssi"]);
        }
        if(typeof c["reload"] != "undefined"){
          console.log("forced reload chart");
          BChart.reqnow();
        }
        if(typeof c["nn"]!="undefined"){
          Q("#hostname").innerHTML = c["nn"];
        }
        if(typeof c["ver"]!="undefined"){
          if(JSVERION != c["ver"]) alert("Version Mismatched!. Reload the page.");
          Q("#verinfo").innerHTML = "v" + c["ver"];
        }
      },
      G:function(c){ gravityDevice(c); }
    }
  });

  BChart.start();
};

var BrewMath={
  abv:function(og,fg){return ((76.08 * (og-fg) / (1.775-og)) * (fg / 0.794)).toFixed(1);},
  att:function(og,fg){ return Math.round((og-fg)/(og -1) * 100);},
  sg2pla:function(sg){ return -616.868 + 1111.14 * sg - 630.272 * sg * sg + 135.997 * sg*sg*sg;},
  pla2sg:function(pla){ return 1 + (pla/(258.6 - ((pla/258.2) * 227.1))); }
};

function updateGravity(sg) {
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

function showgravitydlg(msg) {
  console.log('show')
  Q('#dlg_addgravity .msg').innerHTML=msg;
  Q('#dlg_addgravity').style.display = "flex";
}

function dismissgravity() {
  Q('#dlg_addgravity').style.display = "none";
}

function openDlgLoading() {
  document.getElementById('dlg_loading').style.display = "block";
}

function closeDlgLoading() {
  document.getElementById('dlg_loading').style.display = "none";
}

function inputgravity() {
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

function inputSG() {
  window.isog=false;
  showgravitydlg("Add gravity Record:");
}

function inputOG() {
  window.isog=true;
  showgravitydlg("Set Original Gravity:");
}

function parseLcdText(lines) {
       var status = {};
       var modePatterns = {
           b: /Mode\s+Beer\s+Const/i,
           f: /Mode\s+Fridge\s+Const/i,
           p: /Mode\s+Beer\s+Profile/i,
           o: /Mode\s+Off/i
       };
       var modes = Object.keys(modePatterns);
       status.ControlMode = "i";
       for (var m = 0; m < modes.length; m++) {
           if (modePatterns[modes[m]].test(lines[0])) {
               status.ControlMode = modes[m];
               break;
           }
       }
       status.ControlState = i;
       var tempRE = /\s*(\w+)\s+(.+)\s+(.+)\s+.+([CF])\s*$/;
       for (var i = 1; i < 3; i++) {
           var temps = tempRE.exec(lines[i]);
           status[temps[1] + "Temp"] = temps[2];
           status[temps[1] + "Set"] = temps[3];
           status.format = temps[4];
       }
       var i = 0;
       var statePatterns = [
           /Idling\s+for\s+(\S+)\s*$/i,
           /control\s+OFF/i,
           /Door\s+Open/i,
           /Heating\s+for\s+(\S+)\s*$/i,
           /Cooling\s+for\s+(\S+)\s*$/i,
           /Wait\s+to\s+Cool\s+(\S+)\s*$/i,
           /Wait\s+to\s+Heat\s+(\S+)\s*$/i,
           /Wait\s+for\s+Peak/i,
           /Cool\s+Time\s+left\s+(\S+)\s*$/i,
           /Heat\s+Time\s+left\s+(\S+)\s*$/i
       ];
       status.ControlStateSince = "";
       for (i = 0; i < statePatterns.length; i++) {
           var match = statePatterns[i].exec(lines[3]);
           if (match) {
               if (typeof match[1] !== "undefined") status.ControlStateSince = match[1];
               break;
           }
       }
       status.ControlState = i;
       status.StatusLine = lines[3];
       return status;
   }

   function processLcdText(lines) {
       Q(".error").style.display = "none";
       var status = parseLcdText(lines);
       var ModeString = {
           o: "OFF",
           b: "Beer Constant",
           f: "Fridge Const",
           p: "Beer Profile",
           i: "Invalid"
       };
       Object.keys(status).map(function(key, i) {
           var div = Q("#lcd" + key);
           if (div) {
               if (key == "ControlMode") div.innerHTML = ModeString[status[key]];
               else if (key == "ControlState") div.innerHTML = (status[key] < STATES.length) ? STATES[status[key]].text : "Unknown State";
               else div.innerHTML = status[key];
           }
       });
   }
