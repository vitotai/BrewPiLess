const char LogConfigHtml[] PROGMEM =
R"END(
<html>
<head>
<title>Logging Setting</title>
<script>/*<![CDATA[*/var logurl="log";function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var EI=function(a){return document.getElementById(a)};Number.prototype.format=function(g,a,e,f){var d="\\d(?=(\\d{"+(a||3)+"})+"+(g>0?"\\D":"$")+")",b=this.toFixed(Math.max(0,~~g));return(f?b.replace(".",f):b).replace(new RegExp(d,"g"),"$&"+(e||","))};String.prototype.escapeJSON=function(){return this.replace(/[\\]/g,"\\\\").replace(/[\"]/g,'\\"').replace(/[\/]/g,"\\/").replace(/[\b]/g,"\\b").replace(/[\f]/g,"\\f").replace(/[\n]/g,"\\n").replace(/[\r]/g,"\\r").replace(/[\t]/g,"\\t")};var logs={url:"loglist.php",rmurl:"loglist.php?rm=",starturl:"loglist.php?start=",stopurl:"loglist.php?stop=1",dlurl:"loglist.php?dl=",ll:[],fs:{},logging:false,vname:function(a){if(a==""){return false}if(a.match(/[\W]/g)){return false}return true},dupname:function(b){var a=false;this.ll.forEach(function(c){if(b==c.name){a=true}});return a},fsinfo:function(b,a){EI("fssize").innerHTML=b.format(0,3,",");EI("fsused").innerHTML=a.format(0,3,",");EI("fsfree").innerHTML=(b-a).format(0,3,",")},slog:function(){var b=this;if(b.logging){if(confirm("Stop current logging?")){var c=EI("logname").value.trim();s_ajax({url:b.stopurl+c,m:"GET",success:function(e){location.reload()},fail:function(e){alert("Failed to stop for:"+e)}})}}else{if(b.ll.length>=10){alert("Too many logs. Delete some before creating new.");return}if((b.fs.size-b.fs.used)<=b.fs.block*2){alert("Not enough free space!");return}var a=EI("logname").value.trim();if(b.vname(a)===false){alert("Invalid file name, no special characters allowed.");return}if(b.dupname(a)){alert("Duplicated name.");return}if(confirm("Start new logging?")){s_ajax({url:b.starturl+a,m:"GET",success:function(e){location.reload()},fail:function(e){alert("Failed to start for:"+e)}})}}},recording:function(e,b){this.logging=true;var c=new Date(b*1000);EI("logtitle").innerHTML="Recording since <b>"+c.toLocaleString()+"</b> ";var a=EI("logname");a.value=e;a.disabled=true;EI("logbutton").innerHTML="STOP Logging"},stop:function(){this.logging=false;EI("logtitle").innerHTML="New Log Name:";var a=EI("logname");a.value="";a.disabled=false;EI("logbutton").innerHTML="Start Logging"},rm:function(b){var a=this;if(confirm("Delete the log "+a.ll[b].name)){console.log("rm "+a.ll[b].name);s_ajax({url:a.rmurl+b,m:"GET",success:function(e){var c=JSON.parse(e);a.fs=c;a.fsinfo(c.size,c.used);a.ll.splice(b,1);a.list(a.ll)},fail:function(c){alert("Failed to delete for:"+c)}})}},dl:function(a){window.open(this.dlurl+a)},list:function(b){var a=EI("loglist").querySelector("tbody");var d;while(d=a.querySelector("tr:nth-of-type(2)")){a.removeChild(d)}var c=this;var e=c.row;b.forEach(function(j,f){var h=j.name;var g=new Date(j.time*1000);var k=e.cloneNode(true);k.querySelector(".logid").innerHTML=h;k.querySelector(".logdate").innerHTML=g.toLocaleString();k.querySelector(".dlbutton").onclick=function(){c.dl(f)};k.querySelector(".rmbutton").onclick=function(){c.rm(f)};a.appendChild(k)})},init:function(){var a=this;EI("logbutton").onclick=function(){a.slog()};a.row=EI("loglist").querySelector("tr:nth-of-type(2)");a.row.parentNode.removeChild(a.row);s_ajax({url:a.url,m:"GET",success:function(c){var b=JSON.parse(c);a.fs=b.fs;if(b.rec){a.recording(b.log,b.start)}a.ll=b.list;a.list(b.list);a.fsinfo(b.fs.size,b.fs.used)},fail:function(b){alert("failed:"+b)}})},};function checkurl(a){if(a.value.trim().startsWith("https")){alert("HTTPS is not supported")}}function checkformat(a){if(a.value.length>256){a.value=t.value.substring(0,256)}EI("fmthint").innerHTML=""+a.value.length+"/256"}function mothod(d){var a=document.querySelectorAll('input[name$="method"]');for(var b=0;b<a.length;b++){if(a[b].id!=d.id){a[b].checked=false}}window.selectedMethod=d.value}function update(){if(typeof window.selectedMethod=="undefined"){alert("select Method!");return}var b=EI("format").value.trim();if(window.selectedMethod=="GET"){var c=new RegExp("s","g");if(c.exec(b)){alert("space is not allowed");return}}var a={};a.enabled=EI("enabled").checked;a.url=EI("url").value.trim();a.format=encodeURIComponent(b.escapeJSON());a.period=EI("period").value;a.method=(EI("m_post").checked)?"POST":"GET";a.type=EI("data-type").value.trim();s_ajax({url:logurl,m:"POST",data:"data="+JSON.stringify(a),success:function(e){alert("done")},fail:function(d){alert("failed:"+d)}})}function load(){s_ajax({url:logurl+"?data=1",m:"GET",success:function(b){var a=JSON.parse(b);if(typeof a.enabled=="undefined"){return}EI("enabled").checked=a.enabled;window.selectedMethod=a.method;EI("m_"+a.method.toLowerCase()).checked=true;EI("url").value=(a.url===undefined)?"":a.url;EI("data-type").value=(a.type===undefined)?"":a.type;EI("format").value=(a.format===undefined)?"":a.format;checkformat(EI("format"));EI("period").value=(a.period===undefined)?300:a.period}});logs.init()}function showformat(a){var b=EI("formatlist");var c=a.getBoundingClientRect();b.style.display="block";b.style.left=(c.left)+"px";b.style.top=(c.top+100)+"px"}function hideformat(){EI("formatlist").style.display="none"};/*]]>*/</script>
<style>#loglist td,#loglist tr,#loglist th,#loglist{border:1px solid black}fieldset{margin:10px}#fsinfo{margin:10px}#formatlist{display:none;position:absolute;border:1px solid white;background:lightgray}#formatlist table,#formatlist td,#formatlist th{border:1px solid black;border-collapse:collapse}</style>
</head>
<body onload="load()">
<fieldset>
<legend>Remote Log</legend>
<form>
<table>
<tr>
<th>Enabled:</th>
<td><input type="checkbox" id="enabled" value="yes"></td>
</tr>
<tr>
<th>Method:</th>
<td><input type="checkbox" id="m_get" name="method" value="GET" onchange="mothod(this)">Get
<input type="checkbox" id="m_post" name="method" value="POST" onchange="mothod(this)">Post
<input type="checkbox" id="m_put" name="method" value="PUT" onchange="mothod(this)">Put </td>
</tr>
<tr>
<th>URL:</th>
<td><input type="text" id="url" size="128" placeholder="input link" onchange="checkurl(this)"></td>
</tr>
<tr>
<th></th>
<td>JSON:"application/json", Form Type:"application/x-www-form-urlencoded"</td>
</tr>
<tr>
<th>Data Type:</th>
<td><input type="text" id="data-type" size="42" placeholder="Content-Type" </td></tr>
<tr>
<th></th>
<td><span onmouseover="showformat(this)" onmouseout="hideformat()"><u>Notations...</u></span></td>
</tr>
<tr>
<th>Format:</th>
<td><textarea id="format" rows="4" cols="64" oninput="checkformat(this)"></textarea></td>
</tr>
<tr>
<th></th>
<td>Characters:<span id="fmthint"></span></td>
</tr>
<tr>
<th>Log time period:</th>
<td><input type="text" id="period" size="4">Seconds</td>
</tr>
<tr>
<th></th>
<td><button type="button" onclick="update()">Update</button></td>
</tr>
</table>
</form>
</fieldset>
<fieldset>
<legend>Local Log</legend>
<span id="logtitle">New Log Name:</span><input type="text" id="logname" size="24" maxlength="24"></input> <button id="logbutton">Start Log</button>
<div id="fsinfo">
Free Space: <span id="fsfree">0</span> Bytes, Used Space: <span id="fsused">0</span> Bytes, Total Space: <span id="fssize">0</span> Bytes
</div>
<table id="loglist">
<tr>
<th style="width:30%">Log</th>
<th style="width:40%">Date</th>
<th>Action</th>
</tr>
<tr>
<td class="logid"></td>
<td class="logdate"></td>
<td><button class="dlbutton">Download</button><button class="rmbutton">Delete</button></td>
</tr>
</table>
</fieldset>
<div id="formatlist">
<table>
<tr>
<th>%b</th>
<td>beer temp</td>
</tr>
<tr>
<th>%B</th>
<td>beer setting</td>
</tr>
<tr>
<th>%f</th>
<td>fridge temp</td>
</tr>
<tr>
<th>%F</th>
<td>fridge setting</td>
</tr>
<tr>
<th>%r</th>
<td>room temp</td>
</tr>
<tr>
<th>%g</th>
<td>gravity</td>
</tr>
<tr>
<th>%a</th>
<td>Aux temp.</td>
</tr>
<tr>
<th>%v</th>
<td>device voltage</td>
</tr>
<tr>
<th>%u</th>
<td>Unix timestamp of last gravity update</td>
</tr>
</table>
</div>
</body>
</html>
)END";
