// Remove Aux/Est.Peak/PID Demand legend swap from wdoc index pages.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

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
  let out = html;
  out = out.replace(/function updateAuxPidLegend\([^)]*\)\{[^}]*\}/g, '');
  out = out.replace(/,updateAuxPidLegend\([^)]*\)/g, '');
  out = out.replace(/;updateAuxPidLegend\([^)]*\)/g, '');
  out = out.replace(/"Est\. Peak"/g, '"Aux Temp"');
  out = out.replace(/"PID Demand"/g, '"Aux Temp"');
  return out;
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_index_htm.h`);
  if (!fs.existsSync(file)) continue;
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  const before = html;
  html = patch(html);
  if (html === before) {
    console.log(lang + ': noop');
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': ok ' + bytes.length + '->' + gz.length);
}
