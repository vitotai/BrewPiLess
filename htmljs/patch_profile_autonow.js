// Patch wdoc control pages: empty-profile auto-now + Add uses live beer temp.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
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

const SELECT_OLD =
  'select:function(e){document.getElementById(e+"-m").className+=" nav-selected",document.getElementById(e+"-s").style.display="block"}';
const SELECT_NEW =
  'select:function(e){document.getElementById(e+"-m").className+=" nav-selected",document.getElementById(e+"-s").style.display="block","profile"==e&&0==profileEditor.rowList().length&&profileEditor.startnow()}';

const TEMP_OLD = 't:"C"==BrewPiSetting.tempUnit?20:68';
const TEMP_NEW =
  't:(function(){var t=window.lastBeerTemp;return"number"==typeof t&&!isNaN(t)&&t>-100&&t<100?Math.round(10*t)/10:"C"==BrewPiSetting.tempUnit?20:68})()';

const A_OLD = 'handlers:{A:function(e){void 0!==e.nn&&';
const A_NEW =
  'handlers:{A:function(e){void 0!==e.bt&&-10000<e.bt&&(window.lastBeerTemp=e.bt/100),void 0!==e.nn&&';

function patch(html) {
  let out = html;
  let n = 0;
  if (out.includes(SELECT_NEW)) {
    console.log('  select already');
  } else if (out.includes(SELECT_OLD)) {
    out = out.replace(SELECT_OLD, SELECT_NEW);
    n++;
  } else {
    throw new Error('select pattern not found');
  }
  if (out.includes(TEMP_NEW)) {
    console.log('  temp already');
  } else if (out.includes(TEMP_OLD)) {
    out = out.replace(TEMP_OLD, TEMP_NEW);
    n++;
  } else {
    throw new Error('addRow temp pattern not found');
  }
  if (out.includes('window.lastBeerTemp=e.bt/100')) {
    console.log('  bt already');
  } else if (out.includes(A_OLD)) {
    out = out.replace(A_OLD, A_NEW);
    n++;
  } else {
    throw new Error('A handler pattern not found');
  }
  return { out, n };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_control_htm.h`);
  const raw = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(raw);
  const html = zlib.gunzipSync(bytes).toString('utf8');
  console.log(lang);
  const { out, n } = patch(html);
  if (n === 0) {
    console.log('  noop');
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log('  patched', n, bytes.length, '->', gz.length);
}
