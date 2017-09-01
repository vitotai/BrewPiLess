var ControlChart={
  unit:"C",
  init:function(div, data,unit){
    var t=this;
    t.data=data;
    t.unit=unit;

    var dateFormatter=function(v){
      d=new Date(v);
      return formatDate(d);
    };
    var shortDateFormatter=function(v){
      d=new Date(v);
      var y= d.getYear() +1900;
      var re = new RegExp('[^\d]?'+y +'[^\d]?');
      var n=d.toLocaleDateString();
      return n.replace(re,"");
    };

    var temperatureFormatter=function(v){
      return v.toFixed(1) + "&deg;" + t.unit; }
      ;

      t.chart = new Dygraph(
        document.getElementById(div), t.data,
        {
          colors: [ 'rgb(89, 184, 255)' ],
          axisLabelFontSize:12,
          gridLineColor:'#ccc',
          gridLineWidth:'0.1px',
          labels:["Time","Temperature"],
          labelsDiv: document.getElementById(div + "-label"),
          legend: 'always',
          labelsDivStyles: { 'textAlign': 'right' },
          strokeWidth: 1,
          //        xValueParser: function(x) { return profileTable.parseDate(x); },
          //        underlayCallback: updateCurrentDateLine,
          //        "Temperature" : {},
          axes: {
            y : { valueFormatter: temperatureFormatter, pixelsPerLabel:20, axisLabelWidth: 35 },
            //            x : { axisLabelFormatter:dateFormatter, valueFormatter: dateFormatter, pixelsPerLabel: 30, axisLabelWidth:40 }
            x : { axisLabelFormatter:shortDateFormatter, valueFormatter:dateFormatter, pixelsPerLabel: 30, axisLabelWidth:40 }

          },
          highlightCircleSize: 2,
          highlightSeriesOpts: {
            strokeWidth: 1.5,
            strokeBorderWidth: 1,
            highlightCircleSize: 5
          },

        }
      );
    },
    update:function(data,unit){
      if(data.length == 0) return;
      this.data=data;
      this.unit=unit;
      this.chart.updateOptions( { 'file': this.data } );
    }
  };
