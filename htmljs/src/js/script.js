    var T_CHART_REQUEST = 20000;
    var T_CHART_RETRYTO = 6000;
    var T_CHART_ZERODATA = 10000;
    var T_CHART_REFRESH = 1500;
    var T_CHART_RETRY = 10000;
    var T_LOAD_CHART = 150;
    var T_BWF_RECONNECT = 10000;
    var T_BWF_LCD = 10000;


    var BChart = {
        offset: 0,
        url: 'chart.php',
        calibrating:function(){
            if(typeof this.chart =="undefined") return false;
            return this.chart.calibrating;
        },
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
                    alert("<%= script_fail_update_formula %>" + d);
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
                gravityChangeUpdate(t.chart.filterSg);
                t.chart.sg = NaN;
                //checkfgstate();
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
                    console.log(" Error getting log data");
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
                    gravityChangeUpdate(t.chart.filterSg);
                    t.chart.sg = NaN;
                    //checkfgstate();
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
                console.log("Error getting data");
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
        init: function(id, y1, y2,id2,pl,carbonation,id3,rhLabel) {
            this.chart = new BrewChart(id);
            this.chart.setLabels(y1, y2);
            if(typeof id2 != "undefined") this.chart.setPChart(id2,pl,carbonation)
            if(typeof id3 != "undefined") this.chart.setHChart(id3,rhLabel);
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

    var StateText = [
        "<%= state_text_idle %>",
        "<%= state_text_off %>",
        "<%= state_text_door_Open %>",
        "<%= state_text_heating %>",
        "<%= state_text_cooling %>",
        "<%= state_text_wait_to_cool %>",
        "<%= state_text_wait_to_heat %>",
        "<%= state_text_wait_for_peak %>",
        "<%= state_text_cooling_min_time %>",
        "<%= state_text_heating_min_time %>",
        "<%= state_text_invalid %>"
    ];

    function genStateText(state, duration) {
        if (state == 1 || state == 2 || state == 10 || state == 7) return StateText[state];

        var timestr = "";
        var mm = Math.floor(duration / 60);
        var hh = Math.floor(mm / 60);
        var ss = duration % 60;
        mm = mm - hh * 60;

        function zeropad(n){
            return n>9? ""+n:"0"+n;
        }

        if (hh > 0) {
            timestr = "<%= time_format_long %>".replace("{SS}", zeropad(ss)).replace("{MM}", zeropad(mm)).replace("{HH}", zeropad(hh));
        } else{
            // short
            timestr = "<%= time_format_short %>".replace("{SS}", zeropad(ss)).replace("{MM}", zeropad(mm));
        }
        return StateText[state].replace("{time}", timestr);
    }


    function renderLcdText(info) {
        var div = Q(".error");
        if (div) div.style.display = "none";

        function T(temp) {
            if (temp < -10000) return "--.-";
            return (temp / 100).toFixed(1) + "&deg;" + info.tu;
        }
        var status = {};
        status.ControlStateSince = info.sl;
        status.ControlState = info.st;
        status.ControlMode = info.md;
        status.unit = info.tu;
        status.BeerTemp = T(info.bt);
        status.BeerSet = T(info.bs);
        status.FridgeTemp = T(info.ft);
        status.FridgeSet = T(info.fs);
        status.RoomTemp = T(info.rt);

        var ModeString = {
            o: "<%= mode_off %>",
            b: "<%= mode_beer_const %>",
            f: "<%= mode_fridge_const %>",
            p: "<%= mode_beer_profile %>",
            i: "Invalid"
        };

        Object.keys(status).map(function(key, i) {
            var div = Q("#lcd" + key);
            if (div) {
                if (key == "ControlMode") div.innerHTML = ModeString[status[key]];
                else if (key == "ControlState") div.innerHTML = genStateText(status[key], status.ControlStateSince);
                else div.innerHTML = status[key];
            }
        });
        // keep the info for other usage
        if (typeof status["unit"] != "undefined") window.tempUnit = status.unit;
        if (typeof status["BeerTemp"] != "undefined")  window.beerTemp = (info.bt> -100)? (info.bt/ 100):NaN;
    }

    var roomOfridge = false;

    function simLcd(info) {

        var ModeString = {
            o: "Off",
            b: "Beer Const.",
            f: "Fridge Const.",
            p: "Beer Profile",
            i: "Invalid"
        };

        function showTemp(tp) {
            // always takes 5 chars
            if (tp < -10000) return " --.-";
            var text = (tp / 100.0).toFixed(1);
            var spaces = "";
            var i = text.length;
            for (; i < 5; i++) spaces += " ";
            return spaces + text;
        }

        var lines = [];
        lines[0] = "Mode   " + ModeString[info.md];
        lines[1] = "Beer  " + showTemp(info.bt) + " " + showTemp(info.bs) + " &deg;" + info.tu;
        if (info.rt > -10000 && roomOfridge)
            lines[2] = "Room  " + showTemp(info.rt) + " " + showTemp(-20000) + " &deg;" + info.tu;
        else
            lines[2] = "Fridge" + showTemp(info.ft) + " " + showTemp(info.fs) + " &deg;" + info.tu;
        roomOfridge = !roomOfridge;
        lines[3] = genStateText(info.st, info.sl);
        return lines;
    }

    function displayLcdText(lines) {
        for (var i = 0; i < 4; i++) {
            var d = document.getElementById("lcd-line-" + i);
            if (d) d.innerHTML = lines[i];
        }
    }

    function displayLcd(info) {
        // classic interface
        window.tempUnit = info.tu;
        displayLcdText(simLcd(info));
        // new interface
        renderLcdText(info);
    }

    function hideErrorMsgs() {
        var msgs = document.querySelectorAll(".errormsg");
        for (var i = 0; i < msgs.length; i++)
            msgs[i].style.display = "none";
    }

    function communicationError() {
        var div = Q('.error');
        if (div) {
            hideErrorMsgs();
            Q('#error_connect').style.display = "block";
            div.style.display = "block";
        } else displayLcdText(["Failed to", "connect to", "Server", ""]);
    }

    function controllerError() {
        var div = Q('.error');
        if (div) {
            hideErrorMsgs();
            Q('#error_noupdate').style.display = "block";
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

    function GDSetting(msg) {
        function gdevice(){
            Q(".gravity-device-pane").style.display="block";
            doAll(".gd-option",function(d){
                d.classList.add("no-display");
            });
        }

        if(typeof msg["dev"] != "undefined"){
            window.GravityDevice=msg.dev;
            if(msg.dev ==1){ //ispindel
                gdevice();
                doAll(".ispindel-info",function(d){
                    d.classList.remove("no-display");
                });

            }else if(msg.dev ==2){
                gdevice();
                doAll(".tilt-info",function(d){
                    d.classList.remove("no-display");
                });
            }else if(msg.dev ==3){ //pill
                gdevice();
                doAll(".pill-info",function(d){
                    d.classList.remove("no-display");
                });
            }else{
                Q(".gravity-device-pane").style.display="none";
                return;
            }
        }
        //if (typeof msg["name"] == "undefined") return;
        if (typeof msg["plato"] != "undefined") {
            window.plato = msg.plato;
            if (window.plato) showPlatoUnit();
        }
        if (typeof msg["fpt"] != "undefined") {
            window.npt = msg["fpt"];
        }

        // before iSpindel report to BPL, the name file is "unknown"
        if (typeof msg["name"] == "undefined") return
            //The first report will be "unknown" if (msg.name.startsWith("iSpindel")) {
            // iSpindel
        
        if (typeof msg["lu"] == "undefined") {
            console.log("iSpindel:" + JSON.stringify(msg));
            return;
        } 
        var ndiv = Q("#iSpindel-name");
        if (ndiv) ndiv.innerHTML = msg.name;

    
        if (typeof msg["lpf"] != "undefined")
            GravityFilter.setBeta(msg["lpf"]);

    }



    function respPtDiff(sg,duration){
        var preG =BChart.chart.getGravityOfTime((new Date().getTime())/1000 - duration);

        if(isNaN(preG)) return "--";

        var value=preG - sg;
        if(window.plato) return value.toFixed(1);

        value = value * 1000;
         return value.toFixed(1);
    }

    function gravityChangeUpdate(fsg){
        Q("#sgchanged").innerHTML = respPtDiff(fsg,48*3600) + "/" + respPtDiff(fsg,24*3600)+ "/" + respPtDiff(fsg,12*3600);
    }

    function updateGravity(sg) {
        //if(typeof window.sg != "undefined") return;
        window.sg = sg;
        Q("#gravity-sg").innerHTML = window.plato ? sg.toFixed(1) : sg.toFixed(3);
        if (typeof window.og != "undefined") {
            Q("#gravity-att").innerHTML = window.plato ? BrewMath.attP(window.og, sg) : BrewMath.att(window.og, sg);
            Q("#gravity-abv").innerHTML = window.plato ? BrewMath.abvP(window.og, sg) : BrewMath.abv(window.og, sg);
        }
    }

    function updateOriginGravity(og) {
        if (typeof window.og != "undefined" && window.og == og) return;
        window.og = og;
        Q("#gravity-og").innerHTML = window.plato ? og.toFixed(1) : og.toFixed(3);
        if (typeof window.sg != "undefined")
            updateGravity(window.sg);
    }

    function showgravitydlg(msg) {
        Q('#dlg_addgravity .og').style.display = "none";
        Q('#dlg_addgravity .sg').style.display = "none";
        Q('#dlg_addgravity .' + msg).style.display = "block";
        Q('#dlg_addgravity').style.display = "block";

        var beertemp = parseFloat(Q("#gravity-device-temp").innerHTML);
        if(isNaN(beertemp)){
            beertemp = parseFloat(Q("#lcdBeerTemp").innerHTML);
        }
        if(!isNaN(beertemp)){
            Q("#sginput-ispindel-temp").innerHTML =beertemp;
        }
        // show tilt
        if(window.isog) Q("#tilt-angle").value="--";
        else Q("#tilt-angle").value=(window.GravityDevice==2)? Q("#tilt-raw").textContent:Q("#gdevice-angle").textContent;
        // update temp.
        if (typeof window["tempUnit"] != "undefined") {
            window.celsius = false;
            var defaultTemp = 68;
            var re = /C$/;
            if (re.test(window.tempUnit)) {
                window.celsius = true;
                defaultTemp = 20;
            }

            var tus = document.querySelectorAll("#dlg_addgravity .temp-unit");
            for (var i = 0; i < tus.length; i++)
                tus[i].innerHTML = window.tempUnit;
        } else window.celsius = true;
    }

    function dismissgravity() {
        Q('#dlg_addgravity').style.display = "none";
    }

    function inputsg_change() {
        if(window.isog) return;
        var gravity = parseFloat(Q("#dlg_addgravity .sginput").value);
        if (isNaN(gravity)) return;
        // if iSpindel info is available, or beer temp is available.
        var currentBeerTemp=parseFloat(Q("#sginput-ispindel-temp").innerHTML);
        if(!isNaN(currentBeerTemp)){            
            var temp =  window.celsius ? 20 : 68;
            if (window.plato) {
                var sgc = BrewMath.pTempCorrection(window.celsius, gravity, temp, currentBeerTemp);
                Q("#sginput-sg-ispindel").innerHTML = sgc.toFixed(2);
            } else {
                var sgc = BrewMath.tempCorrection(window.celsius, gravity, temp, currentBeerTemp);
                Q("#sginput-sg-ispindel").innerHTML = sgc.toFixed(3);
            }
        }

    }

    function inputgravity() {
        var gravity =window.isog?  parseFloat(Q("#dlg_addgravity .sginput").value):parseFloat(Q("#sginput-sg-ispindel").innerHTML);
        
        if(isNaN(gravity)) gravity=parseFloat(Q("#dlg_addgravity .sginput").value);

        if (!window.plato && (gravity < 0.8 || gravity > 1.25) || isNaN(gravity)){
            alert("invalid input");
            return;
        } 

        dismissgravity();
        openDlgLoading();

        if (window.isog) updateOriginGravity(gravity);
        else {
            // user input
            updateGravity(gravity);
            gravityChangeUpdate(gravity);
        }
        var data = {
            name: "webjs",
            gravity: gravity
        };
        var raw=parseFloat(Q("#tilt-angle").value);
        if(!isNaN(raw)){
            data.raw = raw;
        }
        if (window.isog) data.og = 1;
        if (window.plato) data.plato = 1;
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
                alert("<%= failed %>:" + d);
                closeDlgLoading();
            }
        });

    }

    function inputSG() {
        window.isog = false;
        showgravitydlg("sg");
    }

    function inputOG() {
        window.isog = true;
        showgravitydlg("og");
    }

    function wifibar(did,x,ble){
        var strength =(typeof ble =="undefined")? [-1000, -90, -80, -70, -67]:[-1000,-100,-80,-55];
        var bar = 4;
        for (; bar >= 0; bar--) {
            if (strength[bar] < x) break;
        }
        var bars = Q(did).getElementsByClassName("rssi-bar");
        for (var i = 0; i < bars.length; i++) {
            bars[i].style.backgroundColor = (i < bar) ? window.rssiBarColor : "rgba(255,255,255,0.05)";
        }
        Q(did).title = (x > 0) ? "?" : ""+x;

    }

    function displayrssi(x) {
        Q("#rssi").title = (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
        wifibar("#rssi",x);
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

    function showPlatoUnit() {
        var units = document.querySelectorAll(".platounit");
        for (var i = 0; i < units.length; i++) {
            units[i].style.display = "inline-block";
        }
    }
    
    function gDeviceInfo(info){

        // last update
        if(info.u> 84879460){
            var lu = new Date(info.u * 1000);
            if (Q("#gravity-device-last")) Q("#gravity-device-last").innerHTML = lu.shortLocalizedString();
        }
        // gravity 
        updateGravity(window.plato? BrewMath.sg2pla(info.g/1000.0):info.g/1000.0);

        // temperature
        if(info.t > -20000){
            Q("#gravity-device-temp").innerHTML= info.t/100 + "&deg;" + window.tempUnit;
            window.gdtemp = info.t/100;
        }
        // rssi, 
        if(Q("#gravity-device-rssi")){
            if(window.GravityDevice ==1) wifibar("#gravity-device-rssi",info.r);
            else wifibar("#gravity-device-rssi",info.r,true);
        }
        // angle
        if(window.GravityDevice == 2){
            if (Q("#tilt-raw")) Q("#tilt-raw").innerHTML =""  + info.a.toFixed(3);
        }
        else if (Q("#gdevice-angle")) Q("#gdevice-angle").innerHTML = ""  + info.a.toFixed(2) +"&deg;";
        //battery
        if (Q("#gdevice-battery")) Q("#gdevice-battery").innerHTML = "" +((window.GravityDevice == 1)? 
                    (parseFloat(info.b).toFixed(2) +"V"):(""+parseInt(info.b) +"%"));

    }

    function BPLMsg(c) {
        BWF.gotMsg = true;

        if (typeof c["rssi"] != "undefined") {
            displayrssi(c["rssi"]);
        }
        if (typeof c["sl"] != "undefined") {
            displayLcd(c);
        }
        if (typeof c["reload"] != "undefined") {
            console.log("Forced reload chart");
            BChart.reqnow();
            if (!Q("#recording").innerHTML || Q("#recording").innerHTML != c.log)
                window.npt = 0; // delete formula to force update to BPL.                
        }
        if (typeof c["nn"] != "undefined") {
            Q("#hostname").innerHTML = c["nn"];
            document.title = c.nn; // + document.title.replace("BrewPiLess", "");
        }
        if (typeof c["ver"] != "undefined") {
            if (JSVERSION != c["ver"]) alert("<%= script_control_version_mismatched %>");
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
        if (typeof c["plato"] != "undefined") {
            window.plato = c["plato"];
            if (window.plato) showPlatoUnit();
        }

        if (typeof c["pm"] != "undefined" && typeof c["psi"] != "undefined") {
            if (c.pm != 0) {
                Q("#pressure-info-pane").style.display = "block";
                Q("#pressure-psi").innerHTML = c.psi;
            }
        }
        if(typeof c["G"] != "undefined") gDeviceInfo(c.G);

        ptcshow(c);
        if(typeof c["h"] != "undefined") {
            Q("#humidity-info").classList.remove("no-display");
            Q("#humidity").innerHTML= (c.h <=100)?  (c.h + "%"):"--";
        }
        if(typeof c["hr"] != "undefined") {
            Q("#room-humidity-info").classList.remove("no-display");
            Q("#room-humidity").innerHTML= (c.hr <=100)?  (c.hr + "%"):"--";
        }

    }

    function connBWF() {
        BWF.init({
            //            reconnect: false,
            onconnect: function() {
                BWF.send("c");
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
                    //BWF.send("l");
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
                closeDlgLoading();
            },
            handlers: {
                /*                L: function(lines) {
                                    BWF.gotMsg = true;
                                    processLcdText(lines);
                            },*/
                A: BPLMsg,
                G: function(c) {
                    GDSetting(c);
                },
                // for control page. 
                C: function(c) { if (typeof ccparameter != "undefined") ccparameter(c); },
                B: function(c) { if (typeof rcvBeerProfile != "undefined") rcvBeerProfile(c); }
            }
        });
    }

    function init_classic() {
        window.plato = false;
        BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML);
        initRssi();
        Capper.init();
        BWF.gotMsg = true;
        initctrl_C();
        connBWF();
        setTimeout(function() { BChart.start(); }, T_LOAD_CHART);
    }

    function init() {
        Q("#pressure-info-pane").style.display = "none";
        Q(".gravity-device-pane").style.display = "none";
        window.plato = false;
        BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML,"div_p",Q('#psilabel').innerHTML,Q('#vollabel').innerHTML,"div_h",Q("#rhlabel").innerHTML);
        initRssi();
        Capper.init();
        BWF.gotMsg = true;
        connBWF();
        setTimeout(function() { BChart.start(); }, T_LOAD_CHART);
        getActiveNavItem();
    }
