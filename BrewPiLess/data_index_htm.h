const char data_index_htm[] PROGMEM =
R"END(
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BrewPi reporting for duty!</title>
<meta name="apple-mobile-web-app-title" content="BrewPiLite">
<meta name="apple-mobile-web-app-capable" content="yes">
<script type="text/javascript" src="bwf.js"></script>
</head>
<style>.lcddisplay{width:280px;height:90px;float:left;margin:5px;background:#000;background:-moz-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-webkit-gradient(linear,left top,left bottom,color-stop(2%,#000000),color-stop(11%,#2b2b2b),color-stop(54%,#212121),color-stop(92%,#212121),color-stop(100%,#000000));background:-webkit-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-o-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);background:-ms-linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#000000',endColorstr='#000000',GradientType=0);background:linear-gradient(top,#000000 2%,#2b2b2b 11%,#212121 54%,#212121 92%,#000000 100%);-webkit-box-shadow:inset 1px 1px 5px #333;-moz-box-shadow:inset 1px 1px 5px #333;box-shadow:inset 1px 1px 5px #333;border:2px solid #333;-webkit-border-radius:2px;-moz-border-radius:2px;border-radius:2px}.lcddisplay .lcd-text{float:left;margin:5px 16px}.lcd-line{float:left;clear:left;font-size:16px;font-weight:normal;font-style:normal;font-family:"Courier New",Courier,monospace;color:#ff0;white-space:pre}.dropdown{position:absolute;left:2px;top:2px;display:inline-block}.dropdown-content{display:none;position:absolute;background-color:#f9f9f9;min-width:160px;overflow:auto;box-shadow:0 8px 16px 0 rgba(0,0,0,0.2)}.dropdown-content a{color:black;padding:12px 16px;text-decoration:none;display:block;border:1px solid}.dropdown a:hover{background-color:#f1f1f1}.show{display:block}</style>
<script>/*<![CDATA[*/function setLcdText(c,a){var b=document.getElementById(c);b.innerHTML=a}function communicationError(){setLcdText("lcd-line-0","Failed to");setLcdText("lcd-line-1","connect to");setLcdText("lcd-line-2","Server");setLcdText("lcd-line-3","")}function controllerError(){setLcdText("lcd-line-0","Controller not");setLcdText("lcd-line-1","updating data");setLcdText("lcd-line-2","...");setLcdText("lcd-line-3","")}function init(){document.getElementById("lcd").onclick=function(){document.getElementById("myDropdown").classList.toggle("show");event.stopPropagation()};var a=true;BWF.init({error:function(b){console.log("error");communicationError()},onopen:function(){BWF.send("l")},handlers:{L:function(b){a=true;for(var c=0;c<4;c++){setLcdText("lcd-line-"+c,b[c])}}}});setInterval(function(){if(!a){controllerError()}BWF.send("l");a=false},5000)}window.onclick=function(c){var d=document.getElementsByClassName("dropdown-content");var b;for(b=0;b<d.length;b++){var a=d[b];if(a.classList.contains("show")){a.classList.remove("show")}}};/*]]>*/</script>
<body onload=init()>
<div id="lcd" class="lcddisplay"><span class="lcd-text">
<span class="lcd-line" id="lcd-line-0">Live LCD waiting</span>
<span class="lcd-line" id="lcd-line-1">for update from</span>
<span class="lcd-line" id="lcd-line-2">script...</span>
<span class="lcd-line" id="lcd-line-3"></span></p><p>
</div>
<div class="dropdown">
<div id="myDropdown" class="dropdown-content">
<a href="/control.htm">Temperature Management</a>
<a href="/log">Data Log</a>
<a href="/setup.htm">Device Setup</a>
<a href="/config">System Config</a>
</div>
</body>
</html>
)END";




























