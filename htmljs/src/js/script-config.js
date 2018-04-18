function formatIP(ip) {
    if (ip == "0.0.0.0") return "";
    return ip;
}

function verifyIP(t) {
    var digits = this.value.split(".");
    var valid = true;
    var val = 0;
    if (digits.length == 4) {
        for (var i = 0; i < 4; i++) {
            var di = parseInt(digits[i]);
            if (di > 255) {
                valid = false;
                break;
            }
        }
    } else if (this.value == "") valid = true;
    else valid = false;
    if (valid) {
        this.saved = this.value;
        this.value = formatIP(this.value);
    } else {
        this.value = formatIP(this.saved);
    }

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
                        div.saved = j[key];
                        div.onchange = verifyIP;
                    } else if (div.type == "checkbox") div.checked = (j[key] != 0);
                    else div.value = j[key];
                } else {
                    div = Q("select[name=" + key + "]");
                    if (div) div.value = j[key];
                }
            });
        },
        fail: function(d) {
            alert("error getting data:" + d);
        }
    });
}

function waitrestart() {
    Q("#waitprompt").style.display = "block";
    Q("#inputform").style.display = "none";
    setTimeout(function() {
        window.location.reload();
    }, 15000);
}

function save() {
    var ins = document.querySelectorAll("input");
    var data = "";
    //(new Uint32Array([-1]))[0]
    var json = {};
    var reboot = false;
    Object.keys(ins).map(function(key, i) {
        if (ins[i].type != "submit") {
            if (ins[i].name && ins[i].name != "") {
                var val;
                if (ins[i].classList.contains("iptype")) val = ins[i].saved;
                else if (ins[i].type == "checkbox") val = (ins[i].checked ? 1 : 0);
                else val = ins[i].value.trim();
                json[ins[i].name] = val;
                if (window.oridata[ins[i].name] != val && !ins[i].classList.contains("nb"))
                    reboot = true;
            }
        }
    });
    var div = Q("select[name=wifi]");
    json["wifi"] = div.value;
    console.log(JSON.stringify(json));
    var url = "config" + (reboot ? "" : "?nb");
    s_ajax({
        url: url,
        data: "data=" + encodeURIComponent(JSON.stringify(json)),
        m: "POST",
        success: function(data) {
            if (reboot) waitrestart();
        },
        fail: function(d) {
            alert("error saving data:" + d);
        }
    });
}

function load() {
    if (Q("#verinfo")) Q("#verinfo").innerHTML = "v" + JSVERSION;
    loadSetting();
}