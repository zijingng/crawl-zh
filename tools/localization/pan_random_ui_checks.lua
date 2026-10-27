local f=assert(io.open('dat/des/branches/pan.des'))
local source=f:read('*a');f:close()
local body=assert(source:match('function pan_declare_demonic_rune%(holy%)(.-)\nend'))
local declare=assert(load('return function(holy)'..body..'\nend'))()
local price=assert(source:match('epilogue {{\n(if not you.have_rune%("demonic"%) then.-)\n}}'))
local priced=assert(load(price))
function pan_random_checks(chinese)
  local messages={}
  crawl.mpr=function(text,channel)table.insert(messages,text)end
  declare(false)
  assert(#messages==2)
  assert(messages[1]==(chinese and '一位强大的魔窟领主居住于此。' or 'A powerful Pandemonium lord resides here.'))
  assert(messages[2]==(chinese and '其守护着恶魔佐特符文。' or 'It guards the demonic rune of Zot.'))
  messages={};declare(true)
  assert(#messages==2)
  assert(messages[2]==(chinese and '这里可以找到恶魔佐特符文。' or 'The demonic rune of Zot may be found here.'))
  you={have_rune=function()return false end}
  messages={};priced();assert(#messages==2)
  assert(messages[2]==(chinese and '<yellow>这里可以找到一枚恶魔佐特符文……只要付出代价。</yellow>'
      or '<yellow>A demonic rune of Zot can be found here... for a price.</yellow>'))
  you.have_rune=function()return true end
  messages={};priced();assert(#messages==0)
end
pan_random_checks(true)
