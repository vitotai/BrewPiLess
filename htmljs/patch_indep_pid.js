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

const RADIO_BLOCK_RE =
  /<div><h6 class=inline-header>Classic start hysteresis<\/h6>[\s\S]*?<label style="display:block;margin:\.25em 0"><input type=radio name=hystMode value=1> Heat\/cool target bands<\/label><\/div>/;

function patchConfig(html) {
  let out = html;
  if (RADIO_BLOCK_RE.test(out)) {
    out = out.replace(RADIO_BLOCK_RE, '');
  }
  out = out.replace(/ id=idleRangeFieldsL/g, '');
  out = out.replace(/ id=idleRangeFields\b/g, '');
  // Remove JS helpers tied to classic hystMode radio if present
  out = out.replace(
    /e\[n\]\.onchange=function\(\)\{this\.name==="hystMode"\?\(window\.ccModified\.hystMode=this\.value,updateHysteresisUi\(this\.value\)\):window\.ccModified\[this\.id\]=this\.value,checkModify\(\)\}/,
    'e[n].onchange=function(){window.ccModified[this.id]=this.value,checkModify()}'
  );
  out = out.replace(
    /function updateHysteresisUi\(e\)\{var n="0"===String\(e\),t=Q\("#idleRangeFields"\),i=Q\("#idleRangeFieldsL"\);t&&\(t\.style\.display=n\?"":"none"\),i&&\(i\.style\.display=n\?"":"none"\)\}/,
    ''
  );
  out = out.replace(
    /Object\.keys\(n\)\.forEach\(e=>\{if\("hystMode"===e\)\{for\(var t=document\.querySelectorAll\("input\[name=hystMode\]"\),i=0;i<t\.length;i\+\+\)t\[i\]\.checked=String\(t\[i\]\.value\)===String\(n\[e\]\);updateHysteresisUi\(n\[e\]\)\}else\{var a=Q\("#"\+e\);a&&\(a\.value=n\[e\]\)\}\}\)/,
    'Object.keys(n).forEach(e=>{if("hystMode"===e)return;var a=Q("#"+e);a&&(a.value=n[e])})'
  );
  return out;
}

const INDEP_NOTE =
  '<div style="margin:16px 0">' +
  '<span>hysteresisMode (PID control)</span>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode id=independent-pid-off value=0> Off (simple thermostat)</label>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode id=independent-pid-on value=1> On (PID on heater; cooler uses cool targets)</label>' +
  '</div>' +
  '<p style="margin:8px 0 16px;font-size:.9em">This mode uses heatTargetH, heatTargetL, coolTargetH, and coolTargetL settings in the Advanced Settings menu for on/off bands.</p>';

const INDEP_INSERT_AFTER =
  /(<input[^>]*id=independent-heat-t[^>]*>[^<]*&deg;[^<]*<\/span><\/div>)/;

function patchControl(html) {
  let out = html;
  if (out.includes('independent-pid-off') || out.includes('hysteresisMode (PID control)')) {
    return out;
  }
  if (!INDEP_INSERT_AFTER.test(out)) {
    throw new Error('independent-heat-t block not found');
  }
  out = out.replace(INDEP_INSERT_AFTER, '$1' + INDEP_NOTE);

  // Apply sends hystMode — must stay expression-safe inside minified ternaries
  // (never inject bare `var` after `:`).
  out = out.replace(
    /BWF\.send\("j\{mode:i, fridgeSet:"\+([^+]+)\+", beerSet:"\+([^+]+)\+"\}"\)/,
    'void function(){var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+$1+", beerSet:"+$2+", hystMode:"+hystMode+"}")}()'
  );

  // ccparameter sets radios
  if (!out.includes('s.hystMode') && !out.includes('typeof s.hystMode')) {
    out = out.replace(
      /(BrewPiSetting=t;)/,
      '$1if(void 0!==s.hystMode){for(var radios=document.querySelectorAll("input[name=hystMode]"),ri=0;ri<radios.length;ri++)radios[ri].checked=String(radios[ri].value)===String(s.hystMode);}'
    );
    // alternate minify: BrewPiSetting=setting
    out = out.replace(
      /(BrewPiSetting=setting;)/,
      '$1if(typeof s.hystMode!="undefined"){var radios=document.querySelectorAll("input[name=hystMode]");for(var i=0;i<radios.length;i++)radios[i].checked=String(radios[i].value)===String(s.hystMode);}'
    );
  }
  return out;
}

function process(kind, patchFn) {
  for (const lang of langs) {
    const file = path.join(wdocDir, `${lang}_${kind}_htm.h`);
    if (!fs.existsSync(file)) {
      console.log(`[skip] ${file}`);
      continue;
    }
    try {
      const raw = fs.readFileSync(file, 'utf8');
      const { name, bytes } = parseHeader(raw);
      const html = zlib.gunzipSync(bytes).toString('utf8');
      const patched = patchFn(html);
      if (patched === html) {
        console.log(`[noop] ${lang} ${kind}`);
        continue;
      }
      const gz = zlib.gzipSync(Buffer.from(patched, 'utf8'), { level: 9 });
      fs.writeFileSync(file, formatHeader(name, gz));
      console.log(`[ok] ${lang} ${kind}: ${bytes.length}->${gz.length}`);
    } catch (e) {
      console.error(`[fail] ${lang} ${kind}: ${e.message}`);
      process.exitCode = 1;
    }
  }
}

process('config', patchConfig);
process('control', patchControl);
