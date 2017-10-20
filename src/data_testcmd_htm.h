const char data_testcmd_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BrewPi Test Commands</title>
<meta name="apple-mobile-web-app-title" content="BrewPiLite">
<meta name="apple-mobile-web-app-capable" content="yes">
<script type="text/javascript" src="bwf.js"></script>
</head>
<style>#log{width:100%;height:40em;background:#eee;overflow:scroll;overflow-wrap:break-word}</style>
<script>/*<![CDATA[*/function escapeHtml(a){return a.replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;").replace(/'/g,"&#039;")}function log(f,a){var e=document.getElementById("log");var c=new Date();var b=c.getHours()+":"+c.getMinutes()+":"+c.getSeconds();e.innerHTML=e.innerHTML+b+" "+((f=="D")?"&darr;":"&uarr;")+" "+escapeHtml(a)+"<br>"}function sendCmd(){var b=document.getElementById("command");var a=b.value.trim();if(a.legnth==0){return}BWF.send(a);log("U",a);return false}function clearLogs(){document.getElementById("log").innerHTML=""}function init(){BWF.init({error:function(a){alert(a)},raw:function(a){log("D",a)}})};/*]]>*/</script>
<body onload=init()>
<div id="log">
</div>
<input id="command" type="text" size="60"></input>
<button onclick=sendCmd()>Send</button>
<button onclick=clearLogs()>Clear</button>
</body>
</html>
)END";
