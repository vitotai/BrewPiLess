const char data_control_htm[] PROGMEM =
R"END(
<!doctype html>
<html>
<head>
<script type=text/javascript src=bwf.js></script>
<meta charset=utf-8 name=viewport content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>BrewPi Control</title>
<style>.corner-top{border-top-right-radius:5px;border-top-left-radius:5px}.corner-bottom{border-bottom-right-radius:5px;border-bottom-left-radius:5px}.navbar{margin:0;padding:.2em .2em 0;border:1px solid #4297d7;background:#5c9ccc;color:#fff;height:3.5em;display:block;position:relative}#set-mode-text{margin-left:2px;float:left;margin:.2em}.navitem{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .2em 0 0;padding:.5em}.nav-selected{background:#fff;font-weight:bold;color:#e17009;margin:.2em .2em 0 0;padding:.5em .5em .6em .5em}.navitems{display:block;margin-top:2em}#apply{background:#dfeffc;font-weight:bold;color:#2e6e9e;margin:.2em .1em 0 0;padding:.5em}#containter{margin:.5em;width:420px}.profileTable td,th{padding:3px;text-align:center}.profileTable tbody TR.odd{background:#f7f7f7}.profileTable tbody TR.even{width:100%}.profileTable TH{border:1px solid #4297d7;background:#5c9ccc;color:#fff;font-weight:bold}table.profileTable{width:100%}button{color:#1d5987;background:#dfeffc;font-weight:bold;border-top-right-radius:5px;border-top-left-radius:5px;border-bottom-right-radius:5px;border-bottom-left-radius:5px}#addbutton{float:right;width:36%;margin:5px 1% 5px 1%}.contextmenu{display:block;position:absolute;background:#D00;color:#fff;font-weight:bold;padding:.5em}#header{width:480px}#clearbtn{width:48%}#savebtn{width:48%}.modal{display:none;position:fixed;z-index:1;padding-top:100px;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:#000;background-color:rgba(0,0,0,0.4)}.modal-content{background-color:#fefefe;margin:auto;padding:20px;border:1px solid #888;width:80%}#ST_sync{display:none;color:#D33}</style>
<script>var BrewPiSetting={valid:false,maxDegree:30,minDegree:0,tempUnit:"C"};function formatDate(f){var j=f.getFullYear();var i=f.getMonth()+1;var g=f.getDate();var e=f.getHours();var b=f.getMinutes();var c=f.getSeconds();function a(d){return(d<10)?"0"+d:d}return a(i)+"/"+a(g)+"/"+j+" "+a(e)+":"+a(b)+":"+a(c)}var profileEditor={dirty:false,tempUnit:"C",C_startday_Id:"startdate",C_savebtn_Id:"savebtn",markdirty:function(a){this.dirty=a;document.getElementById(this.C_savebtn_Id).innerHTML=(a)?"Save*":"Save"},getStartDate:function(){return this.sd},setStartDate:function(a){},startDayChange:function(){var a=new Date(document.getElementById(this.C_startday_Id).value);if(isNaN(a.getTime())){document.getElementById(this.C_startday_Id).value=formatDate(this.sd)}else{this.sd=a;this.sortrow();this.markdirty(true)}},startnow:function(){var a=new Date();document.getElementById(this.C_startday_Id).value=formatDate(a);this.sd=a;this.sortrow();this.markdirty(true)},deleteRow:function(a){var b=document.getElementById("profile_t").getElementsByTagName("tr")[a];b.parentNode.removeChild(b);this.sortrow();this.markdirty(true)},showmenu:function(f,d){var b=d.rowIndex;f.preventDefault();var g=document.createElement("div");g.className="contextmenu";g.innerHTML="Delete!";g.style.top=(f.clientY-1)+"px";g.style.left=(f.clientX-1)+"px";g.onmouseout=function(){g.parentNode.removeChild(g)};var c=this;g.onclick=function(){g.parentNode.removeChild(g);c.deleteRow(b)};var a=document.getElementsByTagName("body")[0];a.appendChild(g)},dayChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML)){a.innerHTML=a.saved}this.sortrow();this.markdirty(true)},tempChange:function(a){if(a.innerHTML==""||isNaN(a.innerHTML)){a.innerHTML=a.saved}else{this.markdirty(true)}},newrow:function(j,f,h){var a=this;var i=document.createElement("tr");var g=document.createElement("td");g.className="pday";g.innerHTML=(typeof j!=="undefined")?j:0;g.contentEditable=true;g.onblur=function(){a.dayChange(this)};g.onfocus=function(){this.saved=this.innerHTML};var e=document.createElement("td");e.className="ptemp";e.innerHTML=(typeof f!=="undefined")?f:20;e.contentEditable=true;e.onblur=function(){a.tempChange(this)};e.onfocus=function(){this.saved=this.innerHTML};var c=document.createElement("td");c.className="pdaystr";c.innerHTML=(h)?h:"";i.appendChild(g);i.appendChild(e);i.appendChild(c);i.oncontextmenu=function(b){a.showmenu(b,this);return false};return i},datestr:function(b){var a=new Date(this.sd.getTime()+Math.round(b*86400)*1000);return formatDate(a)},addRow:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var c=a.getElementsByTagName("tr");var b=this.newrow();b.className=(c.length%2)?"odd":"even";a.appendChild(b);this.markdirty(true)},init:function(c,f){this.sd=f;document.getElementById(this.C_startday_Id).value=formatDate(f);var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];for(var b=0;b<c.length;b++){var e=this.newrow(c[b].d,c[b].t,this.datestr(c[b].d));e.className=(b%2)?"odd":"even";a.appendChild(e)}},sortrow:function(){var b=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var e=b.getElementsByTagName("tr");var d=Array.prototype.slice.call(e);d.sort(function(h,g){var j=parseFloat(h.querySelector("td.pday").innerHTML);var i=parseFloat(g.querySelector("td.pday").innerHTML);if(isNaN(j)||isNaN(i)||(j===i)){return 0}else{return j-i}});for(var c=0;c<d.length;c++){var f=d[c];var a=f.querySelector("td.pday").innerHTML;f.querySelector("td.pdaystr").innerHTML=this.datestr(parseFloat(a));f.className=(c%2)?"odd":"even";f.parentNode.removeChild(f);b.appendChild(f)}},clear:function(){var a=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var e=a.getElementsByTagName("tr");var c=e.length;for(var b=e.length-1;b>=0;b--){var d=e[b];d.parentNode.removeChild(d)}this.markdirty(true)},getProfile:function(){var d=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var f=d.getElementsByTagName("tr");var e=f.length;var h=0;var b=[];for(var c=0;c<f.length;c++){var g=f[c];var a=parseFloat(g.querySelector("td.pday").innerHTML);var j=parseFloat(g.querySelector("td.ptemp").innerHTML);if(isNaN(a)||isNaN(j)){return false}if(j>BrewPiSetting.maxDegree||j<BrewPiSetting.minDegree){return false}if(c==0){if(a!=0){return false}}else{if(h==a){return false}}h=a;b.push({d:a,t:j})}var k=this.sd.toISOString();return{s:k,u:this.tempUnit,t:b}},initProfile:function(b){var a=new Date(b.s);this.tempUnit=b.u;profileEditor.init(b.t,a)},setTempUnit:function(d){if(d==this.tempUnit){return}this.tempUnit=d;var c=document.getElementById("profile_t").getElementsByTagName("tbody")[0];var f=c.getElementsByTagName("tr");for(var e=0;e<f.length;e++){var b=f[e].querySelector("td.ptemp");var a=parseFloat(b.innerHTML);b.innerHTML=(d=="C")?F2C(a):C2F(a)}}};var modekeeper={initiated:false,modes:["profile","beer","fridge","off"],cmode:0,dselect:function(a){var b=document.getElementById(a+"-m");var c=document.getElementById(a+"-m").className.replace(/\snav-selected/,"");b.className=c;document.getElementById(a+"-s").style.display="none"},select:function(a){document.getElementById(a+"-m").className+=" nav-selected";document.getElementById(a+"-s").style.display="block"},init:function(){var c=this;if(c.initiated){return}c.initiated=true;for(var b=0;b<4;b++){var a=c.modes[b];document.getElementById(a+"-s").style.display="none";document.getElementById(a+"-m").onclick=function(){var d=this.id.replace(/-m/,"");c.dselect(c.cmode);c.select(d);c.cmode=d}}c.cmode="profile";c.select(c.cmode)},apply:function(){if(!BrewPiSetting.valid){alert("Not connected to controller.");return}if((this.cmode=="beer")||(this.cmode=="fridge")){var a=document.getElementById(this.cmode+"-t").value;if(a==""||isNaN(a)||(a>BrewPiSetting.maxDegree||a<BrewPiSetting.minDegree)){alert("Invalid Temperature:"+a);return}if(this.cmode=="beer"){console.log("j{mode:b, beerSet:"+a+"}");BWF.send("j{mode:b, beerSet:"+a+"}")}else{console.log("j{mode:f, fridgeSet:"+a+"}");BWF.send("j{mode:f, fridgeSet:"+a+"}")}}else{if(this.cmode=="off"){console.log("j{mode:o}");BWF.send("j{mode:o}")}else{if(profileEditor.dirty){alert("save the profile first before apply");return}console.log("j{mode:p}");BWF.send("j{mode:p}")}}}};function saveprofile(){console.log("save");var b=profileEditor.getProfile();if(b===false){alert("invalid value. check again");return}var a=JSON.stringify(b);console.log("result="+a);BWF.save(BWF.BrewProfile,a,function(){profileEditor.markdirty(false);alert("Done.")},function(c){alert("save failed:"+c)})}function C2F(a){return Math.round((a*1.8+32)*10)/10}function F2C(a){return Math.round((a-32)/1.8*10)/10}function updateTempUnit(a){var c=document.getElementsByClassName("t_unit");for(var b=0;b<c.length;b++){c[b].innerHTML=a}}function openDlgLoading(){document.getElementById("dlg_loading").style.display="block"}function closeDlgLoading(){document.getElementById("dlg_loading").style.display="none"}function settime(){var a=new Date();var b=Math.round(a.getTime()/1000);console.log("settime:"+b);invoke({m:"POST",url:"/time",data:"time="+b,success:function(c){document.getElementById("ST_sync").style.display="none"}})}var SL_diff;function checktime(){invoke({m:"GET",url:"/time",success:function(c){console.log("server time:"+c);var b=JSON.parse(c);var a=new Date();SL_diff=b.e*1000-a.getTime();if(Math.abs(SL_diff)>3600000){document.getElementById("ST_sync").style.display="block";setInterval(function(){var d=new Date();var e=new Date(d.getTime()+SL_diff);document.getElementById("servertime").innerHTML=e.toLocaleString()},1000)}}})}function onload(){modekeeper.init();openDlgLoading();checktime();function a(){var c;var b=true;function d(){if(!b){return}b=false;clearTimeout(c);closeDlgLoading()}BWF.init({error:function(){console.log("error connect to BrwePiLess!");d()},onopen:function(){BWF.send("c");c=setTimeout(function(){console.log("Timeout of C command");d()},5000)},handlers:{C:function(f){var e={valid:true,minDegree:f.tempSetMin,maxDegree:f.tempSetMax,tempUnit:f.tempFormat};if(e.tempUnit!=BrewPiSetting.tempUnit){updateTempUnit(e.tempUnit);profileEditor.setTempUnit(e.tempUnit)}BrewPiSetting=e;d()}}})}BWF.load(BWF.BrewProfile,function(c){var b=JSON.parse(c);updateTempUnit(b.u);BrewPiSetting.tempUnit=b.u;profileEditor.initProfile(b);a()},function(b){profileEditor.init([],new Date());a()})};</script>
</head>
<body onload=onload()>
<div class="navbar corner-top corner-bottom" id=header>
<div id=set-mode-text>Set temperature mode:</div>
<div class=navitems>
<span class="navitem corner-top" id=profile-m>Beer profile</span>
<span class="navitem corner-top" id=beer-m>Beer Const.</span>
<span class="navitem corner-top" id=fridge-m>Fridge Const.</span>
<span class="navitem corner-top" id=off-m>Off</span>
<button id=modekeeper.apply() class="corner-top corner-bottom" onclick=modekeeper.apply()>Apply</button>
</div>
</div>
<div id=containter>
<div id=ST_sync>!!!BrewPiLess Time:<span id=servertime></span><button onclick=settime()>SET Time</button></div>
<div id=profile-s class=detail>
<div>
<div><span>Start Date:</span><input type=text size=40 id=startdate onchange=profileEditor.startDayChange()>
<button id=setnow onclick=profileEditor.startnow()>Now</button></div>
<button id=addbutton onclick=profileEditor.addRow()>Add</button>
</div>
<table class=profileTable id=profile_t>
<thead><tr><th>Day</th><th>Temperature(&deg;<span class=t_unit>C</span>)</th><th>Date and Time</th></tr></thead>
<tbody></tbody>
</table>
<div><button id=clearbtn onclick=profileEditor.clear()>Clear</button><button id=savebtn onclick=saveprofile()>Save</button></div>
</div>
<div id=beer-s class=detail>
Set Beer temp:
<input type=text size=6 id=beer-t></input>&deg;<span class=t_unit>C</span>
</div>
<div id=fridge-s class=detail>
Set Fridge temp:
<input type=text size=6 id=fridge-t></input>&deg;<span class=t_unit>C</span>
</div>
<div id=off-s class=detail>Turning temperature controll Off.</div>
</div>
<div id=dlg_loading class=modal>
<div class=modal-content>
<p>Loading setting from BrewPi controller..</p>
</div>
</div>
</body>
</html>
)END";











































































