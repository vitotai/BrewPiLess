    var JSVERION = "2.4";

    var BChart = {
        offset: 0,
        url: 'chart.php',
        toggle: function(line) {
            this.chart.toggleLine(line);
        },
        reqdata: function() {
            var t = this;
            var PD = 'offset=' + t.offset;

            if (typeof t.startOff != "undefined" && t.startOff !== null)
                PD = PD + "&index=" + t.startOff;
            var xhr = new XMLHttpRequest();
            xhr.open('GET', t.url + '?' + PD);
            //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            //	xhr.setRequestHeader("Content-length", PD.length);
            xhr.timeout = 5000;
            xhr.ontimeout = function(e) {
                console.error("Timeout!!")
            };
            xhr.responseType = 'arraybuffer';
            xhr.onload = function(e) {
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
                    setTimeout(function() {
                        t.reqdata();
                    }, 3000);
                    return;
                }
                var res = t.chart.process(data);
                if (res.nc) {
                    t.offset = data.length;
                    t.startOff = xhr.getResponseHeader("LogOffset");
                    t.getLogName();
                    //console.log("new chart, offset="+t.startOff);
                    if (t.chart.calibrating) {
                        t.chart.getFormula();
                        //  do it again
                        t.chart.process(data);
                        if (t.chart.calculateSG) Q("#formula-btn").style.display = "block";
                    }
                } else {
                    t.offset += data.length;
                    if (t.chart.calibrating && res.sg) {
                        // new calibration data available. 
                        //force to reload and re-process the data
                        console.log("New SG availbe. reload");
                        setTimeout(function() {
                            t.offset = 0;
                            t.chart.calculateSG = false;
                            t.reqnow();
                        }, 150);
                        return;
                    }
                }

                t.chart.updateChart();

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
            xhr.onerror = function() {
                console.log("error getting data.");
                setTimeout(function() {
                    t.reqdata();
                }, 10000);
            };
            //console.log(PD);
            xhr.send();
        },
        settimer: function() {
            var t = this;
            //console.log("start timer at "+ t.chart.interval);
            t.timer = setInterval(function() {
                t.reqdata();
            }, t.chart.interval * 1000);
        },
        init: function(id) {
            this.chart = new BrewChart(id);
        },
        timer: null,
        start: function() {
            if (this.running) return;
            this.running = true;
            this.offset = 0;
            this.reqdata();
        },
        reqnow: function() {
            var t = this;
            if (t.timer) clearInterval(t.timer);
            t.timer = null;
            t.reqdata();
        },
        getLogName: function() {
            s_ajax({
                url: "loglist.php",
                m: "GET",
                success: function(d) {
                    var r = JSON.parse(d);
                    if (r.rec) {
                        Q("#recording").innerHTML = r.log;
                    } else {
                        Q("#recording").innerHTML = "";
                    }
                },
                fail: function(d) {
                    alert("failed:" + d);
                }
            });
        }
    };
    /* LCD information */
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
        var tempRE = /\s*([a-zA-Z]+)\s*([-\d\.]+)\s+([-\d\.]+)\s+(\S+[CF])\s*$/i;
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
        //            Q("#status-pane .error").style.display = "none";
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
        displayLcdText(lines);
    }

    function displayLcdText(lines) {
        for (var i = 0; i < 4; i++) {
            var d = document.getElementById("lcd-line-" + i);
            if (d) d.innerHTML = lines[i];
        }
    }

    function communicationError() {
        var div = Q('.error');
        if (div) div.innerHTML = "Failed to connect to server.";
        else displayLcdText(["Failed to", "connect to", "Server", ""]);
    }

    function controllerError() {
        var div = Q('.error');
        if (div) div.innerHTML = "Controller not updating data.";
        else displayLcdText(["Controller not", "updating data", "...", ""]);
    }

    function checkTime(time, tzoff) {
        var d = new Date();
        var tmoff = 0 - d.getTimezoneOffset() * 60;
        var tm = Math.round(d.getTime() / 1000);
        if (tzoff != tmoff || Math.abs(tm - time) > 10) {
            // update time & timezone
            s_ajax({
                url: "time",
                m: "POST",
                mime: "application/x-www-form-urlencoded",
                data: "time=" + tm + "&off=" + tmoff,
                success: function() {}
            });

        }
    }

    function gravityDevice(msg) {
        if (typeof msg["name"] == "undefined") return;
        if (msg.name.startsWith("iSpindel")) {
            // iSpindel
            if (typeof msg["lu"] == "undefined") {
                console.log("iSpindel:" + JSON.stringify(msg));
                return;
            }

            if (typeof window.iSpindel == "undefined") {
                window.iSpindel = true;
                Q("#iSpindel-pane").style.display = "block";
            }
            Q("#iSpindel-name").innerHTML = msg.name;
            if (typeof msg["battery"] != "undefined")
                Q("#iSpindel-battery").innerHTML = msg.battery;

            var lu;
            if (typeof msg["lu"] != "undefined")
                lu = new Date(msg.lu * 1000);
            else
                lu = new Date();
            Q("#iSpindel-last").innerHTML = lu.shortLocalizedString();

            if (!BChart.chart.calibrating && typeof msg["sg"] != "undefined")
                updateGravity(msg["sg"]);

            if (typeof msg["angle"] != "undefined") {

                Q("#iSpindel-tilt").innerHTML = "" + msg["angle"];
            }
        }
        if (typeof msg["lpf"] != "undefined")
            GravityFilter.setBeta(msg["lpf"]);

        if (typeof msg["stpt"] != "undefined")
            GravityTracker.setThreshold(msg["stpt"]);
    }


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
        Q('#dlg_addgravity').style.display = "block";
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

        var data = {
            name: "webjs",
            gravity: gravity
        };
        if (window.isog)
            data.og = 1;
        s_ajax({
            url: "gravity",
            m: "POST",
            mime: "application/json",
            data: JSON.stringify(data),
            success: function(d) {
                closeDlgLoading();
            },
            fail: function(d) {
                alert("failed:" + d);
                closeDlgLoading();
            }
        });

    }

    function inputSG() {
        window.isog = false;
        showgravitydlg("Add gravity Record:");
    }

    function inputOG() {
        window.isog = true;
        showgravitydlg("Set Original Gravity:");
    }


    function displayrssi(x) {
        var strength = [-1000, -90, -80, -70, -67];
        var bar = 4;
        for (; bar >= 0; bar--) {
            if (strength[bar] < x) break;
        }
        var bars = document.getElementsByClassName("rssi-bar");
        for (var i = 0; i < bars.length; i++) {
            bars[i].style.backgroundColor = (i < bar) ? window.rssiBarColor : "rgba(255,255,255,0.05)";
        }
        Q("#rssi").title = (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
        if (Q("#wifisignal"))
            Q("#wifisignal").innerHTML = (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
    }

    function initRssi() {
        var rssi = Q("#rssi");
        window.rssiBarColor = window.getComputedStyle(Q('.rssi-bar1')).getPropertyValue('background-color');
        if (Q("#wifisignal")) {
            rssi.onmouseover = function() {
                Q("#wifisignal").style.display = "block";
            };
            rssi.onmouseout = function() {
                Q("#wifisignal").style.display = "none";
            };
        }
    }

    function connBWF() {
        BWF.init({
            reconnect: false,
            onconnect: function() {
                BWF.send("l");
                if (window.lcdTimer) clearInterval(window.lcdTimer);
                window.lcdTimer = setInterval(function() {
                    if (!gotMsg) {
                        controllerError();
                        BWF.reconnect();
                    }
                    gotMsg = false;
                    BWF.send("l");
                }, 5000);
            },
            error: function(e) {
                //console.log("error");
                communicationError();
                setTimeout(function() {
                    BWF.reconnect();
                }, 12000);
            },
            handlers: {
                L: function(lines) {
                    gotMsg = true;
                    processLcdText(lines);
                },
                V: function(c) {
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
                    if (typeof c["tm"] != "undefined" && typeof c["off"] != "undefined") {
                        checkTime(c.tm, c.off);
                    }
                },
                G: function(c) {
                    gravityDevice(c);
                }
            }
        });
    }

    function init_classic() {
        BChart.init("div_g");
        initRssi();
        var gotMsg = true;
        onloadCtrl(function() {
            connBWF();
            BChart.start();

        });
    }

    function init() {
        BChart.init("div_g");
        initRssi();
        var gotMsg = true;
        connBWF();
        BChart.start();

    }