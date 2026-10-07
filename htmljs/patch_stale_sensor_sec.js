// Insert staleSensorSec field into wdoc/*_config_htm.h Advanced Settings.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const FIELD =
  '<div layout horizontal cross-center><h6 class=inline-header>staleSensorSec</h6>' +
  '<input type=number id=staleSensorSec size=4 min=0 max=120> Seconds (hold last temp on brief disconnect; 0 = off, default 20)</div>';

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
  if (html.includes('id=staleSensorSec ') || html.includes('id=staleSensorSec>')) {
    return { html, how: 'already' };
  }
  const needle = '<hr><div><input type=checkbox id=warning name=warning>';
  if (!html.includes(needle)) {
    throw new Error('warning checkbox insert point not found');
  }
  let out = html.replace(needle, FIELD + '<hr><div><input type=checkbox id=warning name=warning>');
  if (!/,staleSensorSec:/.test(out)) {
    out = out.replace(/deadTime:(\d+),fridgeFastFilt:/, 'deadTime:$1,staleSensorSec:20,fridgeFastFilt:');
    if (!/,staleSensorSec:/.test(out)) {
      out = out.replace(/deadTime:(\d+),lah:/, 'deadTime:$1,staleSensorSec:20,lah:');
    }
  }
  return { html: out, how: 'inserted' };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  if (!fs.existsSync(file)) {
    console.log(lang + ': missing');
    continue;
  }
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
