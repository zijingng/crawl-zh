// Run with node tools/localization/test_webtiles_scroller.js.
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
let handlers, shown;
function node() {
    const n = {classes: {}, 0: {}, clone: () => node(), children: () => node(),
        toggleClass(name, value) { this.classes[name] = value; return this; }};
    for (const method of ["attr", "html", "remove", "on", "off"])
        n[method] = () => n;
    return n;
}
const $ = () => node();
const dependencies = {
    jquery: $, comm: {register_handlers: x => handlers = x}, client: {},
    "./ui": {show_popup: popup => shown = popup},
    "./util": {formatted_string_to_html: x => x},
    "./scroller": () => ({scrollElement: {addEventListener() {}}}),
    "./options": {get: () => false}
};
vm.runInNewContext(fs.readFileSync(path.resolve(__dirname,
    "../../crawl-ref/source/webserver/game_data/static/ui-layouts.js"), "utf8"), {
    define: (names, factory) => factory(...names.map(n => dependencies[n] || {})),
    document: {}, console: {log: err => { throw err; }}
});
for (const wrap_text of [true, false, undefined, "true"]) {
    handlers["ui-push"]({type: "formatted-scroller", tag: "help", text: "Long text",
        highlight: "", title: "", more: "", wrap_text});
    assert.equal(shown.classes["wrap-text"], wrap_text === true);
}
console.log("WebTiles scroller: flow text wraps; preformatted and old messages retain layout.");
