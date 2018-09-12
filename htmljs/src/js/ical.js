    /* ispindel calibration */
    var PolyRegression = {
        allpoints: [],
        clear: function() {
            var tbody = document.getElementById("pointlist").getElementsByTagName("tbody")[0];
            var rl = tbody.querySelectorAll("tr.pl_calpoint");
            var count = rl.length;
            for (var i = rl.length - 1; i >= 0; i--) {
                var tr = rl[i];
                tr.parentNode.removeChild(tr);
            }
            return tbody;
        },
        newrow: function(values) {
            var tr = this.row.cloneNode(true);

            var td2 = tr.querySelector("td.pl_tilt");
            td2.innerHTML = values[0].toFixed(2);

            var td4 = tr.querySelector("td.pl_sg");
            td4.innerHTML = this.plato ? values[1].toFixed(2) : values[1].toFixed(4); // BrewMath.brix2sg(this.points[i][1]).toFixed(3);

            var td6i = tr.querySelector("td.pl_value");
            td6i.innerHTML = this.plato ? values[2].toFixed(2) : values[2].toFixed(4);

            var td6 = tr.querySelector("td.pl_error");
            td6.innerHTML = this.plato ? values[3].toFixed(2) : (values[3] * 1000).toFixed(1);

            var ig_input = tr.querySelector("input.pl_ignored_cb");
            ig_input.checked = values[4];
            var t = this;
            ig_input.onchange = function() { t.igchanged(this); };
            return tr;
        },
        igchanged: function(checkbox) {
            var tbody = document.getElementById("pointlist").getElementsByTagName("tbody")[0];
            var ilist = tbody.querySelectorAll("input.pl_ignored_cb");
            var mask = 0;
            var notignored = 0;
            for (var i = 0; i < ilist.length; i++) {
                if (ilist[i].checked) mask = mask | (0x1 << i);
                else notignored++;
            }
            if (notignored < 2) {
                console.log("less than 2.");
                checkbox.checked = false;
            } else {
                this.cal_igmask = mask;
                this.show();
            }
        },
        show: function() {
            if (!this.row) {
                this.row = Q("#pointlist tr.pl_calpoint");
                this.row.parentNode.removeChild(this.row);
            }
            this.getFormula();
            var tbody = this.clear();
            for (var i = 0; i < this.ptlist.length; i++) {
                tbody.appendChild(this.newrow(this.ptlist[i]));
            }
            this.chart();
        },
        getFormula: function() {
            var t = this;
            if (t.allpoints.length < 2) return;
            t.points = [];
            for (var i = 0; i < t.allpoints.length; i++) {
                if (!((0x1 << i) & t.cal_igmask)) t.points.push(t.allpoints[i]);
            }
            if (t.points.length < 2) {
                // ignore too much. ignore the ignore
                t.points = t.alpoints;
                t.cal_igmask = 0;
            }

            var poly = regression('polynomial', t.points, (t.points.length > 3) ? 3 : ((t.points.length > 2) ? 2 : 1), {
                precision: 9
            });
            t.regression = poly;
            Q("#polynormial").innerHTML = poly.string;
            // caluate errors

            t.sgByTilt = (t.points.length > 3) ?
                function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x +
                        poly.equation[2] * x * x +
                        poly.equation[3] * x * x * x;
                } : ((t.points.length > 2) ? function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x +
                        poly.equation[2] * x * x;
                } : function(x) {
                    return poly.equation[0] +
                        poly.equation[1] * x;
                });

            var point_list = [];

            for (var i = 0; i < t.allpoints.length; i++) {
                var tilt = t.allpoints[i][0];
                var realsg = t.allpoints[i][1];
                var cal_sg = t.sgByTilt(tilt);
                var error = realsg - cal_sg;
                var ignored = ((0x1 << i) & t.cal_igmask) != 0;
                point_list.push([tilt, realsg, cal_sg, error, ignored]);
            }
            t.ptlist = point_list;
        },
        chart: function() {
            var data = [];
            for (var i = 0; i < this.allpoints.length; i++) {
                data.push([this.ptlist[i][0], this.ptlist[i][1], this.ptlist[i][2]]);
            }
            if (typeof this.graph == "undefined") {
                this.graph = new Dygraph(
                    document.getElementById("graph"), data, {
                        labels: ["Tilt", "SG", "Interpolated"],
                        colors: ["rgb(240, 100, 100)", "rgb(89, 184, 255)"],
                        series: {
                            'SG': {
                                drawPoints: true,
                                pointSize: 4,
                                strokeWidth: 0
                            }
                        },
                        axisLabelFontSize: 12,
                        gridLineColor: '#ccc',
                        gridLineWidth: '0.1px',
                        strokeWidth: 1,
                        xRangePad: 10,
                        axes: {
                            y: {
                                axisLabelWidth: 40,
                                axisLabelFormatter: function(y) {
                                    return y.toFixed(3);
                                },
                                valueFormatter: function(y) {
                                    return y.toFixed(3);
                                }
                            },
                            x: {
                                pixelsPerLabel: 30,
                                axisLabelWidth: 40
                            }
                        }
                    }
                );
            } else {
                this.graph.updateOptions({
                    'file': data
                });
                this.graph.resize();
            }
        }
    };

    function applyIgnoreMask() {
        BChart.setIgnoredMask(PolyRegression.cal_igmask);
    }

    function openpolynomialpane() {
        Q("#polynomialpane").style.display = "block";
        PolyRegression.allpoints = BChart.chart.calibrationPoints;
        PolyRegression.cal_igmask = BChart.chart.cal_igmask;
        PolyRegression.plato = BChart.chart.plato;
        PolyRegression.show();
    }

    function closepolynomialpane() {
        Q("#polynomialpane").style.display = "none";
    }
    /* end of calibration */