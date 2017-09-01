
/* PL: profle list */
var PL = {
  pl_path: "P",
  url_list: "/list",
  url_save: "/fputs",
  url_del: "/rm",
  url_load: "pl.php?ld=",
  div: "#profile-list-pane",
  shown: false,
  initialized: false,
  plist: [],
  path: function(a) {
    return "/" + this.pl_path + "/" + a
  },
  depath: function(a) {
    return a.substring(this.pl_path.length + 1)
  },
  rm: function(e) {
    var f = this;
    var c = "path=" + f.path(f.plist[e]);
    s_ajax({
      url: f.url_del,
      m: "DELETE",
      data: c,
      success: function(a) {
        f.plist.splice(e, 1);
        f.list(f.plist)
      },
      fail: function(a) {
        alert("failed:" + a)
      }
    })
  },
  load: function(e) {
    var f = this;
    var c = f.path(f.plist[e]);
    s_ajax({
      url: c,
      m: "GET",
      success: function(b) {
        var a = JSON.parse(b);
        profileEditor.loadProfile(a);
      },
      fail: function(a) {
        //alert("failed:" + a);
      }
    })
  },
  list: function(i) {
    var a = this;
    var h = Q(a.div).querySelector("tbody");
    var e;
    while (e = h.querySelector("tr:nth-of-type(1)")) {
      h.removeChild(e)
    }
    var b = a.row;
    i.forEach(function(f, g) {
      var c = b.cloneNode(true);
      c.querySelector(".profile-name").innerHTML = f;
      c.querySelector(".profile-name").onclick = function(j) {
        j.preventDefault();
        a.load(g);
        return false
      };
      c.querySelector(".rmbutton").onclick = function() {
        a.rm(g)
      };
      h.appendChild(c)
    })
  },
  append: function(b) {
    if (!this.initialized) {
      return
    }
    this.plist.push(b);
    this.list(this.plist)
  },
  init: function() {
    var a = this;
    a.initialized = true;
    a.row = Q(a.div).querySelector("tr:nth-of-type(1)");
    a.row.parentNode.removeChild(a.row);
    s_ajax({
      url: a.url_list,
      m: "POST",
      data: "dir=" + a.path(""),
      success: function(c) {
        a.plist = [];
        var b = JSON.parse(c);
        b.forEach(function(e) {
          if (e.type == "file") {
            a.plist.push(a.depath(e.name))
          }
        });
        a.list(a.plist)
      },
      fail: function(b) {
        alert("failed:" + b)
      }
    })
  },
  toggle: function() {
    if (!this.initialized) {
      this.init()
    }
    this.shown = !this.shown;
    if (this.shown) {
      Q(this.div).style.left = "0px"
    } else {
      Q(this.div).style.left = "-300px"
    }
  },
  saveas: function() {
    Q("#dlg_saveas").style.display = "block"
  },
  cancelSave: function() {
    Q("#dlg_saveas").style.display = "none"
  },
  doSave: function() {
    var e = Q("#dlg_saveas input").value;
    if (e == "") {
      return
    }
    if (e.match(/[\W]/g)) {
      return
    }
    var g = profileEditor.getProfile();
    if (g === false) {
      alert("invalid value. check again");
      return
    }
    var f = this;
    var c = "path=" + f.path(e) + "&content=" + encodeURIComponent(JSON.stringify(g));
    var f = this;
    s_ajax({
      url: f.url_save,
      m: "POST",
      data: c,
      success: function(a) {
        f.append(e);
        f.cancelSave()
      },
      fail: function(a) {
        alert("failed:" + a)
      }
    })
  }
};
/* end of PL*/
