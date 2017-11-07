const char config_html[]  PROGMEM =R"END(
<html>
<head>
<title>Configuration</title>
<script>/*<![CDATA[*/function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var Q=function(a){return document.querySelector(a)};function loadSetting(){s_ajax({url:"brewpi.cfg",m:"GET",success:function(b){var a=JSON.parse(b);Object.keys(a).map(function(c){var d=Q("input[name="+c+"]");if(d){if(d.type=="checkbox"){d.checked=(a[c]!=0)}else{d.value=a[c]}}})},fail:function(a){alert("error getting data:"+a)}})}function waitrestart(){Q("#waitprompt").style.display="block";Q("#inputform").style.display="none";setTimeout(function(){window.location.reload()},15000)}function save(){var a=document.querySelectorAll("input");var b="";Object.keys(a).map(function(d,c){if(a[c].type!="submit"){var e=(a[c].type=="checkbox")?(a[c].checked?1:0):encodeURIComponent(a[c].value.trim());b=((b=="")?"":(b+"&"))+a[c].name+"="+e}});s_ajax({url:"config",data:b,m:"POST",success:function(c){waitrestart()},fail:function(c){alert("error saving data:"+c)}})}function load(){loadSetting()};/*]]>*/</script>
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
<td>Always softAP</td>
<td><input type="checkbox" name="ap"></td>
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