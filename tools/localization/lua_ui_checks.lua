-- Run with the game's crawl display bindings in an isolated Lua state.
local function equal(actual, expected)
  assert(actual == expected, tostring(actual) .. " ~= " .. expected)
end

equal(crawl.ui_format("lua.ui", "No spell in slot %s!", "No spell in slot a!", "a"),
      "法术栏位a中没有法术！")
equal(crawl.ui_format("lua.ui", "No spell in slot %s!", "original", nil), "original")
equal(crawl.ui_format("lua.ui", "No spell in slot %s!", "original", "a", "extra"), "original")
equal(crawl.ui_format("lua.ui", "New upstream %s.", "original", "中文"), "original")
equal(crawl.ui_format("lua.ui", "No spell in slot %s!", "original", "100%s"),
      "法术栏位100%s中没有法术！")
assert(crawl.ui_name("display.spell", "Unknown upstream spell") == nil)

crawl_require = function () end
dofile("dat/dlua/util.lua")
dofile("dat/dlua/lm_pdesc.lua")
dofile("dat/dlua/lm_1way.lua")
dofile("dat/dlua/lm_trove.lua")
dofile("dat/dlua/lm_tmsg.lua")

local toll = {base_type="scroll", sub_type="acquirement", quantity=2}
local portal = setmetatable({toll={item=toll}, props={toll={item=toll}}}, TroveMarker)
equal(portal:item_name(), "2 scrolls of acquirement")
equal(portal:display_item_name(), "2×获取卷轴")
equal(portal:overview_note(), "交出2×获取卷轴")
local canonical = portal:item_name()
assert(portal:describe():find("2×获取卷轴", 1, true))
equal(portal:item_name(), canonical)
toll.sub_type = "upstream scroll"
assert(portal:display_item_name() == nil)
equal(portal:describe(), "The portal requires 2 scrolls of upstream scroll for entry.\n")
toll.base_type = "weapon"; toll.sub_type = "demon whip"; toll.quantity = 1; toll.plus1 = 3
equal(portal:display_item_name(), "+3 恶魔武器")

local timer = setmetatable({noisemaker="bell", verb="tolling",
  range_adjectives={{28,"very distant"},{21,"distant"},{14,"$F nearby"},{7,"$F very nearby"},{0,"$F"}}}, TimedMessaging)
timer.player_distance = function () return 21 end
equal(timer:display_noisemaker({}), "远处的钟")
timer.player_distance = function () return 0 end
equal(timer:display_noisemaker({}), "钟")
timer.noisemaker = "upstream noisemaker"
assert(timer:display_noisemaker({}) == nil)
equal(util.display_message("portcullis", "$F{The} has been lowered almost to the ground."),
      "闸门已降至接近地面。")
equal(util.display_message("upstream entity", "$F{The} has been lowered almost to the ground."),
      "the upstream entity has been lowered almost to the ground.")
equal(util.display_message("portcullis", "New upstream $F."), "New upstream a portcullis.")

-- Exercise real autofight call sites without moving or attacking.
local output
crawl.mpr = function (msg) output = msg end
chk_lua_option = {}
dofile("dat/clua/autofight.lua")
you = {caught=function () return nil end, confused=function () return false end,
       hp=function () return 1, 100 end, spell_table=function () return {} end}
assert(not autofight_check_preconditions(false))
equal(output, "你伤势过重，不能鲁莽地战斗！")
AUTOMAGIC_SPELL_SLOT = "a"
hit_magic()
equal(output, "法术栏位a中没有法术！")

chk_lua_save = {}
dofile("dat/clua/automagic.lua")
crawl.getch = function () return 97 end
crawl.clear_messages = function () end
crawl.setopt = function (option) equal(option, "automagic_enable = true") end
you.spell_table = function () return {a="Magic Dart"} end
AUTOMAGIC_ACTIVE = false
am_set_spell()
equal(output, "已启用自动施法，将使用栏位a中的法术（魔法飞弹）。")
equal(AUTOMAGIC_SPELL_SLOT, "a")
AUTOMAGIC_ACTIVE = true
you.spell_table = function () return {a="Unknown upstream spell"} end
am_set_spell()
equal(output, "Automagic will cast spell in slot a (Unknown upstream spell).")
you.spell_table = function () return {} end
am_set_spell()
equal(output, "自动施法将使用栏位a中的法术（当前没有法术）。")

dofile("dat/lua-wiz/damtally.lua")
you.turns = function () return 10 end
you.time = function () return 20 end
you.mp = function () return 5 end
damt.turns = 8; damt.time = 10; damt.mana = 8
damt.calc_total_damage = function () return 10 end
damt.calc_num_damaged = function () return 2 end
damt.show()
assert(output:find("回合：", 1, true))
assert(output:find("每只怪物平均伤害：", 1, true))
local format = crawl.ui_format
crawl.ui_format = function (context, source, original, ...)
  if source:find("dpm:", 1, true) then return original end
  return format(context, source, original, ...)
end
damt.show()
assert(output:find("Turns:", 1, true))
assert(not output:find("回合", 1, true))
crawl.ui_format = format
