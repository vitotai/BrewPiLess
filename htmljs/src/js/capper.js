function TabPane(modes) {
    var t = this;
    t.cmode = null;

    function dselect(m) {
        var d = document.getElementById(m + "-m");
        //        var nc = document.getElementById(m + "-m").className.replace(/\snav-selected/, '');
        var nc = d.className.replace(/\snav-selected/, '');
        d.className = nc;

        document.getElementById(m + "-s").style.display = "none";
    }

    function select(m) {
        var d = document.getElementById(m + "-m");
        if (d.className.indexOf("nav-selected") < 0)
            d.className += ' nav-selected';

        document.getElementById(m + "-s").style.display = "block";
        t.cmode = m;
    }

    for (var i = 0; i < modes.length; i++) {
        var m = modes[i];
        document.getElementById(m + "-s").style.display = "none";
        document.getElementById(m + "-m").onclick = function() {
            var tm = this.id.replace(/-m$/, '');
            //deselect current selected 
            dselect(t.cmode);
            // select current
            select(tm);
            return false;
        };
    }
    // select the first one
    select(modes[0]);
    t.select = select;
}

var Capper = {
    target_psi: 0,
    psi_valid: false,
    hidepset: function(hide) {
        this.psi_valid = !hide;
        var psets = document.querySelectorAll(".psi-set-group");
        for (var i = 0; i < psets.length; i++) {
            psets[i].style.display = hide ? "none" : "block";
        }
    },
    setpsi: function(psi) {
        this.target_psi = psi;
        var psets = document.querySelectorAll(".cappressure");
        for (var i = 0; i < psets.length; i++) {
            psets[i].value = psi;
        }
    },
    init: function() {
        var t = this;
        // three conditions:
        //  classic, info-pane within capper-frame
        //  Tom's : info-pane only in index.htm
        //  Tom's UI: capper-frame(control) in control.htm

        var cp = Q(".capping-info-pane");
        if (cp) {
            // classic or Tom's index.htm, do no harm in classic
            cp.style.display = "none";
        }

        var cf = Q("#capper-frame");
        if (cf) {
            // classic or Tom's control.htm
            cf.style.display = "none";
            t.initCtrl();
        }
    },
    initCtrl: function() {
        var t = this;
        t.tabs = new TabPane(["tab-gravity", "tab-time", "tab-manual"]);
        var date_in = Q("#captimeinput");
        t.time = new Date();
        date_in.onchange = function() {
            var nd = new Date(date_in.value);
            if (isNaN(nd.getTime())) {
                // console.log("invalid date");
                t.setInputTime(t.time);
            } else {
                t.setInputTime(nd);
            }
        };
        Q("#cap-apply").onclick = function() {
            // get psi when needed
            var psiarg = t.psi_valid ? "psi=" + t.target_psi + "&" : "";

            var mode = t.tabs.cmode;
            if (mode == "tab-gravity") {
                var sg = Q("#capgravityinput").value;
                /*if (isNaN(sg) || sg > 2 || sg < 0.8) alert("<%= capper_invalid_gravity %>");
                else*/
                t.send(psiarg + "sg=" + sg);
            } else if (mode == "tab-time") {
                var time = new Date(Q("#captimeinput").value);
                if (isNaN(time.getTime())) {
                    alert("<%= capper_invalid_time %>");
                    return;
                } else t.send(psiarg + "at=" + (time.getTime() / 1000));
            } else {
                if (Q("#capswitch").checked) t.send(psiarg + "cap=1");
                else t.send(psiarg + "cap=0");
            }
        };
        t.hidepset(true);
        var psets = document.querySelectorAll(".cappressure");
        for (var i = 0; i < psets.length; i++) {
            psets[i].onchange = function() {
                t.setpsi(this.value);
            };
        }
    },
    send: function(arg) {
        console.log("send " + arg);
        s_ajax({
            url: "cap?" + arg,
            m: "GET",
            success: function(b) {
                alert("<%= done %>!");
            },
            fail: function(a) {
                alert("<%= capper_failed_set_capper %>");
            }
        })

    },
    setcap: function(capped) {
        if (!Q("#capstate-open")) return;
        if (capped) {
            Q("#capstate-open").style.display = "none";
            Q("#capstate-close").style.display = "inline-block";
        } else {
            Q("#capstate-open").style.display = "inline-block";
            Q("#capstate-close").style.display = "none";
        }
    },
    setInputTime: function(d) {
        this.time = d;
        var date_in = Q("#captimeinput");
        date_in.value = (date_in.type == "datetime-local") ? formatDateForPicker(d) : formatDate(d);
    },
    status: function(capst) {
        // first set cap
        //0: none, 1: open, 2: close, 3:time, 4: gravity
        //  might need to hide the DOM, but a reload will solve this. just save some code
        if (typeof capst["m"] == "undefined" || capst.m == 0) return;

        this.statusInfo(capst);
        this.updateCtrl(capst);
    },
    statusInfo: function(capst) {
        // cap status
        var cp = Q(".capping-info-pane");
        if (cp) {
            cp.style.display = "block";

            this.setcap(capst["c"]);
            // info: cap condition
            var IDs = ["", "cs-manopen", "cs-mancap", "cs-timecon", "cs-sgcon"];
            for (var i = 1; i < IDs.length; i++) {
                if (i == capst.m) Q("#" + IDs[i]).style.display = "inline-block";
                else Q("#" + IDs[i]).style.display = "none";
            }

            if (typeof capst["g"] != "undefined")
                Q("#capgravityset").innerHTML = capst["g"];

            if (typeof capst["t"] != "undefined")
                Q("#captimeset").innerHTML = formatDate(new Date(capst["t"] * 1000));
        }
    },
    updateCtrl: function(capst) {
        // cap control
        var cf = Q("#capper-frame");
        if (cf) {
            cf.style.display = "block";

            if (typeof capst["g"] != "undefined")
                Q("#capgravityinput").value = capst["g"];

            if (typeof capst["t"] != "undefined")
                this.setInputTime(new Date(capst["t"] * 1000));
            else
                this.setInputTime(new Date());

            // check mode
            if (capst.m == 1) Q("#capswitch").checked = false;
            else if (capst.m == 2) Q("#capswitch").checked = true;
            // pressure control mode
            if (capst.pm == 2) {
                this.hidepset(false);
                this.setpsi(capst.psi);
            }
        }
    }
};