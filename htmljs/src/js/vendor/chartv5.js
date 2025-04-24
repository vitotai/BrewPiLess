        /* chart.js */
        // gravity tracking
        var GravityFilter = {
            b: 0.1,
            y: 0,
            reset: function() {
                this.y = 0;
            },
            add: function(gravity) {
                if (this.y == 0) this.y = gravity;
                else this.y = this.y + this.b * (gravity - this.y);
                return Math.round(this.y * 10000) / 10000;
            },
            setBeta: function(beta) {
                this.b = beta;
            }
        };
        var GravityTracker = {
            NumberOfSlots: 48,
            InvalidValue: 0xFF,
            ridx: 0,
            record: [],
            threshold: 1,
            setThreshold: function(t) {
                this.threshold = t;
            },
            addRecord: function(v) {
                this.record[this.ridx++] = v;
                if (this.ridx >= this.NumberOfSlots) this.ridx = 0;
            },
            stable: function(duration, to) {
                to = (typeof to == "undefined") ? this.threshold : to;
                var current = this.ridx - 1;
                if (current < 0) current = this.NumberOfSlots - 1;
                var previous = this.NumberOfSlots + this.ridx - duration;
                while (previous >= this.NumberOfSlots) previous -= this.NumberOfSlots;
                return (this.record[previous] - this.record[current]) <= to;
            },
            Period: 60 * 60,
            init: function() {
                this.curerntStart = 0;
                this.lastValue = 0;
            },
            add: function(gravity, time) {
                //gravity = Math.round(fgravity * 1000, 1);
                var timediff = time - this.curerntStart;

                if (timediff > this.Period) {
                    this.addRecord(gravity);
                    if (this.lastValue != 0) {
                        timediff -= this.Period;
                        while (timediff > this.Period) {
                            timediff -= this.Period;
                            this.addRecord(this.lastValue);
                        }
                    }
                    this.curerntStart = time;
                    this.lastValue = gravity;
                }
            }
        };


        function fgstate(duration) {
            var Color = {
                0: "red",
                12: "orange",
                24: "yellow",
                48: "green"
            };
            Q("#fgstate").style.backgroundColor = Color[duration];
        }

        function checkfgstate() {
            if (GravityTracker.stable(12)) {
                if (GravityTracker.stable(24)) {
                    if (GravityTracker.stable(48)) fgstate(48);
                    else fgstate(24); // 24
                } else fgstate(12); // 
            } else fgstate(0);
        }
        // gravity tracking
        var GravityIndex = 6;
        var TiltAngleIndex = 7;
        var RoomTemperatureIndex = 4;

        var BrewChart = function(div) {
            var t = this;
            t.cid = div;
            t.ctime = 0;
            t.interval = 60;
            t.numLine = 7;
            t.numData = 8;
            t.calculateSG = false;
            t.calibrating = false;

            t.lidx = 0;
            t.celius = true;
            t.clearData();
        };
        var colorIdle = "white";
        var colorCool = "rgba(0, 0, 255, 0.4)";
        var colorHeat = "rgba(255, 0, 0, 0.4)";
        var colorWaitingHeat = "rgba(255, 0, 0, 0.2)";
        var colorWaitingCool = "rgba(0, 0, 255, 0.2)";
        var colorHeatingMinTime = "rgba(255, 0, 0, 0.6)";
        var colorCoolingMinTime = "rgba(0, 0, 255, 0.6)";
        var colorWaitingPeakDetect = "rgba(0, 0, 0, 0.2)";
        var STATE_LINE_WIDTH = 15;
        var STATES = [{
            name: "IDLE",
            color: colorIdle,
            text: "<%= chart_state_idle %>"
        }, {
            name: "STATE_OFF",
            color: colorIdle,
            text: "<%= chart_state_off %>"
        }, {
            name: "DOOR_OPEN",
            color: "#eee",
            text: "<%= chart_state_door_Open %>",
            doorOpen: true
        }, {
            name: "HEATING",
            color: colorHeat,
            text: "<%= chart_state_heating %>"
        }, {
            name: "COOLING",
            color: colorCool,
            text: "<%= chart_state_cooling %>"
        }, {
            name: "WAITING_TO_COOL",
            color: colorWaitingCool,
            text: "<%= chart_state_wait_to_cool %>",
            waiting: true
        }, {
            name: "WAITING_TO_HEAT",
            color: colorWaitingHeat,
            text: "<%= chart_state_wait_to_heat %>",
            waiting: true
        }, {
            name: "WAITING_FOR_PEAK_DETECT",
            color: colorWaitingPeakDetect,
            text: "<%= chart_state_wait_for_peak %>",
            waiting: true
        }, {
            name: "COOLING_MIN_TIME",
            color: colorCoolingMinTime,
            text: "<%= chart_state_cooling_min_time %>",
            extending: true
        }, {
            name: "HEATING_MIN_TIME",
            color: colorHeatingMinTime,
            text: "<%= chart_state_heating_min_time %>",
            extending: true
        }, {
            name: "INVALID",
            color: colorHeatingMinTime,
            text: "<%= chart_state_invalid %>"
        }];
        BrewChart.Mode = {
            b: "Beer Constant",
            f: "Fridge Constant",
            o: "Off",
            p: "Profile"
        };
        BrewChart.Colors = ["rgb(240, 100, 100)", "rgb(41,170,41)", "rgb(89, 184, 255)", "rgb(255, 161, 76)", "#AAAAAA", "#f5e127", "rgb(153,0,153)", "#000abb"];
        BrewChart.Labels = ['Time', 'beerSet', 'beerTemp', 'fridgeTemp', 'fridgeSet', 'roomTemp', 'auxTemp', 'gravity', 'filtersg'];
        BrewChart.ClassLabels = ['', 'beer-set', 'beer-temp', 'fridge-temp', 'fridge-set', 'room-temp', 'aux-temp', 'gravity', 'filtersg'];
        var BeerTempLine = 2;
        var BeerSetLine = 1;
        var FridgeTempLine = 3;
        var FridgeSetLine = 4;
        var RoomTempLine = 5;
        var AuxTempLine = 6;
        var GravityLine = 7;
        var FilteredSgLine = 8;

        BrewChart.prototype.clearData = function() {
            this.laststat = [NaN, NaN, NaN, NaN, NaN, NaN, NaN, NaN];
            this.sg = NaN;
            this.og = NaN;
        };

        BrewChart.prototype.setCelius = function(c) {
            this.celius = c;
            this.ylabel(STR.ChartLabel + '(' + (c ? "°C" : "°F") + ')');
        };

        BrewChart.prototype.incTime = function() {
            // format time, use hour and minute only.
            this.ctime += this.interval;
            //	console.log("incTime:"+ this.ctime/this.interval);
        };

        BrewChart.prototype.formatDate = function(d) {
            var HH = d.getHours();
            var MM = d.getMinutes();
            var SS = d.getSeconds();

            function T(x) {
                return (x > 9) ? x : ("0" + x);
            }
            return d.toLocaleDateString() + " " + T(HH) + ":" + T(MM) + ":" + T(SS);
        };

        BrewChart.prototype.formatDuration = function(elapsed) {
            var str = "";
            var days = Math.floor(elapsed / 86400);
            if (days > 0) {
                str = days + "d";
                elapsed -= days * 86400;
            }
            var hours = elapsed / 3600;
            str = str + hours.toFixed(1) + "h";
            return str;
        };

        BrewChart.prototype.showLegend = function(date, row) {
            var d = new Date(date);
            Q(".beer-chart-legend-time").innerHTML = this.formatDate(d);
            if (Q(".beer-chart-legend-elapse")) Q(".beer-chart-legend-elapse").innerHTML = this.formatDuration(d.getTime() / 1000 - this.starttime);

            Q(".chart-legend-row.beer-temp .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, BeerTempLine));
            Q(".chart-legend-row.beer-set .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, BeerSetLine));
            Q(".chart-legend-row.fridge-temp .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, FridgeTempLine));
            Q(".chart-legend-row.fridge-set .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, FridgeSetLine));
            Q(".chart-legend-row.room-temp .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, RoomTempLine));

            Q(".chart-legend-row.aux-temp .legend-value").innerHTML = this.tempFormat(this.chart.getValue(row, AuxTempLine));

            var g = this.chart.getValue(row, GravityLine);
            Q(".chart-legend-row.gravity .legend-value").innerHTML = (!g || isNaN(g)) ? "--" : (this.plato ? g.toFixed(2) + "&deg;P" : g.toFixed(4));
            var filteredG = this.chart.getValue(row, FilteredSgLine);
            Q(".chart-legend-row.filtersg .legend-value").innerHTML = (!filteredG || isNaN(filteredG)) ? "--" : (this.plato ? filteredG.toFixed(2) + "&deg;P" : filteredG.toFixed(4));

            var state = parseInt(this.state[row]);
            if (!isNaN(state)) {
                Q('.beer-chart-state').innerHTML = STATES[state].text;
            }
        };

        BrewChart.prototype.hideLegend = function() {
            var v = document.querySelectorAll(".legend-value");
            v.forEach(function(val) {
                val.innerHTML = "--";
            });
            Q(".beer-chart-legend-time").innerHTML = this.dateLabel; //"Date/Time";
            Q('.beer-chart-state').innerHTML = "<%= chart_state_label %>";
        };

        BrewChart.prototype.tempFormat = function(y) {
            var v = parseFloat(y);
            if (isNaN(v)) return "--";
            var DEG = this.celius ? "&deg;C" : "&deg;F";
            return parseFloat(v).toFixed(2) + DEG;
        };
        BrewChart.prototype.initLegend = function() {
            // init color
            if (Q(".beer-temp .toggle")) {
                for (var i = 1; i < BrewChart.ClassLabels.length; i++) {
                    var label = BrewChart.ClassLabels[i];
                    Q(".chart-legend-row." + label).style.color = BrewChart.Colors[i - 1];
                    Q("." + label + ".toggle").style.backgroundColor = BrewChart.Colors[i - 1];
                }
            }
            this.dateLabel = Q(".beer-chart-legend-time").innerHTML;
        };
        BrewChart.prototype.toggleLine = function(line) {
            this.shownlist[line] = !this.shownlist[line];
            var divclass = BrewChart.ClassLabels[line];
            if (this.shownlist[line]) {
                if (Q("." + divclass + " .toggle")) Q("." + divclass + " .toggle").style.backgroundColor = Q(".chart-legend-row." + divclass).style.color;
                this.chart.setVisibility(line - 1, true);
            } else {
                if (Q("." + divclass + " .toggle")) Q("." + divclass + " .toggle").style.backgroundColor = "transparent";
                this.chart.setVisibility(line - 1, false);
            }
        };
        BrewChart.prototype.setLabels = function(y1, y2) {
            this.ylabel = y1;
            this.y2label = y2;
        };
        BrewChart.prototype.createChart = function() {
            var t = this;
            t.initLegend();
            t.shownlist = [true, true, true, true, true, true, true, true, true];

            var ldiv = document.createElement("div");
            ldiv.className = "hide";
            var ylabel = (t.ylabel ? t.ylabel : 'Temperature') + '(&deg;' + (t.celius ? 'C' : 'F') + ')';
            var y2label = t.y2label ? t.y2label : 'Gravity';
            document.body.appendChild(ldiv);
            var opt = {
                labels: BrewChart.Labels,
                colors: BrewChart.Colors,
                connectSeparatedPoints: true,
                ylabel: ylabel,
                y2label: y2label,
                series: {
                    'gravity': {
                        axis: 'y2',
                        drawPoints: true,
                        pointSize: 2,
                        highlightCircleSize: 4
                    },
                    'filtersg': {
                        axis: 'y2',
                    }
                },
                axisLabelFontSize: 12,
                animatedZooms: true,
                gridLineColor: '#ccc',
                gridLineWidth: '0.1px',
                labelsDiv: ldiv,
                labelsDivStyles: {
                    'display': 'none'
                },
                displayAnnotations: true,
                //showRangeSelector: true,
                strokeWidth: 1,
                axes: {
                    y: {
                        valueFormatter: function(y) {
                            return t.tempFormat(y);
                        }
                    },
                    y2: {
                        valueFormatter: function(y) {
                            return t.plato ? y.toFixed(1) : y.toFixed(3);
                        },
                        axisLabelFormatter: function(y) {
                            var range = this.yAxisRange(1);
                            if (t.plato) return (range[1] - range[0] > 1) ? y.toFixed(1) : y.toFixed(2);

                            if (range[1] - range[0] > 0.002)
                                return y.toFixed(3).substring(1);
                            else
                                return y.toFixed(4).substring(2);
                        }
                    }
                },
                highlightCircleSize: 2,
                highlightSeriesOpts: {
                    strokeWidth: 1.5,
                    strokeBorderWidth: 1,
                    highlightCircleSize: 5
                },
                highlightCallback: function(e, x, pts, row) {
                    t.showLegend(x, row);
                },
                unhighlightCallback: function(e) {
                    t.hideLegend();
                },
                underlayCallback: function(ctx, area, graph) {
                        ctx.save();
                        try {
                            t.drawBackground(ctx, area, graph);
                        } finally {
                            ctx.restore();
                        }
                    }
                    /*                drawCallback: function(beerChart, is_initial) {
                                        if (is_initial) {
                                            if (t.anno.length > 0) {
                                                t.chart.setAnnotations(t.anno);
                                            }
                                        }
                                    }*/
            };
            t.chart = new Dygraph(document.getElementById(t.cid), t.data, opt);
        };

        BrewChart.prototype.findNearestRow = function(g, time) {
            "use strict";
            var low = 0,
                high = g.numRows() - 1;
            var mid, comparison;

            while (low < high) {
                mid = Math.floor((low + high) / 2);
                comparison = g.getValue(mid, 0) - time;
                if (comparison < 0) {
                    low = mid + 1;
                    continue;
                }
                if (comparison > 0) {
                    high = mid - 1;
                    continue;
                }
                return mid;
            }
            return low;
        };
        BrewChart.prototype.findStateBlocks = function(g, start, end) {
            "use strict";
            var result = [];
            var state = this.state[start]; //getState(g, start);             // current state
            var newState;
            for (var i = start; i < end; i++) { // find the next change
                newState = this.state[i]; //getState(g, i);
                if (newState !== state) {
                    result.push({
                        row: i,
                        state: state
                    });
                    state = newState;
                }
            }
            result.push({
                row: end,
                state: state
            });
            return result;
        };
        BrewChart.prototype.getTime = function(g, row) {
            "use strict";
            if (row >= g.numRows()) {
                row = g.numRows() - 1;
            }
            return g.getValue(row, 0);
        };
        BrewChart.prototype.drawBackground = function(ctx, area, graph) {
            var timeStart = graph.toDataXCoord(area.x);
            var timeEnd = graph.toDataXCoord(area.x + area.w);
            // the data rows for the range we are interested in. 0-based index. This is deliberately extended out one row
            // to be sure the range is included
            var rowStart = Math.max(this.findNearestRow(graph, timeStart) - 1, 0);
            var rowEnd = this.findNearestRow(graph, timeEnd) + 1;
            if (rowStart === null || rowEnd === null) {
                return;
            }
            var blocks = this.findStateBlocks(graph, rowStart, rowEnd); // rowEnd is exclusive

            var startX = 0; // start drawing from 0 - the far left
            for (var i = 0; i < blocks.length; i++) {
                var block = blocks[i];
                var row = block.row; // where this state run ends
                var t = this.getTime(graph, row); // convert to time. Using time ensures the display matches the plotted resolution
                // of the graph.
                var r = (t - timeStart) / (timeEnd - timeStart); // as a fraction of the entire display
                var endX = Math.floor(area.x + (area.w * r));

                var state = STATES[parseInt(block.state, 10)];
                if (state === undefined) {
                    state = STATES[0];
                }
                //var borderColor = (state.waiting || state.extending) ? setAlphaFactor(state.color, 0.5) : undefined;
                //var bgColor = (state.waiting) ? bgColor = colorIdle : state.color;
                ctx.fillStyle = state.color;
                ctx.fillRect(startX, area.h - STATE_LINE_WIDTH, endX - startX, area.h);
                startX = endX;
            }
        };
        BrewChart.prototype.addMode = function(m, x) {
            var s = String.fromCharCode(m);
            this.anno.push({
                series: "beerTemp",
                x: x,
                shortText: s.toUpperCase(),
                text: BrewChart.Mode[s],
                attachAtBottom: true
            });
        };

        BrewChart.testData = function(data) {
            if (data[0] != 0xFF) return false;
            var s = data[1] & 0x07;
            if (s != 5) return false;

            return {
                sensor: s,
                f: data[1] & 0x10
            };
        };

        BrewChart.prototype.addResume = function(delta) {
            this.anno.push({
                series: "beerTemp",
                x: this.ctime * 1000,
                shortText: 'R',
                text: 'Resume',
                attachAtBottom: true
            });
        };

        BrewChart.prototype.getTiltAround = function(idx) {
            var t = this;
            var left = -1;
            var right = -1;

            if (t.angles[idx] != null) return [t.angles[idx], t.data[idx][AuxTempLine]];

            for (var i = idx - 1; i >= 0; i--) {
                if (t.angles[i] != null) {
                    left = i;
                    break;
                }
            }
            for (var i = idx + 1; i < t.angles.length > 0; i++) {
                if (t.angles[i] != null) {
                    right = i;
                    break;
                }
            }
            if (left < 0 && right < 0) return null;
            if (left < 0) return [t.angles[right], t.data[right][AuxTempLine]];
            if (right < 0) return [t.angles[left], t.data[left][AuxTempLine]];
            return [t.angles[left] + (t.angles[right] - t.angles[left]) / (right - left) * (idx - left),
                (t.data[left][AuxTempLine] + t.data[right][AuxTempLine]) / 2
            ];
        };

        BrewChart.prototype.getCalibration = function() {
            var pairs = [];
            for (var i = 0; i < this.data.length; i++) {
                if (this.data[i][GravityLine]) {
                    var data = this.getTiltAround(i);
                    // corrected the reading into current beer data
                    if (data) {
                        var beerTemp = this.celius ? C2F(data[1]) : data[1];
                        var gravity = this.data[i][GravityLine];
                        var converted;
                        if (this.plato)
                            converted = BrewMath.sg2pla(BrewMath.tempCorrectionF(BrewMath.pla2sg(gravity), C2F(this.coTemp), beerTemp));
                        else converted = BrewMath.tempCorrectionF(gravity, C2F(this.coTemp), beerTemp);
                        pairs.push([data[0], converted]);
                    }
                }
            }
            pairs.push([this.tiltInWater, this.readingInWater]);
            return pairs;
        };
        BrewChart.prototype.filterPoints = function(points, mask) {
            var nps = [];
            for (var i = 0; i < points.length; i++) {
                if (!(mask & (0x1 << i))) nps.push(points[i]);
            }
            return nps;
        };

        BrewChart.prototype.setIgnoredMask = function(mask) {
            if (this.cal_igmask == mask) return false;
            this.cal_igmask = mask;
            return true;
        };

        BrewChart.prototype.getFormula = function() {
            var points = this.getCalibration();
            if (points.length < 2) return;
            var cpoints = this.filterPoints(points, this.cal_igmask);
            if (cpoints.length < 2) {
                cpoints = points;
                this.cal_igmask = 0;
            }
            var poly = regression('polynomial', cpoints, (cpoints.length > 3) ?
                3 : ((cpoints.length > 2) ? 2 : 1), {
                    precision: 9
                });
            this.calibrationPoints = points;
            //this.equation = poly.equation;
            this.calculateSG = true;

            this.sgByTilt = (cpoints.length > 3) ?
                function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x +
                        poly.equation[2] * x * x +
                        poly.equation[3] * x * x * x;
                } : ((cpoints.length > 2) ? function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x +
                        poly.equation[2] * x * x;
                } : function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x;
                });

            this.coefficients = (cpoints.length > 3) ? [poly.equation[0], poly.equation[1], poly.equation[2], poly.equation[3]] :
                ((cpoints.length > 2) ? [poly.equation[0], poly.equation[1], poly.equation[2], 0] : [poly.equation[0], poly.equation[1], 0, 0]);
            this.npt = points.length;
        };
        BrewChart.prototype.process = function(data) {
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
                    this.clearData();
                    newchart = true;
                    // gravity tracking
                    GravityFilter.reset();
                    GravityTracker.init();
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
                    t.processRecord();
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
                            t.processRecord();
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
        BrewChart.prototype.getXRange = function() {
            if (typeof this.chart == "undefined") return [0, 0];
            return this.chart.xAxisRange();
        };
        BrewChart.prototype.setXRange = function(range) {
            if (typeof this.chart == "undefined") return;
            this.chart.updateOptions({ dateWindow: range });
        };
        BrewChart.prototype.updateChart = function() {
            var t = this;
            if (typeof t.chart == "undefined") t.createChart();
            else t.chart.updateOptions({
                'file': t.data
            });
            t.chart.setAnnotations(t.anno);
        };
        BrewChart.prototype.processRecord = function() {
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
                    if (t.plato)
                        GravityTracker.add(Math.round(t.filterSg * 10), t.ctime);
                    else
                        GravityTracker.add(Math.round(t.filterSg * 1000), t.ctime);
                }

                if (!isNaN(t.sg)) dataset.push(t.filterSg);
                else dataset.push(null);

                t.data.push(dataset);
                t.state.push(t.cstate);
                t.angles.push(t.dataset[8]);
                t.rawSG.push(rawSG);
                t.incTime(); // add one time interval
            }
        };
        /* end of chart.js */