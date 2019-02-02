var PCTRL = {
    init: function() {
        // get values from BPL
        s_ajax({
            url: "psi",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                if (d.mode == 1) {
                    Q("#pt-enabled").checked = true;
                    Q("#pt-control").checked = false;
                } else if (d.mode == 2) {
                    Q("#pt-enabled").checked = true;
                    Q("#pt-control").checked = true;
                } else {
                    Q("#pt-enabled").checked = false;
                    Q("#pt-control").checked = false;
                }
                Q("#fpb").value = d.b;
                Q("#fpa").value = d.a;
            },
            fail: function(b) {
                alert("failed to connect to BPL.");
            }
        });

    },
    apply: function() {
        // save data to BPL
        var data = {};
        if (Q("#pt-enabled").checked) {
            if (Q("#pt-control").checked) data.mode = 2;
            else data.mode = 1;
        } else data.mode = 0;
        data.a = parseFloat(Q("#fpa").value);
        data.b = parseFloat(Q("#fpb").value);
        var json = JSON.stringify(data);
        s_ajax({
            url: "psi",
            m: "POST",
            mime: "application/x-www-form-urlencoded",
            data: "data=" + encodeURIComponent(json),
            success: function(d) {
                alert("<%= done %>")
            },
            fail: function(d) {
                alert("<%= script_control_failed_to_save %>");
            }
        });

    },
    cal: function() {
        Q("#dlg_calibrate").style.display = "block";
        //        Q("#cal1").disabled = true;
    },
    xcal: function() {
        Q("#dlg_calibrate").style.display = "none";
    },
    cal0: function() {
        s_ajax({
            url: "psi?r=1",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                //                Q("#cal1").disabled = false;
                Q("#fpb").value = d.a0;
            },
            fail: function() {
                alert("failed to connect to BPL.");
            }
        });
    },
    cal1: function() {
        s_ajax({
            url: "psi?r=1",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                Q("#fpa").value = PCTRL.conv(d.a0);
            },
            fail: function() {
                alert("failed to connect to BPL.");
            }
        });
    },
    conv: function(a0) {
        var b = parseFloat(Q("#fpb").value);
        var psi = parseFloat(Q("#calpsi").value);
        //(a0 - b) * a =  psi
        // a = psi / (a0-b)
        return (psi / (a0 - b)).toFixed(4);
    }
};

function loaded() {
    PCTRL.init();
}