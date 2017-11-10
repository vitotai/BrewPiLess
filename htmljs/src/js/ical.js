    /* ispindel calibration */
    var PolyRegression = {
        points: [],
        clear: function() {
            var tbody = document.getElementById("pointlist").getElementsByTagName("tbody")[0];
            var rl = tbody.getElementsByTagName("tr");
            var count = rl.length;
            for (var i = rl.length - 1; i > 0; i--) {
                var tr = rl[i];
                tr.parentNode.removeChild(tr);
            }
            return tbody;
        },
        newrow: function(i) {
            var tr = document.createElement("tr");
            var td2 = document.createElement("td");
            td2.className = "pl_tilt";
            td2.innerHTML = this.points[i][0].toFixed(2);

            var td4 = document.createElement("td");
            td4.className = "pl_sg";
            td4.innerHTML = this.points[i][1]; // BrewMath.brix2sg(this.points[i][1]).toFixed(3);

            var td6i = document.createElement("td");
            if (typeof this.values != "undefined")
                td6i.innerHTML = this.values[i].toFixed(4);


            var td6 = document.createElement("td");
            if (typeof this.errors != "undefined")
                td6.innerHTML = (this.errors[i] * 1000).toFixed(1);

            tr.appendChild(td2);
            tr.appendChild(td4);
            tr.appendChild(td6i);
            tr.appendChild(td6);
            return tr;
        },
        show: function() {
            this.getFormula();
            var tbody = this.clear();
            for (var i = 0; i < this.points.length; i++) {
                tbody.appendChild(this.newrow(i));
            }
            this.chart();
        },
        getFormula: function() {
            if (this.points.length < 2) return;

            var poly = regression('polynomial', this.points, (this.points.length > 3) ? 3 : ((this.points.length > 2) ? 2 : 1), {
                precision: 9
            });
            Q("#polynormial").innerHTML = poly.string;
            // caluate errors
            var errors = [];
            var values = [];
            var thirdcoe = poly.equation;
            for (var i = 0; i < this.points.length; i++) {
                var y = poly.points[i][1];
                var t = this.points[i][1];
                errors.push(y - t);
                values.push(y);
            }
            this.errors = errors;
            this.values = values;
            this.regression = poly;
        },
        chart: function() {
            var data = [];
            for (var i = 0; i < this.points.length; i++) {
                data.push([this.points[i][0], this.points[i][1], this.regression.points[i][1]]);
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

    function openpolynomialpane() {
        Q("#polynomialpane").style.display = "block";
        PolyRegression.points = BChart.chart.calibrationPoints;
        PolyRegression.show();
    }

    function closepolynomialpane() {
        Q("#polynomialpane").style.display = "none";
    }
    /* end of calibration */