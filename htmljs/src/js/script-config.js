function formatIP(ip) {
    if (ip == "0.0.0.0") return "";
    return ip;
}


function loadSetting() {
    s_ajax({
        url: "config?cfg=1",
        m: "GET",
        success: function(data) {
            var j = JSON.parse(data);
            window.oridata = j;
            Object.keys(j).map(function(key) {
                var div = Q("input[name=" + key + "]");
                if (div) {
                    if (div.classList.contains("iptype")) {
                        div.value = formatIP(j[key]);
                    } else if (div.type == "checkbox") div.checked = (j[key] != 0);
                    else div.value = j[key];
                } else {
                    // wifi mode
                    div = Q("select[name=" + key + "]");
                    if (div) {
                        div.value = j[key];
                    }
                }
            });
            // system info
            var mac="";
            for(var i=0;i<j.mac.length;i++){
                mac += ((mac=="")?  "":":") + j.mac[i].toString(16);
            }
            Q("#mac-address").innerText=mac;
            Q("#flash-id").innerText = "0x" + j.fid.toString(16);
            Q("#real-flash-size").innerText=j.frsize;
            Q("#specified-flash-size").innerText=j.fsize;
            Q("#fs-size").innerText=j.fs;
        },
        fail: function(d) {
            alert("<%= script_config_error_getting_data %>:" + d);
        }
    });
}

function hidesysinfo(){
    Q("#sysinfo").style.display = "none";
}
function showsysinfo(){
    Q("#sysinfo").style.display = "block";
}
function waitrestart() {
    Q("#waitprompt").style.display = "block";
    Q("#inputform").style.display = "none";
    setTimeout(function() {
        window.location.reload();
    }, 15000);
}

function save() {
    var ins = document.querySelectorAll("#sysconfig input");
    var data = "";
    //(new Uint32Array([-1]))[0]
    var json = {};
    var reboot = false;
    Object.keys(ins).map(function(key, i) {
        if (ins[i].type != "submit") {
            if (ins[i].name && ins[i].name != "") {
                var val;
                if (ins[i].type == "checkbox") val = (ins[i].checked ? 1 : 0);
                else val = ins[i].value.trim();
                json[ins[i].name] = val;
                if (window.oridata[ins[i].name] != val && !ins[i].classList.contains("nb"))
                    reboot = true;
            }
        }
    });
    var div = Q("select[name=wifi]");
    json["wifi"] = div.value;
    json["dis"] = Q("select[name=dis]").value;
    //console.log(JSON.stringify(json));
    var url = "config" + (reboot ? "" : "?nb");
    s_ajax({
        url: url,
        data: "data=" + encodeURIComponent(JSON.stringify(json)),
        m: "POST",
        success: function(data) {
            if (reboot) waitrestart();
        },
        fail: function(d) {
            alert("<%= script_config_error_saving_data %>:" + d);
        }
    });
}

function load() {
    if (Q("#verinfo")) {
        Q("#verinfo").innerHTML = "v" + JSVERSION;
        getActiveNavItem();
    }
    loadSetting();
    Net.init();

    Q("#submitsave").onclick = function(e) {
        e.preventDefault();
        save();
        return false;
    };
}


function validIP(t) {
    var digits = t.split(".");
    var value = 0;
    if (digits.length != 4) return false;
    for (var i = 0; i < 4; i++) {
        var di = parseInt(digits[i]);
        value = (value << 8) + di;
        if (di > 255) {
            return false;
        }
    }
    return value;
}

var Net = {
    select: function(l) {
        document.getElementById('ssid').value = l.innerText || l.textContent;
        document.getElementById('nwpass').focus();
        return false;
    },
    init: function() {
        this.litem = Q(".nwlist");
        this.litem.parentNode.removeChild(this.litem);
        this.setupEvent();
        this.hide();
    },
    nwevent: function(data) {
        var me = this;
        //console.log("ws:" + data);
        if (typeof data["list"] != "undefined") {
            me.list(data.list);
        } else if (typeof data["ssid"] != "undefined") {
            if (data.ssid != "") {
                Q("#connnected-ssid").innerHTML = data.ssid;
            }
            if (typeof data["ip"] != "undefined")
                if (data.ip != "") {
                    Q("#sta-ip").innerHTML = data.ip;

                }
        }
    },
    rssi: function(x) {
        return (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
    },
    list: function(nwlist) {
        var nws = Q("#networks");
        nws.innerHTML = "";
        for (var i = 0; i < nwlist.length; i++) {
            var nl = this.litem.cloneNode(true);
            nl.getElementsByTagName("a")[0].innerHTML = nwlist[i].ssid;
            var signal = nl.getElementsByTagName("span")[0];
            signal.innerHTML = "" + this.rssi(nwlist[i].rssi) + "%";
            if (nwlist[i].enc) signal.className = signal.className + " l";
            nws.appendChild(nl);
        }
    },
    setupEvent: function() {
        var b = this;

        BWF.init({
            handlers: {
                W: function(ev) {
                    b.nwevent(ev);
                }
            }
        });
    },
    scan: function() {
        var lists = Q("#networks");
        lists.innerHTML = "Scanning...";

        s_ajax({
            m: "GET",
            url: "/wifiscan",
            success: function() {}
        });
        return false;
    },
    save: function() {
        var data = "nw=" + encodeURIComponent(Q("#ssid").value);
        if (Q("#nwpass").value != "") data = data + "&pass=" + encodeURIComponent(Q("#nwpass").value);
        var ip = validIP(Q("#staticip").value);
        var gw = validIP(Q("#gateway").value);
        var nm = validIP(Q("#netmask").value);
        var dns = validIP(Q("#dns").value);
        if (ip && gw && nm) {
            data = data + "&ip=" + Q("#staticip").value.trim() +
                "&gw=" + Q("#gateway").value.trim() +
                "&nm=" + Q("#netmask").value.trim() +
                "&dns=" + Q("#dns").value.trim();
        }
        s_ajax({
            m: "POST",
            url: "/wificon",
            data: data,
            success: function() {}
        });
        this.hide();
        return false;
    },
    show: function() {
        Q("#networkselection").style.display = "block";
    },
    hide: function() {
        Q("#networkselection").style.display = "none";
    }
};

function savewifi(){
    Net.save();
    div = Q("select[name=wifi]");
    if(div.value ==2){
        div.value = 3;
        if(typeof window.oridata != "undefined"){
            window.oridata.wifi=3;
        }
    }
}
function initAdvanced(){
    var inputs = document.querySelectorAll('#advanced input,select'), i;

    for (i = 0; i < inputs.length; ++i) {
        inputs[i].onchange = function(){
         //   console.log("item:"+ this.id +" value:" + this.value);
            window.ccModified[this.id]=this.value;
            checkModify();

        }
    } 

    Q("#warning").onchange=function(){
        checkModify();
    }
}


function checkModify(){
    var dirty=false;
    Object.keys(window.ccModified).forEach(key => {
        if(window.ccModified[key] != window.ccConstants[key])
            //console.log("modified:" + key +" value:" + window.ccModified[key]);
        dirty = true;
    });


    Q("#adv-update").disabled= ! Q("#warning").checked || !dirty;
}

function cResponse(cc){
    window.ccModified={};
    window.ccConstants=cc;
    Q("#adv-update").disabled=true;
    Object.keys(cc).forEach(key => {
    //    console.log(key, cc[key]);

        Q("#" + key).value = cc[key];
    });
}
var testConfig={"tempFormat":"C","tempSetMin": 1.0,"tempSetMax": 30.0,"pidMax": 10.000,"Kp": 5.000,"Ki": 0.250,"Kd":-1.500,"iMaxErr": 0.500,"idleRangeH": 1.000,"idleRangeL":-1.000,"heatTargetH": 0.299,"heatTargetL":-0.199,"coolTargetH": 0.199,"coolTargetL":-0.299,"maxHeatTimeForEst":600,"maxCoolTimeForEst":1200,"minCoolTime":180,"minCoolIdleTime":300,"minHeatTime":180,"minHeatIdleTime":300,"deadTime":600,"fridgeFastFilt":1,"fridgeSlowFilt":4,"fridgeSlopeFilt":3,"beerFastFilt":3,"beerSlowFilt":4,"beerSlopeFilt":4,"lah":0,"hs":0};

function showAdvanced(){
    Q("#advanced").style.display = "block";
    if(typeof BWF.handlers['C'] == "undefined"){
        BWF.on('C',cResponse);
        initAdvanced();
    }
    window.ccModified={};
    Q("#adv-update").disabled=true;
    //Test code
    //BWF.handlers.C(testConfig);
    BWF.send("c");
}

function hideAdvanced(){
    Q("#advanced").style.display = "none";
}


function setAdvancedOptions(){
    var cmd={};
    Object.keys(window.ccModified).forEach(key => {
        if(window.ccModified[key] != window.ccConstants[key])
            //console.log("modified:" + key +" value:" + window.ccModified[key]);
        cmd[key] = window.ccModified[key];
    });

    //console.log("j" +JSON.stringify(cmd));
    BWF.send("j" + JSON.stringify(cmd));
}