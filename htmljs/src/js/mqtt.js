function mqttLoadSetting() {
    s_ajax({
        url: "mqtt",
        m: "GET",
        success: function(data) {
            var j = JSON.parse(data);
            Object.keys(j).map(function(key) {
                var name = "mqtt_" + key;
                var div = Q(".mqtt-input[name=" + name + "]");
                if (div) {
                    if (div.type == "checkbox") div.checked = (j[key] != 0);
                    else div.value = j[key];
                }
            });
        },
        fail: function(d) {
            alert("<%= script_config_error_getting_data %>:" + d);
        }
    });
}

function mqttSave() {
    var ins = document.querySelectorAll(".mqtt-input");
    var json = {};
    Object.keys(ins).map(function(key, i) {
        if (ins[i].type != "submit") {
            if (ins[i].name && ins[i].name != "") {
                var val;
                if (ins[i].type == "checkbox") val = (ins[i].checked ? 1 : 0);
                else val = ins[i].value.trim();
                json[ins[i].name.split("_")[1]] = val;
            }
        }
    });

    console.log(JSON.stringify(json));
    s_ajax({
        url: "mqtt",
        data: "data=" + encodeURIComponent(JSON.stringify(json)),
        m: "POST",
        success: function(data) {
            alert("done");
        },
        fail: function(d) {
            alert("<%= script_config_error_saving_data %>:" + d);
        }
    });
}

function mqttInit() {
    mqttLoadSetting();
    Q("#submitsavemqtt").onclick = function(e) {
        e.preventDefault();
        mqttSave();
        return false;
    };

}