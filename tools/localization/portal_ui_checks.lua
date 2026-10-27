-- Run with the real display bindings and timer classes in an isolated state.
crawl_require = function () end
dofile('dat/dlua/util.lua')
dofile('dat/dlua/lm_tmsg.lua')
dofile('dat/dlua/lm_pdesc.lua')
dofile('dat/dlua/lm_1way.lua')
dofile('dat/dlua/lm_timed.lua')

local function equal(a, b) assert(a == b, tostring(a) .. ' ~= ' .. tostring(b)) end
local function translated(context, message)
  local shown = crawl.ui_name(context, message)
  assert(shown and shown ~= message, context .. ': ' .. message)
  return shown
end

-- Extract a literal timer table from the real map, respecting quoted braces.
-- This is test input preparation; production still uses the normal map loader.
local function table_at(source, start)
  local depth, quote, escape = 0, nil, false
  for i = start, #source do
    local c = source:sub(i, i)
    if quote then
      if escape then escape = false
      elseif c == '\\' then escape = true
      elseif c == quote then quote = nil end
    elseif c == '"' or c == "'" then quote = c
    elseif c == '{' then depth = depth + 1
    elseif c == '}' then
      depth = depth - 1
      if depth == 0 then return source:sub(start, i) end
    end
  end
  error('Unclosed timer table')
end

local output = {}
crawl.mpr = function (text, channel) table.insert(output, {text, channel}) end
you = {hear_pos=function () return true end, pos=function () return 0, 0 end,
       see_cell=function () return false end, taking_stairs=function () return false end}
local marker = {pos=function () return 10, 10 end}
for _, name in ipairs({'bailey', 'bazaar', 'desolation', 'gauntlet', 'icecave',
                      'necropolis', 'ossuary', 'sewer', 'volcano', 'wizlab'}) do
  local f = assert(io.open('dat/des/portals/' .. name .. '.des'))
  local source = f:read('*a'); f:close()
  local _, start = assert(source:find('timed_msg%s*{'))
  local literal = table_at(source, start)
  local value = assert(load('return ' .. literal))()
  equal(#value.initmsg, 2)
  local original = table.concat(value.initmsg, '\n')
  local timer = TimedMessaging:new(value)
  local entity = source:match("entity%s*=%s*'([^']+)'")
  timer:init({dur=10000, props={entity=entity}}, marker, true)
  for i, text in ipairs(timer.initmsg) do
    equal(output[#output-2+i][1], translated('lua.map', text))
  end
  -- Exercise all configured pace and distance branches in the actual method.
  for _, distance in ipairs({30, 25, 18, 10, 0}) do
    timer.player_distance = function () return distance end
    for _, range in ipairs(timer.ranges) do
      timer:say_message(marker, math.max(101, range[1] + 1))
      local shown = output[#output][1]
      assert(shown:find('你听到', 1, true), name .. ': ' .. shown)
      assert(not shown:find('You hear', 1, true))
    end
  end
  timer:say_message(marker, 50)
  equal(output[#output][1], util.display_message(timer.entity, timer.finalmsg))
  assert(output[#output][1] ~= timer.finalmsg)
  equal(table.concat(timer.initmsg, '\n'), original)
  -- Whole-message fallback for unreviewed templates and unknown entities.
  equal(util.display_message('unknown', 'New upstream message.'), 'New upstream message.')
  local dstart, dend = source:find('disappear%s*=%s*')
  if dstart then
    local tail = source:sub(dend+1)
    local expression = tail:match('^(.-),%s*\n')
    local disappear = assert(load('return ' .. expression))()
    equal(util.display_message(nil, disappear), translated('lua.map', disappear))
  end
end

equal(crawl.ui_translate('lua.ui', 'The walls and floor vibrate strangely for a moment.'),
      '墙壁和地面奇异地震动了片刻。')
-- Invoke the unseen timeout branch, without changing the map or timer state.
local timed = setmetatable({props={}}, TimedMarker)
timed.disappear = function () end
timed:timeout(marker, true)
equal(output[#output][1], '墙壁和地面奇异地震动了片刻。')
