
BrewChart.prototype.calInfo=function(){
    var t=this;
    var points=(t.version == CHART_V6)? t.getCalibration(): t.calpoints;

    var cdata=new Uint8Array(2 + points.length * 4);
    var idx=0;
    cdata[idx++]=0xF3;
    cdata[idx++]=points.length;
    for(var i=0;i<points.length;i++){
        var raw =Math.round((t.devType == 2)? points[i][0]*10000:points[i][0]*100);
        var gravity =Math.round(t.plato? points[i][1]*100:points[i][1]*10000);
        cdata[idx++]= (raw >> 8) & 0xFF;
        cdata[idx++]= raw & 0xFF;
        cdata[idx++]= (gravity >> 8) & 0xFF;
        cdata[idx++]= gravity & 0xFF;
    }
    return cdata;
}
BrewChart.prototype.partial = function(start, end) {
    var me = this;

    var srow = me.findNearestRow(this.chart, start);
    var erow = me.findNearestRow(this.chart, end);
    var data = [];
    var VERTAG = 7;
    var st = Math.round(start / 1000);

    // create a header
    // header 
    var tag = VERTAG | (me.celius ? 0xE0 : 0xF0);
    if (me.calibrating) tag = tag ^ 0x20;
    if (me.plato) tag = tag ^ 0x40;
    // headerx4
    var header = new Uint8Array([0xFF, tag, me.interval >> 8, me.interval & 0xFF,
        (st >> 24) & 0xFF, (st >> 16) & 0xFF, (st >> 8) & 0xFF, st & 0xFF
    ]);

    //  Period Record x 2
    data.push(header);
    if(typeof this["savedDevInfo"] != "undefined") data.push(this.savedDevInfo);
    else data.push(new Uint8Array([1,1])); // default to iSpindel

    data.push(this.calInfo());
    /*
    #define OrderBeerSet 0
    #define OrderBeerTemp 1
    #define OrderFridgeTemp 2
    #define OrderFridgeSet 3
    #define OrderRoomTemp 4

    #define OrderExtTemp 5
    #define OrderGravity 6
    #define OrderTilt 7
    */
    function encodeTemp(t) {
        // valid temp range, 225 ~ -100 
        // 0 ~ 225: 
        // -100 ~ 0 :  226  - t  , maximum 32500 ( max uint16 32767)  
        if (isNaN(t)) return [0x7F, 0xFF];
        var it = parseInt(t * 100);
        if (it < 0) it = 22500 - it;
        return [(it >> 8) & 0x7F, it & 0xFF];
    }

    function encodeGravity(g) {
        if (isNaN(g)) return [0x7F, 0xFF];
        var sgint = me.plato? Math.round(plato * 100):Math.round(g * 10000);
        return [(sgint >> 8) & 0xFF, sgint & 0xFF];
    }

    function encodePressure(p) {
        if (isNaN(p) || p==null) return [0x7F, 0xFF];
        var sgint = Math.round(p * 10);
        return [(sgint >> 8) & 0xFF, sgint & 0xFF];
    }
    
    function encodeTilt(tlt) {
        //        if(isNaN(tlt)) return [0x7F,0xFF];
        var tltInt = tlt * 100;
        return [(tltInt >> 8) & 0xFF, tltInt & 0xFF];
    }
    var values = [null, null, null, null, null, null, null, null];

    function periodRecord(row) {
        var rec = [0xF0, 0x00];
        var currentValues = [
            me.chart.getValue(row, BeerSetLine),
            me.chart.getValue(row, BeerTempLine),
            me.chart.getValue(row, FridgeTempLine),
            me.chart.getValue(row, FridgeSetLine),
            me.chart.getValue(row, RoomTempLine),
            me.chart.getValue(row, AuxTempLine),
            me.angles[row],
            me.chart.getValue(row, PressureLine)
        ];
        var mask = 0;

        for (var i = 0; i < 8; i++) {
            if (values[i] != currentValues[i]) {
                mask = mask | (1 << i);
                if (i < 5) {
                    rec = rec.concat(encodeTemp(currentValues[i]));
                } else {
                    if (i == 5) {
                        var auxtemp = currentValues[5];
                        if (auxtemp && !isNaN(auxtemp)) {
                            rec = rec.concat(encodeTemp(auxtemp));
                        }
                    } else if (i == 6) {
                        // gravity, after v6, this might be tilt or gravity
                        // The gravit data is user-input or calculated by BPL. 
                        if(me.calibrating){
                            var tilt = currentValues[6];
                            if (tilt != null && !isNaN(tilt)) {
                                rec = rec.concat(encodeTilt(tilt));
                            }    
                        }else{
                            var sg = currentValues[6];
                            if (sg != null && !isNaN(sg)) {
                                rec = rec.concat(encodeGravity(sg));
                            }
                        }
                    } else { // pressure
                        var pressure = currentValues[7];
                        if (pressure != null && !isNaN(pressure)) {
                            rec = rec.concat(encodePressure(pressure));
                        }
                    }
                    currentValues[i] = null;
                }
            }
        }
        rec[1] = mask;
        values = currentValues;
        return rec;
    }
    data.push(new Uint8Array(periodRecord(srow)));
    // mode y state
    var brewpistate = me.state[srow];
    data.push(new Uint8Array([0xF4, me.getModeBeforeTime(start),
        0xF1, brewpistate
    ]));
    // OG, if exisist
    if (!isNaN(me.og)) {
        var og = [0xF8, 0];
        data.push(new Uint8Array(og.concat(encodeGravity(me.og))));
    }
    // tilt in water
    if (me.calibrating) {
        var twa = [0xF9, 0];
        data.push(new Uint8Array(twa.concat(encodeTilt(me.tiltInWater))));
    }
    var anno = this.anno;
    var aidx = 0;

    while (aidx < anno.length && anno[aidx].x <= start) aidx++;

    for (var r = srow + 1; r <= erow; r++) {
        // check annotation.
        var time = me.data[r][0].getTime();
        if (aidx < anno.length && time >= anno[aidx].x && anno[aidx].shortText == 'R') {
            tdiff = Math.round((time - start) / 1000);
            data.push(new Uint8Array([0xFE, (tdiff >> 16) & 0xFF, (tdiff >> 8) & 0xFF, tdiff && 0xFF]));
            aidx++;
        }

        data.push(new Uint8Array(periodRecord(r)));
        if (brewpistate != me.state[r]) {
            brewpistate = me.state[r];
            data.push(new Uint8Array([0xF1, brewpistate]));
        }

        if (aidx < anno.length && time >= anno[aidx].x && anno[aidx].shortText != 'R') {
            // mode
            data.push(new Uint8Array([0xF4, anno[aidx].shortText.charCodeAt(0)]));
            aidx++;
        }


        if(me.calibrating && me.rawSG[r] !=null && !isNaN(me.rawSG[r]) ) {
            var sg = [0xFB, 0];
            data.push(new Uint8Array(sg.concat(encodeGravity(me.rawSG[r]))));
        }
    
    
    
    }
    return data;
};

BrewChart.prototype.getModeBeforeTime = function(start) {
    var anno = this.anno;
    var i = 0;
    var mode = anno[0].shortText;
    var allmode = "OBPF";
    for (var i = 0; i < anno.length; i++) {
        if (anno[i].x > start) break;
        if (allmode.indexOf(anno[i].shortText) >= 0) {
            // mode annotiin
            mode = anno[i].shortText;
        }
    }
    return mode.charCodeAt(0);
};
BrewChart.prototype.end=function(){
    return this.data[this.data.length -1][0];
};
BrewChart.prototype.start=function(){
    return this.data[0][0];
};

BrewChart.prototype.getIndexOfTime = function(time) {
    var start =this.start();
    var end= this.end();
    if( time < start || time > end) return -1;
    return Math.round(this.data.length * (time - start)/(end - start));
}

BrewChart.prototype.getTiltAroundTime = function(time) {
    var idx= this.getIndexOfTime(time);
    if(idx < 0) return idx;
    return this.getTiltAround(idx)[0];
};

BrewChart.prototype.addCalibration=function(time,sg){
    var me=this;

    var idx=me.getIndexOfTime(time);
    var tidx;
    var max= me.data.length;
    for(var i=1;i<max;i++){
        tidx= idx -i;
        if(tidx > 0 && tidx < max){
            if(me.rawSG[tidx] == null){
                me.rawSG[tidx] = sg;
                return;
            }
        }
        tidx = idx +1;
        if(tidx > 0 && tidx < max){
            if(me.rawSG[tidx] == null){
                me.rawSG[tidx] = sg;
                return;
            }          
        }
    }
};