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
