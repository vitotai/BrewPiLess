var passcode="thisistest";

function doPost(e) {
  var res="<b>Post</b>";
  var p= e.parameter;
  if(p.pc === undefined)
    return HtmlService.createHtmlOutput("Missing passcode.");

  if(p.pc != passcode)
      return HtmlService.createHtmlOutput("Wrong passcode.");

if (p.bt !== undefined
  && p.bs !== undefined
  && p.ft !== undefined
  && p.fs !== undefined
  && p.ss !== undefined
  && p.st !== undefined){
     var ss = SpreadsheetApp.openById(p.ss);
     var sheet = ss.getSheetByName(p.st);

     var lastRow = sheet.getLastRow();
     var range = sheet.getRange(lastRow+1,1,1,5);
     var range_last = sheet.getRange(2, 1,1,5);
     range.setValues([[new Date(),p.bt,p.bs,p.ft,p.fs]]);
     range_last.setValues([[new Date(),p.bt,p.bs,p.ft,p.fs]]);
   
   }else{
     res = res + "undefined e.parameter.bs";
   }
   return HtmlService.createHtmlOutput(res);
}
