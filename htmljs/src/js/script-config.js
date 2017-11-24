function loadSetting() {
    s_ajax({
        url: "brewpi.cfg",
        m: "GET",
        success: function(data) {
            var j = JSON.parse(data);
            Object.keys(j).map(function(key) {
                var div = Q("input[name=" + key + "]");
                if (div) {
                    if (div.type == "checkbox") div.checked = (j[key] != 0);
                    else div.value = j[key];
                }
            });
        },
        fail: function(d) {
            alert("error getting data:" + d);
        }
    });
}

function waitrestart() {
    Q("#waitprompt").style.display = "flex";
    setTimeout(function() {
        window.location.reload();
    }, 15000);
}

function save() {
    var ins = document.querySelectorAll("input");
    var data = "";
    Object.keys(ins).map(function(key, i) {
        if (ins[i].type != "submit") {
            if (ins[i].name && ins[i].name != "") {
                var val = (ins[i].type == "checkbox") ? (ins[i].checked ? 1 : 0) : encodeURIComponent(ins[i].value.trim());
                data = ((data == "") ? "" : (data + "&")) + ins[i].name + "=" + val;
            }
        }
    });
    s_ajax({
        url: "config",
        data: data,
        m: "POST",
        success: function(data) {
            waitrestart();
        },
        fail: function(d) {
            alert("error saving data:" + d);
        }
    });
}

function load() {
    loadSetting();
}