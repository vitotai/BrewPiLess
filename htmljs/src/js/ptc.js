var valueurl = "/ptc";
var saveurl = "/ptc"

var PTC = {
    fill: function(setting) {
        for (var name in setting) {
            var ele = Q("input[name=" + name + "]");
            if (ele) {
                ele.value = setting[name];
            }
        }
    },

    apply: function() {
        var inputs = this.div.querySelectorAll("input");
        var setting = {};
        for (var i = 0; i < inputs.length; i++) {
            var ele = inputs[i];
            if (ele.name && ele.name != "") {
                setting[ele.name] = parseFloat(ele.value);
            }
        }
        console.log("result=" + JSON.stringify(setting));
        s_ajax({
            url: saveurl,
            m: "POST",
            mime: "application/x-www-form-urlencoded",
            data: "c=" + encodeURI(JSON.stringify(setting)),
            success: function(a) {
                alert("done.");
            },
            fail: function(a) {
                alert("failed updating data:" + a)
            }
        });
    },

    config: function(a) {
        if (a.enabled) {
            this.div.style.display = "block";
            this.fill(a);
        }
    },
    init: function(div) {
        div.style.display = "none";
        this.div = div;
    }
};