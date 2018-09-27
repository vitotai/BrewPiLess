var BackupFile = "/device.cfg";
var devices = {
    add: function(a, f) {
        var g;
        if (f.h == 2) {
            g = window.sensorContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v
        } else if (f.h == 5) {
            g = window.extsensorContainer.cloneNode(true);
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v;
        } else if (f.h == 3) {
            g = window.owContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-channel").innerHTML = f.n;
            g.querySelector("select.device-pintype").value = f.x;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")
        } else {
            g = window.pinContainer.cloneNode(true);
            g.querySelector("select.device-pintype").value = f.x;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")
        }
        g.querySelector("select.slot-select").value = f.i;
        var c = {
            0: "D3",
            1: "D10",
            2: "D4",
            3: "D9",
            4: "D2",
            5: "D1",
            12: "D6",
            13: "D7",
            14: "D5",
            15: "D8",
            16: "D0",
            17: "A0"
        };
        g.querySelector("span.device-pin").innerHTML = (f.p > 0) ? c[f.p] : "NA";
        g.querySelector("select.device-function").value = f.f;
        g.querySelector("div.device-title").innerHTML = "Device " + a;
        g.querySelector("button").onclick = function() {
            device_apply(a)
        };
        g.hardwaretype = f.h;
        g.pinnumber = f.p;
        var b = (f.i < 0) ? "detected-list" : "installed-list";
        var e = document.getElementById(b);
        e.appendChild(g);
        e.appendChild(document.createElement("br"))
    }
};
var installed_list = [];
var available_list = [];

function cmdfrom(b) {
    var a;
    if (b < installed_list.length) {
        a = installed_list[b]
    } else {
        a = available_list[b - installed_list.length]
    }
    var d = document.querySelectorAll("div.device-container")[b];
    var c = {};
    c.i = d.querySelector("select.slot-select").value;
    c.c = 1;
    c.f = d.querySelector("select.device-function").value;
    if (c.f >= 9 && c.f <= 15) {
        c.b = 1
    } else {
        c.b = 0
    }
    c.h = a.h;
    c.p = a.p;
    if (c.h == 2) {
        c.a = a.a
    } else if (c.h == 3) {
        c.a = a.a;
        c.n = a.n;
        c.x = d.querySelector("select.device-pintype").value
    } else if (c.h == 1) {
        c.x = d.querySelector("select.device-pintype").value
    }
    return c
}

function device_apply(a) {
    blockscreen("<%= script_setup_updating %>");
    var b = cmdfrom(a);
    var c = "U" + JSON.stringify(b);
    console.log(c);
    var tout = setTimeout(function() {
        alert("<%= setup_update_timeout %>")
        unblockscreen();
    }, 5000);

    BWF.on("U", function(d) {
        if (tout) clearTimeout(tout);
        unblockscreen();
    });
    BWF.send(c)
}

function backup() {
    if (installed_list.length == 0) {
        alert("<%= script_setup_no_installed_devices %>");
        return
    }
    var c = [];
    for (var a = 0; a < installed_list.length; a++) {
        var f = installed_list[a];
        var e = {
            i: f.i,
            c: f.c,
            b: f.b,
            f: f.f,
            h: f.h,
            p: f.p
        };
        if (f.h == 2) {
            e.a = f.a
        } else {
            e.x = f.x
        }
        c.push(e)
    }
    var b = JSON.stringify(c);
    console.log(b);
    BWF.save(BackupFile, b, function() {
        alert("<%= done %>")
    }, function(d) {
        alert("<%= script_setup_error_saving %>" + d)
    })
}

function restore() {
    blockscreen("<%= script_setup_restoring %>");
    BWF.load(BackupFile, function(c) {
        var b = JSON.parse(c);
        var a = 0;
        BWF.on("U", function(d) {
            if (++a >= b.length) {
                BWF.on("U", null);
                unblockscreen();
                return
            }
            BWF.send("U" + JSON.stringify(b[a]))
        });
        BWF.send("U" + JSON.stringify(b[a]))
    }, function(a) {
        alert("<%= script_setup_error_load %>" + a);
        unblockscreen()
    })
}

function list() {
    blockscreen("<%= script_setup_retrieving %>");
    installed_list = [];
    available_list = [];
    document.getElementById("detected-list").innerHTML = "";
    document.getElementById("installed-list").innerHTML = "";
    BWF.send("d{r:1}");
    BWF.send("h{u:-1,v:1}")
}

function erase() {
    if (confirm("<%= script_setup_erase_all_setting %>")) BWF.send("E");
}

function listGot() {
    document.getElementById("detected-list").innerHTML = "";
    document.getElementById("installed-list").innerHTML = "";
    var a = 0;
    for (var b = 0; b < installed_list.length; b++) {
        devices.add(a++, installed_list[b])
    }
    for (var b = 0; b < available_list.length; b++) {
        devices.add(a++, available_list[b])
    }
    unblockscreen()
}

function detachNode(query) {
    var d = document.querySelector(query);
    d.parentNode.removeChild(d);
    return d;
}

function init(classic) {
    if (typeof classic == "undefined") classic = false;
    if (!classic) {
        getActiveNavItem();
        Q("#verinfo").innerHTML = "v" + JSVERSION;
    }

    window.sensorContainer = detachNode(".device-container.sensor-device");
    window.pinContainer = detachNode(".device-container.pin-device");
    window.extsensorContainer = detachNode(".device-container.extsensor-device");
    window.owContainer = detachNode(".device-container.ow-device");

    BWF.init({
        error: function(a) {
            //                alert("error communication between server")
        },
        handlers: {
            d: function(a) {
                installed_list = a
            },
            h: function(a) {
                available_list = a;
                listGot()
            }
        }
    })
}

function blockscreen(a) {
    document.getElementById("blockscreencontent").innerHTML = a;
    document.getElementById("blockscreen").style.display = "block"
}

function unblockscreen() {
    document.getElementById("blockscreen").style.display = "none"
};