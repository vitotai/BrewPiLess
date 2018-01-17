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
    sse: true,
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
    send: function(data, opt) {
        opt = (typeof opt == "undefined") ? {} : opt;
        //console.log("snd:" + data);
        var b = this;
        invoke({
            m: "POST",
            url: "/putline",
            mime: "application/x-www-form-urlencoded",
            data: "data=" + encodeURI(data),
            success: function() { if (typeof opt.success !== "undefined") opt.success(); },
            fail: function(a) {
                if (typeof opt.fail !== "undefined") opt.fail(a);
                else b.error(a);
            }
        });
    },
    reconnecting: false,
    connect: function() {
        var b = this;
        var es = new EventSource("/getline");
        es.onmessage = function(e) {
            b.process(e.data);
        };
        es.onerror = function() {
            if (b.reconnecting) return;
            b.error(-2);
            if (b.auto) setTimeout(function() { b.reconnect(); }, 5000);
        };
        es.onopen = function() {
            b.onconnect();
        };
        b.es = es;
    },
    reconnect: function(forced) {
        forced = (typeof forced == "undefined") ? false : forced;
        var me = this;
        if (me.reconnecting) return;
        if (!forced && me.es.readyState == 1) return;
        me.reconnecting = true;
        if (typeof me.es != "undefined") {
            this.es.close();
        } else {
            this.ws.close();
        }
        // this might triger onerror, and result in "reconnect" call again.
        this.connect();
        this.reconnecting = false;
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