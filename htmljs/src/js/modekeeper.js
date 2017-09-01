var modekeeper={
  initiated:false,
  modes:["profile","beer","fridge","off"],
  cmode:0,
  dselect:function(m){
    var d=document.getElementById(m + "-m");
    var nc=document.getElementById(m + "-m").className.replace(/\snav-selected/, '');
    d.className=nc;

    document.getElementById(m + "-s").style.display="none";
  },
  select:function(m){
    document.getElementById(m + "-m").className += ' nav-selected';
    document.getElementById(m + "-s").style.display="block";
  },
  init:function(){
    var me=this;
    if(me.initiated) return;
    me.initiated=true;
    for (var i=0;i<4;i++){
      var m=me.modes[i];
      document.getElementById(m + "-s").style.display="none";
      document.getElementById(m + "-m").onclick=function(){
        var tm=this.id.replace(/-m/,'');
        me.dselect(me.cmode);
        me.select(tm);
        me.cmode=tm;
      };
    }
    me.cmode="profile";
    me.select(me.cmode);
  },
  apply:function(){
    if(!BrewPiSetting.valid){
      alert("Not connected to controller.");
      //		return;
    }
    if((this.cmode == "beer") || (this.cmode == "fridge")){
      var v= document.getElementById(this.cmode + "-t").value;
      if(v == '' || isNaN(v) ||( v > BrewPiSetting.maxDegree || v < BrewPiSetting.minDegree)){
        alert("Invalid Temperature:"+v);
        return;
      }
      if(this.cmode == "beer"){
        //console.log("j{mode:b, beerSet:" + v+ "}");
        BWF.send("j{mode:b, beerSet:" + v+ "}");
      }else{
        console.log("j{mode:f, fridgeSet:" + v+ "}");
        BWF.send("j{mode:f, fridgeSet:" + v+ "}");
      }
    }else if(this.cmode == "off"){
      //console.log("j{mode:o}");
      BWF.send("j{mode:o}");
    } else{
      // should save first.
      if(profileEditor.dirty){
        alert("save the profile first before apply");
        return;
      }
      //console.log("j{mode:p}");
      document.getElementById('dlg_beerprofilereminder').style.display = "block";
      document.getElementById('dlg_beerprofilereminder').querySelector("button.ok").onclick=function(){
        document.getElementById('dlg_beerprofilereminder').style.display = "none";
        var gravity=parseFloat(Q("#dlg_beerprofilereminder input").value);
        updateOriginGravity(gravity);
        var data={name:"webjs",og:1,gravity:gravity};
        s_ajax({
          url:"gravity", m:"POST",mime:"application/json",
          data:JSON.stringify(data),
          success:function(d){
            BWF.send("j{mode:p}");
          },
          fail:function(d){
            alert("failed:"+d);
          }});
        };
        document.getElementById('dlg_beerprofilereminder').querySelector("button.oknog").onclick=function(){
          document.getElementById('dlg_beerprofilereminder').style.display = "none";
          BWF.send("j{mode:p}");
        };
        document.getElementById('dlg_beerprofilereminder').querySelector("button.cancel").onclick=function(){
          document.getElementById('dlg_beerprofilereminder').style.display = "none";
        };
      }
    }
  };
