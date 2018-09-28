        var BChart = {
            toggle: function(type) {
                this.chart.toggleLine(type);
            },
            init: function(id, y1, y2) {
                this.chart = new BrewChart(id);
                this.chart.setLabels(y1, y2);
            },
            setIgnoredMask: function(m) {
                var t = this;
                if (t.chart.cal_igmask == m) return;
                t.chart.calculateSG = false;
                t.chart.process(t.raw);
                // the data will be updated by the "data"
                t.chart.cal_igmask = m;
                t.chart.getFormula();

                t.chart.process(t.raw);

                t.chart.updateChart();
                // the data will be updated by the "data",again
                t.chart.cal_igmask = m;
            }
        };

        function showPlatoUnit() {
            var units = document.querySelectorAll(".platounit");
            for (var i = 0; i < units.length; i++) {
                units[i].style.display = "inline-block";
            }
        }

        function loaded() {
            function openfile(f) {
                if (f) {
                    var r = new FileReader();
                    r.onload = function(e) {
                        window.file = f;
                        //chart.clear();
                        var data = new Uint8Array(e.target.result);
                        if (BrewChart.testData(data) !== false) {
                            BChart.raw = data;
                            BChart.chart.process(data);
                            if (BChart.chart.calibrating) {
                                BChart.chart.getFormula();
                                //  do it again
                                BChart.chart.process(data);
                                if (BChart.chart.calculateSG) Q("#formula-btn").style.display = "block";
                            }
                            BChart.chart.updateChart();
                            var date = new Date(BChart.chart.starttime * 1000);
                            Q("#log-start").innerHTML = BChart.chart.formatDate(date);
                            if (BChart.chart.plato) showPlatoUnit();
                        } else {
                            alert("<%= script_viewer_invalid_log %>");
                        }
                    };
                    r.readAsArrayBuffer(f);
                } else {
                    alert("<%= script_viewer_failed_load_file %>");
                }
            }

            BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML);

            if (Q('#dropfile')) {
                Q('#dropfile').ondragover = function(e) {
                    e.stopPropagation();
                    e.preventDefault();
                    e.dataTransfer.dropEffect = 'copy'; // Explicitly show this is a copy.
                };

                Q('#dropfile').ondrop = function(e) {
                    e.stopPropagation();
                    e.preventDefault();
                    var f = e.dataTransfer.files[0];
                    openfile(f);
                };
            }
            Q('#fileinput').onchange = function(evt) {
                //Retrieve the first (and only!) File from the FileList object
                var f = evt.target.files[0];
                openfile(f);
                BChart.chart.dataset
            };

        }

        function download(blob, file) {
            var link = document.createElement("a");

            if (link.download === undefined) { // feature detection
                alert("<%= script_viewer_not_downloading_file %>");
                return;
            }

            var url = URL.createObjectURL(blob);
            link.setAttribute("href", url);
            link.setAttribute("download", file);
            link.style.visibility = 'hidden';
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);

        }

        function exportcsv() {
            if (typeof window.file == "undefined") return;
            // generate data
            var csv = "Time, Unix Time";
            for (var i = 1; i < BrewChart.Labels.length; i++) {
                csv = csv + ((i == 0) ? "" : ",") + BrewChart.Labels[i];
            }
            csv = csv + ",Tilt,state\n";

            for (var row = 0; row < BChart.chart.data.length; row++) {
                for (var i = 0; i < BrewChart.Labels.length; i++) {
                    var v = BChart.chart.chart.getValue(row, i);
                    if (v === null) v = "";
                    else if (isNaN(v)) v = "";
                    if (i == 0) {
                        var d = new Date(v);
                        csv = csv + d.toISOString() + "," + (v / 1000);
                    } else csv = csv + "," + v;
                }
                csv = csv + "," + BChart.chart.angles[row];

                var state = parseInt(BChart.chart.state[row]);
                var st = (!isNaN(state)) ? STATES[state].text : "";
                csv = csv + "," + st + "\n";
            }
            var blob = new Blob([csv], {
                type: 'text/csv;'
            });
            // Browsers that support HTML5 download attribute
            download(blob, window.file.name + ".csv");
        }

        function cutrange() {
            if (typeof window.file == "undefined") return;
            var ranges = BChart.chart.chart.xAxisRange();
            var data = BChart.chart.partial(ranges[0], ranges[1]);
            download(new Blob(data, { type: 'octet/stream' }), window.file.name + "-partial");
        }

        function cutrange2p() {
            if (typeof window.file == "undefined") return;
            var ranges = BChart.chart.chart.xAxisRange();
            var data = BChart.chart.partial2Plato(ranges[0], ranges[1]);
            download(new Blob(data, { type: 'octet/stream' }), window.file.name + "-partial");
        }