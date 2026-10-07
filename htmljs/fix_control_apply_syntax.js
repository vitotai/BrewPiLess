// Fix broken Independent Apply patch in wdoc/*_control_htm.h
// Bug: surgical patch inserted `var pidRadio=...` inside a ternary, causing
 // SyntaxError so modekeeper.init() never ran and all mode panels stayed visible.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const BROKEN =
  ':var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+l+", beerSet:"+o+", hystMode:"+hystMode+"}")';

const FIXED =
  ':void function(){var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+l+", beerSet:"+o+", hystMode:"+hystMode+"}")}()';

// Alternate minified var names (e/t instead of l/o)
const BROKEN2 =
  /:var pidRadio=document\.querySelector\("input\[name=hystMode\]:checked"\),hystMode=pidRadio\?pidRadio\.value:"0";BWF\.send\("j\{mode:i, fridgeSet:"\+(\w+)\+", beerSet:"\+(\w+)\+", hystMode:"\+hystMode\+"\}"\)/;

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

function fixHtml(html) {
  if (html.includes(BROKEN)) {
    return { html: html.replace(BROKEN, FIXED), how: 'l/o' };
  }
  const m = html.match(BROKEN2);
  if (m) {
    const fixed =
      ':void function(){var pidRadio=document.querySelector("input[name=hystMode]:checked"),hystMode=pidRadio?pidRadio.value:"0";BWF.send("j{mode:i, fridgeSet:"+' +
      m[1] +
      '+", beerSet:"+' +
      m[2] +
      '+", hystMode:"+hystMode+"}")}()';
    return { html: html.replace(m[0], fixed), how: m[1] + '/' + m[2] };
  }
  if (html.includes(':void function(){var pidRadio=document.querySelector("input[name=hystMode]:checked")')) {
    return { html, how: 'already-fixed' };
  }
  // Detect any remaining illegal `:var pidRadio`
  if (html.includes(':var pidRadio=')) {
    throw new Error('unrecognized :var pidRadio pattern');
  }
  return { html, how: 'no-broken-pattern' };
}

for (const lang of langs) {
  const file = path.join(wdocDir, `${lang}_control_htm.h`);
  if (!fs.existsSync(file)) {
    console.log(lang + ': missing');
    continue;
  }
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  const html = zlib.gunzipSync(bytes).toString('utf8');
  const { html: out, how } = fixHtml(html);
  if (how === 'already-fixed' || how === 'no-broken-pattern') {
    console.log(lang + ': ' + how);
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(out, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(lang + ': fixed (' + how + ') ' + bytes.length + ' -> ' + gz.length);
}

// Also harden the source patch script so re-running won't break again
const patchFile = path.join(__dirname, 'patch_indep_pid_control.js');
if (fs.existsSync(patchFile)) {
  let p = fs.readFileSync(patchFile, 'utf8');
  const bad =
    "'var pidRadio=document.querySelector(\"input[name=hystMode]:checked\"),hystMode=pidRadio?pidRadio.value:\"0\";BWF.send(\"j{mode:i, fridgeSet:\"+$1+\", beerSet:\"+$2+\", hystMode:\"+hystMode+\"}\")'";
  const good =
    "'void function(){var pidRadio=document.querySelector(\"input[name=hystMode]:checked\"),hystMode=pidRadio?pidRadio.value:\"0\";BWF.send(\"j{mode:i, fridgeSet:\"+$1+\", beerSet:\"+$2+\", hystMode:\"+hystMode+\"}\")}()'";
  if (p.includes(bad)) {
    p = p.replace(bad, good);
    fs.writeFileSync(patchFile, p);
    console.log('hardened patch_indep_pid_control.js');
  }
}
