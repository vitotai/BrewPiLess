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
var EI = function(i) {
    return document.getElementById(i);
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
        EI("fssize").innerHTML = s.format(0, 3, ',');
        EI("fsused").innerHTML = u.format(0, 3, ',');
        EI("fsfree").innerHTML = (s - u).format(0, 3, ',');
    },
    slog: function() {
        var t = this;
        if (t.logging) {
            // stop
            if (confirm("Stop current logging?")) {
                //console.log("Stop logging");
                var n = EI("logname").value.trim();
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
        } else {
            if (t.ll.length >= 10) {
                alert("Too many logs. Delete some before creating new.");
                return;
            }
            if ((t.fs.size - t.fs.used) <= t.fs.block * 2) {
                alert("Not enough free space!");
                return;
            }
            var name = EI("logname").value.trim();
            if (t.vname(name) === false) {
                alert("Invalid file name, no special characters allowed.");
                return;
            }
            if (t.dupname(name)) {
                alert("Duplicated name.");
                return;
            }

            if (confirm("Start new logging?")) {
                //console.log("Start logging");
                s_ajax({
                    url: t.starturl + name,
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
        EI("logtitle").innerHTML = "Recording since <b>" + d.toLocaleString() + "</b> ";
        var l = EI("logname");
        l.value = n;
        l.disabled = true;
        EI("logbutton").innerHTML = "STOP Logging";
    },
    stop: function() {
        this.logging = false;
        EI("logtitle").innerHTML = "New Log Name:";
        var l = EI("logname");
        l.value = "";
        l.disabled = false;
        EI("logbutton").innerHTML = "Start Logging";
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
        var tb = EI("loglist").querySelector("tbody");
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
        EI("logbutton").onclick = function() {
            t.slog();
        };
        t.row = EI("loglist").querySelector("tr:nth-of-type(2)");
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
    EI("fmthint").innerHTML = "" + ta.value.length + "/256";
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
    var format = EI("format").value.trim();

    if (window.selectedMethod == "GET") {
        var myRe = new RegExp("\s", "g");
        if (myRe.exec(format)) {
            alert("space is not allowed");
            return;
        }
    }

    var r = {};
    r.enabled = EI("enabled").checked;
    r.url = EI("url").value.trim();
    r.format = encodeURIComponent(format.escapeJSON());
    r.period = EI("period").value;
    r.method = (EI("m_post").checked) ? "POST" : "GET";
    r.type = EI("data-type").value.trim();
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

function init() {

    getActiveNavItem();
    Q("#verinfo").innerHTML = "v" + JSVERSION;

    s_ajax({
        url: logurl + "?data=1",
        m: "GET",
        success: function(d) {
                var r = JSON.parse(d);
                if (typeof r.enabled == "undefined") return;
                EI("enabled").checked = r.enabled;
                window.selectedMethod = r.method;
                EI("m_" + r.method.toLowerCase()).checked = true;
                EI("url").value = (r.url === undefined) ? "" : r.url;
                EI("data-type").value = (r.type === undefined) ? "" : r.type;
                EI("format").value = (r.format === undefined) ? "" : r.format;
                checkformat(EI("format"));
                EI("period").value = (r.period === undefined) ? 300 : r.period;
            }
            /*,
                fail:function(d){
                        alert("error :"+d);
                  }*/
    });

    logs.init();
}

function showformat(lab) {
    var f = EI("formatlist");
    var rec = lab.getBoundingClientRect();
    f.style.display = "block";
    f.style.left = (rec.left) + "px";
    f.style.top = (rec.top + 100) + "px";
}

function hideformat() {
    EI("formatlist").style.display = "none";
}