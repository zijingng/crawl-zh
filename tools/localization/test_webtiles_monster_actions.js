const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const root = path.resolve(__dirname, '../../crawl-ref/source/webserver/game_data/static');
function extract(file, name) {
    const text = fs.readFileSync(path.join(root, file), 'utf8');
    const begin = text.indexOf('    function ' + name + '(');
    assert.ok(begin >= 0, name);
    const next = text.indexOf('\n    function ', begin + 1);
    return text.slice(begin, next >= 0 ? next : text.length);
}
const actions = {};
vm.runInNewContext(extract('ui-layouts.js', 'clickify_action') + '\n'
    + extract('ui-layouts.js', 'clickify_actions'), actions);
function checkActions(text, expected, labels) {
    const html = actions.clickify_actions(text);
    const links = [...html.matchAll(/<span data-hotkey='(.)'>(.*?)<\/span>/g)];
    assert.deepEqual(links.map(x => x[1]), expected);
    assert.equal(html.replace(/<\/?span(?: data-hotkey='.')?>/g, ''), text);
    for (let i = 0; i < links.length; ++i) {
        assert.equal((links[i][2].match(/\(.\)/g) || []).length, 1);
        if (labels) assert.ok(links[i][2].includes(labels[i]));
    }
}
checkActions('(u)放下武器，(d)丢弃，(=)调整字母，(s)训练目标，或(i)铭文。',
    ['u', 'd', '=', 's', 'i'], ['放下武器', '丢弃', '调整字母', '训练目标', '铭文']);
checkActions('(u)nwield, (d)rop, (=)adjust, (s)kill target, or (i)nscribe.', ['u', 'd', '=', 's', 'i']);
checkActions('(w)持用，或(g)拾取。', ['w', 'g']);
checkActions('(p)戴上,(d)丢弃, or (i)nscribe.', ['p', 'd', 'i']);
checkActions('e(v)oke, or (q)uiver.', ['v', 'q']);
checkActions('(r)阅读。', ['r']);
checkActions('', []);
const state = {options: {language: 'zh'}};
vm.runInNewContext(extract('options.js', 'is_chinese') + '\n'
    + extract('options.js', 'display'), state);
const list = {options: {display: (...args) => state.display(...args)}};
vm.runInNewContext(extract('monster_list.js', 'monster_list_name'), list);
const mon = {name: 'eight-headed hydra', plural: 'eight-headed hydras', name_display: '八头海德拉'};
assert.equal(list.monster_list_name(mon, 1), '八头海德拉');
assert.equal(list.monster_list_name(mon, 3), '3 × 八头海德拉');
state.options.language = 'en';
assert.equal(list.monster_list_name(mon, 1), 'eight-headed hydra');
assert.equal(list.monster_list_name(mon, 3), '3 eight-headed hydras');
state.options.language = 'zh';
for (const missing of ['', undefined, null]) {
    const legacy = {...mon, name_display: missing};
    assert.equal(list.monster_list_name(legacy, 1), mon.name);
    assert.equal(list.monster_list_name(legacy, 3), '3 ' + mon.plural);
}
state.options = {};
assert.equal(list.monster_list_name(mon, 1), mon.name);
assert.equal(JSON.stringify(mon), JSON.stringify({name: 'eight-headed hydra', plural: 'eight-headed hydras', name_display: '八头海德拉'}));
const source = fs.readFileSync(path.join(root, 'monster_list.js'), 'utf8');
assert.ok(source.includes('group.name_span.text(monster_list_name(monsters[0].mon, 1))'));
assert.ok(source.includes('group.name_span.text(monster_list_name(monsters[0].mon, monsters.length))'));
console.log('WebTiles Chinese/English monster labels and independent action hotkeys passed.');
