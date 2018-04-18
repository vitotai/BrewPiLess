var logurl = "log";

Number.prototype.format = function(n, x, s, c) {
    var re = '\\d(?=(\\d{' + (x || 3) + '})+' + (n > 0 ? '\\D' : '$') + ')',
        num = this.toFixed(Math.max(0, ~~n));

    return (c ? num.replace('.', c) : num).replace(new RegExp(re, 'g'), '$&' + (s || ','));
};
String.prototype.escapeJSON = function() {
    return this.replace(/[\\]/g, '\\\\')
        .replace(/[\"]/g, '\\\"')
        .replace(/[\/]/g, '\\/')
        .replace(/[\b]/g, '\\b')
        .replace(/[\f]/g, '\\f')
        .replace(/[\n]/g, '\\n')
        .replace(/[\r]/g, '\\r')
        .replace(/[\t]/g, '\\t');
};


var logs = {
    url: "loglist.php",
    rmurl: "loglist.php?rm=",
    starturl: "loglist.php?start=",
    stopurl: "loglist.php?stop=1",
    dlurl: "loglist.php?dl=",
    ll: [],
    fs: {},
    logging: false,
    vname: function(name) {
        if (name == "") return false;
        if (name.match(/[\W]/g)) return false;
        return true;
    },
    dupname: function(name) {
        var ret = false;
        this.ll.forEach(function(log) {
            if (name == log.name) ret = true;
        });
        return ret;
    },
    fsinfo: function(s, u) {
        Q("#fssize").innerHTML = s.format(0, 3, ',');
        Q("#fsused").innerHTML = u.format(0, 3, ',');
        Q("#fsfree").innerHTML = (s - u).format(0, 3, ',');
    },
    stoplog: function() {
        var t = this;
        if (t.logging) {
            // stop
            if (confirm("Stop current logging?")) {
                //console.log("Stop logging");
                var n = Q("#logname").value.trim();
                s_ajax({
                    url: t.stopurl + n,
                    m: "GET",
                    success: function(d) {
                        location.reload();
                    },
                    fail: function(d) {
                        alert("Failed to stop for:" + d);
                    }
                });
            }
        }
    },
    startlog: function() {
        var t = this;
        if (!t.logging) {
            if (t.ll.length >= 10) {
                alert("Too many logs. Delete some before creating new.");
                return;
            }
            if ((t.fs.size - t.fs.used) <= t.fs.block * 2) {
                alert("Not enough free space!");
                return;
            }
            var name = Q("#logname").value.trim();
            if (t.vname(name) === false) {
                alert("Invalid file name, no special characters allowed.");
                return;
            }
            if (t.dupname(name)) {
                alert("Duplicated name.");
                return;
            }
            var arg = "";
            var calispindel = Q("#calispindel").checked;
            if (calispindel) {
                var tilt = parseFloat(Q("#tiltinw").value.trim());
                var reading = parseFloat(Q("#hydrometer").value.trim());
                if (isNaN(tilt) || isNaN(reading)) {
                    alert("tilt value and hydrometer reading is necessary!");
                    return;
                }
                arg = "&tw=" + tilt + "&hr=" + reading;
            }

            if (confirm("Start new logging?")) {
                //console.log("Start logging");
                s_ajax({
                    url: t.starturl + name + arg,
                    m: "GET",
                    success: function(d) {
                        location.reload();
                    },
                    fail: function(d) {
                        alert("Failed to start for:" + d);
                    }
                });
            }
        }
    },
    recording: function(n, t) {
        this.logging = true;
        var d = new Date(t * 1000);
        Q("#start-log-date").innerHTML = d.toLocaleString();
        Q("#loggingtitle").innerHTML = n;
        Q("#logstartinput").style.display = "none";
        Q("#logstopinput").style.display = "block";
    },
    stop: function() {
        this.logging = false;
        Q("#logstartinput").style.display = "block";
        Q("#logstopinput").style.display = "none";
    },
    //view:function(n){
    //	alert("View " + this.ll[n].name);
    //	window.open(this.vurl+ n);
    //},
    rm: function(n) {
        var t = this;
        if (confirm("Delete the log " + t.ll[n].name)) {
            console.log("rm " + t.ll[n].name);
            s_ajax({
                url: t.rmurl + n,
                m: "GET",
                success: function(d) {
                    var r = JSON.parse(d);
                    t.fs = r;
                    t.fsinfo(r.size, r.used);
                    t.ll.splice(n, 1);
                    t.list(t.ll);
                },
                fail: function(d) {
                    alert("Failed to delete for:" + d);
                }
            });
        }
    },
    dl: function(n) {
        //console.log("DL " +this.ll[n].name);
        window.open(this.dlurl + n);
    },
    list: function(l) {
        var tb = Q("#loglist").querySelector("tbody");
        var tr;
        while (tr = tb.querySelector("tr:nth-of-type(2)"))
            tb.removeChild(tr);

        var t = this;
        var row = t.row;
        l.forEach(function(i, idx) {
            var name = i.name;
            var date = new Date(i.time * 1000);
            var nr = row.cloneNode(true);
            nr.querySelector(".logid").innerHTML = name;
            nr.querySelector(".logdate").innerHTML = date.toLocaleString();
            nr.querySelector(".dlbutton").onclick = function() {
                t.dl(idx);
            };
            //		nr.querySelector(".viewbutton").onclick=function(){t.view(idx);};
            nr.querySelector(".rmbutton").onclick = function() {
                t.rm(idx);
            };
            tb.appendChild(nr);
        });

    },
    init: function() {
        var t = this;
        Q("#startlogbutton").onclick = function() {
            t.startlog();
        };
        Q("#stoplogbutton").onclick = function() {
            t.stoplog();
        };

        t.row = Q("#loglist").querySelector("tr:nth-of-type(2)");
        t.row.parentNode.removeChild(t.row);
        s_ajax({
            url: t.url,
            m: "GET",
            success: function(d) {
                var r = JSON.parse(d);
                t.fs = r.fs;
                if (r.rec)
                    t.recording(r.log, r.start);
                t.ll = r.list;
                t.list(r.list);
                t.fsinfo(r.fs.size, r.fs.used);
            },
            fail: function(e) {
                alert("failed:" + e);
            }
        });
    },
};

function checkurl(t) {
    if (t.value.trim().startsWith("https")) {
        alert("HTTPS is not supported");
    }
}

function checkformat(ta) {
    if (ta.value.length > 256) {
        ta.value = t.value.substring(0, 256);
    }
    Q("#fmthint").innerHTML = "" + ta.value.length + "/256";
}

function method(c) {
    var inputs = document.querySelectorAll('input[name$="method"]');
    for (var i = 0; i < inputs.length; i++) {
        if (inputs[i].id != c.id)
            inputs[i].checked = false;
    }
    window.selectedMethod = c.value;
}

function update() {

    if (typeof window.selectedMethod == "undefined") {
        alert("select Method!");
        return;
    }
    var format = Q("#format").value.trim();

    if (window.selectedMethod == "GET") {
        var myRe = new RegExp("\s", "g");
        if (myRe.exec(format)) {
            alert("space is not allowed");
            return;
        }
    }

    var r = {};
    r.enabled = Q("#enabled").checked;
    r.url = Q("#url").value.trim();
    r.format = encodeURIComponent(format.escapeJSON());
    r.period = Q("#period").value;
    r.method = (Q("#m_post").checked) ? "POST" : "GET";
    r.type = Q("#data-type").value.trim();
    s_ajax({
        url: logurl,
        m: "POST",
        data: "data=" + JSON.stringify(r),
        success: function(d) {
            alert("done");
        },
        fail: function(e) {
            alert("failed:" + e);
        }
    });

}

function init(classic) {
    if (typeof classic == "undefined") classic = false;
    if (!classic) {
        getActiveNavItem();
        Q("#verinfo").innerHTML = "v" + JSVERSION;
    }

    function readingByTemp() {
        var temp = parseFloat(Q("#watertemp").value);
        var ctemp = parseFloat(Q("#caltemp").value);
        var unit = Q("#tempunit").value;
        if (isNaN(temp) || isNaN(ctemp)) return;
        if (unit == 'C') {
            ctemp = C2F(ctemp);
            temp = C2F(temp);
        }
        var reading = BrewMath.tempCorrectionF(1.0, ctemp, temp);
        Q("#hydrometer").value = reading.toFixed(3);
    }
    Q("#watertemp").onchange = readingByTemp;
    Q("#caltemp").onchange = readingByTemp;
    Q("#tempunit").onchange = readingByTemp;

    s_ajax({
        url: logurl + "?data=1",
        m: "GET",
        success: function(d) {
                var r = JSON.parse(d);
                if (typeof r.enabled == "undefined") return;
                Q("#enabled").checked = r.enabled;
                window.selectedMethod = r.method;
                Q("#m_" + r.method.toLowerCase()).checked = true;
                Q("#url").value = (r.url === undefined) ? "" : r.url;
                Q("#data-type").value = (r.type === undefined) ? "" : r.type;
                Q("#format").value = (r.format === undefined) ? "" : r.format;
                checkformat(Q("#format"));
                Q("#period").value = (r.period === undefined) ? 300 : r.period;
            }
            /*,
                fail:function(d){
                        alert("error :"+d);
                  }*/
    });

    logs.init();
}

function showformat(lab) {
    var f = Q("#formatlist");
    var rec = lab.getBoundingClientRect();
    f.style.display = "block";
    f.style.left = (rec.left) + "px";
    f.style.top = (rec.top + 100) + "px";
}

function hideformat() {
    Q("#formatlist").style.display = "none";
}