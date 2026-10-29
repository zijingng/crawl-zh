// Exercise the real mouse tooltip and monster description modules.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const root = path.resolve(__dirname, '../../crawl-ref/source');
let handlers = {}, events = {}, html = [], shown = false;
const escape = s => s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
function element() {
    let value = '';
    const e = {0:{}, length:3,
        html(v) { if (v === undefined) return escape(value); html.push(v); return e; },
        text(v) {value=v; return e;}, on(name,fn) {events[name]=fn; return e;},
        show() {shown=true; return e;}};
    for (const method of ['off','css','hide','contextmenu','clone','find','children',
                          'eq','filter','parent','remove','addClass','removeClass','append','attr'])
        e[method]=()=>e;
    e.index=()=>0;
    return e;
}
const $ = () => element();
$.Callbacks=()=>({add(){}, fire(){}});
const comm = {register_handlers:h=>Object.assign(handlers,h)};
let options;
vm.runInNewContext(fs.readFileSync(root+'/webserver/game_data/static/options.js','utf8'),
    {define:(_,f)=>options=f($,comm),window:{}});
let cell={mon:{name:'a rat',name_display:'老鼠'}};
const deps={jquery:$, comm, './options':options,
    './map_knowledge':{get:()=>cell,visible:()=>true},
    './enums':{prepare_fg_flags:()=>0},
    './ui':{show_popup:()=>shown=true},
    './util':{formatted_string_to_html:s=>s,init_canvas(){}},
    './cell_renderer':{DungeonCellRenderer:class {
        constructor(){this.cell_width=this.cell_height=32;}
        init(){} draw_foreground(){}
    }},'./scroller':()=>({}),
    './tileinfo-main':{MAIN_MAX:100},'./tileinfo-player':{MCACHE_START:1000}};
const environment={define:(names,f)=>f(...names.map(n=>deps[n]||{})),
    document:{},window:{},console:{log:e=>{throw e;}},
    game:{can_target:()=>true,can_describe:()=>true}};
for(const name of ['mouse_control.js','ui-layouts.js'])
    vm.runInNewContext(fs.readFileSync(root+'/webserver/game_data/static/'+name,'utf8'),environment);
events['game_init.mouse_control']();
const dictionary={};
for(const line of fs.readFileSync(root+'/i18n-catalogue.h','utf8').split('\n')) {
    if(!line.trim().startsWith('{"'))continue;
    const [context,key,value]=JSON.parse('['+line.trim().slice(1,-2).replace(/\\\?/g,'?')+']');
    if(context==='web.monster'||(context==='web.ui'&&key==='Right click: describe'))dictionary[key]=value;
}
assert.equal(dictionary.Description,'说明');
for(const language of ['zh','en',undefined]) {
    handlers.options({options:{language,ui_text:dictionary}});
    html=[];shown=false;
    events.cell_tooltip({cell:{x:0,y:0},pageX:0,pageY:0});
    assert.ok(shown);
    const tooltip=html.at(-1);
    assert.ok(tooltip.startsWith(language==='zh'?'老鼠':'a rat'));
    assert.ok(tooltip.includes(language==='zh'?'左键：选择目标':'Left click: select target'));
    assert.ok(tooltip.includes(language==='zh'?'右键：查看说明':'Right click: describe'));
    for(const quote of ['','A quote'])for(const status of ['','A status']) {
        html=[];shown=false;
        handlers['ui-push']({type:'describe-monster',title:'rat',body:'body',quote,status,
            spellset:[],fg_idx:0,flag:0,icons:[]});
        assert.ok(shown);
        const footer=html.find(s=>typeof s==='string'&&s.startsWith('<b class="fg15">'));
        assert.ok(footer.includes(language==='zh'?'说明':'Description'));
        assert.equal(footer.includes(language==='zh'?'引文':'Quote'),!!quote);
        assert.equal(footer.includes(language==='zh'?'状态':'Status'),!!status);
    }
}
handlers.options({options:{language:'zh',ui_text:dictionary}});
cell.mon={name:'<custom & name>',name_display:''};html=[];
events.cell_tooltip({cell:{x:0,y:0},pageX:0,pageY:0});
assert.ok(html.at(-1).startsWith('&lt;custom &amp; name&gt;'));
assert.equal(cell.mon.name,'<custom & name>');
console.log('Monster tooltips and description footers: Chinese, English, old-server and missing-name fallback passed.');
