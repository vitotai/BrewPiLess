// Reorder independent enable switches: temp left, switch+label right.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

const HTML_OLD =
  '<div style="margin:16px 0" layout horizontal cross-center>' +
  '<input type=checkbox id=fridge-temp name=indep-cooler-en class=switch-input checked>' +
  '<label for=fridge-temp class=switch-label>Cooling Target</label>' +
  '<input type=number size=6 id=independent-cool-t style=max-width:200px lang=en inputmode=decimal>&deg;<span class=t_unit>C</span></div>' +
  '<div style="margin:16px 0" layout horizontal cross-center>' +
  '<input type=checkbox id=beer-set name=indep-heater-en class=switch-input checked>' +
  '<label for=beer-set class=switch-label>Heating Target</label>' +
  '<input type=number size=6 id=independent-heat-t style=max-width:200px lang=en inputmode=decimal>&deg;<span class=t_unit>C</span></div>';

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

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
  return {
    name: m[1],
    bytes: Buffer.from(
      m[2]
        .split(',')
        .map((s) => s.trim())
        .filter(Boolean)
        .map((s) => parseInt(s, 16))
    ),
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

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_control_htm.h`);
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  let html = zlib.gunzipSync(bytes).toString('utf8');

  if (html.includes('Enable/Disable COOLER')) {
    console.log(lang + ': already');
    continue;
  }
  if (!html.includes(HTML_OLD)) {
    // show nearby for debug
    const i = html.indexOf('independent-cool-t');
    console.log(lang + ': OLD not found, snippet:', html.slice(Math.max(0, i - 120), i + 200));
    continue;
  }
  html = html.replace(HTML_OLD, HTML_NEW);

  const scripts = [];
  html.replace(/<script[^>]*>([\s\S]*?)<\/script>/gi, (_, s) => {
    if (s && s.length > 200) scripts.push(s);
  });
  for (let i = 0; i < scripts.length; i++) {
    try {
      new Function(scripts[i]);
    } catch (e) {
      console.log(lang + ': PARSE FAIL', i, e.message);
      process.exitCode = 1;
      continue;
    }
  }

  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': ok', bytes.length, '->', gz.length);
}
