    var T_CHART_REQUEST = 12000;
    var T_CHART_RETRYTO = 6000;
    var T_CHART_ZERODATA = 10000;
    var T_CHART_REFRESH = 5000;
    var T_CHART_RETRY = 10000;

    var T_BWF_RECONNECT = 10000;
    var T_BWF_LCD = 4800;
    var BChart = {
        offset: 0,
        url: 'chart.php',
        toggle: function(line) {
            this.chart.toggleLine(line);
        },
        updateFormula: function() {
            var coeff = this.chart.coefficients;
            var npt = (this.chart.npt << 24) | (this.chart.cal_igmask & 0xFFFFFF);
            var changed = true;
            if (typeof window.npt != "undefined" && window.npt == npt) {
                changed = false;
            }
            if (!changed) return;
            var url = "coeff?" + "a0=" + coeff[0].toFixed(9) +
                "&a1=" + coeff[1].toFixed(9) + "&a2=" + coeff[2].toFixed(9) +
                "&a3=" + coeff[3].toFixed(9) + "&pt=" + npt;
            s_ajax({
                url: url,
                m: "GET",
                success: function(d) { window.npt = npt; },
                fail: function(d) {
                    alert("failed sending formula:" + d);
                }
            });
        },
        reprocesData: function() {
            // recalcualte data
            // re process data to get correct calibration points
            var t = this;
            for (var i = 0; i < t.bdata.length; i++)
                t.chart.process(t.bdata[i]);
        },
        updateChartResult: function() {
            var t = this;
            if (t.chart.sg && !isNaN(t.chart.sg)) {
                updateGravity(t.chart.sg);
                t.chart.sg = NaN;
                checkfgstate();
            }
            t.chart.updateChart();
        },
        setIgnoredMask: function(m) {
            var t = this;
            if (t.chart.cal_igmask == m) return;

            t.chart.calculateSG = false;
            t.reprocesData();
            // the data will be updated by the "data"
            t.chart.cal_igmask = m;
            t.chart.getFormula();

            t.reprocesData();

            t.updateChartResult();
            // the data will be updated by the "data",again
            t.chart.cal_igmask = m;
            t.updateFormula();
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
            xhr.timeout = T_CHART_REQUEST;
            xhr.responseType = 'arraybuffer';
            xhr.onload = function(e) {
                if (this.status == 404) {
                    console.log("Error getting log data");
                    return;
                }
                // response is unsigned 8 bit integer
                var data = new Uint8Array(this.response);
                // backup data 
                if (t.offset) t.bdata.push(data);
                else t.bdata = [data];

                if (data.length == 0) {
                    //console.log("zero content");
                    if (t.timer) clearInterval(t.timer);
                    t.timer = null;
                    setTimeout(function() {
                        t.reqdata();
                    }, T_CHART_ZERODATA);
                    return;
                }
                var res = t.chart.process(data);
                if (res.nc) {
                    t.offset = data.length;
                    t.startOff = xhr.getResponseHeader("LogOffset");
                    //t.getLogName();
                    //console.log("new chart, offset="+t.startOff);
                    if (t.chart.calibrating) {
                        t.chart.getFormula();
                        //  do it again
                        t.chart.process(data);
                        if (t.chart.calculateSG) {
                            Q("#formula-btn").style.display = "block";
                            // update formula
                            t.updateFormula();
                        }
                    }
                } else {
                    t.offset += data.length;
                    if (t.chart.calibrating && res.sg) {
                        // new calibration data available. 
                        //force to reload and re-process the data
                        console.log("New SG availbe. reprocess");
                        t.chart.calculateSG = false;
                        t.reprocesData();
                        // the data will be updated by the "data"
                        t.chart.getFormula(); // derive the formula
                        // this time, the gravity is calculated.
                        t.reprocesData();
                        t.updateChartResult();
                        t.updateFormula();
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
            xhr.ontimeout = function(e) {
                console.error("Timeout!" + (new Date()));
                if (t.timer == null) setTimeout(function() {
                    t.reqdata();
                }, T_CHART_RETRYTO);
            };
            xhr.onerror = function() {
                console.log("error getting data.");
                if (t.timer == null) setTimeout(function() {
                    t.reqdata();
                }, T_CHART_RETRY);
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
            }
            /*
            , 
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
                        console.log("get logname fail");
                    }
                });
            }*/
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
            status.unit = temps[4];
            status[temps[1] + "Temp"] = isNaN(Number(temps[2])) ? temps[2] : temps[2] + temps[4];
            status[temps[1] + "Set"] = isNaN(Number(temps[3])) ? temps[3] : temps[3] + temps[4];
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
            /Waiting\s+for\s+Peak/i,
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
        var div = Q(".error");
        if (div) div.style.display = "none";

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
        // keep the info for other usage
        if (typeof status["unit"] != "undefined") window.tempUnit = status.unit;
        if (typeof status["BeerTemp"] != "undefined") {
            var tempRE = /([\d\.]+)/;
            var temp = tempRE.exec(status.BeerTemp);
            if (temp.length > 0) window.beerTemp = temp[0];
        }

        // display classic LCD
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
        if (div) {
            div.innerHTML = "Failed to connect to server.";
            div.style.display = "block";
        } else displayLcdText(["Failed to", "connect to", "Server", ""]);
    }

    function controllerError() {
        var div = Q('.error');
        if (div) {
            div.innerHTML = "Controller not updating data.";
            div.style.display = "block";
        } else displayLcdText(["Controller not", "updating data", "...", ""]);
    }

    function checkTime(time, tzoff) {
        var d = new Date();
        var tmoff = 0 - d.getTimezoneOffset() * 60;
        var tm = Math.round(d.getTime() / 1000);
        if (tzoff != tmoff || Math.abs(tm - time) > 1800) {
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
        // before iSpindel report to BPL, the name file is "unknown"
        if (typeof msg["fpt"] != "undefined") {
            window.npt = msg["fpt"];
        }

        //The first report will be "unknown" if (msg.name.startsWith("iSpindel")) {
        // iSpindel
        if (typeof msg["lu"] == "undefined") {
            console.log("iSpindel:" + JSON.stringify(msg));
            return;
        }

        if (typeof window.iSpindel == "undefined") {
            window.iSpindel = true;
            if (Q("#iSpindel-pane"))
                Q("#iSpindel-pane").style.display = "block";
        }
        var ndiv = Q("#iSpindel-name");
        if (ndiv) ndiv.innerHTML = msg.name;

        if (typeof msg["battery"] != "undefined" && Q("#iSpindel-battery"))
            Q("#iSpindel-battery").innerHTML = msg.battery;

        var lu;
        if (typeof msg["lu"] != "undefined")
            lu = new Date(msg.lu * 1000);
        else
            lu = new Date();
        if (Q("#iSpindel-last"))
            Q("#iSpindel-last").innerHTML = lu.shortLocalizedString();

        if (!BChart.chart.calibrating && typeof msg["sg"] != "undefined")
            updateGravity(msg["sg"]);

        if (typeof msg["angle"] != "undefined") {
            if (Q("#iSpindel-tilt"))
                Q("#iSpindel-tilt").innerHTML = "" + msg["angle"];
        }
        //}
        if (typeof msg["lpf"] != "undefined")
            GravityFilter.setBeta(msg["lpf"]);

        if (typeof msg["stpt"] != "undefined")
            GravityTracker.setThreshold(msg["stpt"]);

        if (typeof msg["ctemp"] != "undefined")
            window.caltemp = msg["ctemp"];
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
        // update temp.
        if (typeof window["tempUnit"] != "undefined") {
            window.celsius = false;
            var defaultTemp = 68;
            var re = /C$/;
            if (re.test(window.tempUnit)) {
                window.celsius = true;
                defaultTemp = 20;
            }
            Q("#dlg_addgravity .tempinput").value = defaultTemp;

            var tus = document.querySelectorAll("#dlg_addgravity .temp-unit");
            for (var i = 0; i < tus.length; i++)
                tus[i].innerHTML = window.tempUnit;
        } else window.celsius = true;
    }

    function dismissgravity() {
        Q('#dlg_addgravity').style.display = "none";
    }

    function inputsg_change() {
        var gravity = parseFloat(Q("#dlg_addgravity .sginput").value);
        var temp = parseFloat(Q("#dlg_addgravity .tempinput").value);
        if (isNaN(gravity) || isNaN(temp)) return;
        // if calibration info is avilable
        var caltemp = (typeof window.caltemp != "undefined") ? window.caltemp : 20;
        caltemp = window.celsius ? caltemp : C2F(caltemp);
        // calibration temperature always use celsius.
        Q("#sginput-hm-cal-temp").innerHTML = caltemp;
        var correctedSg = BrewMath.tempCorrection(window.celsius, gravity, temp, caltemp);
        Q("#sginput-hmc").innerHTML = correctedSg.toFixed(3);
        // if iSpindel info is available, or beer temp is available.
        if (typeof window.beerTemp != "undefined") {
            Q("#sginput-ispindel-temp").innerHTML = window.beerTemp;
            var sgc = BrewMath.tempCorrection(window.celsius, gravity, temp, window.beerTemp);
            Q("#sginput-sg-ispindel").innerHTML = sgc.toFixed(3);
        }
    }

    function inputgravity() {
        var gravity = parseFloat(Q("#sginput-hmc").innerHTML);

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
                setTimeout(function() {
                    // request to 
                    if (BChart.chart.calibrating) BChart.reqnow();
                }, T_CHART_REFRESH);
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

    function ptcshow(msg) {
        if (typeof msg["ptc"] == "undefined" || typeof msg["pt"] == "undefined") return;
        var mode = msg.ptc;
        var time = msg.pt;

        function fortime(t) {
            var hour = Math.floor(t / 3600);
            var min = Math.floor((t - hour * 3600) / 60);
            var sec = t - hour * 3600 - min * 60;
            return ((hour) ? (hour + "H") : "") + ((hour + min) ? (min + "M") : "") + sec + "S";
        }
        var pane = Q("#ptc-pane");
        if (pane) {
            if (mode == "o") pane.style.display = "none";
            else {
                pane.style.display = "block";
            }
        } else return;
        var state = Q("#ptc-state");
        if (state) state.style.backgroundColor = (mode == "c") ? "lightgreen" : "gray";

        var textstateidle = Q("#ptc-state-idle");
        if (textstateidle) {
            var textstaterun = Q("#ptc-state-run");
            if (mode == "c") {
                textstateidle.style.display = "none";
                textstaterun.style.display = "block";
            } else {
                textstateidle.style.display = "block";
                textstaterun.style.display = "none";
            }
        }

        var tinfo = Q("#ptc-time");
        if (tinfo) tinfo.innerHTML = fortime(time);
        if (typeof msg["ptctp"] != "undefined") {
            var temp = Q("#ptc-temp");

            if (temp) temp.innerHTML = (msg.ptctp < -100) ? "NA" : ((msg.ptctp / 100) + "&deg;C");
        }
        if (typeof msg["ptclo"] != "undefined" && typeof msg["ptcup"] != "undefined") {
            var ts = Q("#ptc-set");
            if (ts) ts.innerHTML = (msg.ptclo / 100) + " ~ " + (msg.ptcup / 100) + "&deg;C";
        }
    }

    function BPLMsg(c) {
        if (typeof c["rssi"] != "undefined") {
            displayrssi(c["rssi"]);
        }
        if (typeof c["reload"] != "undefined") {
            console.log("forced reload chart");
            BChart.reqnow();
            if (!Q("#recording").innerHTML || Q("#recording").innerHTML != c.log)
                window.npt = 0; // delete formula to force update to BPL.                
        }
        if (typeof c["nn"] != "undefined") {
            Q("#hostname").innerHTML = c["nn"];
        }
        if (typeof c["ver"] != "undefined") {
            if (JSVERSION != c["ver"]) alert("Version Mismatched!. Reload the page.");
            Q("#verinfo").innerHTML = "v" + c["ver"];
        }
        if (typeof c["tm"] != "undefined" && typeof c["off"] != "undefined") {
            checkTime(c.tm, c.off);
        }
        if (typeof c["log"] != "undefined") {
            Q("#recording").innerHTML = c.log;
        }
        if (typeof c["cap"] != "undefined")
            Capper.status(c["cap"]);

        ptcshow(c);
    }

    function connBWF() {
        BWF.init({
            //            reconnect: false,
            onconnect: function() {
                BWF.send("cl");
                if (window.lcdTimer) clearInterval(window.lcdTimer);
                window.lcdTimer = setInterval(function() {
                    if (!BWF.gotMsg) {
                        if (window.rcTimeout) {
                            // reconnect timer is running.
                            BWF.rcCount++;
                            console.log("rcTimeout failed.");
                            // let the reconnecting timer has more chances to do its job                         
                            if (BWF.rcCount < 3) return;
                            // restart reconect timer
                            clearTimer(window.rcTimeout);
                        }
                        // once connected.
                        //  no data for 5 seconds
                        controllerError();
                        window.rcTimeout = setTimeout(function() {
                            window.rcTimeout = null;
                            if (!BWF.gotMsg) BWF.reconnect(true);
                        }, T_BWF_RECONNECT);
                        BWF.rcCount = 0;
                        // setTimer might not be reliable. when the computer enter suspended state.
                        // keep this timer for saftey.
                        // clearInterval(window.lcdTimer);
                        //window.lcdTimer = null;
                        return;
                    }
                    //gotMsg==true, set flag and send
                    BWF.gotMsg = false;
                    BWF.send("l");
                }, T_BWF_LCD);
            },
            error: function(e) {
                //console.log("error");
                // when connection establishment fails 
                // or connection broken
                communicationError();
                // do nothing, let BWF do the resconnection.
                //              setTimeout(function() {
                //                   if (!BWF.gotMsg) BWF.reconnect();
                //              }, 12000);
            },
            handlers: {
                L: function(lines) {
                    BWF.gotMsg = true;
                    processLcdText(lines);
                },
                A: BPLMsg,
                G: function(c) {
                    gravityDevice(c);
                },
                // for control page. 
                C: function(c) { if (typeof ccparameter != "undefined") ccparameter(c); },
                B: function(c) { if (typeof rcvBeerProfile != "undefined") rcvBeerProfile(c); }
            }
        });
    }

    function init_classic() {
        BChart.init("div_g");
        initRssi();
        Capper.init();
        BWF.gotMsg = true;
        initctrl_C();
        connBWF();
        setTimeout(function() { BChart.start(); }, 250);
    }

    function init() {
        BChart.init("div_g");
        initRssi();
        Capper.init();
        BWF.gotMsg = true;
        connBWF();
        setTimeout(function() { BChart.start(); }, 250);
        getActiveNavItem();
    }