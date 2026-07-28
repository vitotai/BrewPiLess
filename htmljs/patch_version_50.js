const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const dir = path.join(__dirname, '..', 'wdoc');
const VER = '5.0';

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) return null;
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

let patched = 0;
for (const f of fs.readdirSync(dir).filter((x) => x.endsWith('.h'))) {
  const file = path.join(dir, f);
  const raw = fs.readFileSync(file, 'utf8');
  const parsed = parseHeader(raw);
  if (!parsed) continue;
  let html;
  try {
    html = zlib.gunzipSync(parsed.bytes).toString('utf8');
  } catch (e) {
    continue;
  }
  let out = html;
  out = out.replace(/JSVERSION\s*=\s*"4\.[0-9]+"/g, 'JSVERSION="' + VER + '"');
  out = out.replace(/JSVERSION\s*=\s*'4\.[0-9]+'/g, "JSVERSION='" + VER + "'");
  out = out.replace(/JSVERSION\s*=\s*"5\.[0-9]+"/g, 'JSVERSION="' + VER + '"');
  out = out.replace(/(id=verinfo[^>]*>)v4\.[0-9]+/g, '$1v' + VER);
  out = out.replace(/(id=verinfo[^>]*>)v5\.[0-9]+/g, '$1v' + VER);
  out = out.replace(/(header_version-display[^>]*>)v4\.[0-9]+/g, '$1v' + VER);
  out = out.replace(/(header_version-display[^>]*>)v5\.[0-9]+/g, '$1v' + VER);
  if (out === html) continue;
  const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(parsed.name, gz));
  console.log('[ok]', f);
  patched++;
}
console.log('patched', patched);
