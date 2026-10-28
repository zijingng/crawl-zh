// Exercise the unchanged chat handler with the two existing-protocol URLs.
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
const source = fs.readFileSync(path.resolve(__dirname,
    "../../crawl-ref/source/webserver/static/scripts/chat.js"), "utf8");
const start = source.indexOf("    function handle_dump(data)");
const end = source.indexOf("    function ", start + 5);
assert.ok(start >= 0 && end > start);
const messages = [];
const context = {receive_message: msg => messages.push(msg.content), $: () => {
    const span = {0: {}, addClass() { return this; }, text(value) {
        this[0].outerHTML = "<span>" + value + "</span>";
        return this;
    }};
    return span;
}};
vm.runInNewContext(source.slice(start, end) + "\nthis.handle = handle_dump;", context);
context.handle({url: "https://example.test/morgue/Player/Player"});
context.handle({url: "https://example.test/morgue/Player/Player.zh"});
assert.deepEqual(messages, [
    "<span>https://example.test/morgue/Player/Player.txt</span>",
    "<span>https://example.test/morgue/Player/Player.zh.txt</span>"
]);
console.log("Unchanged chat handler displays both English and Chinese dump URLs.");
