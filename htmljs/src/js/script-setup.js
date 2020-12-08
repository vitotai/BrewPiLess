var BackupFile = "/device.cfg";
var devices = {
    pinlabel:function(pin){
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
        if (pin < 0) return "NA";
        if (window.board == "f")  return "GPIO " + pin;
        if (window.board == "e") return c[pin];
        return "Unknown";
    },
    pinFuncChange:function(g){
        if(g.querySelector("select.device-function").value ==8){
            g.querySelectorAll(".device-humidity-sensor-container").forEach(function(div){div.style.display="";});
            g.querySelectorAll(".device-pintype-container").forEach(function(div){div.style.display="none";});
        }else{
            g.querySelectorAll(".device-humidity-sensor-container").forEach(function(div){div.style.display="none";});

            g.querySelectorAll(".device-pintype-container").forEach(function(div){div.style.display="";});
        }
    },
    add: function(a, f) {
        var g;
        if (f.h == 2) { // sensor
            g = window.sensorContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v
            g.querySelector("input.device-calibration").value = f.j;
        } else if (f.h == 5) { // external sensor
            g = window.extsensorContainer.cloneNode(true);
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v;
            g.querySelector("input.device-calibration").value = f.j;
        } else if (f.h == 6) { // temp sensor of DHT1x/DHT2x series
            g = window.dhtsensorContainer.cloneNode(true);
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v;
            g.querySelector("input.device-calibration").value = f.j;
        } else if (f.h == 3) { // owContainer
            g = window.owContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-channel").innerHTML = f.n;
            g.querySelector("select.device-pintype").value = f.x;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")
        } else {
            g = window.pinContainer.cloneNode(true);
            if(f.f == 8){ // humidity sensor
                g.querySelector("select.device-humidity-sensor").value = f.s;
                g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : (f.v + "%");
                g.querySelector("input.device-calibration").value = f.j; 
            }else {

                g.querySelector("select.device-pintype").value = f.x;
                g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")
            }
        }
        g.querySelector("select.slot-select").value = f.i;

        g.querySelector("span.device-pin").innerHTML = this.pinlabel(f.p);
        g.querySelector("select.device-function").value = f.f;
        g.querySelector("select.device-function").onchange=function(){
          devices.pinFuncChange(g);  
        };
        if(f.h ==1 ) devices.pinFuncChange(g); // pin
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
    if (c.h == 2) { // onewire temp sensor
        c.a = a.a
    } else if (c.h == 3) { //  onewire 2413
        c.a = a.a;
        c.n = a.n;
        c.x = d.querySelector("select.device-pintype").value
    } else if (c.h == 1) { // hardware pin
        if( c.f ==8) c.s = d.querySelector("select.device-humidity-sensor").value;
        else c.x = d.querySelector("select.device-pintype").value
    }
    if(c.h == 2 || c.h == 5 ||  c.h == 6 || c.f ==8){ // onewire temp &  external sensor
        c.j = d.querySelector("input.device-calibration").value;
        if(isNaN(c.j)) c.j=0;
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
        if(f.h == 2 || f.h == 5) e.j=f.j;
        c.push(e)
    }
    var b = JSON.stringify(c);
    //console.log(b);
    // Browsers that support HTML5 download attribute
    download(new Blob([b], {type: 'text/json;'}), "device.json");
}

function restoreJson(b){
    blockscreen("<%= script_setup_restoring %>");
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
}

function restore() {
    Q("#dlg_restore").style.display = "block";
    Q("#dlg_restore .cancel").onclick=function(){
        Q("#dlg_restore").style.display = "none";
        return false;
    };

    Q('#restore-file').onchange = function(evt) {
        //Retrieve the first (and only!) File from the FileList object
        var f = evt.target.files[0];
        if (f) {
            var r = new FileReader();
            r.onload = function(e) {
                try{
                  var json=JSON.parse(e.target.result);
                  Q("#dlg_restore").style.display = "none";
                  restoreJson(json);
                }catch(e){
                    alert("invalid format!");
                }
            };
            r.readAsText(f);
        }    
    };


}
/*
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
*/
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
    window.dhtsensorContainer =detachNode(".device-container.dht-temp-device");
    window.board= "e"; //default ESP8266
    BWF.init({
        error: function(a) {
            //                alert("error communication between server")
        },
        onconnect:function(){
            BWF.send("n");
        },
        handlers: {
            d: function(a) {
                installed_list = a
            },
            h: function(a) {
                available_list = a;
                listGot()
            },
            N:function(a){
                window.board= a.b;
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

function download(blob, file) {
    var link = document.createElement("a");

    if (link.download === undefined) { // feature detection
        alert("<%= script_viewer_not_downloading_file %>");
        return;
    }

    var url = URL.createObjectURL(blob);
    link.setAttribute("href", url);
    link.setAttribute("download", file);
    link.style.visibility = 'hidden';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);

}
