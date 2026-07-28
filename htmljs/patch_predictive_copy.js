// Update Independent predictive-heater copy in wdoc control + index legend.
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const wdocDir = path.join(__dirname, '..', 'wdoc');
const langs = ['english', 'spanish', 'slovak', 'portuguese-br', 'norwegian', 'italian', 'chinese'];

const ON_OLD_A = 'On (early shutoff using beer heatEstimator; fridge ignored for heater)';
const ON_OLD_B = 'On (PID on heater; cooler uses cool targets)';
const ON_NEW = 'On (time+slope learner on beer; cycle1 measure, then learned on-time + same-cycle top-up)';

const HELP_OLD_A =
  'Uses heatTargetH/L and coolTargetH/L from Advanced Settings. Predictive On: cycle 1 measures thermostat on-time to beerSet+heatTargetH; later cycles heat for the learned seconds; if the beer peak is low, a same-cycle top-up runs after the peak falls (slow filter, 3 s). Overshoot trims the next cycle by at most 10 s. Cooler is unchanged.';
const HELP_OLD_B =
  'This mode uses heatTargetH, heatTargetL, coolTargetH, and coolTargetL settings in the Advanced Settings menu for on/off bands. Predictive On starts on beer+heatTargetL and stops early from estimated beer peak (same BrewPi estimator idea as classic heating).';
const HELP_OLD_C =
  'This mode uses heatTargetH, heatTargetL, coolTargetH, and coolTargetL settings in the Advanced Settings menu for on/off bands.';
const HELP_NEW =
  'Uses heatTargetH/L and coolTargetH/L from Advanced Settings. Predictive On: cycle 1 measures thermostat on-time to beerSet+heatTargetH; later cycles heat for the learned seconds; if the beer peak is low, a same-cycle top-up runs after the peak falls (slow filter, 3 s). Over/under next-cycle correction uses full slope-based seconds; changes under 10 s are ignored (deadband). Heater always cuts if beer reaches the upper target. Cooler is unchanged.';

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

function patchControl(html) {
  let out = html;
  out = out.replace(/hysteresisMode \(PID control\)/g, 'hysteresisMode (predictive heater)');
  out = out.replace(/Off \(simple thermostat\)/g, 'Off (simple thermostat on beer)');
  if (out.includes(ON_OLD_A)) out = out.split(ON_OLD_A).join(ON_NEW);
  if (out.includes(ON_OLD_B)) out = out.split(ON_OLD_B).join(ON_NEW);
  if (out.includes(HELP_OLD_A)) out = out.split(HELP_OLD_A).join(HELP_NEW);
  else if (out.includes(HELP_OLD_B)) out = out.split(HELP_OLD_B).join(HELP_NEW);
  else if (out.includes(HELP_OLD_C)) out = out.split(HELP_OLD_C).join(HELP_NEW);
  return out;
}

function patchIndex(html) {
  return html.replace(/"PID Demand"/g, '"Est. Peak"');
}

for (const lang of langs) {
  for (const kind of ['control', 'index']) {
    const file = path.join(wdocDir, `${lang}_${kind}_htm.h`);
    if (!fs.existsSync(file)) continue;
    const text = fs.readFileSync(file, 'utf8');
    const { name, bytes } = parseHeader(text);
    let html = zlib.gunzipSync(bytes).toString('utf8');
    const before = html;
    html = kind === 'control' ? patchControl(html) : patchIndex(html);
    if (html === before) {
      console.log(lang + ' ' + kind + ': noop');
      continue;
    }
    const gz = zlib.gzipSync(Buffer.from(html, 'utf8'), { level: 9 });
    fs.writeFileSync(file, formatHeader(name, gz));
    console.log(lang + ' ' + kind + ': ok ' + bytes.length + '->' + gz.length);
  }
}
