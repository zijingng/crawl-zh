# 阶段 0：旧译文审查清单

基线：DCSS `0.34.1`，提交 `1eebc1a2892e1c89776a0d7a10691f8dac8d9796`。盘点日期：2026-10-20。

本清单完成资源定位、键级差异统计和代表性风险确认；正文语义逐条复核仍待进行。所有同键中文初始状态均为“待语义复核”，不能因文件已存在或能成功编译就标为已审。

## 1. 旧资源来源与使用方式

- 当前仓库中文 TextDB：`source/dat/descript/zh/` 与 `source/dat/database/zh/`，共 31 个 TXT。它们可能混有新版更新和旧内容，不能笼统认定全部来自某个旧版本。
- 用户历史导出：外层 `paratranz/2022_07_29_17_00_18_e12bad/utf8/dat/`，共 386 个 CSV。仅作旧原文、旧译文和术语记忆的候选来源；CSV 行号 ID 不是 TextDB 的永久键。
- `localization/glossary.csv`：已交付的 917 条术语表，保留历史/待定位备注；“术语校对”不等同相应说明正文已复核。
- 原始 `paratranz/terms-808.csv`：外层输入备份，不覆盖。历史 ZIP 和 MSYS2 安装程序不是翻译运行时数据。

## 2. 逐文件审查队列

以下路径省略 `crawl-ref/source/dat/`。英文/中文键数按同名文件统计，英文缺译候选数 = 英文键集合减中文键集合；中文独有数 = 中文键集合减英文键集合。

注意：条目可能移入同一逻辑数据库的其他文件；“中文独有”仅是定位候选，必须核查数据库级别后才能停用。相同键也可能已改效果、数值或脚本，不得直接计算为审核覆盖率。

| 对应英文文件 | 中文文件 | 英文键 | 中文键 | 同文件同键 | 缺译候选 | 中文独有 | Lua块 | 优先级 | 状态 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| `descript/features.txt` | 有 | 290 | 242 | 242 | 48 | 0 | 1 | P1 | 待语义/结构复核 |
| `descript/items.txt` | 有 | 307 | 269 | 267 | 40 | 2 | 5 | P0 | 待语义/结构复核 |
| `descript/unident.txt` | 有 | 7 | 7 | 7 | 0 | 0 | 0 | P1 | 待语义/结构复核 |
| `descript/unrand.txt` | 有 | 140 | 133 | 133 | 7 | 0 | 2 | P1 | 待语义/结构复核 |
| `descript/monsters.txt` | 有 | 724 | 640 | 636 | 88 | 4 | 4 | P1 | 待语义/结构复核 |
| `descript/spells.txt` | 有 | 402 | 312 | 299 | 103 | 13 | 0 | P0 | 待语义/结构复核 |
| `descript/gods.txt` | 有 | 82 | 82 | 82 | 0 | 0 | 0 | P0 | 待语义/结构复核 |
| `descript/branches.txt` | 有 | 36 | 34 | 34 | 2 | 0 | 1 | P1 | 待语义/结构复核 |
| `descript/skills.txt` | 有 | 29 | 28 | 28 | 1 | 0 | 2 | P0 | 待语义/结构复核 |
| `descript/ability.txt` | 有 | 154 | 131 | 118 | 36 | 13 | 2 | P0 | 待语义/结构复核 |
| `descript/cards.txt` | 有 | 24 | 24 | 24 | 0 | 0 | 0 | P1 | 待语义/结构复核 |
| `descript/commands.txt` | 有 | 95 | 93 | 92 | 3 | 1 | 0 | P1 | 待语义/结构复核 |
| `descript/clouds.txt` | 有 | 39 | 34 | 33 | 6 | 1 | 0 | P1 | 待语义/结构复核 |
| `descript/status.txt` | 有 | 237 | 152 | 152 | 85 | 0 | 5 | P0 | 待语义/结构复核 |
| `descript/monstatus.txt` | 无 | 139 | 0 | 0 | 139 | 0 | 0 | P0 | 待补译 |
| `descript/mutations.txt` | 有 | 251 | 168 | 161 | 90 | 7 | 0 | P0 | 待语义/结构复核 |
| `descript/passives.txt` | 有 | 20 | 20 | 20 | 0 | 0 | 0 | P0 | 待语义/结构复核 |
| `descript/species.txt` | 有 | 27 | 25 | 23 | 4 | 2 | 0 | P0 | 待语义/结构复核 |
| `descript/backgrounds.txt` | 有 | 26 | 25 | 25 | 1 | 0 | 0 | P0 | 待语义/结构复核 |
| `database/randname.txt` | 无 | 33 | 0 | 0 | 33 | 0 | 0 | P1 | 待补译 |
| `database/rand_wpn.txt` | 无 | 45 | 0 | 0 | 45 | 0 | 0 | P1 | 待补译 |
| `database/rand_arm.txt` | 无 | 19 | 0 | 0 | 19 | 0 | 0 | P1 | 待补译 |
| `database/rand_all.txt` | 无 | 18 | 0 | 0 | 18 | 0 | 0 | P1 | 待补译 |
| `database/randbook.txt` | 无 | 72 | 0 | 0 | 72 | 0 | 0 | P1 | 待补译 |
| `database/monspeak.txt` | 无 | 731 | 0 | 0 | 731 | 0 | 0 | P1 | 待补译 |
| `database/monspell.txt` | 有 | 262 | 212 | 195 | 67 | 17 | 0 | P1 | 待语义/结构复核 |
| `database/monflee.txt` | 有 | 1 | 1 | 1 | 0 | 0 | 0 | P1 | 待语义/结构复核 |
| `database/wpnnoise.txt` | 有 | 65 | 49 | 49 | 16 | 0 | 3 | P1 | 待语义/结构复核 |
| `database/insult.txt` | 无 | 33 | 0 | 0 | 33 | 0 | 0 | P1 | 待补译 |
| `database/godspeak.txt` | 有 | 193 | 114 | 112 | 81 | 2 | 1 | P1 | 待语义/结构复核 |
| `database/monname.txt` | 无 | 52 | 0 | 0 | 52 | 0 | 0 | P1 | 待补译 |
| `database/colourname.txt` | 无 | 9 | 0 | 0 | 9 | 0 | 0 | P1 | 待补译 |
| `database/graffiti.txt` | 无 | 58 | 0 | 0 | 58 | 0 | 0 | P1 | 待补译 |
| `database/miscast.txt` | 无 | 33 | 0 | 0 | 33 | 0 | 0 | P1 | 待补译 |
| `database/shout.txt` | 有 | 93 | 81 | 81 | 12 | 0 | 0 | P1 | 待语义/结构复核 |
| `database/miscname.txt` | 有 | 10 | 9 | 8 | 2 | 1 | 1 | P1 | 待语义/结构复核 |
| `database/godname.txt` | 有 | 23 | 23 | 23 | 0 | 0 | 0 | P1 | 待语义/结构复核 |
| `database/montitle.txt` | 有 | 86 | 81 | 81 | 5 | 0 | 0 | P1 | 待语义/结构复核 |
| `database/decorlines.txt` | 无 | 132 | 0 | 0 | 132 | 0 | 0 | P1 | 待补译 |
| `database/gizmo.txt` | 无 | 3 | 0 | 0 | 3 | 0 | 0 | P1 | 待补译 |
| `descript/quotes.txt` | 有 | 465 | 2 | 2 | 463 | 0 | 0 | P1 | 待语义/结构复核 |
| `database/help.txt` | 有 | 13 | 13 | 13 | 0 | 0 | 1 | P1 | 待语义/结构复核 |
| `database/FAQ.txt` | 有 | 38 | 38 | 38 | 0 | 0 | 0 | P1 | 待语义/结构复核 |
| `descript/hints.txt` | 有 | 116 | 22 | 21 | 95 | 1 | 6 | P0 | 待语义/结构复核 |
| `descript/tutorial.txt` | 有 | 88 | 87 | 87 | 1 | 0 | 0 | P0 | 待语义/结构复核 |
| `descript/egos.txt` | 无 | 80 | 0 | 0 | 80 | 0 | 0 | P0 | 待补译 |

### 每个文件的差异样例

完整差异保存在 JSON 快照；这里每类最多列五个，便于先定位，不代表全部任务。

- **`descript/features.txt`**：缺译候选 `a bloodied mop and bucket`、`a broken floral vase`、`a cache of baked goods`、`a cache of fruit`、`a cache of meat`；中文独有 无。
- **`descript/items.txt`**：缺译候选 `amulet of chemistry`、`amulet of dissipation`、`amulet of wildshape`、`baked corpse`、`book of contraptions`；中文独有 `ring of fire`、`ring of ice`。
- **`descript/unident.txt`**：缺译候选 无；中文独有 无。
- **`descript/unrand.txt`**：缺译候选 `charlatan's orb`、`crown of vainglory`、`fungal fisticloak`、`gloves of the gadgeteer`、`justicar's regalia`；中文独有 无。
- **`descript/monsters.txt`**：缺译候选 `alderking`、`armataur`、`bes kemwar`、`blorkula the orcula`、`bomblet`；中文独有 `blork the orc`、`petrified plant`、`sphinx`、`spriggan hatter`。
- **`descript/spells.txt`**：缺译候选 `acid ball spell`、`alistair's walking alembic spell`、`awaken armour spell`、`awaken flesh spell`、`beckoning gale spell`；中文独有 `acid splash spell`、`animate armour spell`、`aura of brilliance spell`、`black mark spell`、`chilling breath spell`。
- **`descript/gods.txt`**：缺译候选 无；中文独有 无。
- **`descript/branches.txt`**：缺译候选 `crucible`、`necropolis`；中文独有 无。
- **`descript/skills.txt`**：缺译候选 `forgecraft`；中文独有 无。
- **`descript/ability.txt`**：缺译候选 `aphotic marionette ability`、`bat swarm ability`、`bestial takedown ability`、`blood for blood ability`、`brand self ability`；中文独有 `breathe acid ability`、`breathe dispelling energy ability`、`breathe frost ability`、`breathe lightning ability`、`breathe noxious fumes ability`。
- **`descript/cards.txt`**：缺译候选 无；中文独有 无。
- **`descript/commands.txt`**：缺译候选 `cmd_character_dump`、`cmd_explore_no_rest`、`cmd_toggle_keyboard verbose`；中文独有 `cmd_show_keyboard verbose`。
- **`descript/clouds.txt`**：缺译候选 `alcoholic mist cloud`、`bats cloud`、`faint pestilence cloud`、`magnetised fragments cloud`、`rust cloud`；中文独有 `degeneration cloud`。
- **`descript/status.txt`**：缺译候选 `!tele! status`、`-bloodrite status`、`-breath status`、`-gavotte status`、`-gclaw status`；中文独有 无。
- **`descript/monstatus.txt`**：缺译候选 `ablaze with memories monstatus`、`about to teleport monstatus`、`afflicted by rimeblight monstatus`、`ally target monstatus`、`anguished monstatus`；中文独有 无。
- **`descript/mutations.txt`**：缺译候选 `accursed mutation`、`airstrike attack mutation`、`armoured shell mutation`、`artefact enchanting mutation`、`blade parry mutation`；中文独有 `deterioration mutation`、`inability to drink after injury mutation`、`inability to read after injury mutation`、`petrification resistance mutation`、`strong legs mutation`。
- **`descript/passives.txt`**：缺译候选 无；中文独有 无。
- **`descript/species.txt`**：缺译候选 `coglin`、`mountain dwarf`、`poltergeist`、`revenant`；中文独有 `hill orc`、`vampire`。
- **`descript/backgrounds.txt`**：缺译候选 `forgewright`；中文独有 无。
- **`database/randname.txt`**：缺译候选 `_amulet material_`、`_ashenzari_adjective_`、`_ashenzari_bound_adjective_`、`_ashenzari_celestial_adjective_`、`_ashenzari_epithet_`；中文独有 无。
- **`database/rand_wpn.txt`**：缺译候选 `_amphibian_name_`、`_anger_name_`、`_anger_possessive_`、`_anger_quality_`、`_arachnid_name_`；中文独有 无。
- **`database/rand_arm.txt`**：缺译候选 `_armour_animal_`、`_armour_name_shielding_`、`_armour_property_name_`、`_card_`、`_celestial_bodies_`；中文独有 无。
- **`database/rand_all.txt`**：缺译候选 `_adjective_`、`_destiny_name_`、`_fruit_name_`、`_jewellery_animal_`、`_jewellery_special_name_`；中文独有 无。
- **`database/randbook.txt`**：缺译候选 `_beginners_`、`_book_adjective_`、`_book_magic_`、`_book_noun_on_`、`_book_plant_animal_`；中文独有 无。
- **`database/monspeak.txt`**：缺译候选 `'&'`、`'1'`、`'2'`、`'3'`、`'4'`；中文独有 无。
- **`database/monspell.txt`**：缺译候选 `_unseen_breath_cast_`、`_unseen_spit_cast_`、`acid ball nascent plasmodium cast`、`airstrike blizzard demon cast`、`antimagic gaze norris cast`；中文独有 `animate dead cast gestures`、`animate dead cast real`、`battlecry seraph cast`、`blink other statue cast`、`call lost soul cast`。
- **`database/monflee.txt`**：缺译候选 无；中文独有 无。
- **`database/wpnnoise.txt`**：缺译候选 `eel hand actions`、`eel hand solo actions`、`fungus thoughts`、`zonguldrok farewell`、`zonguldrok greeting`；中文独有 无。
- **`database/insult.txt`**：缺译候选 `body_or_spiritual_part`、`demon_taunt`、`demon_taunt_special`、`feast_or_devour`、`generic_insult`；中文独有 无。
- **`database/godspeak.txt`**：缺译候选 `_xom_number_`、`acrobatic_move_name`、`beogh apostle challenge`、`dance_name`、`makhleb executioner chatter`；中文独有 `xom draining or torment`、`xom scenery`。
- **`database/monname.txt`**：缺译候选 `_arabic_female_name_`、`_arabic_male_name_`、`_aztec_female_name_`、`_aztec_male_name_`、`_beogh_name_`；中文独有 无。
- **`database/colourname.txt`**：缺译候选 `any_colour`、`any_colour_pattern`、`any_glowing_colour`、`any_glowing_colour_pattern`、`any_pattern`；中文独有 无。
- **`database/graffiti.txt`**：缺译候选 `_graffiti_advertisement_`、`_graffiti_author_any_`、`_graffiti_author_any_changeable_`、`_graffiti_author_any_changeable_non-randgen_`、`_graffiti_author_any_non-orc_`；中文独有 无。
- **`database/miscast.txt`**：缺译候选 `air miscast monster`、`air miscast player`、`air miscast unseen`、`alchemy miscast monster`、`alchemy miscast player`；中文独有 无。
- **`database/shout.txt`**：缺译候选 `__caw seen`、`__caw unseen`、`__laugh seen`、`__laugh unseen`、`_riddle_adj_`；中文独有 无。
- **`database/miscname.txt`**：缺译候选 `harlequin_trap_lines`、`summon_horrible_things`；中文独有 `sht_int_loss`。
- **`database/godname.txt`**：缺译候选 无；中文独有 无。
- **`database/montitle.txt`**：缺译候选 `cassandra title`、`nobody title`、`sprozz title`、`wiglaf title`、`xak'krixis title`；中文独有 无。
- **`database/decorlines.txt`**：缺译候选 `_baked_good_and_reaction_`、`_fruit_and_reaction_`、`_meat_and_reaction_`、`amphisbaena baked goods cache`、`amphisbaena fruit cache`；中文独有 无。
- **`database/gizmo.txt`**：缺译候选 `gizmo_adjective`、`gizmo_modifier`、`gizmo_noun`；中文独有 无。
- **`descript/quotes.txt`**：缺译候选 `__c_suffix`、`__cap-d_suffix`、`__cap-k_suffix`、`__cap-s_suffix`、`__cap-t_suffix`；中文独有 无。
- **`database/help.txt`**：缺译候选 无；中文独有 无。
- **`database/FAQ.txt`**：缺译候选 无；中文独有 无。
- **`descript/hints.txt`**：缺译候选 `death random 4`、`hint_abyss`、`hint_advise_use_wand`、`hint_auto_exclusion`、`hint_auto_explore`；中文独有 `dissection reminder`。
- **`descript/tutorial.txt`**：缺译候选 `tutorial4 spellquivering`；中文独有 无。
- **`descript/egos.txt`**：缺译候选 `acid (acid) weapon ego`、`air (air) armour ego`、`antimagic (antimagic) weapon ego`、`archery (archery) armour ego`、`atropa-tipped (atropa) missile ego`；中文独有 无。

## 3. 已确认需要处理的代表性问题

以下是盘点中核对过的真实例子；不是全文件语义审查结论。

| 位置 | 已确认事实 | 后续任务 | 优先级 |
| --- | --- | --- | --- |
| `descript/zh/skills.txt` 的 Fighting/Evocations/Shapeshifting | 仍用“格斗/激活/变身”等旧约定，与当前术语表“战斗/唤发/变形术”不同；Invocations 段已含 Djinni 与 MP 规则，不能一概判旧 | 保留机制正确部分，逐条统一术语；不机械重译整文件 | P0 |
| `descript/zh/hints.txt` 的献祭尸体提示 | 存在“如果你不想吃掉尸体…可以献祭”的旧流程 | 对照当前英文与 hints.cc，核查是否为历史遗留入口；不得把它当现行教程指引 | P0 |
| `descript/zh/items.txt` 的 `you.race() == "Ogre"` | 中文资源含旧玩家种族判断 | 与对应英文 Lua 分支逐项比较，确认是否需改 Oni/Armataur 等；不是简单全局把 Ogre 换成 Oni | P0 |
| `descript/zh/status.txt` 的 Vampire 判断 | 当前仍有旧吸血鬼相关描述/逻辑，种族定义已 deprecated | 判定历史兼容、怪物语境与仍可产生的玩家状态，先定位再停用 | P0 |
| 历史 CSV 的 App status | 旧导出把 `you.race() == "Octopode"` 写成 `you.race() == "章鱼"` | 禁止原样导入该 Lua；恢复程序判断常量，中文仅处理返回文本 | P0 |
| 历史状态 Hevn/-Wiz/Lash | 旧表含“宇/巫师/鞭打”等缺少真实语境的译法 | 采用术语校对中的天界风暴、施法控制削弱、腾蛇鞭，并按当前真实英文键映射 | P0 |
| 历史术语 Clouds/Hop | 导出中的真实灯号含前置减号，术语表旧键曾漏写 | 映射到真实 -Clouds/-Hop，保留旧键追溯；不翻译程序键 | P0 |
| 当前 `status.txt` 的 Alive 与历史导出 | 同名英文来源正文不同 | 必须以当前正文重新翻译，验证来源变化会使旧译文失效 | P0 |

### 按主题复核的重点

- [ ] 种族/背景：只对当前可选项作现行说明；deprecated、颜色派生及怪物名分别处理。
- [ ] 技能：核对远程武器合并、Alchemy、Forgecraft、Shapeshifting；避免把兼容槽当可训练技能。
- [ ] 神祇：逐个核查 0.34.1 的神赐、重做与虔诚规则，不能由旧称号推断当前机制。
- [ ] 物品/神器：核对当前键、移除/改名、效果和条件；护身符与变形护符不能混用。
- [ ] 法术：学派、等级、对象、伤害、抗性、范围、持续时间和施法失误逐项核对。
- [ ] 状态/变异：区别过程与完成态、冷却与正面效果、抵抗与免疫、伤害类型与状态。
- [ ] 教程/帮助：饥饿、食物、尸体献祭、旧按键和旧种族条件优先清理，核查真正调用点。
- [ ] 怪物对白/神语：保留随机权重 `w:`、占位符、引用、分隔和发声通道，语气不能改变对象或可见信息。
- [ ] 名称片段：随机名字不等于完整中文句子；先验证组合规则、显示宽度与保存/日志影响。

## 4. 每条译文的复核记录

建议按以下顺序推进，每个文件先做结构检查，再按键逐条复核：

1. 定位当前英文键及正文；对已移动的条目找到其所属逻辑数据库，记录英文来源哈希。
2. 旧英文有快照时比较正文；没有可比旧英文时标记“基线未知”，不凭同键宣称未变。
3. 校验 Lua 判断、引用与程序标记；原始键和内部名称不得汉化。
4. 对照当前机制复核语义、数值、条件、否定、例外、目标和信息可见性。
5. 根据当前术语表统一表达；不把术语候选或历史名称自动升格为现行正文。
6. 在对应界面检查字形、换行、截断、快捷键和中文缺译回退英文。
7. 完成后记录审核状态、对应哈希及必要的场景验证，再允许进入正式生成产物。

建议状态：待定位、待补译、待结构修复、待语义复核、待游戏验证、已复核、停用候选。停用候选必须经定位确认，不按“中文独有”自动删除。

## 5. 历史 Paratranz 导出目录

这些文件在当前仓库之外。导出无表头，字段为旧 ID、英文、中文，文本中的 `\n` 是导出转义。行数及中文非空数只表示数据存在，不表示准确度或可导入性。

| 外层导出路径 | 行数 | 中文非空行 | 使用状态 |
| --- | ---: | ---: | --- |
| `ability.cc.csv` | 366 | 347 | 候选记忆；待结构与版本复核 |
| `abyss.cc.csv` | 51 | 26 | 候选记忆；待结构与版本复核 |
| `abyss.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `acquire.cc.csv` | 32 | 14 | 候选记忆；待结构与版本复核 |
| `actor.cc.csv` | 60 | 48 | 候选记忆；待结构与版本复核 |
| `actor.h.csv` | 7 | 1 | 候选记忆；待结构与版本复核 |
| `adjust.cc.csv` | 19 | 13 | 候选记忆；待结构与版本复核 |
| `AppHdr.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `areas.cc.csv` | 10 | 10 | 候选记忆；待结构与版本复核 |
| `arena.cc.csv` | 112 | 38 | 候选记忆；待结构与版本复核 |
| `art-data.txt.csv` | 115 | 113 | 候选记忆；待结构与版本复核 |
| `art-func.h.csv` | 133 | 52 | 候选记忆；待结构与版本复核 |
| `artefact.cc.csv` | 104 | 66 | 候选记忆；待结构与版本复核 |
| `attack.cc.csv` | 85 | 51 | 候选记忆；待结构与版本复核 |
| `attitude-change.cc.csv` | 27 | 6 | 候选记忆；待结构与版本复核 |
| `beam.cc.csv` | 382 | 196 | 候选记忆；待结构与版本复核 |
| `beam.h.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `behold.cc.csv` | 17 | 17 | 候选记忆；待结构与版本复核 |
| `bitary.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `bloodspatter.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `branch-data.h.csv` | 135 | 115 | 候选记忆；待结构与版本复核 |
| `branch.cc.csv` | 9 | 6 | 候选记忆；待结构与版本复核 |
| `butcher.cc.csv` | 16 | 13 | 候选记忆；待结构与版本复核 |
| `chardump.cc.csv` | 382 | 5 | 候选记忆；待结构与版本复核 |
| `chardump.h.csv` | 1 | 1 | 候选记忆；待结构与版本复核 |
| `cio.cc.csv` | 28 | 0 | 候选记忆；待结构与版本复核 |
| `cio.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `cloud.cc.csv` | 82 | 70 | 候选记忆；待结构与版本复核 |
| `clua.cc.csv` | 87 | 0 | 候选记忆；待结构与版本复核 |
| `clua.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `cluautil.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `cluautil.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `colour.cc.csv` | 100 | 0 | 候选记忆；待结构与版本复核 |
| `command.cc.csv` | 433 | 283 | 候选记忆；待结构与版本复核 |
| `command.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `coord.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `coordit.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `crash.cc.csv` | 37 | 0 | 候选记忆；待结构与版本复核 |
| `ctest.cc.csv` | 36 | 0 | 候选记忆；待结构与版本复核 |
| `dactions.cc.csv` | 42 | 0 | 候选记忆；待结构与版本复核 |
| `dat/database/FAQ.txt.csv` | 40 | 40 | 候选记忆；待结构与版本复核 |
| `dat/database/godname.txt.csv` | 22 | 22 | 候选记忆；待结构与版本复核 |
| `dat/database/godspeak.txt.csv` | 95 | 82 | 候选记忆；待结构与版本复核 |
| `dat/database/help.txt.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `dat/database/insult.txt.csv` | 37 | 23 | 候选记忆；待结构与版本复核 |
| `dat/database/miscname.txt.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `dat/database/monflee.txt.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `dat/database/monname.txt.csv` | 24 | 0 | 候选记忆；待结构与版本复核 |
| `dat/database/monspeak.txt.csv` | 566 | 4 | 候选记忆；待结构与版本复核 |
| `dat/database/monspell.txt.csv` | 175 | 7 | 候选记忆；待结构与版本复核 |
| `dat/database/montitle.txt.csv` | 72 | 15 | 候选记忆；待结构与版本复核 |
| `dat/database/rand_all.txt.csv` | 14 | 14 | 候选记忆；待结构与版本复核 |
| `dat/database/rand_arm.txt.csv` | 20 | 3 | 候选记忆；待结构与版本复核 |
| `dat/database/rand_wpn.txt.csv` | 39 | 2 | 候选记忆；待结构与版本复核 |
| `dat/database/randbook.txt.csv` | 68 | 7 | 候选记忆；待结构与版本复核 |
| `dat/database/randname.txt.csv` | 26 | 9 | 候选记忆；待结构与版本复核 |
| `dat/database/shout.txt.csv` | 76 | 75 | 候选记忆；待结构与版本复核 |
| `dat/database/wpnnoise.txt.csv` | 47 | 1 | 候选记忆；待结构与版本复核 |
| `dat/descript/ability.txt.csv` | 143 | 142 | 候选记忆；待结构与版本复核 |
| `dat/descript/backgrounds.txt.csv` | 24 | 24 | 候选记忆；待结构与版本复核 |
| `dat/descript/branches.txt.csv` | 33 | 33 | 候选记忆；待结构与版本复核 |
| `dat/descript/cards.txt.csv` | 26 | 26 | 候选记忆；待结构与版本复核 |
| `dat/descript/clouds.txt.csv` | 29 | 29 | 候选记忆；待结构与版本复核 |
| `dat/descript/commands.txt.csv` | 90 | 90 | 候选记忆；待结构与版本复核 |
| `dat/descript/features.txt.csv` | 205 | 205 | 候选记忆；待结构与版本复核 |
| `dat/descript/gods.txt.csv` | 80 | 76 | 候选记忆；待结构与版本复核 |
| `dat/descript/hints.txt.csv` | 25 | 25 | 候选记忆；待结构与版本复核 |
| `dat/descript/items.txt.csv` | 243 | 243 | 候选记忆；待结构与版本复核 |
| `dat/descript/monsters.txt.csv` | 591 | 590 | 候选记忆；待结构与版本复核 |
| `dat/descript/quotes.txt.csv` | 391 | 76 | 候选记忆；待结构与版本复核 |
| `dat/descript/skills.txt.csv` | 31 | 31 | 候选记忆；待结构与版本复核 |
| `dat/descript/species.txt.csv` | 27 | 27 | 候选记忆；待结构与版本复核 |
| `dat/descript/spells.txt.csv` | 291 | 291 | 候选记忆；待结构与版本复核 |
| `dat/descript/status.txt.csv` | 135 | 135 | 候选记忆；待结构与版本复核 |
| `dat/descript/tutorial.txt.csv` | 92 | 92 | 候选记忆；待结构与版本复核 |
| `dat/descript/unident.txt.csv` | 8 | 8 | 候选记忆；待结构与版本复核 |
| `dat/descript/unrand.txt.csv` | 115 | 115 | 候选记忆；待结构与版本复核 |
| `database.cc.csv` | 119 | 0 | 候选记忆；待结构与版本复核 |
| `database.h.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `dbg-asrt.cc.csv` | 175 | 0 | 候选记忆；待结构与版本复核 |
| `dbg-maps.cc.csv` | 67 | 0 | 候选记忆；待结构与版本复核 |
| `dbg-objstat.cc.csv` | 324 | 0 | 候选记忆；待结构与版本复核 |
| `dbg-scan.cc.csv` | 86 | 0 | 候选记忆；待结构与版本复核 |
| `dbg-util.cc.csv` | 122 | 0 | 候选记忆；待结构与版本复核 |
| `debug.h.csv` | 9 | 0 | 候选记忆；待结构与版本复核 |
| `decks.cc.csv` | 163 | 87 | 候选记忆；待结构与版本复核 |
| `defines.h.csv` | 9 | 0 | 候选记忆；待结构与版本复核 |
| `delay.cc.csv` | 131 | 80 | 候选记忆；待结构与版本复核 |
| `delay.h.csv` | 28 | 17 | 候选记忆；待结构与版本复核 |
| `describe-god.cc.csv` | 460 | 189 | 候选记忆；待结构与版本复核 |
| `describe-spells.cc.csv` | 68 | 3 | 候选记忆；待结构与版本复核 |
| `describe.cc.csv` | 1129 | 640 | 候选记忆；待结构与版本复核 |
| `describe.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `dgl-message.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `dgn-delve.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `dgn-irregular-box.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `dgn-layouts.cc.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `dgn-overview.cc.csv` | 111 | 80 | 候选记忆；待结构与版本复核 |
| `dgn-shoals.cc.csv` | 10 | 1 | 候选记忆；待结构与版本复核 |
| `directn.cc.csv` | 302 | 197 | 候选记忆；待结构与版本复核 |
| `directn.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `dlua.cc.csv` | 23 | 0 | 候选记忆；待结构与版本复核 |
| `dlua.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `domino.cc.csv` | 8 | 0 | 候选记忆；待结构与版本复核 |
| `dungeon.cc.csv` | 233 | 0 | 候选记忆；待结构与版本复核 |
| `dungeon.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `duration-data.h.csv` | 698 | 280 | 候选记忆；待结构与版本复核 |
| `end.cc.csv` | 52 | 23 | 候选记忆；待结构与版本复核 |
| `end.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `english.cc.csv` | 194 | 0 | 候选记忆；待结构与版本复核 |
| `english.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `errors.cc.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `evoke.cc.csv` | 74 | 51 | 候选记忆；待结构与版本复核 |
| `exclude.cc.csv` | 28 | 3 | 候选记忆；待结构与版本复核 |
| `exclude.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `externs.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `fearmonger.cc.csv` | 4 | 4 | 候选记忆；待结构与版本复核 |
| `feature-data.h.csv` | 332 | 146 | 候选记忆；待结构与版本复核 |
| `feature.cc.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `feature.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `fight.cc.csv` | 36 | 5 | 候选记忆；待结构与版本复核 |
| `fight.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `files.cc.csv` | 293 | 8 | 候选记忆；待结构与版本复核 |
| `files.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `fineff.cc.csv` | 31 | 10 | 候选记忆；待结构与版本复核 |
| `fixedvector.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `fontwrapper-ft.cc.csv` | 11 | 0 | 候选记忆；待结构与版本复核 |
| `food.cc.csv` | 60 | 55 | 候选记忆；待结构与版本复核 |
| `form-data.h.csv` | 191 | 4 | 候选记忆；待结构与版本复核 |
| `format.cc.csv` | 26 | 0 | 候选记忆；待结构与版本复核 |
| `fprop.cc.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `game-options.cc.csv` | 41 | 0 | 候选记忆；待结构与版本复核 |
| `game-options.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `ghost.cc.csv` | 3 | 0 | 候选记忆；待结构与版本复核 |
| `glwrapper-ogl.cc.csv` | 69 | 0 | 候选记忆；待结构与版本复核 |
| `god-abil.cc.csv` | 548 | 244 | 候选记忆；待结构与版本复核 |
| `god-abil.h.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `god-blessing.cc.csv` | 50 | 12 | 候选记忆；待结构与版本复核 |
| `god-companions.cc.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `god-conduct.cc.csv` | 156 | 0 | 候选记忆；待结构与版本复核 |
| `god-menu.cc.csv` | 1 | 1 | 候选记忆；待结构与版本复核 |
| `god-passive.cc.csv` | 187 | 11 | 候选记忆；待结构与版本复核 |
| `god-prayer.cc.csv` | 25 | 0 | 候选记忆；待结构与版本复核 |
| `god-wrath.cc.csv` | 113 | 31 | 候选记忆；待结构与版本复核 |
| `hints.cc.csv` | 1226 | 18 | 候选记忆；待结构与版本复核 |
| `hints.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `hiscores.cc.csv` | 665 | 16 | 候选记忆；待结构与版本复核 |
| `hiscores.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `initfile.cc.csv` | 1060 | 0 | 候选记忆；待结构与版本复核 |
| `initfile.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `invent.cc.csv` | 144 | 56 | 候选记忆；待结构与版本复核 |
| `item-name.cc.csv` | 939 | 790 | 候选记忆；待结构与版本复核 |
| `item-prop.cc.csv` | 138 | 94 | 候选记忆；待结构与版本复核 |
| `item-use.cc.csv` | 235 | 152 | 候选记忆；待结构与版本复核 |
| `items.cc.csv` | 147 | 99 | 候选记忆；待结构与版本复核 |
| `job-data.h.csv` | 114 | 48 | 候选记忆；待结构与版本复核 |
| `jobs.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `json-wrapper.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `json.cc.csv` | 43 | 0 | 候选记忆；待结构与版本复核 |
| `kills.cc.csv` | 87 | 26 | 候选记忆；待结构与版本复核 |
| `l-colour.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `l-crawl.cc.csv` | 133 | 0 | 候选记忆；待结构与版本复核 |
| `l-debug.cc.csv` | 57 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgn.cc.csv` | 192 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgnbld.cc.csv` | 88 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgnevt.cc.csv` | 28 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgngrd.cc.csv` | 12 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgnit.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgnlvl.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgnmon.cc.csv` | 24 | 0 | 候选记忆；待结构与版本复核 |
| `l-dgntil.cc.csv` | 20 | 0 | 候选记忆；待结构与版本复核 |
| `l-feat.cc.csv` | 23 | 0 | 候选记忆；待结构与版本复核 |
| `l-file.cc.csv` | 27 | 0 | 候选记忆；待结构与版本复核 |
| `l-food.cc.csv` | 12 | 0 | 候选记忆；待结构与版本复核 |
| `l-global.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `l-item.cc.csv` | 89 | 0 | 候选记忆；待结构与版本复核 |
| `l-los.cc.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `l-mapgrd.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `l-mapmrk.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `l-moninf.cc.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `l-mons.cc.csv` | 72 | 0 | 候选记忆；待结构与版本复核 |
| `l-option.cc.csv` | 17 | 0 | 候选记忆；待结构与版本复核 |
| `l-spells.cc.csv` | 21 | 0 | 候选记忆；待结构与版本复核 |
| `l-subvault.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `l-travel.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `l-view.cc.csv` | 10 | 0 | 候选记忆；待结构与版本复核 |
| `l-wiz.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `l-you.cc.csv` | 177 | 0 | 候选记忆；待结构与版本复核 |
| `lang-fake.cc.csv` | 481 | 0 | 候选记忆；待结构与版本复核 |
| `libunix.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `libutil.cc.csv` | 62 | 0 | 候选记忆；待结构与版本复核 |
| `libw32c.cc.csv` | 9 | 0 | 候选记忆；待结构与版本复核 |
| `loading-screen.cc.csv` | 1 | 1 | 候选记忆；待结构与版本复核 |
| `lookup-help.cc.csv` | 93 | 1 | 候选记忆；待结构与版本复核 |
| `los.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `losglobal.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `luaterp.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `macro.cc.csv` | 103 | 1 | 候选记忆；待结构与版本复核 |
| `main.cc.csv` | 254 | 182 | 候选记忆；待结构与版本复核 |
| `makeitem.cc.csv` | 10 | 2 | 候选记忆；待结构与版本复核 |
| `map-knowledge.cc.csv` | 2 | 1 | 候选记忆；待结构与版本复核 |
| `mapdef.cc.csv` | 655 | 0 | 候选记忆；待结构与版本复核 |
| `mapdef.h.csv` | 12 | 0 | 候选记忆；待结构与版本复核 |
| `mapmark.cc.csv` | 71 | 0 | 候选记忆；待结构与版本复核 |
| `mapmark.h.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `maps.cc.csv` | 93 | 0 | 候选记忆；待结构与版本复核 |
| `melee-attack.cc.csv` | 277 | 238 | 候选记忆；待结构与版本复核 |
| `menu.cc.csv` | 93 | 1 | 候选记忆；待结构与版本复核 |
| `menu.h.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `message-stream.cc.csv` | 3 | 0 | 候选记忆；待结构与版本复核 |
| `message.cc.csv` | 109 | 33 | 候选记忆；待结构与版本复核 |
| `message.h.csv` | 4 | 2 | 候选记忆；待结构与版本复核 |
| `mgen-data.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `misc.cc.csv` | 12 | 6 | 候选记忆；待结构与版本复核 |
| `mon-abil.cc.csv` | 44 | 12 | 候选记忆；待结构与版本复核 |
| `mon-act.cc.csv` | 106 | 43 | 候选记忆；待结构与版本复核 |
| `mon-behv.cc.csv` | 51 | 1 | 候选记忆；待结构与版本复核 |
| `mon-cast.cc.csv` | 357 | 44 | 候选记忆；待结构与版本复核 |
| `mon-clone.cc.csv` | 8 | 3 | 候选记忆；待结构与版本复核 |
| `mon-data.h.csv` | 540 | 539 | 候选记忆；待结构与版本复核 |
| `mon-death.cc.csv` | 257 | 35 | 候选记忆；待结构与版本复核 |
| `mon-ench.cc.csv` | 286 | 21 | 候选记忆；待结构与版本复核 |
| `mon-gear.cc.csv` | 22 | 1 | 候选记忆；待结构与版本复核 |
| `mon-grow.cc.csv` | 3 | 0 | 候选记忆；待结构与版本复核 |
| `mon-info.cc.csv` | 199 | 34 | 候选记忆；待结构与版本复核 |
| `mon-info.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `mon-movetarget.cc.csv` | 17 | 11 | 候选记忆；待结构与版本复核 |
| `mon-pathfind.cc.csv` | 13 | 5 | 候选记忆；待结构与版本复核 |
| `mon-pick.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `mon-place.cc.csv` | 58 | 10 | 候选记忆；待结构与版本复核 |
| `mon-poly.cc.csv` | 34 | 3 | 候选记忆；待结构与版本复核 |
| `mon-project.cc.csv` | 38 | 10 | 候选记忆；待结构与版本复核 |
| `mon-speak.cc.csv` | 101 | 0 | 候选记忆；待结构与版本复核 |
| `mon-tentacle.cc.csv` | 35 | 6 | 候选记忆；待结构与版本复核 |
| `mon-transit.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `mon-util.cc.csv` | 390 | 126 | 候选记忆；待结构与版本复核 |
| `monster.cc.csv` | 191 | 38 | 候选记忆；待结构与版本复核 |
| `monster.h.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `movement.cc.csv` | 75 | 69 | 候选记忆；待结构与版本复核 |
| `mutant-beast.h.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `mutation-data.h.csv` | 1576 | 907 | 候选记忆；待结构与版本复核 |
| `mutation.cc.csv` | 166 | 24 | 候选记忆；待结构与版本复核 |
| `nearby-danger.cc.csv` | 14 | 9 | 候选记忆；待结构与版本复核 |
| `newgame.cc.csv` | 208 | 122 | 候选记忆；待结构与版本复核 |
| `ng-init.cc.csv` | 20 | 0 | 候选记忆；待结构与版本复核 |
| `ng-input.cc.csv` | 15 | 1 | 候选记忆；待结构与版本复核 |
| `ng-setup.cc.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `ng-wanderer.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `noise.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `notes.cc.csv` | 97 | 7 | 候选记忆；待结构与版本复核 |
| `notes.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `orb.cc.csv` | 7 | 3 | 候选记忆；待结构与版本复核 |
| `ouch.cc.csv` | 54 | 36 | 候选记忆；待结构与版本复核 |
| `outer-menu.cc.csv` | 24 | 0 | 候选记忆；待结构与版本复核 |
| `output.cc.csv` | 321 | 258 | 候选记忆；待结构与版本复核 |
| `package.cc.csv` | 75 | 3 | 候选记忆；待结构与版本复核 |
| `pattern.cc.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `place.cc.csv` | 4 | 1 | 候选记忆；待结构与版本复核 |
| `playable.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `player-act.cc.csv` | 48 | 19 | 候选记忆；待结构与版本复核 |
| `player-equip.cc.csv` | 140 | 96 | 候选记忆；待结构与版本复核 |
| `player-reacts.cc.csv` | 43 | 39 | 候选记忆；待结构与版本复核 |
| `player-stats.cc.csv` | 45 | 23 | 候选记忆；待结构与版本复核 |
| `player.cc.csv` | 379 | 81 | 候选记忆；待结构与版本复核 |
| `player.h.csv` | 16 | 0 | 候选记忆；待结构与版本复核 |
| `potion.cc.csv` | 96 | 38 | 候选记忆；待结构与版本复核 |
| `prompt.cc.csv` | 40 | 3 | 候选记忆；待结构与版本复核 |
| `prompt.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `quiver.cc.csv` | 25 | 1 | 候选记忆；待结构与版本复核 |
| `randbook.cc.csv` | 62 | 3 | 候选记忆；待结构与版本复核 |
| `randbook.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `random-pick.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `random.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `ranged-attack.cc.csv` | 43 | 0 | 候选记忆；待结构与版本复核 |
| `religion.cc.csv` | 420 | 54 | 候选记忆；待结构与版本复核 |
| `religion.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `rot.cc.csv` | 17 | 0 | 候选记忆；待结构与版本复核 |
| `sacrifice-data.h.csv` | 34 | 0 | 候选记忆；待结构与版本复核 |
| `scroller.cc.csv` | 16 | 0 | 候选记忆；待结构与版本复核 |
| `scroller.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `shopping.cc.csv` | 136 | 114 | 候选记忆；待结构与版本复核 |
| `shout.cc.csv` | 139 | 2 | 候选记忆；待结构与版本复核 |
| `show.cc.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `showsymb.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `skill-menu.cc.csv` | 151 | 120 | 候选记忆；待结构与版本复核 |
| `skills.cc.csv` | 277 | 246 | 候选记忆；待结构与版本复核 |
| `sound.cc.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `species.cc.csv` | 17 | 11 | 候选记忆；待结构与版本复核 |
| `spl-book.cc.csv` | 78 | 3 | 候选记忆；待结构与版本复核 |
| `spl-cast.cc.csv` | 121 | 2 | 候选记忆；待结构与版本复核 |
| `spl-cast.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `spl-clouds.cc.csv` | 28 | 19 | 候选记忆；待结构与版本复核 |
| `spl-damage.cc.csv` | 212 | 59 | 候选记忆；待结构与版本复核 |
| `spl-data.h.csv` | 351 | 298 | 候选记忆；待结构与版本复核 |
| `spl-goditem.cc.csv` | 75 | 30 | 候选记忆；待结构与版本复核 |
| `spl-goditem.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `spl-miscast.cc.csv` | 570 | 155 | 候选记忆；待结构与版本复核 |
| `spl-miscast.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `spl-monench.cc.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `spl-other.cc.csv` | 27 | 0 | 候选记忆；待结构与版本复核 |
| `spl-selfench.cc.csv` | 30 | 22 | 候选记忆；待结构与版本复核 |
| `spl-selfench.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `spl-summoning.cc.csv` | 257 | 95 | 候选记忆；待结构与版本复核 |
| `spl-summoning.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `spl-tornado.cc.csv` | 17 | 7 | 候选记忆；待结构与版本复核 |
| `spl-transloc.cc.csv` | 71 | 17 | 候选记忆；待结构与版本复核 |
| `spl-transloc.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `spl-util.cc.csv` | 87 | 2 | 候选记忆；待结构与版本复核 |
| `spl-wpnench.cc.csv` | 15 | 1 | 候选记忆；待结构与版本复核 |
| `sqldbm.cc.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `sqldbm.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `stairs.cc.csv` | 98 | 0 | 候选记忆；待结构与版本复核 |
| `startup.cc.csv` | 47 | 30 | 候选记忆；待结构与版本复核 |
| `stash.cc.csv` | 124 | 1 | 候选记忆；待结构与版本复核 |
| `stash.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `state.cc.csv` | 54 | 0 | 候选记忆；待结构与版本复核 |
| `state.h.csv` | 6 | 0 | 候选记忆；待结构与版本复核 |
| `status.cc.csv` | 203 | 101 | 候选记忆；待结构与版本复核 |
| `store.cc.csv` | 15 | 0 | 候选记忆；待结构与版本复核 |
| `stringutil.cc.csv` | 28 | 0 | 候选记忆；待结构与版本复核 |
| `stringutil.h.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `syscalls.cc.csv` | 18 | 0 | 候选记忆；待结构与版本复核 |
| `tags.cc.csv` | 195 | 15 | 候选记忆；待结构与版本复核 |
| `target.cc.csv` | 53 | 0 | 候选记忆；待结构与版本复核 |
| `teleport.cc.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `terrain.cc.csv` | 82 | 0 | 候选记忆；待结构与版本复核 |
| `throw.cc.csv` | 57 | 2 | 候选记忆；待结构与版本复核 |
| `tile-player-flags.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilecell.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tiledoll.cc.csv` | 26 | 1 | 候选记忆；待结构与版本复核 |
| `tilemcache.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilepick-p.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `tilepick.cc.csv` | 73 | 2 | 候选记忆；待结构与版本复核 |
| `tilereg-abl.cc.csv` | 11 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-abl.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-cmd.cc.csv` | 5 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-cmd.h.csv` | 2 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-dgn.cc.csv` | 52 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-doll.cc.csv` | 40 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-grid.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-inv.cc.csv` | 53 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-inv.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-map.cc.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-mem.cc.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-mem.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-mon.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-msg.cc.csv` | 4 | 1 | 候选记忆；待结构与版本复核 |
| `tilereg-skl.cc.csv` | 15 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-skl.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-spl.cc.csv` | 11 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-spl.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-stat.cc.csv` | 1 | 1 | 候选记忆；待结构与版本复核 |
| `tilereg-tab.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-tab.h.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilereg-text.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tilesdl.cc.csv` | 25 | 0 | 候选记忆；待结构与版本复核 |
| `tiletex.cc.csv` | 7 | 0 | 候选记忆；待结构与版本复核 |
| `tileview.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `tileweb-text.cc.csv` | 21 | 0 | 候选记忆；待结构与版本复核 |
| `tileweb.cc.csv` | 339 | 0 | 候选记忆；待结构与版本复核 |
| `tileweb.h.csv` | 4 | 0 | 候选记忆；待结构与版本复核 |
| `timed-effects.cc.csv` | 56 | 1 | 候选记忆；待结构与版本复核 |
| `transform.cc.csv` | 139 | 2 | 候选记忆；待结构与版本复核 |
| `traps.cc.csv` | 99 | 40 | 候选记忆；待结构与版本复核 |
| `travel.cc.csv` | 142 | 54 | 候选记忆；待结构与版本复核 |
| `tutorial.cc.csv` | 13 | 0 | 候选记忆；待结构与版本复核 |
| `ui.cc.csv` | 49 | 0 | 候选记忆；待结构与版本复核 |
| `uncancel.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `unicode.cc.csv` | 8 | 0 | 候选记忆；待结构与版本复核 |
| `version.cc.csv` | 16 | 0 | 候选记忆；待结构与版本复核 |
| `view.cc.csv` | 76 | 3 | 候选记忆；待结构与版本复核 |
| `viewchar.cc.csv` | 69 | 0 | 候选记忆；待结构与版本复核 |
| `viewgeom.cc.csv` | 3 | 0 | 候选记忆；待结构与版本复核 |
| `viewmap.cc.csv` | 9 | 2 | 候选记忆；待结构与版本复核 |
| `wcwidth.cc.csv` | 1 | 0 | 候选记忆；待结构与版本复核 |
| `windowmanager-sdl.cc.csv` | 38 | 0 | 候选记忆；待结构与版本复核 |
| `wiz-dgn.cc.csv` | 81 | 21 | 候选记忆；待结构与版本复核 |
| `wiz-dump.cc.csv` | 14 | 0 | 候选记忆；待结构与版本复核 |
| `wiz-fsim.cc.csv` | 75 | 0 | 候选记忆；待结构与版本复核 |
| `wiz-fsim.h.csv` | 2 | 2 | 候选记忆；待结构与版本复核 |
| `wiz-item.cc.csv` | 283 | 7 | 候选记忆；待结构与版本复核 |
| `wiz-mon.cc.csv` | 179 | 22 | 候选记忆；待结构与版本复核 |
| `wiz-you.cc.csv` | 102 | 26 | 候选记忆；待结构与版本复核 |
| `wizard.cc.csv` | 123 | 26 | 候选记忆；待结构与版本复核 |
| `xom.cc.csv` | 441 | 148 | 候选记忆；待结构与版本复核 |
| `zap-data.h.csv` | 73 | 58 | 候选记忆；待结构与版本复核 |

## 6. 盘点检查与后续顺序

- 已按 TextDB 注册表列出 46 个独立英文输入文件，并定位 31 个现有中文文件。
- 已记录文件与条目正文 SHA-256、同文件缺项/独有键、重复键及 Lua 分隔符数量；扫描不执行 Lua。
- 已检查资源在注册表中的关系；不把随机权重、别名或相同英文正文直接视为漏译。
- 尚未完成：全量语义复核、Lua 执行与分支覆盖、游戏内版式和回退验证。

下一批建议：先审 `skills.txt`、`species.txt`、`backgrounds.txt`、一小组 `items.txt` 和一组 `status.txt`；同时验证一个带 Lua 的真实条目，再进入阶段 1 双语样例。

盘点快照：[phase-0-text-audit.json](../../localization/manifests/phase-0-text-audit.json)。文本入口：[文本来源清单](text-source-inventory.md)。术语变更：[术语校对记录](phase-0-terminology-review.md)。

### 结构扫描发现

重复键按 TextDB 最后写入覆盖规则计数，以下需定位是否有意覆盖：

- `crawl-ref/source/dat/descript/commands.txt`：英文重复 ['cmd_explore_no_rest']；中文重复 []。
- `crawl-ref/source/dat/descript/status.txt`：英文重复 []；中文重复 ['-cast status']。
- `crawl-ref/source/dat/descript/monstatus.txt`：英文重复 ['damage-immune at range monstatus']；中文重复 []。
- `crawl-ref/source/dat/descript/mutations.txt`：英文重复 ['+los mutation']；中文重复 ['+los mutation']。
- `crawl-ref/source/dat/database/monspeak.txt`：英文重复 ['_laughs_']；中文重复 []。
- `crawl-ref/source/dat/database/decorlines.txt`：英文重复 ['jiyva peaceful fountain_blue']；中文重复 []。
- Lua 起止标记数量异常：未发现。数量相等不代表语法正确。
