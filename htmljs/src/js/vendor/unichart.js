

function UniBrewChart(div)
{
    BrewChart.call(this, div);
}
UniBrewChart.prototype = new BrewChart();
UniBrewChart.prototype.constructor = UniBrewChart;

UniBrewChart.testData = function(data) {
    if (data[0] != 0xFF) return false;
    var s = data[1] & 0x07;
    if (s != 5 && s!=6 && s!=7) return false;

    return {
        sensor: s,
        f: data[1] & 0x10
    };
};

UniBrewChart.prototype.process=function(data){
    var s = data[1] & 0x07;
    this.version = s;
    if (s ==6  || s==7){
        BrewChart.prototype.process.call(this,data);
    }else if (s ==5){
        this.psi = [];
        this.process_v5(data);
    }
};


UniBrewChart.prototype.process_v5 = function(data) {

    var GravityIndex = 6;
    var TiltAngleIndex = 7;

    var newchart = false;
    var sgPoint = false;
    var t = this;
    //t.raw = data;
    t.filterSg = null;
    for (var i = 0; i < data.length;) {
        var d0 = data[i++];
        var d1 = data[i++];
        if (d0 == 0xFF) { // header. 
            if ((d1 & 0xF) != 5) {
                alert("<%= script_log_version_mismatched %>");
                return;
            }
            //console.log(""+t.ctime/t.interval +" header");
            t.celius = (d1 & 0x10) ? false : true;
            t.calibrating = (d1 & 0x20) ? false : true;
            t.plato = (d1 & 0x40) ? false : true;

            var p = data[i++];
            p = p * 256 + data[i++];
            t.interval = p;
            // 
            t.starttime = (data[i] << 24) + (data[i + 1] << 16) + (data[i + 2] << 8) + data[i + 3];
            t.ctime = t.starttime;
            i += 4;
            t.data = [];
            t.anno = [];
            t.state = [];
            t.angles = [];
            t.rawSG = [];
            t.cstate = 0;
            t.coTemp = 20;
            t.cal_igmask = 0;
            t.gravityChanges=[];

            this.clearData();
            newchart = true;
            // gravity tracking
            GravityFilter.reset();
            // gravity tracking
        } else if (d0 == 0xF3) { // correction temperature
            t.coTemp = d1; // always celisus
        } else if (d0 == 0xF4) { // mode
            //console.log(""+t.ctime/t.interval +" Stage:"+d1);
            t.addMode(d1, t.ctime * 1000);
        } else if (d0 == 0xF1) { // state
            t.cstate = d1;
        } else if (d0 == 0xFE) { // resume
            t.lidx = 0;
            var d2 = data[i++];
            var d3 = data[i++];
            var tdiff = d3 + (d2 << 8) + (d1 << 16);
            if(tdiff > 30*24*60*60) tdiff= 30*60; // it's wrong if it's too long.
            var ntime = t.starttime + tdiff;
            if (ntime > t.ctime) {
                // add a gap to it                   
                t.data.push([new Date(t.ctime * 1000), NaN, NaN, NaN, NaN, NaN, NaN, NaN, NaN]);
                t.state.push(null);
                t.angles.push(null);
                t.rawSG.push[null];

                if (ntime - t.ctime > t.interval)
                    t.ctime = ntime;
                else
                    t.ctime += t.interval;
            }
            t.addResume(d1);
            // drop the data
            /*
            if (t.lidx) {
                var idx;
                for (idx = t.lidx; idx < t.numLine; idx++) t.dataset.push(NaN);
                t.data.push(t.dataset);
                t.state.push(null);
                t.angles.push(null);
                t.rawSG.push[null];
            }*/

        } else if (d0 == 0xF8) { //OG
            var hh = data[i++];
            var ll = data[i++];
            var v = (hh & 0x7F) * 256 + ll;
            t.og = t.plato ? v / 100 : v / 10000;
        } else if (d0 == 0xFA) { //Ignored mask
            var b2 = data[i++];
            var b3 = data[i++];
            t.cal_igmask = (d1 << 14) + (b2 << 7) + b3;
        } else if (d0 == 0xF9) { //Tilt in water
            var hh = data[i++];
            var ll = data[i++];
            var v = (hh & 0x7F) * 256 + ll;
            t.tiltInWater = v / 100;
            //
            if (t.plato) t.readingInWater = 0;
            else t.readingInWater = (d1 == 0) ? 1.0 : (0.9 + d1 / 1000);
        } else if (d0 == 0xF0) { // record
            t.changes = d1;
            t.lidx = 0;
            var d = new Date(this.ctime * 1000);
            //t.incTime(); // add one time interval
            t.dataset = [d];
            t.processRecord_v5();
        } else if (d0 < 128) { // temp. or gravity
            var tp = d0 * 256 + d1;
            if (t.lidx == GravityIndex) {
                tp = (tp == 0x7FFF) ? NaN : (t.plato ? tp / 100 : ((tp > 8000) ? tp / 10000 : tp / 1000));
                sgPoint = true;
                // gravity tracking
            } else if (t.lidx == TiltAngleIndex) {
                tp = (tp == 0x7FFF) ? NaN : (tp / 100);
            } else {
                tp = (tp == 0x7FFF) ? NaN : tp / 100;
                if (tp >= 225) tp = 225 - tp;
            }

            if (t.lidx < t.numData) {
                if (typeof t.dataset != "undefined") {
                    t.dataset.push(tp);
                    t.laststat[t.lidx] = tp;
                    t.lidx++;
                    t.processRecord_v5();
                } else {
                    console.log("Error: missing tag.");
                }
            } else {
                console.log("Error: data overlap?");
            }
        }
    }
    return {
        nc: newchart,
        sg: sgPoint
    };
};

UniBrewChart.prototype.processRecord_v5 = function() {
    var t = this;
    while ((((1 << t.lidx) & t.changes) == 0) && t.lidx < t.numData) {
        t.dataset.push((t.lidx > RoomTemperatureIndex) ? null : t.laststat[t.lidx]);
        t.lidx++;
    }
    if (t.lidx >= t.numData) {
        var dataset = t.dataset.slice(0, 8);
        var rawSG = t.dataset[GravityLine];
        // gravity tracking
        var sg = NaN;
        if (!t.calculateSG && t.dataset[GravityLine] != null) {
            sg = t.dataset[GravityLine];
        } else if (t.calculateSG) {
            // data field #8 is tilt in source data
            if (t.dataset[8] == null) dataset[GravityLine] = null;
            else {
                var temp = (this.celius) ? C2F(dataset[AuxTempLine]) : dataset[AuxTempLine];
                sg = t.sgByTilt(t.dataset[8]);

                if (t.plato) {
                    sg = BrewMath.sg2pla(BrewMath.tempCorrectionF(BrewMath.pla2sg(sg), temp, C2F(t.coTemp)));
                }
                dataset[GravityLine] = sg;
            }
        }
        if (!isNaN(sg)) {
            t.sg = sg;
            t.filterSg = GravityFilter.add(sg);
        }

        if (!isNaN(t.sg)) dataset.push(t.filterSg);
        else dataset.push(null);

        t.data.push(dataset);
        t.state.push(t.cstate);
        t.angles.push(t.dataset[8]);
        t.rawSG.push(rawSG);

        if(t.GravityChangeChart){
            function GD(p){
                var v=t.getGravityBefore(p);
                if(isNaN(v)) return NaN;
                if(t.plato) return v-t.filterSg;
                return (v-t.filterSg)*1000;
            }

            if (!isNaN(sg)){
                t.gravityChanges.push([t.dataset[0],
                    GD(GravityChangePeriod1),
                    GD(GravityChangePeriod2),
                    GD(GravityChangePeriod3)
                    ]);
                
            } else t.gravityChanges.push([t.dataset[0], null,null,null]);
        }

        t.incTime(); // add one time interval
    }
};