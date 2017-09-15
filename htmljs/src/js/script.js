/**********************************************************************
created by Vito Tai
Copyright (C) 2016 Vito Tai

This soft ware is provided as-is. Use at your own risks.
You are free to modify and distribute this software without removing
this statement.
BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

function onload(next) {
  function complete() {
    var initial = true;
    function initComp() {
      if (!initial) return;
      initial = false;
      // closeDlgLoading();
      next();
    }
    invoke({
      url: "/tcc", m: "GET",
      fail: function (a) {
        console.log("error connect to BrwePiLess!");
        initComp();
      },
      success: function (d) {
        var s = JSON.parse(d);
        var setting = { valid: true, minDegree: s.tempSetMin, maxDegree: s.tempSetMax, tempUnit: s.tempFormat };
        if (setting.tempUnit != BrewPiSetting.tempUnit) {
          updateTempUnit(setting.tempUnit);
          // profileEditor.setTempUnit(setting.tempUnit);
        }
        BrewPiSetting = setting;
        initComp();

      }
    });
  }
}

function getLogName() {
  s_ajax({
    url: "loglist.php", m: "GET",
    success: function (d) {
      var r = JSON.parse(d);
      if (r.rec) {
        Q("#recording").innerHTML = r.log;
      } else {
        Q("#recording").innerHTML = "";
      }
    },
    fail: function (d) { alert("failed:" + d); }
  });
};

var BChart = {
  offset: 0,
  url: 'chart.php',
  toggle: function (type) {
    this.chart.toggleLine(type);
  },
  reqdata: function () {
    var t = this;
    var PD = 'offset=' + t.offset;

    if (typeof t.startOff != "undefined" && t.startOff !== null)
      PD = PD + "&index=" + t.startOff;
    var xhr = new XMLHttpRequest();
    xhr.open('GET', t.url + '?' + PD);
    //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    //	xhr.setRequestHeader("Content-length", PD.length);
    xhr.timeout = 5000;
    xhr.ontimeout = function (e) {
      console.error("Timeout!!")
    };
    xhr.responseType = 'arraybuffer';
    xhr.onload = function (e) {
      if (this.status == 404) {
        console.log("Error getting log data");
        return;
      }
      // response is unsigned 8 bit integer
      var data = new Uint8Array(this.response);
      if (data.length == 0) {
        //console.log("zero content");
        if (t.timer) clearInterval(t.timer);
        t.timer = null;
        setTimeout(function () {
          t.reqdata();
        }, 3000);
        return;
      }
      var newchart = t.chart.process(data);
      if (newchart) {
        t.offset = data.length;
        t.startOff = xhr.getResponseHeader("LogOffset");
        getLogName();
        //console.log("new chart, offset="+t.startOff);
      } else t.offset += data.length;

      if (!isNaN(t.chart.og)) {
        updateOriginGravity(t.chart.og);
        t.chart.og = NaN;
      }
      if (t.chart.sg && !isNaN(t.chart.sg)) {
        updateGravity(t.chart.sg);
        t.chart.sg = NaN;
        checkfgstate();
      }
      if (t.timer == null) t.settimer();
    };
    xhr.onerror = function () {
      console.log("error getting data.");
      setTimeout(function () {
        t.reqdata();
      }, 10000);
    };
    //console.log(PD);
    xhr.send();
  },
  settimer: function () {
    var t = this;
    //console.log("start timer at "+ t.chart.interval);
    t.timer = setInterval(function () {
      t.reqdata();
    }, t.chart.interval * 1000);
  },
  init: function (id) {
    this.chart = new BrewChart(id);
  },
  timer: null,
  start: function () {
    if (this.running) return;
    this.running = true;
    this.offset = 0;
    this.reqdata();
  },
  reqnow: function () {
    var t = this;
    if (t.timer) clearInterval(t.timer);
    t.timer = null;
    t.reqdata();
  }
};

function gravityDevice(msg) {
  if (typeof msg["name"] == "undefined") return;
  if (msg.name.startsWith("iSpindel")) {
    // iSpindel
    if (typeof window.iSpindel == "undefined") {
      window.iSpindel = true;
      Q("#ispindel-pane").style.display = "block";
    }
    Q("#iSpindel-name").innerHTML = msg.name;
    if (typeof msg["battery"] != "undefined")
      Q("#iSpindel-battery").innerHTML = msg.battery;

    var lu;
    if (typeof msg["lu"] != "undefined")
      lu = new Date(msg.lu * 1000);
    else
      lu = new Date();
    Q("#iSpindel-last").innerHTML = formatDate(lu);

    if (typeof msg["sg"] != "undefined")
      updateGravity(msg["sg"]);

    if (typeof msg["angle"] != "undefined")
      console.log("iSpindel:" + JSON.stringify(msg));
  }
  if (typeof msg["lpf"] != "undefined")
    GravityFilter.setBeta(msg["lpf"]);

  if (typeof msg["stpt"] != "undefined")
    GravityTracker.setThreshold(msg["stpt"]);
};

function displayrssi(x) {
  var strength = [-1000, -90, -80, -70, -67];
  var bar = 4;
  for (; bar >= 0; bar--) {
    if (strength[bar] < x) break;
  }
  var bars = document.getElementsByClassName("rssi-bar");
  for (var i = 0; i < bars.length; i++) {
    bars[i].style.backgroundColor = (i < bar) ? "#888" : "#DDD";
  }
  Q("#rssi").title = (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100) + "%";
};

function init() {
  BChart.init("div_g");
  getActiveNavItem();

  var gotMsg = true;

  BWF.init({
    onconnect: function () {
      BWF.send("l");
      setInterval(function () {
        if (!gotMsg) controllerError();
        BWF.send("l");
        gotMsg = false;
      }, 5000);
    },
    error: function (e) {
      communicationError();
    },
    handlers: {
      L: function (t) {
        e = !0;
        processLcdText(t);
      },
      V: function (c) {
        if (typeof c["rssi"] != "undefined") {
          displayrssi(c["rssi"]);
        }
        if (typeof c["reload"] != "undefined") {
          console.log("forced reload chart");
          BChart.reqnow();
        }
        if (typeof c["nn"] != "undefined") {
          Q("#hostname").innerHTML = c["nn"];
        }
        if (typeof c["ver"] != "undefined") {
          if (JSVERION != c["ver"]) alert("Version Mismatched!. Reload the page.");
          Q("#verinfo").innerHTML = "v" + c["ver"];
        }
      },
      G: function (c) { gravityDevice(c); }
    }
  });

  BChart.start();
};

function updateGravity(sg) {
  //if(typeof window.sg != "undefined") return;
  window.sg = sg;
  Q("#gravity-sg").innerHTML = sg.toFixed(3);
  if (typeof window.og != "undefined") {
    Q("#gravity-att").innerHTML = BrewMath.att(window.og, sg);
    Q("#gravity-abv").innerHTML = BrewMath.abv(window.og, sg);
  }
}

function updateOriginGravity(og) {
  if (typeof window.og != "undefined" && window.og == og) return;
  window.og = og;
  Q("#gravity-og").innerHTML = og.toFixed(3);
  if (typeof window.sg != "undefined")
    updateGravity(window.sg);
}

function showgravitydlg(msg) {
  Q('#dlg_addgravity .message').innerHTML = msg;
  Q('#dlg_addgravity').style.display = "flex";

  var inputElement = Q('#dlg_addgravity input');
  var okButton = Q('#dlg_addgravity .btn--ok');

  checkInputValidity(inputElement, okButton);

  inputElement.oninput = function () {
    checkInputValidity(inputElement, okButton);
  }
}

function checkInputValidity(inputElement, okButton) {
  if (inputElement.checkValidity() && inputElement.value) {
    okButton.disabled = false;
  } else {
    okButton.disabled = true;
  }
}

function dismissgravity() {
  Q('#dlg_addgravity').style.display = "none";
}

function inputgravity() {
  var gravity = parseFloat(Q("#dlg_addgravity input").value);

  if (gravity < 0.8 || gravity > 1.25) return;
  dismissgravity();
  openDlgLoading();

  if (window.isog) updateOriginGravity(gravity);
  else updateGravity(gravity);

  var data = { name: "webjs", gravity: gravity };
  if (window.isog)
    data.og = 1;
  s_ajax({
    url: "gravity", m: "POST", mime: "application/json",
    data: JSON.stringify(data),
    success: function (d) {
      closeDlgLoading();
    },
    fail: function (d) {
      alert("failed:" + d);
      closeDlgLoading();
    }
  });
}

function inputSG() {
  window.isog = false;
  showgravitydlg("Add Gravity Record");
}

function inputOG() {
  window.isog = true;
  showgravitydlg("Set Original Gravity");
}
