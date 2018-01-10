const char config_html[]  PROGMEM =R"END(
<html>
<head>
<title>Configuration</title>
<script>/*<![CDATA[*/function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var Q=function(a){return document.querySelector(a)};function formatIP(d){var c=parseInt(d);if(c==0){return""}return""+((c>>24)&255)+"."+((c>>16)&255)+"."+((c>>8)&255)+"."+(c&255)}function verifyIP(h){var f=this.value.split(".");var e=true;var k=0;if(f.length==4){for(var g=0;g<4;g++){var j=parseInt(f[g]);if(j>255){e=false;break}k=(k<<8)+j}}else{e=false}if(e){this.saved=k;this.value=formatIP(k)}else{this.value=formatIP(this.saved)}}function loadSetting(){s_ajax({url:"brewpi.cfg",m:"GET",success:function(b){var a=JSON.parse(b);Object.keys(a).map(function(d){var c=Q("input[name="+d+"]");if(c){if(c.classList.contains("iptype")){c.value=formatIP(a[d]);c.saved=a[d];c.onchange=verifyIP}else{if(c.type=="checkbox"){c.checked=(a[d]!=0)}else{c.value=a[d]}}}else{c=Q("select[name="+d+"]");if(c){c.value=a[d]}}})},fail:function(a){alert("error getting data:"+a)}})}function waitrestart(){Q("#waitprompt").style.display="block";Q("#inputform").style.display="none";setTimeout(function(){window.location.reload()},15000)}function save(){var h=document.querySelectorAll("input");var d="";var b={};Object.keys(h).map(function(e,f){if(h[f].type!="submit"){if(h[f].name&&h[f].name!=""){var c;if(h[f].classList.contains("iptype")){c=h[f].saved}else{if(h[f].type=="checkbox"){c=(h[f].checked?1:0)}else{c=h[f].value.trim()}}b[h[f].name]=c}}});var a=Q("select[name=wifi]");b.wifi=a.value;console.log(JSON.stringify(b));s_ajax({url:"config",data:"data="+encodeURIComponent(JSON.stringify(b)),m:"POST",success:function(c){waitrestart()},fail:function(c){alert("error saving data:"+c)}})}function load(){loadSetting()};/*]]>*/</script>
<style>#waitprompt{display:none}</style>
</head>
<body onload="load()">
<div id="waitprompt">Congfiuration saved. Wait for restart.. (note: if the hostname is changed, the page won't be reloaded.)</div>
<div id="inputform">
<form action="/setconfig" action="post">
<table>
<tr>
<td>Title</td>
<td><input name="title" type="text" size="12" maxlength="24"></td>
</tr>
<tr>
<td>Host/Network Name</td>
<td><input name="name" type="text" size="12" maxlength="16"></td>
</tr>
<tr>
<td>HTTP Port</td>
<td><input name="port" type="text" size="5" maxlength="5"></td>
</tr>
<tr>
<td>User Name</td>
<td><input name="user" type="text" size="12" maxlength="16"></td>
</tr>
<tr>
<td>Password</td>
<td><input name="pass" type="password" size="12" maxlength="16"></td>
</tr>
<tr>
<td>Always need password</td>
<td><input type="checkbox" name="protect"></td>
</tr>
<tr>
<td>Network</td>
<td><select name="wifi">
<option value="1">Station</option>
<option value="2">AP</option>
<option value="3">Station + AP</option>
</select></td>
</tr>
<tr>
<td>Fixed IP</td>
<td><input type="text" name="ip" size="16" class="iptype"></td>
</tr>
<tr>
<td>Gateway</td>
<td><input type="text" name="gw" size="16" class="iptype"></td>
</tr>
<tr>
<td>Netmask</td>
<td><input type="text" name="mask" size="16" class="iptype"></td>
</tr>
<tr>
<td>Save Change</td>
<td><input type="submit" name="submit" onclick="save();return false"></input>
</td>
</tr>
</table>
</form>
</div>
</body>
</html>
)END";