var BPURL = "/tschedule";
var MAX_STEP = 10;

function formatDate(dt) {
    //	var y = dt.getFullYear();
    //	var M = dt.getMonth() +1;
    //	var d = dt.getDate();
    var h = dt.getHours();
    var m = dt.getMinutes();
    //    var s = dt.getSeconds();
    function dd(n) {
        return (n < 10) ? '0' + n : n;
    }
    //	return dd(M) + "/" + dd(d) + "/" + y +" "+ dd(h) +":"+dd(m)+":"+dd(s);
    //	return dd(M) + "/" + dd(d) +" "+ dd(h) +":"+dd(m);
    return dt.toLocaleDateString() + " " + dd(h) + ":" + dd(m);
}

function formatDateForPicker(date) {
    var h = date.getHours();
    var m = date.getMinutes();

    function dd(n) { return (n < 10) ? '0' + n : n; }
    return date.getFullYear() + "-" + dd(date.getMonth() + 1) + "-" + dd(date.getDate()) + "T" + dd(h) + ":" + dd(m);
}
/* profile.js */
var profileEditor = {
    dirty: false,
    TU: 'C',
    C_startday_Id: "#startdate",
    C_savebtn_Id: "savebtn",
    markdirty: function(d) {
        this.dirty = d;
        document.getElementById(this.C_savebtn_Id).innerHTML = (d) ? "Save*" : "Save";
    },
    getStartDate: function() {
        return this.sd;
    },
    setStartDate: function(d) {
        this.sd = d;
        var date_in = Q(this.C_startday_Id);
        date_in.value = (date_in.type == "datetime-local") ? formatDateForPicker(d) : formatDate(d);
    },
    startDayChange: function() {
        var nd = new Date(Q(this.C_startday_Id).value);
        if (isNaN(nd.getTime())) {
            // console.log("invalid date");
            this.setStartDate(this.sd);
        } else {
            // console.log(nd);
            this.sd = nd;
            this.reorg();
            this.markdirty(true);
        }
    },
    startnow: function() {
        var d = new Date();
        this.setStartDate(d);
        this.reorg();
        this.markdirty(true);
        ControlChart.update(this.chartdata());
    },
    rowList: function() {
        var tb = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
        return tb.getElementsByTagName("tr");
    },
    sgChange: function(td) {
        if (!isNaN(td.innerHTML) || td.innerHTML.match(/^[\d]+%$/) || td.innerHTML == "") {
            td.saved = td.innerHTML;
            this.markdirty(true);
        } else {
            td.innerHTML = td.saved;
        }
    },
    dayChange: function(td) {
        if (td.innerHTML == "" || isNaN(td.innerHTML))
            td.innerHTML = td.saved;
        else {
            this.markdirty(true);
            this.reorg();
            ControlChart.update(this.chartdata());
        }
    },
    tempChange: function(td) {
        if (td.innerHTML == "" || isNaN(td.innerHTML))
            td.innerHTML = td.saved;
        else {
            this.markdirty(true);
            ControlChart.update(this.chartdata());
        }
    },
    stableChange: function(td) {
        if (td.innerHTML.match(/^\s*(\d+)@(\d+)\s*$/)) {
            td.saved = td.innerHTML;
            this.markdirty(true);
        } else if (!isNaN(td.innerHTML)) {
            td.saved = parseInt(td.innerHTML);
            this.markdirty(true);
        } else {
            td.innerHTML = td.saved;
        }
    },
    initrow: function(tr, stage) {
        var b = this;
        // temp setting
        var type = stage.c;
        tr.type = type;
        var tdTemp = tr.getElementsByClassName("stage-temp")[0];

        if (type == "r") {
            tdTemp.innerHTML = "";
        } else {
            tdTemp.innerHTML = stage.t;
            tdTemp.contentEditable = true;
            tdTemp.onblur = function() {
                b.tempChange(this);
            };
            tdTemp.onfocus = function() {
                this.saved = this.innerHTML;
            };
        }
        // day setting
        var tdDay = tr.getElementsByClassName("stage-time")[0];
        tdDay.innerHTML = stage.d;
        tdDay.contentEditable = true;
        tdDay.onblur = function() {
            b.dayChange(this);
        };
        tdDay.onfocus = function() {
            this.saved = this.innerHTML;
        };

        // stable setting
        var tdStable = tr.getElementsByClassName("stage-stabletime")[0];
        // sg. only valid for hold
        var tdSG = tr.getElementsByClassName("stage-sg")[0];

        if (type == "r") {
            tdSG.innerHTML = "";
            tdStable.innerHTML = "";
        } else {
            tdSG.saved = stage.g;
            tdSG.innerHTML = (typeof stage.g == "undefined") ? "" : stage.g;
            tdSG.contentEditable = true;
            tdSG.onblur = function() {
                b.sgChange(this);
            };
            tdSG.onfocus = function() {
                this.saved = this.innerHTML;
            };
            if (typeof stage.s == "undefined") tdStable.innerHTML = "";
            else tdStable.innerHTML = (typeof stage.x == "undefined") ? stage.s : stage.x + "@" + stage.s;
            tdStable.contentEditable = true;
            tdStable.onblur = function() {
                b.stableChange(this);
            };
            tdStable.onfocus = function() {
                this.saved = this.innerHTML;
            };
        }


        var forTime = tr.getElementsByClassName("for-time")[0];
        // condition, only valid for hold
        var conSel = tr.getElementsByClassName("condition")[0];
        /*
           <option value="t" 0>Time</option>
           <option value="g" 1>SG</option>
           <option value="s" 2>Stable</option>
           <option value="a" 3>Time & SG</option>
           <option value="o" 4>Time OR SG</option>
           <option value="u" 5>Time OR Stable</option>
           <option value="v" 6>Time & Stable</option>
            <option value="b" 7>SG OR Stable</option>
            <option value="x" 8>SG & Stable</option>
            <option value="w" 9>ALL</option>
            <option value="e" 10>Either</option>
        */
        var condtionIndex = {
            t: 0,
            g: 1,
            a: 3,
            s: 2,
            o: 4,
            u: 5,
            v: 6,
            b: 7,
            x: 8,
            w: 9,
            e: 10
        };
        if (type == "r") {
            forTime.style.display = "block";
            conSel.style.display = "none";

        } else {
            conSel.value = stage.c;
            conSel.selectedIndex = condtionIndex[stage.c];

            forTime.style.display = "none";
            conSel.style.display = "block";
        }
    },

    datestr: function(diff) {
        var dt = new Date(this.sd.getTime() + Math.round(diff * 86400) * 1000);
        return formatDate(dt);
    },
    reorg: function() {
        var rowlist = this.rowList();
        var utime = this.sd.getTime();
        for (var i = 0; i < rowlist.length; i++) {
            var row = rowlist[i];
            row.className = (i % 2) ? "odd" : "even";
            row.getElementsByClassName("diaplay-time")[0].innerHTML = formatDate(new Date(utime));
            var time = this.rowTime(row);
            utime += Math.round(time * 86400) * 1000;
        }
    },
    chartdata: function() {
        var rowlist = this.rowList();
        if (rowlist.length == 0 || typeof this.sd =="undefined")  return [];

        var utime = this.sd.getTime();
        var row = rowlist[0];
        var start = this.rowTemp(row);

        var list = [];
        list.push([new Date(utime), start]);

        for (var i = 0; i < rowlist.length; i++) {
            var row = rowlist[i];
            var temp;
            if (row.type == "r") {
                temp = this.rowTemp(rowlist[i + 1]);
            } else {
                temp = this.rowTemp(row);
            }
            utime += Math.round(this.rowTime(row) * 86400) * 1000;
            list.push([new Date(utime), temp]);
        }
        return list;
    },
    addRow: function() {
        var tb = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
        var rowlist = tb.getElementsByTagName("tr");

        if (rowlist.length >= MAX_STEP) {
            alert("<%= script_control_too_many_steps %>");
            return;
        }
        var stage;

        if (rowlist.length == 0) {
            var init = (this.TU == 'C') ? 20 : 68;
            stage = {
                c: 't',
                t: init,
                d: 1,
                g: 1.01
            };
        } else {
            var lastRow = rowlist[rowlist.length - 1];

            var tr = this.row.cloneNode(true);
            this.initrow(tr, {
                c: "r",
                d: 1
            });
            tb.appendChild(tr);
            stage = {
                c: 't',
                t: this.rowTemp(lastRow),
                d: 1,
                g: ""
            };
        }

        var tr = this.row.cloneNode(true);
        this.initrow(tr, stage);
        tb.appendChild(tr);

        this.reorg();
        this.markdirty(true);
        ControlChart.update(this.chartdata());
    },
    delRow: function() {
        // delete last row
        var list = this.rowList();
        if (list.length == 0) return;
        var last = list[list.length - 1];

        if (list.length > 1) {
            var lr = list[list.length - 2];
            lr.parentNode.removeChild(lr);
        }

        last.parentNode.removeChild(last);

        this.markdirty(true);
        ControlChart.update(this.chartdata());
    },
    rowTemp: function(row) {
        return parseFloat(row.getElementsByClassName("stage-temp")[0].innerHTML);
    },
    rowCondition: function(row) {
        return row.getElementsByClassName("condition")[0].value;
    },
    rowTime: function(row) {
        return parseFloat(row.getElementsByClassName("stage-time")[0].innerHTML);
    },
    rowSg: function(row) {
        return row.getElementsByClassName("stage-sg")[0].saved;
    },
    rowSt: function(row) {
        var data = row.getElementsByClassName("stage-stabletime")[0].innerHTML;
        if (typeof data != "string") return data;
        var matches = data.match(/^\s*(\d+)@(\d+)\s*$/);
        if (matches) {
            return parseInt(matches[2]);
        } else {
            return parseInt(data);
        }
    },
    rowStsg: function(row) {
        var data = row.getElementsByClassName("stage-stabletime")[0].innerHTML;
        if (typeof data != "string") return false;
        var matches = data.match(/^\s*(\d+)@(\d+)\s*$/);
        if (matches) {
            return parseInt(matches[1]);
        } else {
            return false;
        }
    },
    renderRows: function(g) {
        if (typeof g.length == "undefined")
            console.log("error!");
        var e = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
        for (var f = 0; f < g.length; f++) {
            var c = this.row.cloneNode(true);
            this.initrow(c, g[f]);
            e.appendChild(c)
        }
        this.reorg()
    },

    initable: function(c) {
        if (!this.row) {
            var b = document.getElementById("profile_t").getElementsByTagName("tbody")[0];
            this.row = b.getElementsByTagName("tr")[0];
            b.removeChild(this.row);
        } else {
            this.clear();
        }
        this.renderRows(c)
    },
    clear: function() {
        var rl = this.rowList();

        var count = rl.length;
        for (var i = rl.length - 1; i >= 0; i--) {
            var tr = rl[i];
            tr.parentNode.removeChild(tr);
        }
        this.markdirty(true);
    },
    getProfile: function() {
        var rl = this.rowList();
        var lastdate = 0;
        var temps = [];
        for (var i = 0; i < rl.length; i++) {
            var tr = rl[i];
            var day = this.rowTime(tr);
            if (isNaN(day)) return false;

            if (tr.type == "r") {
                temps.push({
                    c: "r",
                    d: day
                });
            } else {
                var temp = this.rowTemp(tr);
                if (isNaN(temp)) return false;
                if (temp > BrewPiSetting.maxDegree || temp < BrewPiSetting.minDegree) return false;

                /*
                   <option value="t">Time</option>
                   <option value="g">SG</option>
                   <option value="s">Stable</option>
                   <option value="a">Time & SG</option>
                   <option value="o">Time OR SG</option>
                   <option value="u">Time OR Stable</option>
                   <option value="v">Time & Stable</option>
                    <option value="b">SG OR Stable</option>
                    <option value="x">SG & Stable</option>
                    <option value="w">ALL</option>
                    <option value="e">Either</option>
                */
                var condition = this.rowCondition(tr);
                var stage = {
                    c: condition,
                    d: day,
                    t: temp
                };

                var useSg = "gaobxwe";
                var gv = this.rowSg(tr);

                if (useSg.indexOf(condition) >= 0) {
                    if (gv == "") return false;
                    stage.g = gv;
                }
                var useStableTime = "suvbxwe";
                var stv = this.rowSt(tr);
                if (useStableTime.indexOf(condition) >= 0) {
                    if (isNaN(stv)) return false;
                    stage.s = stv;
                    var x = this.rowStsg(tr);
                    if (x) stage.x = x;
                }

                temps.push(stage);

            }
        }
        var s = this.sd.toISOString();
        var ret = {
            s: s,
            v: 2,
            u: this.TU,
            t: temps
        };
        //console.log(ret);
        return ret;
    },
    convertUnit:function(steps,unit){
        if(unit == this.TU) return steps;

        for(var i=0;i< steps.length;i++)
            steps[i].t = (unit == 'F')? F2C(steps[i].t):C2F(steps[i].t);
        
        return steps;
    },
    loadProfile: function(a) {
        this.sd = new Date(a.s);
        this.clear();
        this.renderRows(this.convertUnit(a.t,a.u));
        ControlChart.update(this.chartdata());
    },
    initProfile: function(p) {
        if (typeof p != "undefined") {
            // start date
            var sd = new Date(p.s);
            this.setStartDate(sd);
            this.initable(this.convertUnit(p.t,p.u));
        } else {
            this.setStartDate(new Date());
            this.initable([]);
        }
    },
    setTempUnit: function(u) {
        if (u == this.TU) return;
        this.TU = u;
        var rl = this.rowList();
        for (var i = 0; i < rl.length; i++) {
            var tcell = rl[i].querySelector('td.stage-temp');
            var temp = parseFloat(tcell.innerHTML);
            if (!isNaN(temp)) tcell.innerHTML = (u == 'C') ? F2C(temp) : C2F(temp);
        }
        ControlChart.updateTU(u);
        ControlChart.update(this.chartdata());
    }
};

/* end of profile.js */
/* PL: profle list */
var PL = {
    pl_path: "P",
    url_list: "/list",
    url_save: "/fputs",
    url_del: "/rm",
    url_load: "pl.php?ld=",
    div: "#profile-list-pane",
    shown: false,
    initialized: false,
    plist: [],
    path: function(a) {
        return "/" + this.pl_path + "/" + a
    },
    depath: function(a) {
        return a.substring(this.pl_path.length + 1)
    },
    rm: function(e) {
        var f = this;
        var c = "path=" + f.path(f.plist[e]);
        s_ajax({
            url: f.url_del,
            m: "DELETE",
            data: c,
            success: function(a) {
                f.plist.splice(e, 1);
                f.list(f.plist)
            },
            fail: function(a) {
                alert("<%= failed %>:" + a);
            }
        })
    },
    load: function(e) {
        var f = this;
        var c = f.path(f.plist[e]);
        s_ajax({
            url: c,
            m: "GET",
            success: function(b) {
                var a = JSON.parse(b);
                profileEditor.loadProfile(a);
            },
            fail: function(a) {
                //alert("failed:" + a);
            }
        })
    },
    list: function(i) {
        var a = this;
        var h = Q(a.div).querySelector(".profile-list");
        var lis = h.querySelectorAll("li");
        for (var i = 0; i < lis.length; i++) {
            h.removeChild(lis[i]);
        }
        var b = a.row;
        a.plist.forEach(function(f, g) {
            var c = b.cloneNode(true);
            c.querySelector(".profile-name").innerHTML = f;
            c.querySelector(".profile-name").onclick = function(j) {
                j.preventDefault();
                a.load(g);
                return false
            };
            c.querySelector(".rmbutton").onclick = function() {
                a.rm(g)
            };
            h.appendChild(c)
        })
    },
    append: function(b) {
        if (!this.initialized) {
            return
        }
        this.plist.push(b);
        this.list(this.plist)
    },
    init: function() {
        var a = this;
        a.initialized = true;
        a.row = Q(a.div).querySelector("li");
        a.row.parentNode.removeChild(a.row);
        s_ajax({
            url: a.url_list,
            m: "POST",
            data: "dir=" + a.path(""),
            success: function(c) {
                a.plist = [];
                var b = JSON.parse(c);
                b.forEach(function(e) {
                    if (e.type == "file") {
                        a.plist.push(a.depath(e.name))
                    }
                });
                a.list(a.plist)
            },
            fail: function(b) {
                alert("<%= failed %>:" + b);
            }
        })
    },
    toggle: function() {
        if (!this.initialized) {
            this.init()
        }
        this.shown = !this.shown;
        if (this.shown) {
            Q(this.div).style.display = "block";
        } else {
            Q(this.div).style.display = "none";
        }
    },
    saveas: function() {
        Q("#dlg_saveas").style.display = "block"
    },
    cancelSave: function() {
        Q("#dlg_saveas").style.display = "none"
    },
    doSave: function() {
        var e = Q("#dlg_saveas input").value;
        if (e == "") {
            return
        }
        if (e.match(/[\W]/g)) {
            return
        }
        var g = profileEditor.getProfile();
        if (g === false) {
            alert("<%= script_control_invalid_value_check_again %>");
            return
        }
        var f = this;
        var c = "path=" + f.path(e) + "&content=" + encodeURIComponent(JSON.stringify(g));
        var f = this;
        s_ajax({
            url: f.url_save,
            m: "POST",
            data: c,
            success: function(a) {
                f.append(e);
                f.cancelSave()
            },
            fail: function(a) {
                alert("<%= failed %>:" + a);
            }
        })
    }
};
/* end of PL*/
var BrewPiSetting = {
    valid: false,
    maxDegree: 30,
    minDegree: 0,
    tempUnit: 'C'
};


var ControlChart = {
    unit: "C",
    init: function(div, data, unit) {
        var t = this;
        t.data = data;
        t.unit = unit;

        var dateFormatter = function(v) {
            d = new Date(v);
            return d.shortLocalizedString();
        };
        var shortDateFormatter = function(v) {
            d = new Date(v);
            var y = d.getYear() + 1900;
            var re = new RegExp('[^\d]?' + y + '[^\d]?');
            var n = d.toLocaleDateString();
            return n.replace(re, "");
        };

        var temperatureFormatter = function(v) {
            return v.toFixed(1) + "&deg;" + t.unit;
        };

        t.chart = new Dygraph(
            document.getElementById(div), t.data, {
                colors: ['rgb(89, 184, 255)'],
                axisLabelFontSize: 12,
                gridLineColor: '#ccc',
                gridLineWidth: '0.1px',
                labels: ["<%= script_control_time %>", "<%= script_control_temperature %>"],
                labelsDiv: document.getElementById(div + "-label"),
                legend: 'always',
                labelsDivStyles: {
                    'textAlign': 'right'
                },
                strokeWidth: 1,
                //        xValueParser: function(x) { return profileTable.parseDate(x); },
                //        underlayCallback: updateCurrentDateLine,
                //        "Temperature" : {},
                axes: {
                    y: {
                        valueFormatter: temperatureFormatter,
                        pixelsPerLabel: 20,
                        axisLabelWidth: 35
                    },
                    //            x : { axisLabelFormatter:dateFormatter, valueFormatter: dateFormatter, pixelsPerLabel: 30, axisLabelWidth:40 }
                    x: {
                        axisLabelFormatter: shortDateFormatter,
                        valueFormatter: dateFormatter,
                        pixelsPerLabel: 30,
                        axisLabelWidth: 40
                    }

                },
                highlightCircleSize: 2,
                highlightSeriesOpts: {
                    strokeWidth: 1.5,
                    strokeBorderWidth: 1,
                    highlightCircleSize: 5
                },

            }
        );
    },
    update: function(data) {
        if (data.length == 0) return;
        this.data = data;
        this.chart.updateOptions({
            'file': this.data
        });
    },
    updateTU: function(unit) {
        this.unit = unit;
    }
};


var modekeeper = {
    initiated: false,
    modes: ["profile", "beer", "fridge", "off"],
    cmode: 0,
    dselect: function(m) {
        var d = document.getElementById(m + "-m");
        var nc = document.getElementById(m + "-m").className.replace(/\snav-selected/, '');
        d.className = nc;

        document.getElementById(m + "-s").style.display = "none";
    },
    select: function(m) {
        document.getElementById(m + "-m").className += ' nav-selected';
        document.getElementById(m + "-s").style.display = "block";
    },
    init: function() {
        var me = this;
        if (me.initiated) return;
        me.initiated = true;
        for (var i = 0; i < 4; i++) {
            var m = me.modes[i];
            document.getElementById(m + "-s").style.display = "none";
            document.getElementById(m + "-m").onclick = function() {
                var tm = this.id.replace(/-m/, '');
                me.dselect(me.cmode);
                me.select(tm);
                me.cmode = tm;
                return false;
            };
        }
        me.cmode = "profile";
        me.select(me.cmode);
    },
    apply: function() {
        if (!BrewPiSetting.valid) {
            alert("<%= script_control_not_conected_to_controller %>");
            //		return;
        }
        if ((this.cmode == "beer") || (this.cmode == "fridge")) {
            var v = document.getElementById(this.cmode + "-t").value;
            if (v == '' || isNaN(v) || (v > BrewPiSetting.maxDegree || v < BrewPiSetting.minDegree)) {
                alert("<%= script_control_invalid_temperature %>" + v);
                return;
            }
            if (this.cmode == "beer") {
                //console.log("j{mode:b, beerSet:" + v+ "}");
                BWF.send("j{mode:b, beerSet:" + v + "}");
            } else {
                console.log("j{mode:f, fridgeSet:" + v + "}");
                BWF.send("j{mode:f, fridgeSet:" + v + "}");
            }
        } else if (this.cmode == "off") {
            //console.log("j{mode:o}");
            BWF.send("j{mode:o}");
        } else {
            // should save first.
            if (profileEditor.dirty) {
                alert("<%= script_control_save_profile_before_applay %>");
                return;
            }
            //console.log("j{mode:p}");
            document.getElementById('dlg_beerprofilereminder').style.display = "block";
            document.getElementById('dlg_beerprofilereminder').querySelector("button.ok").onclick = function() {
                document.getElementById('dlg_beerprofilereminder').style.display = "none";
                var gravity = parseFloat(Q("#dlg_beerprofilereminder input").value);
                if (typeof updateOriginGravity == "function") updateOriginGravity(gravity);
                var data = {
                    name: "webjs",
                    og: 1,
                    gravity: gravity
                };
                s_ajax({
                    url: "gravity",
                    m: "POST",
                    mime: "application/json",
                    data: JSON.stringify(data),
                    success: function(d) {
                        BWF.send("j{mode:p}");
                    },
                    fail: function(d) {
                        alert("<%= failed %>:" + d);
                    }
                });
            };
            document.getElementById('dlg_beerprofilereminder').querySelector("button.oknog").onclick = function() {
                document.getElementById('dlg_beerprofilereminder').style.display = "none";
                BWF.send("j{mode:p}");
            };
            document.getElementById('dlg_beerprofilereminder').querySelector("button.cancel").onclick = function() {
                document.getElementById('dlg_beerprofilereminder').style.display = "none";
            };
        }
    }
};

function saveprofile() {
    //console.log("save");
    var r = profileEditor.getProfile();
    if (r === false) {
        alert("<%= script_control_invalid_value_check_again %>");
        return;
    }
    var json = JSON.stringify(r);
    console.log("result=" + json);

    s_ajax({
        url: BPURL,
        m: "POST",
        mime: "application/x-www-form-urlencoded",
        data: "data=" + encodeURIComponent(json),
        success: function(d) {
            profileEditor.markdirty(false);
            alert("<%= done %>")
        },
        fail: function(d) {
            alert("<%= script_control_failed_to_save %>");
        }
    });
}

function updateTempUnit(u) {
    var Us = document.getElementsByClassName("t_unit");
    for (var i = 0; i < Us.length; i++) {
        Us[i].innerHTML = u;
    }
}

function ccparameter(s) {
    var setting = {
        valid: true,
        minDegree: s.tempSetMin,
        maxDegree: s.tempSetMax,
        tempUnit: s.tempFormat
    };    
    if (setting.tempUnit != BrewPiSetting.tempUnit) {
        updateTempUnit(setting.tempUnit);
        profileEditor.setTempUnit(setting.tempUnit);
    }
    BrewPiSetting = setting;
}

function rcvBeerProfile(p) {
    closeDlgLoading();
    updateTempUnit(p.u); // using profile temp before we get from controller
    BrewPiSetting.tempUnit = p.u;
    profileEditor.initProfile(p);
    ControlChart.init("tc_chart", profileEditor.chartdata(), p.u);
}

function HC_init(){
    Q("#humidity-control").style.display="none";
}
function HC_show(config){
    Q("#humidity-control").style.display="";
    Q("#hc-mode").value = config.m;
    Q("#hc-target").value = config.t;
}

function HC_apply(){
    var target = Q("#hc-target").value;
    var mode =  Q("#hc-mode").value;

    s_ajax({
        url:"/rh",
        m: "POST",
        data: "m=" + mode +"&t=" + target,
        success: function(a) {
            alert("<%= done %>")
        },
        fail: function(a) {
            alert("<%= failed %>:" + a);
        }
    })

}
/* deprecated
function initctrl_C(next) {
//    modekeeper.init();
    Capper.init();
    modekeeper.init();
    openDlgLoading();
}
*/

function communicationError() {
    var div = Q('.error');
    if (div) {
        div.innerHTML = "Failed to connect to server.";
        div.style.display = "block";
    }
}

function initctrl() {
    getActiveNavItem();
    Capper.init();
    modekeeper.init();
    PTC.init(Q("#ptc-control"));
    openDlgLoading();

    BWF.init({
        onconnect: function() {
            BWF.send("c");
        },
        error: function(e) {
            //console.log("error");
            closeDlgLoading();
            communicationError();
        },
        handlers: {
            A: function(c) {
                if (typeof c["nn"] != "undefined") {
                    Q("#hostname").innerHTML = c["nn"];
                }
                if (typeof c["ver"] != "undefined") {
                    if (JSVERSION != c["ver"]) alert("<%= script_control_version_mismatched %>");
                    Q("#verinfo").innerHTML = "v" + c["ver"];
                }
                if (typeof c["cap"] != "undefined")
                    Capper.status(c["cap"]);
                if (typeof c["ptcs"] != "undefined")
                    PTC.config(c.ptcs);
                if (typeof c["rh"] != "undefined")
                    HC_show(c.rh);
                
            },
            C: function(c) { ccparameter(c); },
            B: rcvBeerProfile
        }
    });
}