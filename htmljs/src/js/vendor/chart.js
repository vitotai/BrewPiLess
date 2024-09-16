/* chart.js */
var  CHART_VERSION = 7;
var  CHART_V6 = 6;

var GravityChangePeriod1 = 6 * 3600;
var GravityChangePeriod2 = 12 * 3600;
var GravityChangePeriod3 =24 * 3600;
    function SimpleFilter(beta) {
        this.b = beta;
        this.y =0;
    }
    SimpleFilter.prototype.reset =function() {
        this.y = 0;
    };
    SimpleFilter.prototype.add= function(gravity) {
        if (this.y == 0) this.y = gravity;
        else this.y = this.y + this.b * (gravity - this.y);
        return this.y;
    };
    SimpleFilter.prototype.setBeta=function(beta) {
        this.b = beta;
    };

    var GravityFilter= new SimpleFilter(0.1);


        // gravity tracking
        var GravityAndTiltIndex = 6;
        var PSIIndex = 7;
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
            t.GravityChangeChart=false;
            t.data = [];
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

        // line colors

        var ColorBeerTemp="rgb(41,170,41)";
        var ColorBeerSet ="rgb(240, 100, 100)";
        var ColorFridgeTemp="rgb(89, 184, 255)";
        var ColorFridgeSet ="rgb(255, 161, 76)";
        var ColorRoomTemp = "#AAAAAA";
        var ColorAuxTemp =  "#f5e127";
        var ColorGravity="rgb(153,0,153)";
        var ColorFiltersg ="#000abb";

        var colorPressure="#0000EE";
        var colorPressureSet="rgb(240, 100, 100)";
        var colorCarbonation="gray";

        var colorHumidity="#2222DD";
        var colorHumiditySet="#EE1111";
        var colorRoomHumidity="#AAAAAA";
        var SgChange1Color="#DD33DD";
        var SgChange2Color="#3333DD";
        var SgChange3Color="#33DD33";

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



        BrewChart.Colors = [ColorBeerSet,ColorBeerTemp, ColorFridgeTemp, ColorFridgeSet, ColorRoomTemp, ColorAuxTemp,ColorGravity, ColorFiltersg,
                        colorPressure,colorPressureSet,colorCarbonation,
                        colorHumidity,colorHumiditySet,colorRoomHumidity,SgChange1Color,SgChange2Color,SgChange3Color];
        BrewChart.Labels = ['Time', 'beerSet', 'beerTemp', 'fridgeTemp', 'fridgeSet', 'roomTemp', 'auxTemp', 'gravity', 'filtersg'];
        BrewChart.ClassLabels = ['', 'beer-set', 'beer-temp', 'fridge-temp', 'fridge-set', 'room-temp', 'aux-temp', 'gravity', 'filtersg',
                            'pressure','pressure-set','carbonation',
                            'humidity','humidity-set','room-humidity','sgchange1','sgchange2','sgchange3'];

        var BeerSetLine = 1;
        var BeerTempLine = 2;
        var FridgeTempLine = 3;
        var FridgeSetLine = 4;
        var RoomTempLine = 5;
        var AuxTempLine = 6;
        var GravityLine = 7;
        var FilteredSgLine = 8;

        var PressureLine = 9;
        var PressureSetLine = 10;
        var CarbonationLine = 11;


        var ChamberHumidityLine = 12;
        var SetHumidityLine = 13;
        var RoomHumidityLine = 14;


        var SgChange1Line = 15;
        var SgChange2Line = 16;
        var SgChange3Line = 17;

        var NumberOfLines =17;



        var PSIDataIndex = 8;

        BrewChart.prototype.clearData = function() {
            this.laststat = [NaN, NaN, NaN, NaN, NaN, NaN, NaN, NaN,NaN];
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
            if(this.psiAvail){
                var psi = this.pchart.getValue(row, 1);
                Q(".chart-legend-row.pressure .legend-value").innerHTML = (psi == null || isNaN(psi))? "--":psi.toFixed(1);
                var psiSet=this.pchart.getValue(row, 2 );
                Q(".chart-legend-row.pressure-set .legend-value").innerHTML = (psiSet == null || isNaN(psiSet))? "--":Math.round(psiSet);
                var carbo=this.pchart.getValue(row, 3 );
                Q(".chart-legend-row.carbonation .legend-value").innerHTML = (carbo == null || isNaN(carbo))? "--":carbo.toFixed(1);
            }
            if(this.rhValid){
                var rh = this.hchart.getValue(row, 1);
                Q(".chart-legend-row.humidity .legend-value").innerHTML = (isNaN(rh) || rh == null || rh==255)? "--":(rh+"%");
                var sh = this.hchart.getValue(row, 2);
                Q(".chart-legend-row.set-humidity .legend-value").innerHTML = (isNaN(sh) || sh == null || sh==255)? "--":(sh+"%");
                var room = this.hchart.getValue(row, 3);
                Q(".chart-legend-row.room-humidity .legend-value").innerHTML = (isNaN(room) || room == null || room==255)? "--":(room+"%");
            }

            if(this.GravityChangeChart){
                var v1 = this.gcchart.getValue(row, 1);
                var v2 = this.gcchart.getValue(row, 2);
                var v3 = this.gcchart.getValue(row, 3);

                Q(".chart-legend-row.sgchange1 .legend-value").innerHTML = ( (isNaN(v1) || v1==null)? "--":v1.toFixed(1)); 
                Q(".chart-legend-row.sgchange2 .legend-value").innerHTML = ( (isNaN(v2) || v2==null)? "--":v2.toFixed(1)); 
                Q(".chart-legend-row.sgchange3 .legend-value").innerHTML = ( (isNaN(v3) || v3==null)? "--":v3.toFixed(1));                      
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
            var me=this;
            me.shownlist[line] = !me.shownlist[line];
            var divclass = BrewChart.ClassLabels[line];
            
            

            var chart =(line >=SgChange1Line)?   me.gcchart:((line >= ChamberHumidityLine)? me.hchart:(line >= PressureLine)? me.pchart:me.chart);
            var base=(line >=SgChange1Line)?    SgChange1Line:((line >= ChamberHumidityLine)? ChamberHumidityLine:(line >= PressureLine)? PressureLine:1);
            chart.setVisibility(line - base, me.shownlist[line]);

            if (Q("." + divclass + " .toggle")) Q("." + divclass + " .toggle").style.backgroundColor =
                (me.shownlist[line])? Q(".chart-legend-row." + divclass).style.color:"transparent";
        };
        BrewChart.prototype.setLabels = function(y1, y2) {
            this.ylabel = y1;
            this.y2label = y2;
        };
        BrewChart.prototype.setPChart = function(id,label,carbonation) {
            this.pcid=id;
            this.plabel=label;
            this.clabel=carbonation;
        };
        BrewChart.prototype.createPSIChart = function() {
            var t=this;
            var ldiv = document.createElement("div");
            ldiv.className = "hide";
            document.body.appendChild(ldiv);
            var opt = {
                labels: ["Time","psi","psiset","co2"],
                colors: BrewChart.Colors.slice(PressureLine-1,CarbonationLine),
                connectSeparatedPoints: true,
                ylabel: t.plabel,
                y2label: t.clabel,
                series: {
                    'co2': {
                        axis: 'y2',
                        drawPoints: false
                    }
                }, 

                axisLabelFontSize: 12,
//                animatedZooms: true,
                gridLineColor: '#ccc',
                gridLineWidth: '0.1px',
                labelsDiv: ldiv,
                labelsDivStyles: {
                    'display': 'none'
                },
                //displayAnnotations: true,
                //showRangeSelector: true,
                strokeWidth: 1,
                axes: {
                    y: {
                        valueFormatter: function(y) {
                            return y.toFixed(1);
                        },
                        axisLabelFormatter: function(y) {
                            return y.toFixed(1);
                        }
                    },
                    y2: {
                        valueFormatter: function(y) {
                            return y.toFixed(1);
                        },
                        axisLabelFormatter: function(y) {
                            return y.toFixed(1);
                        }
                    }
                }, 
                highlightCallback: function(e, x, pts, row) {
                    t.showLegend(x, row);
                },
                unhighlightCallback: function(e) {
                    t.hideLegend();
                }
            };
            t.pchart = new Dygraph(document.getElementById(t.pcid), t.psi, opt);
            t.pchart.setVisibility(0,true);
        };
        BrewChart.prototype.createChart = function() {
            var t = this;
            t.initLegend();
            t.shownlist =[];
            for(var i=0;i<=NumberOfLines;i++) t.shownlist.push(true);
            t.showPsi = true;
            var ldiv = document.createElement("div");
            ldiv.className = "hide";
            var ylabel = (t.ylabel ? t.ylabel : 'Temperature') + '(&deg;' + (t.celius ? 'C' : 'F') + ')';
            var y2label = t.y2label ? t.y2label : 'Gravity';
            document.body.appendChild(ldiv);
            var opt = {
                labels: BrewChart.Labels,
                colors: BrewChart.Colors.slice(0,FilteredSgLine),
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
/*                ,
                interactionModel:{ 
                    mousedown: Dygraph.defaultInteractionModel.mousedown, 
                    mousemove: Dygraph.defaultInteractionModel.mousemove, 
                    mouseup: Dygraph.defaultInteractionModel.mouseup, 
//                    touchstart:Dygraph.defaultInteractionModel.touchstart,
                    touchstart: function(event, g, context){
                        event.stopPropagation();
                        t.chart.setSelection(t.findNearestRow(g,t.chart.toDataXCoord(event.touches[0].clientX)));
                    }, 
                    touchend: Dygraph.defaultInteractionModel.mouseup, 
//                    touchend:function(event,g,context){
//                    },
//                    touchmove: Dygraph.defaultInteractionModel.touchmove
                    touchmove:function(event,g,context){
                        event.stopPropagation();
                        t.chart.setSelection(t.findNearestRow(g,t.chart.toDataXCoord(event.touches[0].clientX)));
                    }
                }
*/
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
        BrewChart.prototype.getGravityOfTime=function(time,filtered=true){
            if(typeof filtered == "undefined") filtered=true;
            var duration =  this.ctime - time;
            return this.getGravityBefore(duration>0? duration:0,filtered);
        };
        BrewChart.prototype.getGravityBefore=function(duration,filtered){
            if(typeof filtered == "undefined") filtered=true;
            var row = this.data.length - Math.round(duration/this.interval);
            var index=filtered? FilteredSgLine:GravityLine;
            while(row>=0){
                var gravity=this.data[row][index];
                if(gravity != null) return gravity;
                row--;
            }
            return NaN;
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
            if (s != CHART_VERSION && s!= CHART_V6) return false;

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
                if (this.rawSG[i]) {
                    var data = this.getTiltAround(i);
                    // corrected the reading into current beer data
                    if (data) {
                        var beerTemp = this.celius ? C2F(data[1]) : data[1];
                        var gravity = this.rawSG[i];
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
            var points=(this.version <= CHART_V6)? this.getCalibration(): this.calpoints;

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
        BrewChart.prototype.deviceInfo=function(data){
            // tlv.
            var i=0;
            while(i<data.length){
                var type=data[i++];
                var length=data[i++];
                if(type == 1){ // address
                    var address=[];
                    for(var a=0;a<length;a++){
                        address.push(data[i+a]);
                    }
                    this.address = address;
                }else if (type ==2){ // name
                    var devname="";
                    for(var a=0;a<length;a++){
                        devname = devname + String.fromCharCode(data[i+a]);
                    }
                    this.devName = devname;
                }
                i+= length;
            }
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
                    t.version = d1 & 0xF; 
                    if (t.version != CHART_VERSION && t.version != CHART_V6) {
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

                    t.devType = 0; // default to none
                    if(t.version == 7){
                        var didx = i;
                        var bsize = data[i++];
                        t.devType = data[i++];
                        // save this block for future used
                        t.savedDevInfo=data.slice(didx,didx+bsize+1);

                        if(bsize > 1){
                            t.deviceInfo(data.slice(i,i+bsize-1));
                        }
                        i+=  bsize - 1;
                    }
                    t.data = [];
                    t.anno = [];
                    t.state = [];
                    t.angles = [];
                    t.rawSG = [];
                    t.psi = [];
                    t.cstate = 0;
                    t.coTemp = 20;
                    t.cal_igmask = 0;
                    t.specificGravity = null;
                    t.rh=[];
                    t.lastRh=255;
                    t.lastRoomRh=255;
                    t.lastSetRh=255;
                    t.rhValid=false;
                    t.gravityChanges=[];

                    t.targetPsi = NaN; // to denote "no line/point"

                    this.clearData();
                    newchart = true;
                    t.psiAvail = false;
                    if(t.version == 7) t.calpoints=[];
                    // gravity tracking
                    GravityFilter.reset();
                    // gravity tracking
                } else if (d0 == 0xF3) { // correction temperature
                    if(t.version == CHART_V6) t.coTemp = d1; // always celisus
                    else{
                        t.calpoints = [];
                        for(var n=0;n<d1;n++){
                            var traw = data[i++];
                            traw = (traw << 8) | data[i++];
                            var graw = data[i++];
                            graw = (graw << 8) | data[i++];
                            var gravity= t.plato ? graw/100:graw/10000;
                            var raw = (t.devType == 2)? traw/10000:traw/100;               
                            t.calpoints.push([raw,gravity]);
                        }
                        t.calpoints.sort(function(a,b){return a[0]-b[0];});
                    }
                } else if (d0 == 0xF4) { // mode
                    //console.log(""+t.ctime/t.interval +" Stage:"+d1);
                    t.addMode(d1, t.ctime * 1000);
                } else if (d0 == 0xF5) { // targetPSI
                    t.targetPsi = (d1==0)? NaN:d1;
                } else if (d0 == 0xF1) { // state
                    t.cstate = d1;
                } else if (d0 == 0xF6) { // Time Sync
                    var utime = (data[i] << 24) + (data[i + 1] << 16) + (data[i + 2] << 8) + data[i + 3];
                    if(utime > t.ctime) t.ctime =utime;
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
                        t.psi.push( [new Date(t.ctime * 1000),  NaN,NaN,NaN]);
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
                } else if (d0 == 0xFC) { //Humidity
                    
                    if(d1 != 0xFF){
                        if(d1 & 0x80){ // room
                            t.lastRoomRh = d1 & 0x7F;
                            t.rhValid=true;
                        }else{
                            t.lastRh = d1;
                            t.rhValid=true;
                        }
                    }else if(t.rhValid){
                        t.lastRh = d1;
                    }
                } else if (d0 == 0xFD) { //Humidity target
                    t.lastSetRh = d1;
                } else if (d0 == 0xF8) { //OG
                    var hh = data[i++];
                    var ll = data[i++];
                    var v = (hh & 0x7F) * 256 + ll;
                    t.og = t.plato ? v / 100 : v / 10000;
                } else if (d0 == 0xFB) { //SG
                    var hh = data[i++];
                    var ll = data[i++];
                    var v = (hh & 0x7F) * 256 + ll;
                    if(t.plato){
                        // negative value in two's complement
                        t.specificGravity = (v>16383)?  ((v- 32768)/100):(v / 100);
                    }else{
                        t.specificGravity =  v / 10000;
                    }
                    // Gravity Data in calibration is out-of-band data
                    // To get it processed as soon as possible. 
                    // Insert into previous record
                    if(t.calibrating){
                        sgPoint = true;
                        t.insertOOBGravity();
                    }
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
                    if(t.processRecord() && t.calibrating) sgPoint=true;
                    
                } else if (d0 < 128) { // temp. or gravity
                    var tp = d0 * 256 + d1;
                    if (t.lidx == GravityAndTiltIndex) {
                        // gravity or gravity
                        if(t.calibrating){
                            // tilt value
                            tp = (tp == 0x7FFF) ? NaN :( (t.devType == 2)? (tp/10000):(tp / 100));
                        }else{
                            tp = (tp == 0x7FFF) ? NaN : (t.plato ? tp / 100 : ((tp > 8000) ? tp / 10000 : tp / 1000));
                            sgPoint = true;
                        }
                    } else if (t.lidx == PSIIndex) {
                        // pressure
                        if(tp == 0x7FFF) tp=null;
                        else tp =  tp / 10 - 100;
                        
                    } else {
                        // temperature
                        tp = (tp == 0x7FFF) ? NaN : tp / 100;
                        if (tp >= 225) tp = 225 - tp;
                    }

                    if (t.lidx < t.numData) {
                        if (typeof t.dataset != "undefined") {
                            t.dataset.push(tp);
                            t.laststat[t.lidx] = tp;
                            t.lidx++;
                            if(t.processRecord() && t.calibrating) sgPoint=true;
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
        BrewChart.prototype.desync=function(){
            if(typeof this.sync != "undefined") this.sync.detach();
        };
        BrewChart.prototype.synchronize=function(){
            var t=this;
            var charts=[t.chart];
            if(t.GravityChangeChart) charts.push(t.gcchart);
            if(t.psiAvail) charts.push(t.pchart);
            if(t.rhValid) charts.push(t.hchart);
            if(charts.length >1) t.sync = Dygraph.synchronize(charts,{selection: true,zoom:true,range:false});
        };
        BrewChart.prototype.updateChart = function() {
            var t = this;
            if (typeof t.chart == "undefined") t.createChart();
            else t.chart.updateOptions({
                'file': t.data
            });
            t.chart.setAnnotations(t.anno);

            var sync=false;
            if(t.GravityChangeChart){
                if(typeof t.gcchart == "undefined"){
                    document.querySelectorAll(".sgchange-group").forEach(function(ele){
                        ele.classList.remove("forced-hidden");
                    });
                    t.createGravityChangeChart();
                    sync=true;
                }else{
                    t.gcchart.updateOptions({
                        'file': t.gravityChanges,
                        'dateWindow':[t.psi[0][0],t.psi[t.psi.length-1][0]]
                    });
                }
            }

            if(t.psiAvail){
                if(typeof t.pchart == "undefined"){
                    document.querySelectorAll(".pressure-group").forEach(function(ele){
                        ele.classList.remove("forced-hidden");
                    });

                    t.createPSIChart();
                    sync=true;  
                }
                else t.pchart.updateOptions({
                    'file': t.psi,
                    'dateWindow':[t.psi[0][0],t.psi[t.psi.length-1][0]]
                });
            }

            if(t.rhValid){
                if(typeof t.hchart == "undefined"){
                    document.querySelectorAll(".humidity-group").forEach(function(ele){
                        ele.classList.remove("forced-hidden");
                    });

                    t.createHumidityChart();
                    sync=true;  
                }
                else t.hchart.updateOptions({
                    'file': t.rh,
                    'dateWindow':[t.rh[0][0],t.rh[t.rh.length-1][0]]                    
                });
            }

            if(sync) t.synchronize();

        };

        BrewChart.prototype.insertOOBGravity=function(){
            var t=this;
            if(t.rawSG.length > 0){
                t.rawSG[t.rawSG.length-1]=t.specificGravity;
                t.specificGravity=null;
            }
        };
        
        BrewChart.prototype.processRecord = function() {
            var t = this;
            // fill blank/unchanged fileds by checking the change mask(t.chnages)
            while ((((1 << t.lidx) & t.changes) == 0) && t.lidx < t.numData) {
                // gravity data is independant, use "null" to connect the line. (NaN) to disconnect.
                t.dataset.push((t.lidx > RoomTemperatureIndex && t.lidx != PSIDataIndex) ? null : t.laststat[t.lidx]);
                t.lidx++;
            }
            if (t.lidx >= t.numData) {
               // already get all data in a period record
                // get all data
                var minuteRecord = t.dataset.slice(0, 8);
    
                // handle gravity
                //  1. calculated
                //  2. in period record
                //  3. in special record
                
                var sg = NaN;
                var gravityTilt = t.dataset[GravityLine];
                if(! t.calibrating){
                    if(gravityTilt != null) sg = gravityTilt;
                }else{ 
                    // calibrating
                    if (!t.calculateSG) {
                        // calibrating, but not having formula
                        // if "gravity" data is available and currently not "calculating"(first run or not calibrating)
                        //sg = t.specificGravity;
                        // it's tilt data */
                        minuteRecord[GravityLine] = null;
                    } else {
                        //if (t.calculateSG) 
                        // must be in calibrating mode
                        // data field #8 is tilt in source data
                        if (minuteRecord[GravityLine] != null){
                            var temp = (this.celius) ? C2F(t.dataset[AuxTempLine]) : t.dataset[AuxTempLine];
                            sg = t.sgByTilt(t.dataset[GravityLine]);

                            if (t.plato) {
                                sg = BrewMath.sg2pla(BrewMath.tempCorrectionF(BrewMath.pla2sg(sg), temp, C2F(t.coTemp)));
                            }
                            minuteRecord[GravityLine] = sg;
                        }
                    }
                }
                if (!isNaN(sg)) {
                    t.sg = sg;
                    t.filterSg = GravityFilter.add(sg);
                }

                if (!isNaN(t.sg)) minuteRecord.push(t.filterSg);
                else minuteRecord.push(null);

                if(! isNaN(t.dataset[PSIDataIndex]) && t.dataset[PSIDataIndex]!=null) t.psiAvail = true;

                var vol =null;
                if(! isNaN(t.dataset[PSIDataIndex]) && t.dataset[PSIDataIndex]!=null){
                    if(!isNaN(t.dataset[BeerTempLine])){
                        var T = (t.celius)? C2F(t.dataset[BeerTempLine]):t.dataset[BeerTempLine];
                        vol = (t.dataset[PSIDataIndex] + 14.695) * (0.01821 + 0.090115 * Math.exp( (32 - T)/43.11 )) - 0.003342;
                        vol = Math.round(vol * 10)/10.0;
                    }
                }
                t.psi.push( [t.dataset[0], t.dataset[PSIDataIndex],t.targetPsi,vol]);
                
                t.data.push(minuteRecord);
                t.state.push(t.cstate);
                var ret=false;
                if(t.calibrating){
                    t.angles.push(gravityTilt);
                    t.rawSG.push(t.specificGravity);
                    if(t.specificGravity != null) ret =true;
                    t.specificGravity = null;
                }
                // humidity
                t.rh.push([t.dataset[0],
                        (t.lastRh <=100)? t.lastRh:NaN,
                        (t.lastSetRh<=100)? t.lastSetRh:NaN,
                        (t.lastRoomRh <=100)? t.lastRoomRh:NaN]);
                
                
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
                return ret;
            }
        };


        BrewChart.prototype.setHChart = function(id,label) {
            this.hcid=id;
            this.hlabel=label;
        };
        BrewChart.prototype.createHumidityChart = function() {
            var t=this;
            var ldiv = document.createElement("div");
            ldiv.className = "hide";
            document.body.appendChild(ldiv);

            t.hchart = new Dygraph(document.getElementById(t.hcid), t.rh, {
                labels: ["Time","rh","set","Room"],
                colors: BrewChart.Colors.slice(ChamberHumidityLine-1,RoomHumidityLine),
                connectSeparatedPoints: true,
                ylabel: t.hlabel,
                y2label: "%",
                axisLabelFontSize: 12,
                gridLineColor: '#ccc',
                gridLineWidth: '0.1px',
                labelsDiv: ldiv,
                labelsDivStyles: {
                    'display': 'none'
                },
                //displayAnnotations: true,
                //showRangeSelector: true,
                strokeWidth: 1,
                highlightCallback: function(e, x, pts, row) {
                    t.showLegend(x, row);
                },
                unhighlightCallback: function(e) {
                    t.hideLegend();
                }
            });
            t.hchart.setVisibility(0,true);
        };
        BrewChart.prototype.setGcChart = function(id,label) {
            this.gccid=id;
            this.gclabel=label;
        };

        BrewChart.prototype.createGravityChangeChart = function() {
            var t=this;
            var ldiv = document.createElement("div");
            ldiv.className = "hide";
            document.body.appendChild(ldiv);

            t.gcchart = new Dygraph(document.getElementById(t.gccid), t.gravityChanges, {
                labels: ["Time","gc3","gc6","gc12"],
                colors: [SgChange1Color,SgChange2Color,SgChange3Color],
                connectSeparatedPoints: true,
                ylabel: t.gclabel,
                y2label: t.gclabel,
                series: {
                    'gc12': {
                        axis: 'y2',
                        drawPoints: false
                    }
                }, 

                axisLabelFontSize: 12,
                gridLineColor: '#ccc',
                gridLineWidth: '0.1px',
                labelsDiv: ldiv,
                labelsDivStyles: {
                    'display': 'none'
                },
                //displayAnnotations: true,
                //showRangeSelector: true,
                strokeWidth: 1,
                axes: {
                    y: {
                        valueFormatter: function(y) {
                            return y.toFixed(1);
                        },
                        axisLabelFormatter: function(y) {
                            return y.toFixed(1);
                        }
                    },
                    y2: {
                        valueFormatter: function(y) {
                            return y.toFixed(1);
                        },
                        axisLabelFormatter: function(y) {
                            return y.toFixed(1);
                        }
                    }
                }, 
                highlightCallback: function(e, x, pts, row) {
                    t.showLegend(x, row);
                },
                unhighlightCallback: function(e) {
                    t.hideLegend();
                }
            });
            t.gcchart.setVisibility(0,true);
        };

        /* end of chart.js */