// Patch wdoc/*_config_htm.h: Independent-only note before heatTargetH fields.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = process.argv.slice(2).length
  ? process.argv.slice(2)
  : ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

const NOTE =
  '<p style="margin:.5em 0;font-size:.9em">' +
  'Used as on/off bands in Independent mode: heatTargetH, heatTargetL, coolTargetH, coolTargetL.' +
  '</p>';

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
  const name = m[1];
  const bytes = Buffer.from(
    m[2]
      .split(',')
      .map((s) => s.trim())
      .filter(Boolean)
      .map((s) => parseInt(s, 16))
  );
  return { name, bytes };
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

function patch(html) {
  if (html.includes('Used as on/off bands in Independent mode')) {
    return { html, log: ['note-already'] };
  }
  const needle =
    '</div><div layout horizontal cross-center><h6 class=inline-header>heatTargetH</h6>';
  if (!html.includes(needle)) {
    throw new Error('heatTargetH insert point not found');
  }
  const out = html.replace(
    needle,
    '</div>' + NOTE + '<div layout horizontal cross-center><h6 class=inline-header>heatTargetH</h6>'
  );
  return { html: out, log: ['note-before-heatTargetH'] };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  if (!fs.existsSync(file)) {
    console.log(lang + ': skip (missing)');
    continue;
  }
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  const html = zlib.gunzipSync(bytes).toString('utf8');
  const { html: out, log } = patch(html);
  if (log[0] === 'note-already') {
    console.log(lang + ': ' + log.join(', '));
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': ' + log.join(', ') + ' (' + bytes.length + ' -> ' + gz.length + ')');
}
