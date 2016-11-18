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
	//console.log("rcv:" + msg);
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
	//console.log("snd:" + data);
	var b=this;
	invoke({m:"POST", url:"/putline",mime:"application/x-www-form-urlencoded",
		data:"data="+encodeURI(data),
		success:function(){if(typeof opt.success !=="undefined") opt.success();},
		fail:function(a){if(typeof opt.fail !=="undefined") opt.fail(a); else b.error(a);}
	});
},
connect:function(){
	var b=this;
	var es = new EventSource("/getline");
	es.onmessage = function(e) {
		b.process(e.data);
	};
	es.onerror=function(){
		setTimeout(function(){
			b.connnect();
		},3100);
//		b.error(-2);
	};
	setTimeout(function(){
		b.onopen();
	},1000);
},
init:function(arg){
	var b=this;
	b.error = (typeof arg.error == "undefined")? function(){}:arg.error;
	b.handlers=(typeof arg.handlers == "undefined")? {}:arg.handlers;
	b.raw=(typeof arg.raw == "undefined")? null:arg.raw;
	b.onopen=(typeof arg.onopen == "undefined")? function(){}:arg.onopen;
	
	if(typeof EventSource ==="undefined") {
		console.log("not support SSE");
		b.sse=false;
		var p=(typeof arg.polling == "undefined")? 5000:arg.polling;
		// setup timer
		setInterval(function(){
			invoke({m:"GET",url:"/getline_p",
				success:function(d){b.process(d);},
				fail:function(a){b.error(a);}
			});
		},p);
		return;
	}
	b.connect();
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
