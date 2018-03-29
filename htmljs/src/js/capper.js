function TabPane(modes) {
    var t = this;
    t.cmode = null;

    function dselect(m) {
        var d = document.getElementById(m + "-m");
        var nc = document.getElementById(m + "-m").className.replace(/\snav-selected/, '');
        d.className = nc;

        document.getElementById(m + "-s").style.display = "none";
    }

    function select(m) {
        document.getElementById(m + "-m").className += ' nav-selected';
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
        };
    }
    // select the first one
    select(modes[0]);
    t.select = select;
}

var Capper = {
    init: function() {
        var t = this;
        t.tabs = new TabPane(["tab-gravity", "tab-time", "tab-manual"]);
        Q("#capper-frame").style.display = "none";
        var date_in = Q("#captimeinput");
        t.time = new Date();
        date_in.onchange = function() {
            var nd = new Date(date_in.value);
            if (isNaN(nd.getTime())) {
                // console.log("invalid date");
                t.setTime(t.time);
            } else {
                t.setTime(nd);
            }
        };
        Q("#cap-apply").onclick = function() {
            var mode = t.tabs.cmode;
            if (mode == "tab-gravity") {
                var sg = Q("#capgravityinput").value;
                if (isNaN(sg) || sg > 2 || sg < 0.8) alert("Invalid Gravity");
                else t.send("sg=" + sg);
            } else if (mode == "tab-time") {
                var time = new Date(Q("#captimeinput").value);
                if (isNaN(time.getTime())) {
                    alert("Invalid Time");
                    return;
                } else t.send("at=" + (time.getTime() / 1000));
            } else {
                if (Q("#capswitch").checked) t.send("cap=0");
                else t.send("cap=1");
            }
        };
    },
    send: function(arg) {
        console.log("send " + arg);
        s_ajax({
            url: "cap?" + arg,
            m: "GET",
            success: function(b) {
                alert("done!");
            },
            fail: function(a) {
                alert("failed to set capper");
            }
        })

    },
    setcap: function(capped) {
        if (capped) {
            Q("#capstate-open").style.display = "none";
            Q("#capstate-close").style.display = "inline-block";
        } else {
            Q("#capstate-open").style.display = "inline-block";
            Q("#capstate-close").style.display = "none";
        }
    },
    setTime: function(d) {
        this.time = d;
        var date_in = Q("#captimeinput");
        date_in.value = (date_in.type == "datetime-local") ? formatDateForPicker(d) : formatDate(d);
    },
    status: function(capst) {
        // first set cap
        if (typeof capst["c"] != "undefined") this.setcap(capst["c"]);
        //0: none, 1: open, 2: close, 3:time, 4: gravity
        if (typeof capst["m"] == "undefined") return;
        var mode = capst.m;
        if (mode == 0) return;
        Q("#capper-frame").style.display = "block";
        var IDs = ["", "cs-manopen", "cs-mancap", "cs-timecon", "cs-sgcon"];
        for (var i = 1; i < IDs.length; i++) {
            if (i == mode) Q("#" + IDs[i]).style.display = "inline-block";
            else Q("#" + IDs[i]).style.display = "none";
        }
        if (typeof capst["g"] != "undefined") {
            Q("#capgravityset").innerHTML = capst["g"];
            Q("#capgravityinput").value = capst["g"];
        }
        if (typeof capst["t"] != "undefined") {
            var dt = new Date(capst["t"] * 1000);
            Q("#captimeset").innerHTML = formatDate(dt);
            this.setTime(dt);
        } else {
            this.setTime(new Date());
        }
    }
};