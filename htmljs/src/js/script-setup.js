var ErrMsg={
    2: "Out of Memory",
	3: "Device definition update specification is invalid",
	4: "Invalid chamber id %0",
	5: "Invalid beer id %0",
	6: "Invalid device function id %0",
	7: "Invalid config for device owner type %0 beer=%1 chamber=%2",
	8: "Cannot assign device type %0 to hardware %1",
	9: "Device is onewire but pin %0 is not configured as a onewire bus"
};
var BackupFile = "/device.cfg";
var Func_ChamberHumSensor=8;
var Func_RoomHumSensor=18;
var HW_PIN=1;
var HW_1W_SENSOR=2;
var HW_1W_2413=3;
var HW_EXT_SENSOR=5;
var HW_ENV_SENSOR= 6;
var HW_BME280=7;

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
        if(g.querySelector("select.device-function").value ==Func_ChamberHumSensor || g.querySelector("select.device-function").value == Func_RoomHumSensor){
            g.querySelectorAll(".device-humidity-sensor-container").forEach(function(div){div.style.display="";});
            g.querySelectorAll(".device-pintype-container").forEach(function(div){div.style.display="none";});
        }else{
            g.querySelectorAll(".device-humidity-sensor-container").forEach(function(div){div.style.display="none";});

            g.querySelectorAll(".device-pintype-container").forEach(function(div){div.style.display="";});
        }
    },
    add: function(a, f) {
        var g;
        if (f.h == HW_1W_SENSOR) { // sensor
            g = window.sensorContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v
            g.querySelector("input.device-calibration").value = f.j;
        } else if (f.h == HW_EXT_SENSOR) { // external sensor
            g = window.extsensorContainer.cloneNode(true);
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v;
            g.querySelector("input.device-calibration").value = f.j;
        } else if (f.h == HW_ENV_SENSOR) { // temp sensor of humidity sensor/DHT1x/DHT2x series
            g = window.dhtsensorContainer.cloneNode(true);
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : f.v;
            g.querySelector("input.device-calibration").value = f.j;
            if(f.p){ // 0 is chamber
                g.querySelector(".chamber-sensor").style.display="none";
            }else{
                g.querySelector(".room-sensor").style.display="none";
            }
        } else if (f.h ==HW_1W_2413) { // onewire switch/2413
            g = window.owContainer.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = f.a;
            g.querySelector("span.device-channel").innerHTML = f.n;
            g.querySelector("select.device-pintype").value = f.x;
            g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")

        } else if (f.h == HW_BME280) { // BME280
            g = window.bme280Container.cloneNode(true);
            g.querySelector("span.device-address").innerHTML = "0x" + parseInt(f.p).toString(16);
        } else {
            // pin devices
            g = window.pinContainer.cloneNode(true);
            if(f.f ==Func_ChamberHumSensor || f.f == Func_RoomHumSensor){ // humidity sensor
                g.querySelector("select.device-humidity-sensor").value = f.s;
                g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : (f.v + "%");
                g.querySelector("input.device-calibration").value = f.j; 
            }else {

                g.querySelector("select.device-pintype").value = f.x;
                g.querySelector("span.device-value").innerHTML = (typeof f.v === "undefined") ? "-" : ((f.v) ? "active" : "inactive")
            }
        }
        g.querySelector("select.slot-select").value = f.i;
        if(f.h != HW_BME280 && f.h != HW_ENV_SENSOR) g.querySelector("span.device-pin").innerHTML = this.pinlabel(f.p);
        g.querySelector("select.device-function").value = f.f;
        g.querySelector("select.device-function").onchange=function(){
          devices.pinFuncChange(g);  
        };
        if(f.h ==HW_PIN ) devices.pinFuncChange(g); // pin
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
    var div = document.querySelectorAll("div.device-container")[b];
    var c = {};
    c.i = parseInt(div.querySelector("select.slot-select").value);
    c.c = 1;
    c.f = parseInt(div.querySelector("select.device-function").value);
    if (c.f >= 9 && c.f <= 15) {
        c.b = 1
    } else {
        c.b = 0
    }
    c.h = a.h;
    c.p = a.p;
    if (c.h == HW_1W_SENSOR) { // onewire temp sensor
        c.a = a.a
    } else if (c.h == HW_1W_2413) { //  onewire 2413
        c.a = a.a;
        c.n = a.n;
        c.x = parseInt(div.querySelector("select.device-pintype").value);
    } else if (c.h == HW_PIN) { // hardware pin
        if( c.f == Func_ChamberHumSensor || c.f == Func_RoomHumSensor) c.s = parseInt(div.querySelector("select.device-humidity-sensor").value);
        else c.x = parseInt(div.querySelector("select.device-pintype").value);
    }
    if(c.h == HW_1W_SENSOR || c.h == HW_EXT_SENSOR ||  c.h == HW_ENV_SENSOR || c.f ==Func_ChamberHumSensor || c.f ==Func_RoomHumSensor){ // onewire temp &  external sensor
        c.j = parseFloat(div.querySelector("input.device-calibration").value);
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
    BWF.on("D", function(d) {
        if (tout) clearTimeout(tout);
        unblockscreen();
        if(typeof d["logID"] != "undefined"){
            var msg = ErrMsg[d.logID];
            if(typeof d["V"] == "array"){
                d.V.forEach(function(value,index){
                    msg = msg + "," + value;
                    re = new RegExp("%" +index,"g");
                    msg = msg.replace(re, value);
                });
                alert(msg);
            }
        }else alert("Unknown Error!");
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
        if (f.h == HW_1W_SENSOR) {
            e.a = f.a
        } else {
            e.x = f.x
        }
        if(f.h == HW_1W_SENSOR || f.h == HW_EXT_SENSOR || c.h == HW_ENV_SENSOR || c.f ==Func_ChamberHumSensor || c.f ==Func_RoomHumSensor) e.j=f.j;
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
    window.dhtsensorContainer =detachNode(".device-container.env-temp-device");
    window.bme280Container=detachNode(".device-container.bme280-device");
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
                list();
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
