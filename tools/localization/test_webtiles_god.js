// Run after generating i18n-catalogue.h: node tools/localization/test_webtiles_god.js
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
const root = path.resolve(__dirname, "../../crawl-ref/source");
let handlers, shown, appended;
function node() {
    const n = {0: {}, length: 4, clone: () => node(), find: () => node(),
        children: () => node(), eq: () => node(), filter: () => node(), index: () => 0,
        append(value) { appended.push(value); return this; }};
    for (const method of ["addClass", "removeClass", "html", "remove", "on", "off"])
        n[method] = () => n;
    return n;
}
let util;
vm.runInNewContext(fs.readFileSync(root + "/webserver/game_data/static/util.js", "utf8"),
    {define: (_, factory) => util = factory()});
util.init_canvas = () => {};
const dependencies = {jquery: () => node(),
    comm: {register_handlers: x => handlers = x},
    "./ui": {show_popup: popup => shown = popup}, "./util": util,
    "./cell_renderer": {DungeonCellRenderer: class {
        constructor() { this.cell_width = 32; this.cell_height = 32; }
        init() {} draw_from_texture() {}
    }},
    "./scroller": () => ({}), "./options": {get: () => false}};
vm.runInNewContext(fs.readFileSync(root + "/webserver/game_data/static/ui-layouts.js", "utf8"), {
    define: (names, factory) => factory(...names.map(n => dependencies[n] || {})),
    document: {}, console: {log: err => { throw err; }}
});
const lines = ["<darkgrey>中文神力描述。 (2 MP)", "<darkgrey>English description. (2 MP)",
    "<darkgrey>没有句号的未来神力", "<darkgrey>中文描述。（无消耗）", "", "<darkgrey>None."];
for (const line of fs.readFileSync(root + "/i18n-catalogue.h", "utf8").split("\n")) {
    if (line.trim().startsWith('{"d.god.power",')) {
        const [, english, chinese] = JSON.parse("[" + line.trim().slice(1, -2).replace(/\\\?/g, "?") + "]");
        lines.push("<darkgrey>" + (/^[A-Z]/.test(english) ? english : "You can " + english + "."));
        lines.push("<darkgrey>" + chinese);
    }
}
assert.ok(lines.length > 20, "Test the current reviewed god power catalogue");
for (const power of lines) {
    appended = []; shown = null;
    handlers["ui-push"]({type: "describe-god", extra: "", colour: 7, name: "God",
        tile: {t: 1, tex: 0}, is_altar: true, service_fee: "", description: "Description",
        title: "", favour: "", powers_list: "\n\nGranted powers:\n" + power + "\n",
        powers: "Powers", info_table: "", wrath: "Wrath"});
    assert.ok(shown, power);
    if (power) assert.ok(appended.some(x => typeof x === "string" && x.includes("class='power")), power);
}
console.log("God altar UI renders " + lines.length + " English/Chinese and unmatched power rows without throwing.");
