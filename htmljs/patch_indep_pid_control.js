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

const INDEP_NOTE =
  '<div style="margin:16px 0">' +
  '<span>hysteresisMode (PID control)</span>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode id=independent-pid-off value=0> Off (simple thermostat)</label>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode id=independent-pid-on value=1> On (PID on heater; cooler uses cool targets)</label>' +
  '</div>' +
  '<p style="margin:8px 0 16px;font-size:.9em">This mode uses heatTargetH, heatTargetL, coolTargetH, and coolTargetL settings in the Advanced Settings menu for on/off bands.</p>';

function patchControl(html) {
  let out = html;
  if (out.includes('independent-pid-off')) {
    return out;
  }
  const needle =
    'id=independent-heat-t style=max-width:200px>&deg;<span class=t_unit>C</span></div>';
  if (!out.includes(needle)) {
    throw new Error('independent-heat-t HTML block not found');
  }
  out = out.replace(needle, needle + INDEP_NOTE);

  out = out.replace(
    /BWF\.send\("j\{mode:i, fridgeSet:"\+([^+]+)\+", beerSet:"\+([^+]+)\+"\}"\)/,
    'var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+$1+", beerSet:"+$2+", hystMode:"+hystMode+"}")'
  );

  // Minified: function ccparameter(s){var t={valid:!0,...};BrewPiSetting=t;...
  // Minified ccparameter(e){e={valid:!0,... uses e then overwrites it — capture hystMode first.
  if (!out.includes('independent-pid-hyst-loaded') && out.includes('function ccparameter(e){e={valid:!0')) {
    out = out.replace(
      'function ccparameter(e){e={valid:!0',
      'function ccparameter(e){var independentPidHyst=e.hystMode;e={valid:!0'
    );
    out = out.replace(
      'BrewPiSetting=e,',
      'BrewPiSetting=e,void 0!==independentPidHyst&&(function(){for(var radios=document.querySelectorAll("input[name=hystMode]"),ri=0;ri<radios.length;ri++)radios[ri].checked=String(radios[ri].value)===String(independentPidHyst)}()),/*independent-pid-hyst-loaded*/'
    );
  } else if (!out.includes('independent-pid-hyst-loaded')) {
    throw new Error('ccparameter pattern not found');
  }
  return out;
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_control_htm.h`);
  try {
    const raw = fs.readFileSync(file, 'utf8');
    const { name, bytes } = parseHeader(raw);
    const html = zlib.gunzipSync(bytes).toString('utf8');
    const patched = patchControl(html);
    if (patched === html) {
      console.log(`[noop] ${lang}`);
      continue;
    }
    const gz = zlib.gzipSync(Buffer.from(patched, 'utf8'), { level: 9 });
    fs.writeFileSync(file, formatHeader(name, gz));
    console.log(`[ok] ${lang}: ${bytes.length}->${gz.length}`);
  } catch (e) {
    console.error(`[fail] ${lang}: ${e.message}`);
    process.exitCode = 1;
  }
}
