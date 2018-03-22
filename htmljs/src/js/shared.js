var JSVERSION = "2.7";

function s_ajax(b) {
    var c = new XMLHttpRequest();
    c.onreadystatechange = function() {
        if (c.readyState == 4) {
            if (c.status == 200) {
                b.success(c.responseText)
            } else {
                c.onerror(c.status)
            }
        }
    };
    c.ontimeout = function() {
        if (typeof b["timeout"] != "undefined") b.timeout();
        else c.onerror(-1)
    }, c.onerror = function(a) {
        if (typeof b["fail"] != "undefined") b.fail(a)
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

function C2F(c) {
    return Math.round((c * 1.8 + 32) * 10) / 10
}

function F2C(f) {
    return Math.round((f - 32) / 1.8 * 10) / 10
}

function openDlgLoading() {
    document.getElementById('dlg_loading').style.display = "block";
}

function closeDlgLoading() {
    document.getElementById('dlg_loading').style.display = "none";
}

var BrewMath = {
    abv: function(og, fg) {
        return ((76.08 * (og - fg) / (1.775 - og)) * (fg / 0.794)).toFixed(1);
    },
    att: function(og, fg) {
        return Math.round((og - fg) / (og - 1) * 100);
    },
    sg2pla: function(sg) {
        return -616.868 + 1111.14 * sg - 630.272 * sg * sg + 135.997 * sg * sg * sg;
    },
    pla2sg: function(pla) {
        return 1 + (pla / (258.6 - ((pla / 258.2) * 227.1)));
    }
};

Date.prototype.shortLocalizedString = function() {
    var y = this.getYear() + 1900;
    var re = new RegExp('[^\d]?' + y + '[^\d]?');
    var n = this.toLocaleDateString();
    var ds = n.replace(re, "");
    var HH = this.getHours();
    var MM = this.getMinutes();

    function T(x) {
        return (x > 9) ? x : ("0" + x);
    }
    return ds + " " + T(HH) + ":" + T(MM);
};

function getActiveNavItem() {
    var path = window.location.pathname.split("/").pop();
    if (path == "") path = "index.htm";
    var element = Q('.options>li>a[href="/' + path + '"]');
    element.className += 'active';
}