// Update hystMode help text in wdoc/*_config_htm.h after Independent also respects hystMode.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

const OLD =
  'Fridge / beer / profile modes only. Independent mode always uses heat/cool target bands.';
const NEW =
  'Start threshold for fridge / beer / profile and Independent. Stop still uses heat/cool target bands. Independent heater uses beer + idleRangeL / heatTargetL; cooler uses fridge + idleRangeH / coolTargetH.';

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
  const name = m[1];
  const bytes = Buffer.from(
    m[2].split(',').map((s) => s.trim()).filter(Boolean).map((s) => parseInt(s, 16))
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

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  const raw = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(raw);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  if (!html.includes(OLD)) {
    if (html.includes(NEW)) {
      console.log(`[noop] ${lang}`);
      continue;
    }
    console.error(`[fail] ${lang}: old help text not found`);
    process.exitCode = 1;
    continue;
  }
  html = html.split(OLD).join(NEW);
  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(`[ok] ${lang}: ${bytes.length}->${gz.length}`);
}
