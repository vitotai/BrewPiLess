var gdcurl = "/gdc";

function toFixed() {
    var texts = document.querySelectorAll("input[type=text]");
    for (var i = 0; i < texts.length; i++) {
        texts[i].onchange = function() {
            if (this.value.match(/[\-\d\.]+e[\+\-][\d]+/))
                this.value = Number(this.value).toFixed(9);
        };
    }
}
function ptcConvert(pts,en){
    var ret=[];
    pts.forEach(function(pt){
        if(en) ret.push([pt[0]*1000,pt[1]*1000]);
        else ret.push([pt[0]/1000,pt[1]/1000]);
    });
    return ret;
}

function fill(setting) {
    for (var name in setting) {
        var ele = Q("input[name=" + name + "]");
        if (ele) {
            if (ele.type == "checkbox") ele.checked = setting[name];
            else ele.value = setting[name];
        }
    }
    if(setting.dev == 1){
        Q("#tilthydrometer").checked = false;
        Q("#ispindel").checked = true;
    }else if(setting.dev == 2){
        Q("#tilthydrometer").checked = true;
        Q("#ispindel").checked = false;
    }else{
        Q("#tilthydrometer").checked = false;
        Q("#ispindel").checked = false;
    }
    if(typeof setting["color"] != "undefined") Q("#tiltcolor").value = setting.color;
    if(typeof setting["tcpts"] != "undefined"){
        TCEditor.setPoints(ptcConvert(setting.tcpts,false));
    }
}

function getTiltCals(pts){
    if(pts.length ==0){
        return [0,1,0,0];
    } else if(pts.length ==1){
        return [pts[0][1] - pts[0][0] ,1,0,0];
    } else {
        var poly = regression('polynomial', pts, (pts.length > 3) ? 3 : ((pts.length > 2) ? 2 : 1), {
            precision: 9
        });
        return [
            poly.equation[0],
            poly.equation[1],
            (pts.length > 2)? poly.equation[2]:0,
            (pts.length > 3)? poly.equation[3]:0,
        ];
    }
}

function save() {
    var inputs = document.getElementsByTagName("input");
    var setting = {};
    for (var i = 0; i < inputs.length; i++) {
        var ele = inputs[i];
        if(ele.name != "ispindel" && ele.name!="tilthydrometer"){
            if (ele.type == "checkbox") setting[ele.name] = ele.checked;
            else if (ele.type == "text") setting[ele.name] = ele.value;
        }
    }
    setting.color =  Q("#tiltcolor").value;
    setting.tcpts= ptcConvert(TCEditor.getPoints(),1);
    setting.tiltcoe = getTiltCals(TCEditor.getPoints());
    setting.dev = Q("#tilthydrometer").checked? 2:(Q("#ispindel").checked? 1:0);
    //    console.log("result=" + setting);
    s_ajax({
        url: gdcurl,
        m: "POST",
        mime: "aplication/json",
        data: JSON.stringify(setting),
        success: function(a) {
            alert("<%= done %>");
        },
        fail: function(a) {
            alert("<%= script_control_failed_updating_data %>" + a)
        }
    });
}

function displaysub(){
    if (Q("#tilthydrometer").checked){
        doAll(".tiltsub",function(div){
            div.style.display="";
        });
    }else{
        doAll(".tiltsub",function(div){
            div.style.display="none";
        });
    }

    if (Q("#ispindel").checked){
        doAll(".ispindelsub",function(div){
            div.style.display="";
        });        
    }else{
        doAll(".ispindelsub",function(div){
            div.style.display="none";
        });        
    }
}
function validSG(g){
    if(g > 0.8 && g<1.5) return true;
    return false;
}
var TCEditor={
init:function(){
    var tr=Q("#tiltcalpoints tr.calpoint");
    tr.parentNode.removeChild(tr);
    this.tr=tr;
    this.points=[];
},
getPoints:function(){
    return this.points;
},
setPoints:function(points){
    this.points=points;
    this.render();
},
add:function(){
    var uncal = parseFloat(Q("#uncalvalue").value);
    var cal = parseFloat(Q("#calvalue").value);
    if(!validSG(uncal) || !validSG(cal)) return;

    this.points.push([uncal,cal]);
    Q("#tiltcalpoints tbody").appendChild(this.newPoint(this.points.length-1 ,[uncal,cal]));
},
clear:function(){
    doAll("#tiltcalpoints tr.calpoint",function(div){
        div.parentNode.removeChild(div);
    });
    this.points=[];
},
newPoint:function(idx,data){
    var tr= this.tr.cloneNode(true);
    var t=this;
    tr.querySelector(".delbutton").onclick=function(e){
        e.preventDefault();
        console.log("del "+idx);
        t.points.splice(idx,1);
        doAll("#tiltcalpoints tr.calpoint",function(div){
            div.parentNode.removeChild(div);
        });
        t.render();
        return false;
    };

    tr.querySelector(".uncaldata").innerHTML="" + data[0];
    tr.querySelector(".caldata").innerHTML="" + data[1];

    return tr;
},

render:function(){
    for(var i=0;i< this.points.length;i++){
        var tr=this.newPoint(i,this.points[i]);
        Q("#tiltcalpoints tbody").appendChild(tr);
    }
}
};

function tiltcal(){
    Q("#dlg_addcalpoint").style.display="block";
    readTilt();
}
function closeTiltCal(){
    Q("#dlg_addcalpoint").style.display="none";
    return false;
}
function scan(){
    s_ajax({
        url: "/tcmd?scan=1",
        success: function(a) {
        },
        fail: function(a) {
            Q("#scanreading").innerHTML="error requesting data";
            rescan();
        }
    });

}
function rescan(){
    setTimeout(function(){
        scan();
    },5000);
}
function scanResult(r){
    //"tilts:[{c:"color",r:"rssi",g:"gravity",t:"temperature"}]"
    var target = Q("#tiltcolor").value;
    var label =  Q("#tiltcolor").options[Q("#tiltcolor").selectedIndex].text;
    if(typeof r["tilts"]!= "undefined"){
        r.tilts.forEach(function(tilt){
            console.log("color:"+ tilt.c + " rssi:"+tilt.r+" gravity:"+tilt.g+" temp:"+tilt.t);
            if(tilt.c == target){
                var temp= (window.TU=="F")? tilt.t:F2C(tilt.t) ;
                Q("#scanreading").innerHTML="Tilt&trade; " +label + " : " + (tilt.g/1000) + " @ " + 
                   temp +"&deg;" + window.TU;
            }
        });
    }
}
function readTilt(){
    if(typeof window.connect != "undefined") return;
    window.connect=true;
    window.TU = 'F';
    BWF.init({
        //     reconnect: false,
        onconnect: function() {
            // send 
            scan();
        },
        error:function(e){
            Q("#scanreading").innerHTML="error connecting controller.";
        },
        handlers:{
            T:function(j){
                scanResult(j);
                rescan();                
            },
            A:function(info){
                if(typeof info["tu"] != "undefined") window.TU = info.tu;
            }
        }});
    return false;
}

function init(classic) {
    if (typeof classic == "undefined") classic = false;

    Q("#tilthydrometer").onchange=function(){
        if(this.checked) Q("#ispindel").checked = false;
        displaysub();
    };

    Q("#ispindel").onchange=function(){
        if(this.checked) Q("#tilthydrometer").checked = false;
        displaysub();
    };


    toFixed();
    s_ajax({
        url: gdcurl + "?data",
        m: "GET",
        success: function(a) {
            fill(JSON.parse(a));
            displaysub();
        },
        fail: function(a) {
            alert("failed getting data:" + a)
        }
    });
    TCEditor.init();
//    TCEditor.setPoints([[1.003,1.0],[1.063,1.060],[1.045,1.046]]);

    if (!classic) {
        getActiveNavItem();
        Q("#verinfo").innerHTML = "v" + JSVERSION;
    }

}