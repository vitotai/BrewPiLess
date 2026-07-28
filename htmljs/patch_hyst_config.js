// Surgical patch: add hystMode radio + show/hide JS into wdoc/*_config_htm.h
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const langs = process.argv.slice(2).length
  ? process.argv.slice(2)
  : ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];
const wdocDir = path.join(__dirname, '..', 'wdoc');

const RADIO_HTML =
  '<div>' +
  '<h6 class=inline-header>Classic start hysteresis</h6>' +
  '<p style="margin:0 0 .5em;font-size:.9em">Fridge / beer / profile modes only. Independent mode always uses heat/cool target bands.</p>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode value=0> Classic idle range</label>' +
  '<label style="display:block;margin:.25em 0"><input type=radio name=hystMode value=1> Heat/cool target bands</label>' +
  '</div>';

function parseHeader(text) {
  const m = text.match(/const unsigned char (\w+)\[\] PROGMEM = \{([\s\S]*?)\};/);
  if (!m) throw new Error('array not found');
  const name = m[1];
  const bytes = Buffer.from(
    m[2]
      .split(',')
      .map((s) => s.trim())
      .filter(Boolean)
      .map((s) => parseInt(s, 16))
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

function patch(html) {
  let out = html;
  const log = [];

  // 1) Insert radio UI between iMaxErr block and idleRangeH (or heatTargetH)
  if (!/name=hystMode/.test(out)) {
    const needle =
      '</div><div layout horizontal cross-center><h6 class=inline-header>idleRangeH</h6>';
    const needleHeat =
      '</div><div layout horizontal cross-center><h6 class=inline-header>heatTargetH</h6>';
    if (out.includes(needle)) {
      out = out.replace(needle, '</div>' + RADIO_HTML + '<div layout horizontal cross-center><h6 class=inline-header>idleRangeH</h6>');
      log.push('radio-before-idleRange');
    } else if (out.includes(needleHeat)) {
      // idleRange absent: insert radio + idleRange fields before heatTargetH
      const fields =
        RADIO_HTML +
        '<div id=idleRangeFields layout horizontal cross-center><h6 class=inline-header>idleRangeH</h6><input type=number id=idleRangeH size=4 step=0.001> &deg;</div>' +
        '<div id=idleRangeFieldsL layout horizontal cross-center><h6 class=inline-header>idleRangeL</h6><input type=number id=idleRangeL size=4 step=0.001> &deg;</div>';
      out = out.replace(needleHeat, '</div>' + fields + '<div layout horizontal cross-center><h6 class=inline-header>heatTargetH</h6>');
      log.push('radio+idleRange-before-heatTarget');
    } else {
      throw new Error('insert point for radio not found');
    }
  } else {
    log.push('radio-already');
  }

  // 2) Add ids on idleRange wrapper divs for show/hide
  if (!out.includes('id=idleRangeFields')) {
    out = out.replace(
      '<div layout horizontal cross-center><h6 class=inline-header>idleRangeH</h6>',
      '<div id=idleRangeFields layout horizontal cross-center><h6 class=inline-header>idleRangeH</h6>'
    );
    out = out.replace(
      '<div layout horizontal cross-center><h6 class=inline-header>idleRangeL</h6>',
      '<div id=idleRangeFieldsL layout horizontal cross-center><h6 class=inline-header>idleRangeL</h6>'
    );
    log.push('idleRange-ids');
  }

  // 3) initAdvanced onchange
  const oldInit =
    'e[n].onchange=function(){window.ccModified[this.id]=this.value,checkModify()}';
  const newInit =
    'e[n].onchange=function(){this.name==="hystMode"?(window.ccModified.hystMode=this.value,updateHysteresisUi(this.value)):window.ccModified[this.id]=this.value,checkModify()}';
  if (out.includes(oldInit)) {
    out = out.replace(oldInit, newInit);
    log.push('initAdvanced');
  } else if (!out.includes('updateHysteresisUi(this.value)')) {
    throw new Error('initAdvanced pattern not found');
  } else {
    log.push('initAdvanced-already');
  }

  // 4) updateHysteresisUi helper
  if (!out.includes('function updateHysteresisUi')) {
    const fn =
      'function updateHysteresisUi(e){var n="0"===String(e),t=Q("#idleRangeFields"),i=Q("#idleRangeFieldsL");t&&(t.style.display=n?"":"none"),i&&(i.style.display=n?"":"none")}';
    if (!out.includes('function checkModify()')) throw new Error('checkModify not found');
    out = out.replace('function checkModify()', fn + 'function checkModify()');
    log.push('updateHysteresisUi');
  } else {
    log.push('updateHysteresisUi-already');
  }

  // 5) cResponse
  const oldCR =
    'Object.keys(n).forEach(e=>{Q("#"+e).value=n[e]})';
  const newCR =
    'Object.keys(n).forEach(e=>{if("hystMode"===e){for(var t=document.querySelectorAll("input[name=hystMode]"),i=0;i<t.length;i++)t[i].checked=String(t[i].value)===String(n[e]);updateHysteresisUi(n[e])}else{var a=Q("#"+e);a&&(a.value=n[e])}})';
  if (out.includes(oldCR)) {
    out = out.replace(oldCR, newCR);
    log.push('cResponse');
  } else if (!out.includes('"hystMode"===e')) {
    throw new Error('cResponse pattern not found');
  } else {
    log.push('cResponse-already');
  }

  // 6) testConfig: ensure hystMode present
  if (!/hystMode\s*:/.test(out)) {
    if (/iMaxErr:[^,]+,idleRangeH:/.test(out)) {
      out = out.replace(/iMaxErr:([^,]+),idleRangeH:/, 'iMaxErr:$1,hystMode:0,idleRangeH:');
      log.push('testConfig-hystMode');
    } else if (/iMaxErr:[^,]+,heatTargetH:/.test(out)) {
      out = out.replace(
        /iMaxErr:([^,]+),heatTargetH:/,
        'iMaxErr:$1,hystMode:0,idleRangeH:1,idleRangeL:-1,heatTargetH:'
      );
      log.push('testConfig-hystMode+idle');
    } else {
      throw new Error('testConfig pattern not found');
    }
  } else {
    log.push('testConfig-already');
  }

  return { html: out, log };
}

function processLang(lang) {
  const file = path.join(wdocDir, `${lang}_config_htm.h`);
  if (!fs.existsSync(file)) {
    console.log(`[skip] missing ${file}`);
    return;
  }
  const raw = fs.readFileSync(file, 'utf8');
  const { name, bytes } = parseHeader(raw);
  const html = zlib.gunzipSync(bytes).toString('utf8');
  const { html: patched, log } = patch(html);
  if (patched === html) {
    console.log(`[noop] ${lang}: ${log.join(',')}`);
    return;
  }
  const gz = zlib.gzipSync(Buffer.from(patched, 'utf8'), { level: 9 });
  fs.writeFileSync(file, formatHeader(name, gz));
  console.log(`[ok] ${lang}: ${log.join(',')} gz ${bytes.length}->${gz.length}`);
}

for (const lang of langs) {
  try {
    processLang(lang);
  } catch (e) {
    console.error(`[fail] ${lang}: ${e.message}`);
    process.exitCode = 1;
  }
}
