// Fix control page: all mode panels visible because modekeeper JS fails to parse.
// Two missing '}' from enable-switches surgical patch:
// 1) after wireIndep IIFE: need }}/,apply (close if + close init)
// 2) before saveprofile: need }}} (close apply's {{ + close modekeeper)
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const INIT_BROKEN = 'wireIndepActuator("beer-set","independent-heat-t")})()},apply:function(){';
const INIT_FIXED = 'wireIndepActuator("beer-set","independent-heat-t")})()}},apply:function(){';
const END_BROKEN = '})}};function saveprofile';
const END_FIXED = '})}}};function saveprofile';

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
  if (!fs.existsSync(file)) {
    console.log(lang + ': missing');
    continue;
  }
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  let n = 0;

  if (html.includes(INIT_FIXED)) {
    console.log(lang + ': init already');
  } else if (html.includes(INIT_BROKEN)) {
    html = html.replace(INIT_BROKEN, INIT_FIXED);
    n++;
    console.log(lang + ': fixed init');
  } else {
    console.log(lang + ': init pattern missing');
  }

  if (html.includes(END_FIXED)) {
    console.log(lang + ': end already');
  } else if (html.includes(END_BROKEN)) {
    html = html.replace(END_BROKEN, END_FIXED);
    n++;
    console.log(lang + ': fixed end');
  } else {
    // localized? try regex
    const re = /\)\}\};function saveprofile/;
    if (re.test(html)) {
      html = html.replace(re, ')}}};function saveprofile');
      n++;
      console.log(lang + ': fixed end (re)');
    } else {
      console.log(lang + ': end pattern missing');
    }
  }

  const scripts = [];
  html.replace(/<script[^>]*>([\s\S]*?)<\/script>/gi, (_, s) => {
    if (s && s.length > 200) scripts.push(s);
  });
  let ok = true;
  for (let i = 0; i < scripts.length; i++) {
    try {
      new Function(scripts[i]);
    } catch (e) {
      console.log(lang + ': PARSE FAIL', i, e.message);
      ok = false;
    }
  }
  if (!ok) continue;

  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': wrote OK', n, bytes.length, '->', gz.length);
}
