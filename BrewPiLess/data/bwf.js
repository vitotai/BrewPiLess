function invoke(arg)
{
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
    	if (xhttp.readyState == 4){
    	 	if(xhttp.status == 200) {
    	 		arg.success(xhttp.responseText);
    	 	}else{
    	 		xhttp.onerror(xhttp.status);
    	 	}
    	 }
	};

	xhttp.ontimeout=function(){
    	if(typeof arg["timeout"] != "undefined")
    		arg.timeout();
    	else
    		xhttp.onerror(-1);
	},
  	xhttp.onerror=function(a){
    	if(typeof arg["fail"] != "undefined")
    		arg.fail(a);
	};
	
  	xhttp.open(arg.m,arg.url, true);
  	if(typeof arg["data"] != "undefined")
  	{
	  	xhttp.setRequestHeader("Content-Type",(typeof arg["mime"] != "undefined")? arg["mime"]:"application/x-www-form-urlencoded");
  		xhttp.send(arg.data);
  	}
  	else
	  	xhttp.send();
}

var BWF={
BrewProfile:"/brewing.json",
sse:true,
process:function(msg){
	if(this.raw !=null){
		this.raw(msg);
		return;
	}
	console.log("rcv:" + msg);
	eval("m={" + msg + "}");
//	console.log("json:"+m);
	for(var key in m){ 
		if(typeof this.handlers[key] != "undefined"){
			this.handlers[key](m[key]);
		}
	}
},
on:function(lb,handler){
	this.handlers[lb]=handler; 
},
send:function(data,opt){
	opt = (typeof opt == "undefined")? {}:opt;
	console.log("snd:" + data);
	var b=this;
	b.ws.send(data);
},
init:function(arg){
	var b=this;
	b.error = (typeof arg.error == "undefined")? function(e){alert("error:"+e);}:arg.error;
	b.handlers=(typeof arg.handlers == "undefined")? {}:arg.handlers;
	b.raw=(typeof arg.raw == "undefined")? null:arg.raw;
	b.onopen=(typeof arg.onopen == "undefined")? function(){}:arg.onopen;
	b.onclose=(typeof arg.onopen == "undefined")? function(){console.log("connection close");}:arg.onclose;
	
	if(typeof WebSocket ==="undefined") {
		console.log("not support S");
		alert("WebSocket not Supporte");
		return;
	}
	b.ws= new WebSocket('ws://'+document.location.host+'/websocket');

    b.ws.onopen = function(){
       console.log("Connected");
       b.onopen();
    };
    b.ws.onclose = function(){
      console.log("Disconnected");
      b.onclose();
    };
    b.ws.onerror = function(e){
        console.log("ws error", e);
        b.error(e);
    };
    b.ws.onmessage = function(e){
		b.process(e.data);
	};
},
save:function(file,data,success,fail){
	invoke({m:"POST",url:"/fputs",
		data:"path=" + file +"&content="+data,
		success:function(){success();},
		fail:function(e){fail(e);}
		});
},
load:function(file,success,fail){
	invoke({m:"GET",url:file,
		success:function(d){success(d);},
		fail:function(e){ fail(e);}
	});
}
};
