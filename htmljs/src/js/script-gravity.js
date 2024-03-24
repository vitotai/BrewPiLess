var gdcurl = "/gdc";
const DTTilt = 2;
const DTiSpindel=1;
const DTPill = 3;
const MaxCalPoint=10;

function toFixed() {
    var texts = document.querySelectorAll("input[type=text]");
    for (var i = 0; i < texts.length; i++) {
        texts[i].onchange = function() {
            if (this.value.match(/[\-\d\.]+e[\+\-][\d]+/))
                this.value = Number(this.value).toFixed(9);
        };
    }
}

function fromSetting(points){
    var pts=[];
    for(var i=0;i<points.length;i++){
        var raw = (window.dev==DTTilt)? points[i][0]/10000:points[i][0]/100;
        var gravity= window.plato? points[i][1]/100:points[i][1]/10000;
        pts.push([raw,gravity]);
    }
    return pts;
}

function toSetting(points){
    var pts=[];
    for(var i=0;i<points.length;i++){
        var raw = (window.dev==DTTilt)? points[i][0]*10000:points[i][0]*100;
        var gravity= window.plato? points[i][1]*100:points[i][1]*10000;
        pts.push([Math.round(raw),Math.round(gravity)]);
    }
    return pts;
}

function fill(setting) {
    window.device=setting.dev;
    window.plato =setting.plato;
    for (var name in setting) {
        var ele = Q("input[name=" + name + "]");
        if (ele) {
            if (ele.type == "checkbox") ele.checked = setting[name];
            else ele.value = setting[name];
        }
    }
    if(typeof setting["color"] != "undefined") Q("#tiltcolor").value = setting.color;
    if(typeof setting["calpts"] != "undefined"){
        TCEditor.setPoints(fromSetting(setting.calpts));
    }
    if(typeof setting["mac"]!= "undefined"){
        Q("#pilladdr").innerHTML = pillAddress(setting["mac"]);
        window.pMAC = setting["mac"];
    }
}

function getFormula(){
    var pts = TCEditor.getPoints();
    var coes;
    if(pts.length < 2){
        coes= [0,1,0,0];
    } else {
        var poly = regression('polynomial', pts, (pts.length > 3) ? 3 : ((pts.length > 2) ? 2 : 1), {
            precision: 9
        });
        coes= [
            poly.equation[0],
            poly.equation[1],
            (pts.length > 2)? poly.equation[2]:0,
            (pts.length > 3)? poly.equation[3]:0,
        ];
    }
    for(var i=0;i<4;i++) Q('input[name="a' + i +'"]').value = coes[i];
}

function save() {
    var inputs = document.getElementsByTagName("input");
    var setting = {};

    setting.dev = 1; // always iSpindel
    setting.calpts= toSetting(TCEditor.getPoints());

    for (var i = 0; i < inputs.length; i++) {
        var ele = inputs[i];
        if(ele.name != "" && ! ele.classList.contains('notsave') ){
            if (ele.type == "checkbox") setting[ele.name] = ele.checked;
            else if (ele.type == "text") setting[ele.name] = ele.value;
        }
    }
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


function validSG(g){
    if(g > 0.8 && g<1.5) return true;
    return false;
}
var TCEditor={
    dt:0,
    init:function(){
        var tr=Q("#tiltcalpoints tr.calpoint");
        tr.parentNode.removeChild(tr);
        this.tr=tr;
        this.points=[];
        Q("#btn-addcal").disabled=false;
        Q("#btn-addcal").onclick=function(){ TCEditor.add();};
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
    //    if(!validSG(uncal) || !validSG(cal)) return;
    
        this.points.push([uncal,cal]);
        this.remove_all();
        this.render();
    },
    remove_all:function(){
        doAll("#tiltcalpoints tr.calpoint",function(div){
            div.parentNode.removeChild(div);
        });
    },
    clear:function(){
        this.removeall();
        this.points=[];
    },
    newPoint:function(idx,data){
        var tr= this.tr.cloneNode(true);
        var t=this;
        tr.querySelector(".delbutton").onclick=function(e){
            e.preventDefault();
            console.log("del "+idx);
            t.points.splice(idx,1);
            t.remove_all();
            t.render();
            return false;
        };
    
        tr.querySelector(".uncaldata").innerHTML="" + data[0];
        tr.querySelector(".caldata").innerHTML="" + data[1];
    
        return tr;
    },
    
    render:function(){
        this.points.sort((a,b)=>{
            if(a[0] < b[0]) return -1;
            else if(a[0] > b[0]) return 1;
            else{
                if (a[1] < b[1]) return -1;
                else if (a[1] > b[1]) return 1;
                else return 0;
            }
        });
        for(var i=0;i< this.points.length;i++){
            var tr=this.newPoint(i,this.points[i]);
            Q("#tiltcalpoints tbody").appendChild(tr);
        }
        Q("#btn-addcal").disabled =(this.points.length >= MaxCalPoint);
    }
    };

function tiltcal(){
    Q("#dlg_addcalpoint").style.display="block";
}

function closeTiltCal(){
    Q("#dlg_addcalpoint").style.display="none";
    getFormula();
    return false;
}

function init(classic) {
    if (typeof classic == "undefined") classic = false;
    window.device=1; // default to NONE


    toFixed();
    s_ajax({
        url: gdcurl + "?data",
        m: "GET",
        success: function(a) {
            fill(JSON.parse(a));
        },
        fail: function(a) {
            alert("failed getting data:" + a)
        }
    });
    TCEditor.init();
    
    if (!classic) {
        getActiveNavItem();
        Q("#verinfo").innerHTML = "v" + JSVERSION;
    }
}


