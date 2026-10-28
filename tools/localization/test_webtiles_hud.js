// Run with node tools/localization/test_webtiles_hud.js.
// Exercises the real AMD options/player modules without a browser or game server.
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
const root = path.resolve(__dirname, "../../crawl-ref/source/webserver/game_data/static");
const nodes = new Map(), events = {}, handlers = {};
const document = {documentElement: {style: {setProperty() {}}}};
function $(selector) {
    if (typeof selector !== "string") selector = "document";
    if (!nodes.has(selector)) {
        const node = {value: "", attrs: {},
            text(value) { if (value === undefined) return this.value; this.value = value; return this; },
            html(value) { return this.text(value); },
            attr(key, value) { if (value === undefined) return this.attrs[key]; this.attrs[key] = value; return this; },
            on(name, fn) { events[name] = fn; return this; },
            hide() { this.visible = false; return this; },
            show() { this.visible = true; return this; },
            each() { return this; }};
        for (const method of ["css", "toggleClass", "removeClass", "addClass", "append", "off", "trigger", "triggerHandler"])
            node[method] = () => node;
        nodes.set(selector, node);
    }
    return nodes.get(selector);
}
$.extend = Object.assign;
$.Callbacks = () => {
    const callbacks = [];
    return {add: fn => callbacks.push(fn), fire: () => callbacks.forEach(fn => fn())};
};
const comm = {register_handlers: x => Object.assign(handlers, x), send_message() {}};
function load(name, dependencies) {
    let exported;
    vm.runInNewContext(fs.readFileSync(path.join(root, name + ".js"), "utf8"), {
        define: (names, factory) => exported = factory(...names.map(n => dependencies[n])),
        window: {}, document, console
    }, {filename: name + ".js"});
    return exported;
}
const options = load("options", {jquery: $, comm});
assert.equal(options.text("Magic:"), "Magic:");
assert.equal(options.display("Djinni", "巨灵"), "Djinni");
const player = load("player", {jquery: $, comm, client: {is_watching: () => false},
    "./enums": {}, "./map_knowledge": {touch() {}}, "./messages": {new_command() {}},
    "./options": options, "./util": {formatted_string_to_html: x => x}});
events["game_init.player"]();
const settings = {tile_font_stat_size: 0, tile_font_stat_family: "monospace",
    hp_colour: [], mp_colour: [], stat_colour: [], always_show_doom_contam: false, show_game_time: false};
handlers.options({options: {...settings, language: "zh", ui_text: {"Health:": "生命:", "Time:": "时间:"}}});
const update = {name: "Tester", title: "the Fighter", title_line_display: "Tester 战士",
    species: "Djinni", species_display: "巨灵", god: "Gozag", species_god_display: "巨灵，信仰戈扎格",
    place: "Dungeon", place_display: "地下城", depth: 2, hp: 10, hp_max: 10, real_hp_max: 10,
    mp: 5, mp_max: 5, quiver_desc: "Fire: stone", quiver_desc_display: "射击：石头",
    inv: {0: {name: "a dagger", name_display: "匕首"}},
    status: [{text: "silence", light: "Sil", light_display: "沉默", desc: "Silenced", desc_display: "你被沉默了。"}]};
handlers.player(update);
assert.equal($("#stats_titleline").text(), "Tester 战士");
assert.equal($("#stats_species_god").text(), "巨灵，信仰戈扎格");
assert.equal($("#stats_place").text(), "地下城:2");
assert.equal($("#stats_hpline > .stats_caption").text(), "生命:");
assert.equal(player.inventory_item_desc(0).text(), "匕首");
assert.equal(player.has_status("silence"), true); // canonical rule keys survive
assert.equal($("#stats_mpline").visible, false); // Djinni rule still uses English
assert.equal($("#stats_gozag_gold_label").text(), " Gold: ");
assert.equal(player.species, "Djinni");
assert.equal(player.god, "Gozag");
// Missing a component must fall back to the complete original expression.
handlers.player({species_god_display: "", title_line_display: "", place_display: ""});
assert.equal($("#stats_species_god").text(), "Djinni of Gozag");
assert.equal($("#stats_titleline").text(), "Tester the Fighter");
assert.equal($("#stats_place").text(), "Dungeon:2");
// English mode ignores even stale Chinese fields and dictionaries.
handlers.options({options: {...settings, language: "en", ui_text: {"Health:": "生命:"}}});
assert.equal($("#stats_hpline > .stats_caption").text(), "Health:");
assert.equal(player.inventory_item_desc(0).text(), "a dagger");
// Older servers do not send a language field.
handlers.options({options: settings});
assert.equal(options.text("Time:"), "Time:");
assert.equal(options.display("Dungeon", "地下城"), "Dungeon");
options.clear();
assert.equal(options.text("Time:"), "Time:");
events["game_init.player"]();
assert.equal(player.species_god_display, "");
assert.equal(player.title_line_display, "");
console.log("WebTiles HUD language, display, canonical-key and English fallback checks passed.");
