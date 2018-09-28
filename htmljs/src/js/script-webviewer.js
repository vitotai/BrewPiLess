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

        function getParameterByName(name, url) {
            if (!url) url = window.location.href;
            name = name.replace(/[\[\]]/g, "\\$&");
            var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
                results = regex.exec(url);
            if (!results) return null;
            if (!results[2]) return '';
            return decodeURIComponent(results[2].replace(/\+/g, " "));
        }

        function dataUrl() {
            return getParameterByName("log");
        }

        function getRangeURL(range) {
            var re = /([^\?]+)/;
            var result = window.location.href.match(re);
            return result[1] + "?log=" + encodeURIComponent(getParameterByName("log")) +
                "&r=" + range[0] + "-" + range[1];
        }

        function getFilename() {
            var log = getParameterByName("log");
            var re = /.([^\/]+)$/;
            var matches = re.exec(log);
            if (matches) return matches[1];
            return log;
        }

        function showPlatoUnit() {
            var units = document.querySelectorAll(".platounit");
            for (var i = 0; i < units.length; i++) {
                units[i].style.display = "inline-block";
            }
        }

        function loaded() {
            // get range, if any
            var range = getParameterByName("r");
            if (range) {
                window.iniRange = range.split("-");
            }
            BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML);
            Q("#div_g").oncontextmenu = function(ev) {
                ev = ev || window.event;

                Q("#myDropdown").classList.toggle("show");
                Q("#myDropdown").style.left = ev.clientX + "px";
                Q("#myDropdown").style.top = ev.clientY + "px";
                ev.stopPropagation();
                ev.preventDefault();
                return false;
            };


            window.addEventListener("click", function() {
                var dd = Q("#myDropdown");
                if (dd.classList.contains("show")) dd.classList.remove("show");
            });

            Q("#open-selection").onclick = function() {
                var ranges = BChart.chart.getXRange();
                console.log("range:" + ranges[0] + "-" + ranges[1]);
                window.open(getRangeURL(ranges), '_blank');
                return false;
            };

            Q("#viewlogname").innerHTML = getFilename();

            var xhr = new XMLHttpRequest();
            xhr.open('GET', dataUrl());
            //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            //	xhr.setRequestHeader("Content-length", PD.length);
            xhr.responseType = 'arraybuffer';
            xhr.onload = function(e) {
                if (this.status == 404) {
                    console.log("error getting log data.");
                    return;
                }
                // response is unsigned 8 bit integer
                var data = new Uint8Array(this.response);

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
                    if (typeof window.iniRange !== "undefined") BChart.chart.setXRange(window.iniRange);
                    if (BChart.chart.plato) showPlatoUnit();
                } else {
                    alert("<%= script_viewer_invalid_log %>");
                }
            };
            xhr.ontimeout = function(e) {
                console.error("Timeout!");
            };
            xhr.onerror = function() {
                console.log("error getting data.");
            };
            //console.log(PD);
            xhr.send();
        }