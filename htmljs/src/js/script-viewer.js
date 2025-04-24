        var BChart = {
            toggle: function(line,p) {
                if(typeof p !="undefined" && p)  this.chart.togglePsiLine(line);
                else this.chart.toggleLine(line);
            },
    
            init: function(id, y1, y2,id2,pl,carbonation,id3,rhLabel,id4,gclabel) {
                this.chart = new UniBrewChart(id);
                this.chart.setLabels(y1, y2);
                this.chart.setPChart(id2,pl,carbonation)
                this.chart.setHChart(id3,rhLabel);
                this.chart.setGcChart(id4,gclabel);
                //GravityChangeChart 
                this.chart.GravityChangeChart=true;
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
                        if (UniBrewChart.testData(data) !== false) {
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

//            BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML,"div_p",Q('#psilabel').innerHTML,Q('#vollabel').innerHTML);
//              BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML,"div_p",Q('#psilabel').innerHTML,Q('#vollabel').innerHTML,"div_h",Q("#rhlabel").innerHTML);
//GravityChangeChart            
BChart.init("div_g", Q('#ylabel').innerHTML, Q('#y2label').innerHTML,"div_p",Q('#psilabel').innerHTML,Q('#vollabel').innerHTML,"div_h",Q("#rhlabel").innerHTML,"div_gc",Q("#gclabel").innerHTML);

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
            csv = csv + ",Tilt,state,pressure\n";

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
                csv = csv + "," + st;
                csv = csv + "," + BChart.chart.psi[row][1];
                csv = csv + "\n";
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

        function calibrationTimeChanged(){
            var ntime=new Date(Q("#newcaltime").value);
            var tilt = BChart.chart.getTiltAroundTime(ntime);
            Q("#tilt-value").innerText = (tilt < 0)? "--":""+tilt;
        }
        
        PolyRegression.appendPoint=function(tilt,ng){
            PolyRegression.allpoints.push([tilt,ng]);
            PolyRegression.allpoints.sort(function(a,b){
                return  b[0] - a[0];
            });
            PolyRegression.igchanged();            
        };

        function addNewCalibrationPoint(){
            var ng= parseFloat(Q("#newcalsg").value);
            var tilt= parseFloat(Q("#tilt-value").innerText);

            if( isNaN(ng) ) return;
            if(isNaN(tilt)) return;

            PolyRegression.appendPoint(tilt,ng);
            // record new temp
            if(typeof window.newPts == "undefined") window.newPts=Array();
            
            window.newPts.push({
                time: new Date(Q("#newcaltime").value),
                gravity: ng 
            });
        }

        function openNewCalibrationPointInput(){
            var end=BChart.chart.end();
            var dd= new Date( end.getTime() - end.getTimezoneOffset() * 60000);
            Q("#newcaltime").value =dd.toISOString().slice(0, 16);
            calibrationTimeChanged();
        }
        function applyPolynomial(){
            if(typeof window.newPts !="undefined" && window.newPts.length > 0){
                for(var i=0;i< window.newPts.length;i++){
                    var r=window.newPts[i];
                    BChart.chart.addCalibration(r.time,r.gravity);
                }
                BChart.chart.getFormula();
                // rowSG is overwritten by the following function call
                // a new function that update Gravity only should be created
                // Now, just take a easy and dirty way
                BChart.chart.process(BChart.raw);
                BChart.chart.updateChart();
                for(var i=0;i< window.newPts.length;i++){
                    var r=window.newPts[i];
                    BChart.chart.addCalibration(r.time,r.gravity);
                }

            }else{
                BChart.setIgnoredMask(PolyRegression.cal_igmask);
            }
        }
/*
        function cutrange2p() {
            if (typeof window.file == "undefined") return;
            var ranges = BChart.chart.chart.xAxisRange();
            var data = BChart.chart.partial2Plato(ranges[0], ranges[1]);
            download(new Blob(data, { type: 'octet/stream' }), window.file.name + "-partial");
        }
        */