// Insert oneWireRetryCount into wdoc/*_config_htm.h Advanced Settings.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const FIELD =
  '<div layout horizontal cross-center><h6 class=inline-header>oneWireRetryCount</h6>' +
  '<input type=number id=oneWireRetryCount size=4 min=0 max=20> Additional OneWire read attempts after failure (0 = off, default 2)</div>';

function parseHeader(text) {
  const match = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!match) throw new Error('array not found');
  return {
    name: match[1],
    bytes: Buffer.from(
      match[2]
        .split(',')
        .map((value) => value.trim())
        .filter(Boolean)
        .map((value) => parseInt(value, 16))
    ),
  };
}

function formatHeader(name, buffer) {
  const lines = [`const unsigned char ${name}[] PROGMEM = {`];
  for (let index = 0; index < buffer.length; index += 12) {
    const chunk = [];
    for (let offset = index; offset < Math.min(index + 12, buffer.length); offset++) {
      chunk.push('0x' + buffer[offset].toString(16).padStart(2, '0'));
    }
    lines.push('  ' + chunk.join(', ') + (index + 12 < buffer.length ? ',' : ''));
  }
  lines.push('};');
  lines.push(`unsigned int ${name}_len = ${buffer.length};`);
  lines.push('');
  return lines.join('\n');
}

function patch(html) {
  if (html.includes('id=oneWireRetryCount ') || html.includes('id=oneWireRetryCount>')) {
    return { html, how: 'already' };
  }
  const insertPoint = '<hr><div><input type=checkbox id=warning name=warning>';
  if (!html.includes(insertPoint)) {
    throw new Error('warning checkbox insert point not found');
  }
  let output = html.replace(insertPoint, FIELD + '<hr><div><input type=checkbox id=warning name=warning>');
  if (!/,oneWireRetryCount:/.test(output)) {
    output = output.replace(/staleSensorSec:(\d+),fridgeFastFilt:/, 'staleSensorSec:$1,oneWireRetryCount:2,fridgeFastFilt:');
  }
  return { html: output, how: 'inserted' };
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
  const { html: output, how } = patch(html);
  if (how === 'already') {
    console.log(lang + ': already');
    continue;
  }
  const gzip = zlib.gzipSync(Buffer.from(output, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gzip));
  console.log(lang + ': ' + how + ' ' + bytes.length + '->' + gzip.length);
}