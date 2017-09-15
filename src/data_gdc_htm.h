const char gravityconfig_html[]  PROGMEM =R"END(
<html>
<head>
<title>Gravity Device</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script>/*<![CDATA[*/var gdcurl="/gdc";function s_ajax(a){var d=new XMLHttpRequest();d.onreadystatechange=function(){if(d.readyState==4){if(d.status==200){a.success(d.responseText)}else{d.onerror(d.status)}}};d.ontimeout=function(){if(typeof a.timeout!="undefined"){a.timeout()}else{d.onerror(-1)}},d.onerror=function(b){if(typeof a.fail!="undefined"){a.fail(b)}};d.open(a.m,a.url,true);if(typeof a.data!="undefined"){d.setRequestHeader("Content-Type",(typeof a.mime!="undefined")?a.mime:"application/x-www-form-urlencoded");d.send(a.data)}else{d.send()}}var Q=function(a){return document.querySelector(a)};function fill(b){for(var a in b){var c=Q("input[name="+a+"]");if(c.type=="checkbox"){c.checked=b[a]}else{c.value=b[a]}}}function save(){var a=document.getElementsByTagName("input");var c={};for(var b=0;b<a.length;b++){var d=a[b];if(d.type=="checkbox"){c[d.name]=d.checked}else{if(d.type=="text"){c[d.name]=d.value}}}console.log("result="+c);s_ajax({url:gdcurl,m:"POST",mime:"aplication/json",data:JSON.stringify(c),success:function(e){alert("done.")},fail:function(e){alert("failed updating data:"+e)}})}function init(){s_ajax({url:gdcurl+"?data",m:"GET",success:function(b){fill(JSON.parse(b))},fail:function(b){}})};/*]]>*/</script>
</head>
<body onload=init()>
<form action="" method="post">
<table>
<tr>
<td>iSpindel</td>
<td><input type="checkbox" name="ispindel" value="1"></td>
</tr>
<tr>
<td>SG Calibration</td>
<td><input type="text" name="gc" size=4> point</td>
</tr>
<tr>
<td>Temp. Correction</td>
<td><input type="checkbox" name="tc" value="1"> @ <input type="text" name="ctemp" size=4>&deg;C </td>
</tr>
<tr>
<td>Coefficients</td>
<td><input type="text" name="a3" size=15>*x^3 + <input type="text" name="a2" size=15>*x^2+ <input type="text" name="a1" size=15>*x + <input type="text" name="a0" size=15> </td>
</tr>
<tr>
<td>Save Change</td>
<td><input type="submit" name="submit" onclick="save();return false"></input>
</td>
</tr>
</table>
</form>
</form>
</body>
</html>
)END";
