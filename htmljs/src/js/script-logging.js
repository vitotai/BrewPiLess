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
            if (confirm("<%= script_logging_stop_current_logging %>")) {
                //console.log("Stop logging");
                var n = Q("#logname").value.trim();
                s_ajax({
                    url: t.stopurl + n,
                    m: "GET",
                    success: function(d) {
                        location.reload();
                    },
                    fail: function(d) {
                        alert("<%= script_logging_failed_stop_for %>" + d);
                    }
                });
            }
        }
    },
    startlog: function() {
        var t = this;
        if (!t.logging) {
            if (t.ll.length >= 10) {
                alert("<%= script_logging_too_many_logs %>");
                return;
            }
            if ((t.fs.size - t.fs.used) <= t.fs.block * 2) {
                alert("<%= script_logging_not_free_space %>");
                return;
            }
            var name = Q("#logname").value.trim();
            if (t.vname(name) === false) {
                alert("<%= script_logging_invalid_file_name %>");
                return;
            }
            if (t.dupname(name)) {
                alert("<%= script_logging_duplicated_name %>");
                return;
            }
            var arg = (Q("#calispindel").checked)? "&raw=1":"";

            var wobf = Q("#wobf").checked;
            arg += "&wobf=" + (wobf? "1":"0");

            if (confirm("<%= script_logging_start_new_log %>")) {
                //console.log("Start logging");
                s_ajax({
                    url: t.starturl + name + arg,
                    m: "GET",
                    success: function(d) {
                        location.reload();
                    },
                    fail: function(d,s) {
                        alert("<%= script_logging_failed_start_for %> " + d +" "+s);
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
        if (confirm("<%= script_logging_delete_the_log %> " + t.ll[n].name)) {
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
                    alert("<%= script_logging_failed_delete_for %>" + d);
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

                Q("#wobf").checked = (r.wobf !=0);

                if (typeof r["plato"] != "undefined" && r.plato) {
                    window.plato = true;
                    var th = document.querySelectorAll(".tiltwatercorrect");
                    for (var i = 0; i < th.length; i++)
                        th[i].style.display = "none";
                } else window.plato = false;
            },
            fail: function(e) {
                alert("<%= failed %>:" + e);
            }
        });
    },
};
// for remote logging
function checkurl(t) {
    if (t.value.trim().startsWith("https")) {
        alert("<%= script_logging_https_not_supported %>");
    }
}

function checkformat(ta) {
    if (ta.value.length > 256) {
        ta.value = t.value.substring(0, 256);
    }
    Q("#fmthint").innerHTML = "" + ta.value.length + "/256";
}

function cmethod(c) {
    var inputs = document.querySelectorAll('input[name$="method"]');
    for (var i = 0; i < inputs.length; i++) {
        if (inputs[i].id != c.id)
            inputs[i].checked = false;
    }
    window.selectedMethod = c.value;
}

//Serivce specif widget processing
// generic http
function generichttp_get() {
    if (typeof window.selectedMethod == "undefined") {
        alert("<%= script_logging_select_method %>");
        return null;
    }
    var format = Q("#format").value.trim();

    if (window.selectedMethod == "GET") {
        var myRe = new RegExp("\s", "g");
        if (myRe.exec(format)) {
            alert("<%= script_logging_space_not_allowed %>");
            return null;
        }
    }

    var r = {};
    r.url = Q("#url").value.trim();
    r.format = encodeURIComponent(format.escapeJSON());
    r.method = (Q("#m_post").checked) ? "POST" : "GET";
    r.type = Q("#data-type").value.trim();
    r.service = 3;
    return r;
}

function generichttp_set(r) {
    Q("#service-type").value = "generichttp";
    serviceOption("generichttp");
    window.selectedMethod = r.method;
    Q("#m_" + r.method.toLowerCase()).checked = true;
    Q("#url").value = (r.url === undefined) ? "" : r.url;
    Q("#data-type").value = (r.type === undefined) ? "" : r.type;
    Q("#format").value = (r.format === undefined) ? "" : r.format;
    checkformat(Q("#format"));
}
// ubidots.com
function ubidots_set(r) {
    Q("#service-type").value = "ubidots";
    serviceOption("ubidots");

    // different api    
    var match = /http:\/\/([\w\.]+)\.ubidots\.com\/api\/v1\.6\/devices\/(\w+)\/\?token=(\w+)$/.exec(r.url);

    Q("select[name=ubidots-account]").value = (match[1] == "things") ? 1 : 2;
    Q("#ubidots-device").value = match[2];
    Q("#ubidots-token").value = match[3];

}

function ubidots_get() {
    var device = Q("#ubidots-device").value.trim();
    if (!device) return null;
    var token = Q("#ubidots-token").value.trim();
    if (!token) return null;
    var info = {};
    info.url = (Q("select[name=ubidots-account]").value == 1) ?
        "http://things.ubidots.com/api/v1.6/devices/" + device + "/?token=" + token :
        "http://industrial.api.ubidots.com/v1.6/devices/" + device + "/?token=" + token;

    info.format = encodeURIComponent("{}".escapeJSON());
    info.method = "POST";
    info.type = "application/json";
    info.service = 1;
    return info;
}
// thingspeak.com
function thingspeak_set(r) {
    Q("#service-type").value = "thingspeak";
    serviceOption("thingspeak");

    var values = {};
    var fields = r.format.split('&');
    for (var i = 0; i < fields.length; i++) {
        var pair = fields[i].split("=");
        values[pair[0]] = pair[1];
    }

    Q("#thingspeak-apikey").value = values["api_key"];

    for (var i = 1; i < 9; i++)
        Q("select[name=thingspeak-f" + i + "]").value = (typeof values["field" + i] == "undefined") ?
        "unused" : values["field" + i].substring(1);
}

function thingspeak_get() {
    var apikey = Q("#thingspeak-apikey").value.trim();
    if (!apikey) return null;
    apikey = "api_key=" + apikey;
    var format = apikey;
    for (var i = 1; i < 9; i++) {
        var v = Q("select[name=thingspeak-f" + i + "]").value;
        if (v != "unused") format = format + "&field" + i + "=%" + v;
    }
    if (format == apikey) return null;

    var info = {};
    info.url = "http://api.thingspeak.com/update";
    info.format = encodeURIComponent(format.escapeJSON());
    info.method = "POST";
    info.type = "application/x-www-form-urlencoded";
    info.service = 0;
    return info;
}
//brewfahter
function brewfather_set(r) {
    Q("#service-type").value = "brewfather";
    serviceOption("brewfather");

    var match = /http:\/\/log\.brewfather\.net\/brewpiless\?id=(\w+)$/.exec(r.url);
    Q("#brewfather-id").value = match[1];
    var idmatch = /"id":"([^"]+)"/.exec(r.format);
    Q("#brewfather-device").value = idmatch[1];
}

function brewfather_get(r) {
    var uid = Q("#brewfather-id").value.trim();
    var device = Q("#brewfather-device").value.trim();
    if (!uid || !device) return null;

    var info = {};
    info.url = "http://log.brewfather.net/brewpiless?id=" + uid;

    var format = "{\"id\":\"" + device +
        "\",\"beerTemp\":%b,\"beerSet\":%B,\"fridgeTemp\":%f,\"fridgeSet\":%F,\"roomTemp\":%r,\"gravity\":%g,\"tiltValue\":%t,\"auxTemp\":%a,\"extVolt\":%v,\"timestamp\":%u,\"tempUnit\":\"%U\",\"pressure\":%P,\"mode\":\"%M\",\"humidity\":%h}";

    info.format = encodeURIComponent(format.escapeJSON());;

    info.method = "POST";
    info.type = "application/json";
    info.service = 0;
    return info;
}

//brewfahter
function brewersfriend_set(r) {
    Q("#service-type").value = "brewersfriend";
    serviceOption("brewersfriend");

    var match = /\/\/log\.brewersfriend\.com\/stream\/(\w+)$/.exec(r.url);


    Q("#brewersfriend-apikey").value = match[1];

    var beermatch = /"beer":"([^"]+)"/.exec(r.format);
    Q("#brewersfriend-beer").value = beermatch[1];

    var gumatch = /"gravity_unit":"([P|G])"/.exec(r.format);
    if (gumatch[1] == "P") {
        Q("#gu-sg").checked = false;
        Q("#gu-plato").checked = true;
    } else {
        Q("#gu-sg").checked = true;
        Q("#gu-plato").checked = false;
    }
}

function brewersfriend_get(r) {
    var gf = "%g";
    var gu = "G";
    if (Q('input[name="BF-gu"]:checked').value == "gu-plato") {
        var gf = "%p";
        var gu = "P";
    }
    //http://log.brewersfriend.com/stream/[API KEY]
    var apikey = Q("#brewersfriend-apikey").value.trim();
    var beer = Q("#brewersfriend-beer").value.trim();

    var format = "{\"name\":\"%H\",\"temp\": %b,\"temp_unit\": \"%U\",\"gravity\":" + gf +
        ",\"gravity_unit\":\"" + gu + "\",\"ph\": \"\",\"comment\": \"\",\"beer\":\"" + beer + "\",\"battery\":%v,\"RSSI\": \"\",\"angle\": %t}";

    var info = {};
    info.url = 'http://log.brewersfriend.com/stream/' + apikey;

    info.format = encodeURIComponent(format.escapeJSON());;

    info.method = "POST";
    info.type = "application/json";
    info.service = 2; // null string instead of null
    return info;
}
//
function service_set(r) {
    if (r.service == 1) { // ubidots.com 
        ubidots_set(r);
    } if (r.service == 0){ // generic http, auto
        if (/http:\/\/api\.thingspeak\.com\//.exec(r.url))
            thingspeak_set(r);
        else if (/http:\/\/log\.brewfather\.net\//.exec(r.url))
            brewfather_set(r);
        else if (/http:\/\/log\.brewersfriend\.com\//.exec(r.url))
            brewersfriend_set(r);
    } else
        generichttp_set(r);
}

function update() {
    var service = Q("#service-type").value;
    var r;
    var enabled = Q("#enabled").checked;
    if (service == "generichttp") r = generichttp_get();
    else if (service == "ubidots") r = ubidots_get();
    else if (service == "thingspeak") r = thingspeak_get();
    else if (service == "brewfather") r = brewfather_get();
    else if (service == "brewersfriend") r = brewersfriend_get();

    if (enabled && !r) return;
    if (!r) {
        // default
        r = { url: "", format: "", method: "POST", type: "", service:3 };
    }
    r.enabled = enabled;
    r.period = Q("#period").value;
    if (r.period < 60) r.period = 60;
    if (r.period < 900 && (service == "brewfather" || service == "brewersfriend")) r.period=900;

    s_ajax({
        url: logurl,
        m: "POST",
        data: "data=" + JSON.stringify(r),
        success: function(d) {
            alert("<%= done %>");
        },
        fail: function(e) {
            alert("<%= failed %>:" + e);
        }
    });

}

function remote_init(classic) {
    var MinPeriod = { generichttp: 60, thingspeak: 15, brewfather: 900, ubidots: 60,brewersfriend:900 };
    Q("#period").onchange = function() {
        var min = MinPeriod[Q("#service-type").value];
        if (Q("#period").value < min) Q("#period").value = min;
    };

    serviceOption("generichttp");

    s_ajax({
        url: logurl + "?data=1",
        m: "GET",
        success: function(d) {
                var r = JSON.parse(d);
                if (typeof r.enabled == "undefined") return;
                Q("#enabled").checked = r.enabled;
                Q("#period").value = (r.period === undefined) ? 300 : r.period;
                service_set(r);
            }
            /*,
                fail:function(d){
                        alert("error :"+d);
                  }*/
    });
}

function showformat(lab) {
    var f = Q("#formatlist");
    var rec = lab.getBoundingClientRect();
    f.style.display = "block";
    f.style.left = (rec.right + 5) + "px";
    f.style.top = (rec.bottom + 5) + "px";
}

function hideformat() {
    Q("#formatlist").style.display = "none";
}

function serviceOption(opt) {
    var divs = document.querySelectorAll("#service-opt > div");
    for (var i = 0; i < divs.length; i++) {
        var div = divs[i];
        if (div.id == opt) div.style.display = "block";
        else div.style.display = "none";
    }
    Q("#period").onchange();
}

function serviceChange() {
    serviceOption(Q("#service-type").value);
}

function init(classic) {
    if (typeof classic == "undefined") classic = false;
    if (!classic) {
        getActiveNavItem();
        Q("#verinfo").innerHTML = "v" + JSVERSION;
    }

    remote_init(classic);
    logs.init();

    mqttInit();
}