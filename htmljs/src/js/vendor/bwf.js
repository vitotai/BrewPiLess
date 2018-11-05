function invoke(arg) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4) {
            if (xhttp.status == 200) {
                arg.success(xhttp.responseText);
            } else {
                xhttp.onerror(xhttp.status);
            }
        }
    };
    xhttp.ontimeout = function() {
            if (typeof arg["timeout"] != "undefined")
                arg.timeout();
            else
                xhttp.onerror(-1);
        },
        xhttp.onerror = function(a) {
            if (typeof arg["fail"] != "undefined")
                arg.fail(a);
        };

    xhttp.open(arg.m, arg.url, true);
    if (typeof arg["data"] != "undefined") {
        xhttp.setRequestHeader("Content-Type", (typeof arg["mime"] != "undefined") ? arg["mime"] : "application/x-www-form-urlencoded");
        xhttp.send(arg.data);
    } else
        xhttp.send();
}

var BWF = {
    BrewProfile: "/brewing.json",
    process: function(msg) {
        if (this.raw != null) {
            this.raw(msg);
            return;
        }
        //console.log("rcv:" + msg);
        eval("m={" + msg + "}");
        //	console.log("json:"+m);
        for (var key in m) {
            if (typeof this.handlers[key] != "undefined") {
                this.handlers[key](m[key]);
            }
        }
    },
    on: function(lb, handler) {
        this.handlers[lb] = handler;
    },
    send: function(data) {
        if (this.ws.readyState == 1) this.ws.send(data);
    },
    reconnecting: false,
    connect: function() {
        var me = this;
        if (typeof WebSocket !== "undefined") {
            var ws = new WebSocket('ws://' + document.location.host + '/ws');
            me.ws = ws;
            ws.onopen = function() {
                console.log("Connected");
                me.onconnect();
            };

            ws.onclose = function() {
                if (me.reconnecting) return;
                console.log("WS close");
                me.error(-2);
                if (me.auto) setTimeout(function() { me.reconnect(); }, 5000);
            };

            /*ws.onerror = function() {
                console.log("ws error");
            };*/

            ws.onmessage = function(e) {
                me.process(e.data);
            };
        } else {
            //console.log("not support WebSocket");
            alert("Error! WebSocket Not Supported!");
        }
    },
    reconnect: function(forced) {
        forced = (typeof forced == "undefined") ? false : true;
        var me = this;
        if (me.reconnecting) return;
        if (!forced && me.ws.readyState == 1) return;
        console.log("reconnect forced:" + forced + " state:" + me.ws.readyState);
        me.reconnecting = true;
        me.ws.close();
        // this might triger onerror, and result in "reconnect" call again.
        me.connect();
        me.reconnecting = false;
    },
    init: function(arg) {
        var b = this;
        b.error = (typeof arg.error == "undefined") ? function() {} : arg.error;
        b.handlers = (typeof arg.handlers == "undefined") ? {} : arg.handlers;
        b.raw = (typeof arg.raw == "undefined") ? null : arg.raw;
        b.onconnect = (typeof arg.onconnect == "undefined") ? function() {} : arg.onconnect;
        b.auto = (typeof arg.reconnect == "undefined") ? true : arg.reconnect;

        b.connect();
    },
    save: function(file, data, success, fail) {
        invoke({
            m: "POST",
            url: "/fputs",
            data: "path=" + file + "&content=" + encodeURIComponent(data),
            success: function() { success(); },
            fail: function(e) { fail(e); }
        });
    },
    load: function(file, success, fail) {
        invoke({
            m: "GET",
            url: file,
            success: function(d) { success(d); },
            fail: function(e) { fail(e); }
        });
    }
};