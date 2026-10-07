// Patch wdoc control: independent enable switches + comma normalize + null sets.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
  return {
    name: m[1],
    bytes: Buffer.from(m[2].split(',').map((s) => s.trim()).filter(Boolean).map((s) => parseInt(s, 16))),
  };
}

function formatHeader(name, buf) {
  const lines = [`const unsigned char ${name}[] PROGMEM = {`];
  for (let i = 0; i < buf.length; i += 12) {
    const chunk = [];
    for (let j = i; j < Math.min(i + 12, buf.length); j++) {
      chunk.push('0x' + buf[j].toString(16).padStart(2, '0'));
    }
    lines.push('  ' + chunk.join(', ') + (i + 12 < buf.length ? ',' : ''));
  }
  lines.push('};');
  lines.push(`unsigned int ${name}_len = ${buf.length};`);
  lines.push('');
  return lines.join('\n');
}

const HTML_OLD =
  '<div style="margin:16px 0" layout horizontal cross-center><span>Cooling Target</span><input type=number size=6 id=independent-cool-t style=max-width:200px>&deg;<span class=t_unit>C</span></div>' +
  '<div style="margin:16px 0" layout horizontal cross-center><span>Heating Target</span><input type=number size=6 id=independent-heat-t style=max-width:200px>&deg;<span class=t_unit>C</span></div>';

const HTML_NEW =
  '<div style="margin:16px 0" layout horizontal cross-center>' +
  '<span>Cooling Target</span>' +
  '<input type=number size=6 id=independent-cool-t style="max-width:200px;margin-left:8px" lang=en inputmode=decimal>&deg;<span class=t_unit>C</span>' +
  '<input type=checkbox id=fridge-temp name=indep-cooler-en class=switch-input checked>' +
  '<label for=fridge-temp class=switch-label style=margin-left:12px>Enable/Disable COOLER</label></div>' +
  '<div style="margin:16px 0" layout horizontal cross-center>' +
  '<span>Heating Target</span>' +
  '<input type=number size=6 id=independent-heat-t style="max-width:200px;margin-left:8px" lang=en inputmode=decimal>&deg;<span class=t_unit>C</span>' +
  '<input type=checkbox id=beer-set name=indep-heater-en class=switch-input checked>' +
  '<label for=beer-set class=switch-label style=margin-left:12px>Enable/Disable HEATER</label></div>';

const APPLY_OLD =
  '"independent"==this.cmode?(function(){var l=document.getElementById("independent-cool-t").value,o=document.getElementById("independent-heat-t").value;return""==l||isNaN(l)||l>BrewPiSetting.maxDegree||l<BrewPiSetting.minDegree?alert("Invalid Temperature:"+l):""==o||isNaN(o)||o>BrewPiSetting.maxDegree||o<BrewPiSetting.minDegree?alert("Invalid Temperature:"+o):void function(){var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+l+", beerSet:"+o+", hystMode:"+hystMode+"}")}()})()';

const APPLY_NEW =
  '"independent"==this.cmode?(function(){' +
  'function normTempStr(s){return String(null==s?"":s).replace(/,/g,".").trim()}' +
  'var coolEn=document.getElementById("fridge-temp"),heatEn=document.getElementById("beer-set"),' +
  'coolerOn=!coolEn||coolEn.checked,heaterOn=!heatEn||heatEn.checked;' +
  'if(!coolerOn&&!heaterOn)return alert("Invalid Temperature:");' +
  'var vc=normTempStr(document.getElementById("independent-cool-t").value),' +
  'vh=normTempStr(document.getElementById("independent-heat-t").value);' +
  'if(coolerOn&&(""===vc||isNaN(vc)||vc>BrewPiSetting.maxDegree||vc<BrewPiSetting.minDegree))return alert("Invalid Temperature:"+vc);' +
  'if(heaterOn&&(""===vh||isNaN(vh)||vh>BrewPiSetting.maxDegree||vh<BrewPiSetting.minDegree))return alert("Invalid Temperature:"+vh);' +
  'var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";' +
  'BWF.send("j{mode:i, "+(coolerOn?"fridgeSet:"+vc:"fridgeSet:null")+", "+(heaterOn?"beerSet:"+vh:"beerSet:null")+", hystMode:"+hystMode+"}")})()';

const INIT_OLD = 't.cmode="profile",t.select(t.cmode)}},apply:function()';
const INIT_NEW =
  't.cmode="profile",t.select(t.cmode),' +
  '(function(){function wireIndepActuator(a,b){var c=document.getElementById(a),d=document.getElementById(b);if(c&&d){var e=function(){d.disabled=!c.checked};c.onchange=function(){c._userTouched=!0,e()},e()}}' +
  'wireIndepActuator("fridge-temp","independent-cool-t"),wireIndepActuator("beer-set","independent-heat-t")})()' +
  // Close if(!initiated) AND init function: minified is `if(!t.initiated){...}},apply`
  '}},apply:function(){';

const STATUS_OLD =
  '"i"===e.md&&function(){var t=Q("#independent-cool-t");t&&""===t.value&&void 0!==e.fs&&-10000<e.fs&&(t.value=(e.fs/100).toFixed(1));var n=Q("#independent-heat-t");n&&""===n.value&&void 0!==e.bs&&-10000<e.bs&&(n.value=(e.bs/100).toFixed(1))}()';

const STATUS_NEW =
  '"i"===e.md&&function(){var coolBox=Q("#independent-cool-t"),heatBox=Q("#independent-heat-t"),coolEn=document.getElementById("fridge-temp"),heatEn=document.getElementById("beer-set");' +
  'coolEn&&!coolEn._userTouched&&coolBox&&""===coolBox.value&&(void 0!==e.fs&&-10000<e.fs?(coolEn.checked=!0,coolBox.value=(e.fs/100).toFixed(1),coolBox.disabled=!1):(coolEn.checked=!1,coolBox.disabled=!0));' +
  'heatEn&&!heatEn._userTouched&&heatBox&&""===heatBox.value&&(void 0!==e.bs&&-10000<e.bs?(heatEn.checked=!0,heatBox.value=(e.bs/100).toFixed(1),heatBox.disabled=!1):(heatEn.checked=!1,heatBox.disabled=!0))}()';

function patch(html) {
  let out = html;
  let n = 0;

  if (out.includes('name=indep-cooler-en')) {
    console.log('  html already');
  } else if (out.includes(HTML_OLD)) {
    out = out.replace(HTML_OLD, HTML_NEW);
    n++;
  } else {
    throw new Error('HTML_OLD not found');
  }

  if (out.includes('fridgeSet:null')) {
    console.log('  apply already');
  } else if (out.includes(APPLY_OLD)) {
    out = out.replace(APPLY_OLD, APPLY_NEW);
    n++;
  } else {
    // Localized alert text — match structurally
    const re =
      /"independent"==this\.cmode\?\(function\(\)\{var l=document\.getElementById\("independent-cool-t"\)\.value,o=document\.getElementById\("independent-heat-t"\)\.value;return""==l\|\|isNaN\(l\)\|\|l>BrewPiSetting\.maxDegree\|\|l<BrewPiSetting\.minDegree\?alert\("([^"]+)"\+l\):""==o\|\|isNaN\(o\)\|\|o>BrewPiSetting\.maxDegree\|\|o<BrewPiSetting\.minDegree\?alert\("([^"]+)"\+o\):void function\(\)\{var pidRadio=document\.querySelector\("input\[name=hystMode\]:checked"\),hystMode=pidRadio\?pidRadio\.value:"0";BWF\.send\("j\{mode:i, fridgeSet:"\+l\+", beerSet:"\+o\+", hystMode:"\+hystMode\+"\}"\)\}\(\)\}\)\(\)/;
    const m = out.match(re);
    if (!m) throw new Error('APPLY pattern not found');
    const a1 = m[1];
    const a2 = m[2];
    const repl =
      '"independent"==this.cmode?(function(){' +
      'function normTempStr(s){return String(null==s?"":s).replace(/,/g,".").trim()}' +
      'var coolEn=document.getElementById("fridge-temp"),heatEn=document.getElementById("beer-set"),' +
      'coolerOn=!coolEn||coolEn.checked,heaterOn=!heatEn||heatEn.checked;' +
      'if(!coolerOn&&!heaterOn)return alert("' + a1 + '");' +
      'var vc=normTempStr(document.getElementById("independent-cool-t").value),' +
      'vh=normTempStr(document.getElementById("independent-heat-t").value);' +
      'if(coolerOn&&(""===vc||isNaN(vc)||vc>BrewPiSetting.maxDegree||vc<BrewPiSetting.minDegree))return alert("' + a1 + '"+vc);' +
      'if(heaterOn&&(""===vh||isNaN(vh)||vh>BrewPiSetting.maxDegree||vh<BrewPiSetting.minDegree))return alert("' + a2 + '"+vh);' +
      'var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";' +
      'BWF.send("j{mode:i, "+(coolerOn?"fridgeSet:"+vc:"fridgeSet:null")+", "+(heaterOn?"beerSet:"+vh:"beerSet:null")+", hystMode:"+hystMode+"}")})()';
    out = out.replace(re, repl);
    n++;
  }

  if (out.includes('wireIndepActuator')) {
    console.log('  init already');
  } else if (out.includes(INIT_OLD)) {
    out = out.replace(INIT_OLD, INIT_NEW);
    n++;
  } else {
    throw new Error('INIT_OLD not found');
  }

  if (out.includes('coolEn._userTouched') || out.includes('!coolEn._userTouched')) {
    console.log('  status already');
  } else if (out.includes(STATUS_OLD)) {
    out = out.replace(STATUS_OLD, STATUS_NEW);
    n++;
  } else {
    const rhEnd = 'void 0!==e.rh&&HC_show(e.rh)},C:function(e){ccparameter(e)}';
    if (out.includes(rhEnd)) {
      out = out.replace(
        rhEnd,
        'void 0!==e.rh&&HC_show(e.rh),' + STATUS_NEW + '},C:function(e){ccparameter(e)}'
      );
      n++;
      console.log('  status injected');
    } else {
      console.log('  WARN: status prefill skipped');
    }
  }

  // beer/fridge comma
  const beerOld = 'var e=document.getElementById(this.cmode+"-t").value;';
  // may use different var — find pattern
  if (!out.includes('replace(/,/g,".")')) {
    // independent already has it; beer path:
  }
  const reBeer =
    /("beer"==this\.cmode\|\|"fridge"==this\.cmode)\?(?:function\(\))?\{?var (\w+)=document\.getElementById\(this\.cmode\+"-t"\)\.value/;
  // Looking at actual: "beer"==this.cmode||"fridge"==this.cmode?""==(e=document.getElementById(this.cmode+"-t").value)
  const beerExact = 'var e;BrewPiSetting.valid||alert("Not connected to controller."),"beer"==this.cmode||"fridge"==this.cmode?""==(e=document.getElementById(this.cmode+"-t").value)';
  // dump might differ - soft try
  out = out.replace(
    /(\w+)=document\.getElementById\(this\.cmode\+"-t"\)\.value/g,
    '$1=String(document.getElementById(this.cmode+"-t").value||"").replace(/,/g,".").trim()'
  );

  return { out, n };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_control_htm.h`);
  console.log(lang);
  const raw = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(raw);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  try {
    // Localize APPLY_OLD alert if needed — english uses Invalid Temperature
    let { out, n } = patch(html);
    const scripts = [];
    out.replace(/<script[^>]*>([\s\S]*?)<\/script>/gi, (_, s) => {
      if (s && s.length > 200 && !s.includes('Dygraph')) scripts.push(s);
    });
    for (let i = 0; i < scripts.length; i++) {
      try {
        new Function(scripts[i]);
      } catch (e) {
        console.log('  PARSE FAIL', i, e.message);
        const idx = scripts[i].indexOf('independent');
        if (idx >= 0) console.log(scripts[i].slice(idx, idx + 500));
        throw e;
      }
    }
    if (!out.includes('name=indep-cooler-en')) throw new Error('switch missing after patch');
    if (!out.includes('fridgeSet:null')) throw new Error('null send missing');
    const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
    fs.writeFileSync(file, formatHeader(name, gz));
    console.log('  ok', n, bytes.length, '->', gz.length);
  } catch (e) {
    console.log('  FAIL', e.message);
  }
}
