var JSVERION="2.3.2";
var Q=function(d){return document.querySelector(d);};

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

function communicationError() {
  Q('.error').innerHTML="Failed to connect to server.";
}

function controllerError() {
  Q('.error').innerHTML="Controller not updating data.";
};

function openDlgLoading() {
  document.getElementById('dlg_loading').style.display = "block";
}

function closeDlgLoading() {
  document.getElementById('dlg_loading').style.display = "none";
}

var BrewMath = {
    abv: function(og, fg) {
        return ((76.08 * (og - fg) / (1.775 - og)) * (fg / 0.794)).toFixed(1);
    },
    att: function(og, fg) {
        return Math.round((og - fg) / (og - 1) * 100);
    },
    sg2pla: function(sg) {
        return -616.868 + 1111.14 * sg - 630.272 * sg * sg + 135.997 * sg * sg * sg;
    },
    pla2sg: function(pla) {
        return 1 + (pla / (258.6 - ((pla / 258.2) * 227.1)));
    }
};

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
