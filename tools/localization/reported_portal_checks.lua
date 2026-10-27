local function arrival(kind)
  local f=assert(io.open('dat/des/portals/'..kind..'.des'))
  local source=f:read('*a');f:close()
  local body=assert(source:match('function '..kind..'_milestone %((.-)\nend'))
  return assert(load('return function('..body..'\nend'))()
end
local wizlab,trove=arrival('wizlab'),arrival('trove')
function reported_portal_checks(chinese)
  local messages,notes,milestones={},{},{}
  crawl.mpr=function(s)table.insert(messages,s)end
  crawl.take_note=function(s)table.insert(notes,s)end
  crawl.mark_milestone=function(_,s)table.insert(milestones,s)end
  wizlab(nil,"Lehudib's Moon Base")
  trove(nil,"Rutra's Hidden Sanctum")
  wizlab(nil,'Unreviewed Moon')
  assert(messages[1]==(chinese and '欢迎来到雷胡迪布的月球基地！' or "Welcome to Lehudib's Moon Base!"))
  assert(messages[2]==(chinese and '你发现了鲁特拉的隐秘圣所！' or "You've discovered Rutra's Hidden Sanctum!"))
  assert(messages[3]=='Welcome to Unreviewed Moon!')
  assert(notes[1]=="Entered Lehudib's Moon Base")
  assert(milestones[2]=="entered Rutra's Hidden Sanctum.")
end
reported_portal_checks(true)
