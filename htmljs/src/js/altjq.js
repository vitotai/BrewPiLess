function s_ajax(b) {
    var c = new XMLHttpRequest();
    c.onreadystatechange = function() {
        if (c.readyState == 4) {
            if (c.status == 200) {
                b.success(c.responseText)
            } else {
                c.onerror(c.status,c.responseText)
            }
        }
    };
    c.ontimeout = function() {
        if (typeof b["timeout"] != "undefined") b.timeout();
        else c.onerror(-1,"timeout")
    }, c.onerror = function(a,r) {
        if (typeof b["fail"] != "undefined") b.fail(a,r)
    };
    c.open(b.m, b.url, true);
    if (typeof b["data"] != "undefined") {
        c.setRequestHeader("Content-Type", (typeof b["mime"] != "undefined") ? b["mime"] : "application/x-www-form-urlencoded");
        c.send(b.data)
    } else c.send()
}

var Q = function(d) {
    return document.querySelector(d);
};