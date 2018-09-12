BrewChart.prototype.partial = function(start, end) {
    var me = this;

    var srow = me.findNearestRow(this.chart, start);
    var erow = me.findNearestRow(this.chart, end);
    var data = [];
    var VERTAG = 5;
    var st = Math.round(start / 1000);

    // create a header
    // header 
    var tag = VERTAG | (me.celius ? 0xE0 : 0xF0);
    if (me.calibrating) tag = tag ^ 0x20;
    // headerx4
    var header = new Uint8Array([0xFF, tag, me.interval >> 8, me.interval & 0xFF,
        (st >> 24) & 0xFF, (st >> 16) & 0xFF, (st >> 8) & 0xFF, st & 0xFF
    ]);

    //  Period Record x 2
    data.push(header);
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
        var sgint = Math.round(g * 10000);
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
            me.rawSG[row],
            me.angles[row]
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
                        // gravity 
                        // The gravit data is user-input or calculated by BPL. 
                        var sg = currentValues[6];
                        if (sg && !isNaN(sg)) {
                            rec = rec.concat(encodeGravity(sg));
                        }
                    } else {
                        var tilt = currentValues[7];
                        if (tilt && !isNaN(tilt)) {
                            rec = rec.concat(encodeTilt(tilt));
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

BrewChart.prototype.partial2Plato = function(start, end) {
    var me = this;

    var srow = me.findNearestRow(this.chart, start);
    var erow = me.findNearestRow(this.chart, end);
    var data = [];
    var VERTAG = 5;
    var st = Math.round(start / 1000);

    // create a header
    // header 
    var tag = VERTAG | (me.celius ? 0xE0 : 0xF0);
    if (me.calibrating) tag = tag ^ 0x20;
    tag = tag ^ 0x40; // force plato
    // headerx4
    var header = new Uint8Array([0xFF, tag, me.interval >> 8, me.interval & 0xFF,
        (st >> 24) & 0xFF, (st >> 16) & 0xFF, (st >> 8) & 0xFF, st & 0xFF
    ]);

    //  Period Record x 2
    data.push(header);
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

        var plato = BrewMath.sg2pla(g);
        var sgint = Math.round(plato * 100);
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
            me.rawSG[row],
            me.angles[row]
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
                        // gravity 
                        // The gravit data is user-input or calculated by BPL. 
                        var sg = currentValues[6];
                        if (sg && !isNaN(sg)) {
                            rec = rec.concat(encodeGravity(sg));
                        }
                    } else {
                        var tilt = currentValues[7];
                        if (tilt && !isNaN(tilt)) {
                            rec = rec.concat(encodeTilt(tilt));
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