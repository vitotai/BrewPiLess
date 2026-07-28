// Insert maxHeatTime field into wdoc/*_config_htm.h Advanced Settings.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const FIELD =
  '<div layout horizontal cross-center><h6 class=inline-header>maxHeatTime</h6>' +
  '<input type=number id=maxHeatTime size=4> Seconds (0 = unlimited)</div>';

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

function patch(html) {
  if (html.includes('id=maxHeatTime ') || html.includes('id=maxHeatTime>')) {
    return { html, how: 'already' };
  }
  const needle =
    '</div><div layout horizontal cross-center><h6 class=inline-header>minHeatIdleTime</h6>';
  if (!html.includes(needle)) {
    throw new Error('minHeatIdleTime insert point not found');
  }
  let out = html.replace(needle, '</div>' + FIELD + '<div layout horizontal cross-center><h6 class=inline-header>minHeatIdleTime</h6>');
  // testConfig
  if (!/,maxHeatTime:/.test(out)) {
    out = out.replace(/minHeatTime:(\d+),minHeatIdleTime:/, 'minHeatTime:$1,maxHeatTime:0,minHeatIdleTime:');
  }
  return { html: out, how: 'inserted' };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  if (!fs.existsSync(file)) continue;
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  const html = zlib.gunzipSync(bytes).toString('utf8');
  const { html: out, how } = patch(html);
  if (how === 'already') {
    console.log(lang + ': already');
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': ' + how + ' ' + bytes.length + '->' + gz.length);
}
