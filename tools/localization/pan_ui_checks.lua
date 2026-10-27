-- Load the real map's warning function without generating or entering a level.
local f = assert(io.open('dat/des/branches/pan.des'))
local source = f:read('*a'); f:close()
local body = assert(source:match('function pan_lord_epilogue%(name%)(.-)\nend'))
local epilogue = assert(load('return function(name)' .. body .. '\nend'))()
function pan_display_checks(chinese)
  for _, pair in ipairs({{'Mnoleg','glowing'}, {'Lom Lobon','magical'},
                         {'Cerebov','fiery'}, {'Gloorx Vloq','dark'}}) do
    local name, rune = pair[1], pair[2]
    local messages, notes, milestones = {}, {}, {}
    crawl.mpr = function(text, channel) table.insert(messages, text) end
    crawl.take_note = function(text) table.insert(notes, text) end
    crawl.mark_milestone = function(kind, text) table.insert(milestones, {kind,text}) end
    epilogue(name)
    assert(#messages==3 and #notes==1 and #milestones==1)
    assert(notes[1]=='Entered the realm of ' .. name .. '.')
    assert(milestones[1][1]=='br.enter')
    assert(milestones[1][2]=='entered the realm of ' .. name .. '.')
    if chinese then
      assert(messages[2]=='强大的魔窟领主' .. crawl.ui_name('display.monster',name) .. '居住于此。')
      assert(messages[3]=='其守护着' .. crawl.ui_name('display.item',rune .. ' rune of Zot') .. '。')
      assert(not messages[1]:find('The ',1,true) and not messages[1]:find('You ',1,true))
    else
      assert(messages[2]=='The mighty Pandemonium lord ' .. name .. ' resides here.')
      assert(messages[3]=='They guard the ' .. rune .. ' rune of Zot.')
    end
  end
  assert(crawl.ui_format('lua.pan','The mighty Pandemonium lord %s resides here.',
         'The mighty Pandemonium lord Unknown resides here.', nil)
         == 'The mighty Pandemonium lord Unknown resides here.')
end
pan_display_checks(true)
