// Fix <divL from bad id=idleRangeFields* strip (prefix collision with idleRangeFieldsL).
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

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  if (!fs.existsSync(file)) continue;
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  if (!html.includes('<divL')) {
    console.log(lang + ': ok (no divL)');
    continue;
  }
  const n = (html.match(/<divL/g) || []).length;
  html = html.replace(/<divL\b/g, '<div');
  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': fixed ' + n + ' divL');
}
