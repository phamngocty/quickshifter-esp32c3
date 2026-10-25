const fs = require('fs');
const html = fs.readFileSync('data/index.html', 'utf8');

// Simple DOM mock
const elements = {};
function createEl(tag, id, val) {
  return {
    id,
    value: val || '',
    innerText: '',
    textContent: '',
    classList: {
      add: function(c) { this[c] = true; },
      remove: function(c) { delete this[c]; },
      toggle: function(c, v) { if (v) this[c]=true; else delete this[c]; }
    },
    style: {}
  };
}

elements['lockPassInput'] = createEl('input', 'lockPassInput', '1001');
for (let i = 0; i < 4; i++) elements['morseBox' + i] = createEl('div', 'morseBox' + i, '');
elements['txtMorseDuration'] = createEl('div', 'txtMorseDuration', '');
elements['lockStatusLabel'] = createEl('div', 'lockStatusLabel', '');
elements['lockShortMax'] = createEl('input', 'lockShortMax', '280');
elements['lockLongMin'] = createEl('input', 'lockLongMin', '550');

const document = {
  getElementById: (id) => elements[id],
  querySelectorAll: () => []
};
const window = { location: { protocol: 'http:' }, addEventListener: () => {} };
function markDirty() {}
function playBeep() {}
function showToast(msg) { console.log('Toast:', msg); }

// Extract script
const scriptMatch = html.match(/<script[\s\S]*?<\/script>/gi)[0];
const scriptCode = scriptMatch.replace(/^<script[^>]*>/i, '').replace(/<\/script>$/i, '');

// Evaluate script in context
const fn = new Function('document', 'window', 'markDirty', 'playBeep', 'showToast', scriptCode + '; return { onLockPassInput, renderMorseTargetBoxes, handleMorseLeverPress, updateMorseBoxes };');
const res = fn(document, window, markDirty, playBeep, showToast);

console.log('Testing renderMorseTargetBoxes():');
res.renderMorseTargetBoxes('1001');
console.log('Boxes after 1001:', [0,1,2,3].map(i => elements['morseBox' + i].innerText));

console.log('Testing onLockPassInput(0110):');
res.onLockPassInput('0110');
console.log('Boxes after onLockPassInput(0110):', [0,1,2,3].map(i => elements['morseBox' + i].innerText));
console.log('Input value:', elements['lockPassInput'].value);

console.log('Testing onLockPassInput(1):');
res.onLockPassInput('1');
console.log('Boxes after onLockPassInput(1):', [0,1,2,3].map(i => elements['morseBox' + i].innerText));

console.log('Testing onLockPassInput empty:');
res.onLockPassInput('');
console.log('Boxes after empty:', [0,1,2,3].map(i => elements['morseBox' + i].innerText));
