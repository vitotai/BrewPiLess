// Patch wdoc index pages: Aux Temp / PID Demand legend swap on WS `pd`.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const files = fs.readdirSync(wdocDir).filter(
  (f) => f.endsWith('_htm.h') && (f.includes('_index_') || f.includes('index_s') || f.includes('_indexs_'))
);

const NEEDLE =
  'function displayLcd(info){window.tempUnit=info.tu,displayLcdText(simLcd(info)),renderLcdText(info)}';
const REPLACEMENT =
  'function updateAuxPidLegend(e){var t=document.querySelector("label[for=aux-temp]");if(t){t.getAttribute("data-label-aux")||t.setAttribute("data-label-aux",t.textContent.trim()||"Aux Temp");var n=1==e?"PID Demand":t.getAttribute("data-label-aux");t.textContent!==n&&(t.textContent=n)}}' +
  'function displayLcd(info){window.tempUnit=info.tu,displayLcdText(simLcd(info)),renderLcdText(info),updateAuxPidLegend(info.pd)}';

// Also match non-minified / alternate spacing from other builds
const NEEDLES = [
  NEEDLE,
  'function displayLcd(info){window.tempUnit=info.tu;displayLcdText(simLcd(info));renderLcdText(info)}',
  'function displayLcd(t){window.tempUnit=t.tu,displayLcdText(simLcd(t)),renderLcdText(t)}',
];

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

let ok = 0;
for (const f of fs.readdirSync(wdocDir).filter((x) => x.endsWith('.h'))) {
  if (!/index/i.test(f)) continue;
  const file = path.join(wdocDir, f);
  const text = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(text);
  let html = zlib.gunzipSync(bytes).toString('utf8');
  if (html.includes('updateAuxPidLegend')) {
    console.log(f + ': already');
    continue;
  }
  let replaced = false;
  for (const n of NEEDLES) {
    if (html.includes(n)) {
      // Build matching replacement for this needle's param name
      let rep = REPLACEMENT;
      if (n.includes('(t)')) {
        rep =
          'function updateAuxPidLegend(e){var t=document.querySelector("label[for=aux-temp]");if(t){t.getAttribute("data-label-aux")||t.setAttribute("data-label-aux",t.textContent.trim()||"Aux Temp");var n=1==e?"PID Demand":t.getAttribute("data-label-aux");t.textContent!==n&&(t.textContent=n)}}' +
          'function displayLcd(t){window.tempUnit=t.tu,displayLcdText(simLcd(t)),renderLcdText(t),updateAuxPidLegend(t.pd)}';
      } else if (n.includes('window.tempUnit=info.tu;')) {
        rep =
          'function updateAuxPidLegend(e){var t=document.querySelector("label[for=aux-temp]");if(t){t.getAttribute("data-label-aux")||t.setAttribute("data-label-aux",t.textContent.trim()||"Aux Temp");var n=1==e?"PID Demand":t.getAttribute("data-label-aux");t.textContent!==n&&(t.textContent=n)}}' +
          'function displayLcd(info){window.tempUnit=info.tu;displayLcdText(simLcd(info));renderLcdText(info);updateAuxPidLegend(info.pd)}';
      }
      html = html.replace(n, rep);
      replaced = true;
      break;
    }
  }
  if (!replaced) {
    // Fuzzy: find displayLcd and inject call
    const m = html.match(/function displayLcd\((\w+)\)\{([^}]{0,200})\}/);
    if (m && !m[0].includes('updateAuxPidLegend')) {
      const arg = m[1];
      const body = m[2];
      if (body.includes('renderLcdText')) {
        const helper =
          'function updateAuxPidLegend(e){var t=document.querySelector("label[for=aux-temp]");if(t){t.getAttribute("data-label-aux")||t.setAttribute("data-label-aux",t.textContent.trim()||"Aux Temp");var n=1==e?"PID Demand":t.getAttribute("data-label-aux");t.textContent!==n&&(t.textContent=n)}}';
        const newFn =
          helper +
          'function displayLcd(' +
          arg +
          '){' +
          body.replace(/renderLcdText\(([^)]+)\)/, 'renderLcdText($1),updateAuxPidLegend(' + arg + '.pd)') +
          '}';
        // If body used semicolons, comma inject may be wrong — try append instead
        if (body.includes(';')) {
          const newBody = body.replace(
            /renderLcdText\(([^)]+)\);?/,
            'renderLcdText($1);updateAuxPidLegend(' + arg + '.pd);'
          );
          html = html.replace(m[0], helper + 'function displayLcd(' + arg + '){' + newBody + '}');
        } else {
          html = html.replace(m[0], newFn);
        }
        replaced = true;
      }
    }
  }
  if (!replaced) {
    console.log(f + ': FAIL no displayLcd match');
    continue;
  }
  const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(f + ': patched (' + bytes.length + ' -> ' + gz.length + ')');
  ok++;
}
console.log('done', ok);
