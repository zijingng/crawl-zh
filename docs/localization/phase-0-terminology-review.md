# 阶段 0：术语表校对记录

日期：2026-10-20。基线：官方 DCSS `0.34.1`，提交 `1eebc1a2892e1c89776a0d7a10691f8dac8d9796`。

## 交付与范围

输入：[terms-808.csv](../../../paratranz/terms-808.csv)。输出：[校对后的 CSV](../../../paratranz/terms-808-reviewed-0.34.1.csv)。原文件未改动。

原表 744 条，保留全部原始 Term 和原始顺序；修改 91 条中文译文，另补充 173 条，共 917 条。共有 276 个原条目的译文、词性、别名或备注发生变化，其中 185 条仅修改非译文字段。

格式保留 `Term,Translation,Part of Speech,Variants,Note` 五列，UTF-8 BOM、正确引用多行内容。新增项追加到末尾，不覆盖现有项目中的空 glossary.csv，也不修改游戏源码或运行时数据库。

本次完成的是阶段 0 中的术语盘点、错漏校对和版本适用性标注；没有把整个游戏文本清单、双语截图或运行时翻译验证冒充已完成。

## 方法与覆盖

逐项阅读原表，结合当前源码/英文说明核对语境，并扫描核心 C++、头文件、顶层英文 TextDB 及种族/背景/形态 YAML。历史 status 标签另参考用户提供的 2022 年 Paratranz 导出，仅用于解释旧词，不把旧机制用于当前版本。

完整对应当前说明清单中的 29 个技能、26 个背景、35 个非 deprecated 种族名称和 26 位现行神祇。种族名称数包含龙人基础名和八种颜色，不能理解成 35 个独立创建选项。

现行技能以英文 `dat/descript/skills.txt` 为清单，而不是把 `#if TAG_MAJOR_VERSION == 34` 中的兼容技能全部算成可训练技能。种族排除 deprecated 文件；现行神祇排除 Pakellas，并单独处理 Jiyva 的动态名称。

其他物品、怪物和状态按本次原表及高频缺项检查，并未穷尽所有法术、神器、地图和风味文本。英文原词检索不到不等于已经删除：可能是分段拼接、词形差异或旧名称，均用“待定位”备注保留。源码仍有某个名字，也不等于该内容仍能在常规游戏生成。

主要源码依据：

- `crawl-ref/source/dat/descript/skills.txt`、`skills.cc`：技能类别及效果。
- `crawl-ref/source/dat/species/*.yaml`、`dat/jobs/*.yaml`：现行种族和背景。
- `crawl-ref/source/religion.cc`、`god-type.h`、`dat/descript/gods.txt`：神祇。
- `crawl-ref/docs/crawl_manual.rst`：AC、EV、SH 等属性的实际标签和含义。
- `crawl-ref/source/dat/descript/items.txt`、`unrand.txt`、`monsters.txt`、`features.txt`、`branches.txt`：物品、怪物、地形和分支。
- `crawl-ref/source/status.cc`、`duration-data.h`、`dat/descript/status.txt`：状态灯号及说明。
- `crawl-ref/docs/changelog.txt`：技能合并、种族移除等版本变化。

## 关键处理

- ability/skill、Armour/AC、Dodging/EV、Shields/SH、Evocable/Evocations 分开，避免把类别、属性和技能混成一个词。
- “唤发”作为 Evocations 的项目建议译名；其当前说明不包含提高 MP 上限。Invocations 的 MP 规则单独保留。
- 普通物品 shield 可以译盾牌，但本表原 Shield 条目的上下文是 SH 属性，校对为格挡值。
- scourge 的原导出上下文是怪物，不是武器；神圣重鞭使用独立的 sacred scourge 条目。
- portal/transporter、Vaults/Trove、amulet/talisman 分开，避免不同区域或物品类别相互污染。
- “正在石化/已石化”“中毒/毒性辐射”“引导魔力/空间通道”按实际描述区分。
- -Wiz、Hevn、Lash 等旧短标签通过历史原文解释，不能逐字猜测。Clouds/Hop 旧键漏负号问题保留原记录，追加真实 -Clouds/-Hop 标签。
- Alive 同键对应的历史英文与当前英文有明显差异，按当前“充满鲜血”说明处理，展示为何必须追踪英文正文变化。
- magic points 的宽泛别名 Magic、max 的 reserves、background 的 class、throw 的 throwing 移除，避免不同语境被自动当成同义词。
- 旧食物系统、独立 Bows/Crossbows/Slings、Poison Magic/Transmutations、Charms 和废弃玩家种族只保留历史或上下文用途。

## 译名建议与保留限制

其中 9 项译文变化属于译名或一致性建议，不是可以证明唯一答案的客观错字。例如 Wu Jian→武剑会、Ziggurat→神塔、Trove→秘藏、scourge→灾厄巨蛇。其英文对象和机制有源码依据，中文专名属于本项目约定，后续可统一调整。

所有新增中文名同样是建议术语，不宣称是官方中文。原表未发现需要随意重写的音译大多沿用。保留“Ru→入”等既有专名，并补充组合语境。

本表是辅助翻译的术语资产，不是运行时替换表。不能据它批量替换英文子串、数据库键、Lua 字符串判断、快捷键或已带英文语法的对象名。带“待定位”或“历史”备注的条目不能自动进入当前版本正式译文。此 CSV 不包含平台内部审核状态，不等同 Paratranz 已批准标记。

## 全部译文修改

以下逐条列出中文译文发生变化的原条目。非译文字段变化和范围标注保存在各行 Note 中。

| 英文术语 | 原译 | 校对译文 | 类型与理由 |
| --- | --- | --- | --- |
| ability | 技能 | 能力 | 修订：与 skill（技能）区分；包括神赐、种族及其他主动能力。 |
| AC | 护甲等级 | 护甲值 | 修订：与 Armour（护甲技能/防具）区分；缩写在 HUD 可保留 AC。 |
| armour class | 护甲 | 护甲值 | 修订：与 AC 统一；表示减伤属性，不是 D&D 中用于判定命中的护甲等级。具体减伤受攻击类型影响，不把 0～AC 简化为无例外公式。 |
| Fighting | 格斗 | 战斗 | 修订：物理战斗通用技能，亦增加生命上限；与 Unarmed Combat（徒手格斗）区分。 |
| evasion | 闪避 | 闪避值 | 修订：属性 EV；与 Dodging（闪避技能）区分。 |
| Shield | 格挡 | 格挡值 | 修订：原表备注明确是属性；官方手册也以 Shield 为 SH 标题。此处保留该语境；普通物品 shield 仍译盾牌，必须按上下文区分。 |
| Maces & Flails | 锤 | 锤与连枷 | 修订：补出 Flails，避免把整个类别缩成单一锤类。 |
| Polearms | 长柄 | 长柄武器 | 修订：补全类别名称。 |
| Staves | 棍 | 棍杖 | 修订：技能包含棍棒及法杖的近战使用；staff 作为魔法装备仍译法杖。 |
| Evocations | 魔法物品 | 唤发 | 修订：技能而非魔法物品；影响魔杖和其他可唤发物品的强度或成功率，当前说明不含提高 MP 上限。 |
| Evocable | 魔法物品 | 可唤发物品 | 修订：与 Evocations（唤发技能）区分；作形容词时译“可唤发的”。 |
| Hexes | 诅咒系魔法 | 妖术系魔法 | 修订：区分敌对魔法学派与物品诅咒，也与 hexer（妖术师）统一。 |
| Charms | 附魔系魔法 | 增益系魔法 | 修订：旧学派，侧重增益，不与 Hexes 混用；现行技能表仅保留兼容槽位。 |
| Charms school | 附魔系魔法 | 增益系魔法 | 修订：旧版学派说明用语，不是 0.34.1 可训练学派。 |
| aptitude | 天赋 | 技能资质 | 修订：数值修正的技能成长资质，与天生特性/变异区分。 |
| background | 职业 | 背景 | 修订：角色创建的起始背景，不是固定职业成长系统。 |
| animate | 赋予生命，活化 | 活化 | 修订：单一首选译法；动词按宾语可用“赋予生命”，不将多个候选一起显示。 |
| necromantic | 巫术 | 死灵的 | 修订：形容词指向 Necromancy（死灵系魔法），不泛译巫术。 |
| undead | 不死 | 亡灵 | 修订：指不死生物类别，并不表示无法被杀死。 |
| undead allies | 不死盟国 | 亡灵盟友 | 修订：allies 是盟友，不是盟国。 |
| drain | 衰竭 | 吸能 | 修订：吸能状态与负能量、属性降低并非同一概念；具体消息区分吸取 HP、MP、最大 HP 等。 |
| draining | 流失 | 吸能 | 修订：与 drain 统一；作为武器印记或负能量效果时按场景表达，不机械译为流失。 |
| magical protection | 魔法抗性 | 魔法防护 | 修订：含义依上下文；不可泛作 magic resistance/willpower，避免与元素抗性和意志混淆。 |
| magic points | 魔法值 | 魔力值 | 修订：HP 对应生命值，MP 统一魔力值；删除过宽别名 Magic，保留 MP。 |
| max | (生命值/魔法值)上限 | 最大值 | 修订：通用上限词，不限于 HP/MP；reserves 表储备，不是 maximum 同义词。 |
| rPois | 毒抗，毒抗性 | 毒抗 | 修订：首选译法统一；HUD 可保留 rPois，不在正式显示中列多个候选。 |
| SInv | 反隐 | 看破隐形 | 修订：与 ring of see invisible 统一；不表示驱散隐形。 |
| ring of see invisible | 隐形侦测戒指 | 看破隐形戒指 | 修订：检测隐形生物，不是让隐形状态消失。 |
| Mephitic Cloud | 臭云，臭云术 | 臭云术 | 修订：法术采用单一首选译名；云雾本体可译臭云。 |
| exit | 出口； 退出 | 出口 | 修订：名词用出口；命令动词用退出，按上下文处理，不把两者拼成一条输出。 |
| mace | 权杖 | 钉头锤 | 修订：武器名称，不是象征权力的 sceptre（权杖）。 |
| bardiche | 大砍刀 | 长柄大斧 | 修订：原文明确为长杆末端装大刃的武器，不是大砍刀。 |
| scourge | 祸根 | 灾厄巨蛇 | 译名建议：旧导出中的本条来自 monsters.txt，当前同名怪物描述为巨大病蛇；不能按武器 scourge 翻译。神圣鞭类武器另用 sacred scourge。 |
| great mace "Firestarter" | 「烈焰」大矛锤 | 「烈焰」巨型钉头锤 | 修订：mace 为锤，纠正“大矛锤”；保留专名旧译。 |
| great mace "Undeadhunter" | 「不死猎人」大矛锤 | 「亡灵猎手」巨型钉头锤 | 修订：纠正武器类型，与 undead 术语统一。 |
| boots of the spider | 蛛蛛靴 | 蜘蛛靴 | 修订：纠正“蛛蛛”错字。 |
| caustic shrike | 强酸伯劳⻦. | 强酸伯劳鸟 | 修订：移除尾部句点，将部首字符“⻦”改为正常汉字“鸟”。 |
| shard shrike | 碎片伯劳⻦ | 碎片伯劳鸟 | 修订：将部首字符“⻦”改为正常汉字“鸟”。 |
| thin mist cloud | 蔳雾云 | 薄雾云 | 修订：纠正“蔳”错字。 |
| demon trident "Rift" | 「裂㾗」恶魔三叉戟 | 「裂痕」恶魔三叉戟 | 修订：纠正“裂㾗”异体/错字。 |
| throwing net | 网 | 投掷网 | 修订：补出用途，避免与地形或约束状态的普通网混用。 |
| simulacrum | 幻象 | 冰骸 | 修订：该怪物是以冰雪构成的遗骸复制体，不是普通幻象。 |
| spellforged servitor | 魔法仆人 | 法术锻造仆从 | 修订：旧版怪物名，保留造物含义；当前另有 spellspark servitor，不自动视为同一英文条目。 |
| revenant | 鬼火骷髅 | 还魂者 | 修订：当前同时是可选种族与怪物名；原“鬼火骷髅”仅描述形象，统一为可用于种族界面的名称。 |
| Spriggan's Knife | 守宝妖精的刀 | 斯普利根之刃 | 一致性建议：与补充的 Spriggan 种族译名一致；专名译法建议。 |
| Trove | 宝库 | 秘藏 | 一致性建议：与 Vaults（宝库）分支区分，指需满足条件才能进入的藏宝传送门。 |
| portal to a secret trove of treasure | 通往秘密宝库的传送门 | 通往秘密藏宝处的传送门 | 修订：与 Trove 术语含义对应，不误作 Vaults 分支入口。 |
| Ziggurat | 金字塔 | 神塔 | 一致性建议：与原表 figurine of a ziggurat（神塔小雕像）统一；它是特殊挑战区域，不泛称金字塔。 |
| gateway to a ziggurat | 金字塔的入口 | 通往神塔的入口 | 一致性建议：与 Ziggurat 统一。 |
| Desolation | 荒地 | 荒盐之地 | 一致性建议：指 Desolation of Salt；补出盐的主题，与普通荒地区分。 |
| transporter | 传送门 | 转移装置 | 修订：同层单向转移设施，与通往其他区域的 portal 区分。 |
| transporter landing site | 传送门出口 | 转移装置落点 | 修订：不能由落点原路返回，不称双向传送门出口。 |
| dispersal trap | 驱散陷阱 | 空间移位陷阱 | 修订：dispersal 在此是空间位移而非 dispel（驱散魔法）。 |
| miscast | 施法失败 | 施法失误 | 修订：区分普通施法失败与其可能产生的 miscast 效果；失败率本身用 failure rate。 |
| Channel Magic | 魔力通道 | 引导魔力 | 修订：神赐能力，持续恢复魔力，不是建立空间通道。 |
| Wu Jian | 无间 | 武剑会 | 译名建议：作为武术神祇集体的建议译名，与 Council 含义一致；旧备注“无间道”不是英文原文依据，不据此扩写神话背景。 |
| Lash status | 鞭打状态 | 腾蛇鞭状态 | 修订：对应 Serpent's Lash 的高速移动能力，不是受到鞭打。 |
| Hevn status | 宇状态 | 天界风暴状态 | 修订：旧缩写指 Heavenly Storm，不是“宇”；当前灯号使用 Heavenly。 |
| Petr status | 石化状态 | 正在石化状态 | 修订：DUR_PETRIFYING；与 Stone（已经石化）区分。 |
| Stone status | 石状态 | 已石化状态 | 修订：DUR_PETRIFIED；不是普通石头或岩石形态。 |
| Toxic status | 毒状态 | 毒性辐射状态 | 修订：DUR_TOXIC_RADIANCE 表示向外释放毒能，不是 Pois 中毒状态。 |
| MR/2 status | 魔抗/2状态 | 魔抗减半状态 | 修订：旧标签；当前对应意志降低时使用 Will/2，应补充现行条目。 |
| -Potion status | -药水状态 | 禁饮药水状态 | 修订：前置减号表示无法使用或限制，不应直接拼成“-药水”。 |
| -Scroll status | -卷轴状态 | 禁读卷轴状态 | 修订：前置减号表示无法阅读或限制。 |
| Clouds status | -云状态 | 禁云状态 | 修订：旧表键疑漏前置减号；历史导出实际是 -Clouds，指静风使云雾无法形成，不是云雾免疫。保留此旧键供追溯，新增正确键。 |
| Abj status | 驱逐状态 | 召唤驱散状态 | 修订：历史英文说明为缩短附近敌对召唤物持续时间，不等同 banishment（放逐到深渊）。 |
| Prot status | 保护状态 | 神圣庇护状态 | 修订：历史说明为请求 Elyvilon 阻止致命一击；不要与泛指保护的 Protected 混用。 |
| Lich status | 巫妖状态 | 巫妖形态状态 | 修订：旧变身标签；不是当前物种名称。 |
| Wisp status | 气状态 | 气团形态状态 | 修订：历史说明明确是 wisp of gas（无实体气团），不译成气系魔法或火焰鬼火。 |
| Ice status | 冰状态 | 冰兽形态状态 | 修订：旧 Ice Form 变身标签，不泛称冰属性状态。 |
| Blade status | 刃状态 | 刃手状态 | 修订：Blade Hands 的变身标签，不是武器类别。 |
| Hop status | -跳跃状态 | 跳跃冷却状态 | 修订：旧表 -Hop 的负号丢失风险；跳跃能力与其冷却需根据真正英文灯号区分，本旧键不用于运行时映射。 |
| umbraed | 本影 | 处于幽暗光环中 | 修订：表达受到 umbra 影响的状态，不把形容状态仅译为名词本影。 |
| haloed | 光环 | 处于光环中 | 修订：形容状态，不等同光环物体。 |
| disjunction haloed | 分离光环 | 处于空间排斥光环中 | 修订：空间效果，不是普通分离动作。 |
| magical condensation cloud | 魔法聚集云 | 魔法凝结云 | 修订：condensation 为凝结，与普通能量聚集区分。 |
| seething chaos cloud | 暴怒混乱云 | 翻涌混沌云 | 修订：chaos 在此是混沌效果，不是 confusion（混乱状态）。 |
| sparse dust cloud | 薄灰云 | 稀薄尘云 | 修订：dust 为尘土，不是灰烬。 |
| tiles lag | 贴图滞后，贴图延迟 | 图形显示延迟 | 一致性建议：工具/表现问题用语，采用单一首选译法。 |
| Console | 控制台 | 字符版 | 修订：DCSS 版本类型，与 Tiles（图形版）成对；命令行控制台本身按上下文另译。 |
| ruined idol | 毁灭的神像 | 残破神像 | 修订：ruined 表破损状态，不是毁灭神的神像。 |
| rune-carved clear stone wall | 刻有符文的清晰的石墙 | 刻有符文的透明石墙 | 修订：clear 在此是可透视的墙体，不是清晰程度。 |
| gateway to the decaying netherworld of Tartarus | 堕落下界塔尔塔罗斯的入口 | 通往塔尔塔罗斯腐朽冥界的入口 | 修订：decaying 为腐朽，不是道德上的堕落。 |
| great orb of eyes | 巨大眼球 | 巨型眼球聚合体 | 译名建议：由大量眼球组成，避免与普通单颗巨大眼球混淆。 |
| starcursed | 星之诅咒谜团 | 星咒 | 修订：此词为 starcursed mass 的修饰成分，不独自补入 mass；完整怪物名另收录。 |
| -Wiz status | -巫师状态 | 施法控制削弱状态 | 修订：历史说明为施法控制被削弱，不是巫师职业或巫师模式。 |
| Blood status | 血状态 | 凝血护甲状态 | 修订：描述身体表面凝血形成护甲，不是泛指血量。 |
| Channel status | 通道状态 | 引导魔力状态 | 修订：正在引导魔法能量，不是空间通道。 |
| Liquid status | 液状态 | 地面液化状态 | 修订：描述周围地面液化，不是身体化为液体。 |
| Alive status | 生命状态 | 充血状态 | 修订：0.34.1 英文说明为充满鲜血并快速再生，对应旧吸血鬼血量状态；历史导出此条英文曾写禁饮，来源已发生变化，不能仅按同键沿用。 |
| Aug status | 增强状态 | 生命增幅状态 | 修订：增益随当前健康程度变化，不是通用强化。 |

## 新增条目清单

- **现行种族（17）**：`Armataur`、`Coglin`、`Deep Elf`、`Demigod`、`Djinni`、`Draconian`、`Felid`、`Formicid`、`Human`、`Merfolk`、`Minotaur`、`Mountain Dwarf`、`Naga`、`Octopode`、`Oni`、`Poltergeist`、`Spriggan`。
- **现行背景（23）**：`Air Elementalist`、`Alchemist`、`Berserker`、`Brigand`、`Chaos Knight`、`Cinder Acolyte`、`Conjurer`、`Delver`、`Earth Elementalist`、`Enchanter`、`Fire Elementalist`、`Forgewright`、`Gladiator`、`Hedge Wizard`、`Hexslinger`、`Ice Elementalist`、`Monk`、`Necromancer`、`Reaver`、`Shapeshifter`、`Summoner`、`Wanderer`、`Warper`。
- **现行技能（5）**：`Ranged Weapons`、`Throwing`、`Forgecraft`、`Alchemy`、`Shapeshifting`。
- **神祇（2）**：`Lugonu`、`Ignis`。
- **属性（2）**：`SH`、`willpower`。
- **HUD缩写（4）**：`Will`、`rN`、`rElec`、`rCorr`。
- **历史属性（1）**：`magic resistance`。
- **神祇机制（2）**：`piety`、`penance`。
- **时间（1）**：`aut`。
- **战斗机制（2）**：`line of sight`、`noise`。
- **法术属性（2）**：`spell level`、`failure rate`。
- **装备属性（3）**：`encumbrance rating`、`encumbrance`、`slaying`。
- **物品操作（1）**：`evoke`。
- **物品类别（2）**：`evoker`、`talisman`。
- **防具（5）**：`barding`、`armataur barding`、`golden dragon scales`、`kite shield`、`tower shield`。
- **武器（1）**：`sacred scourge`。
- **分支（1）**：`Elven Halls`。
- **区域（2）**：`Desolation of Salt`、`Sewer`。
- **模式（1）**：`Dungeon Sprint`。
- **神赐能力（2）**：`Serpent's Lash`、`Heavenly Storm`。
- **怪物（2）**：`starcursed mass`、`spellspark servitor`。
- **状态（10）**：`confusion`、`paralysis`、`corrosion`、`berserk`、`haste`、`silence`、`invisibility`、`petrifying`、`petrified`、`contamination`。
- **伤害/状态（1）**：`poison`。
- **状态灯号（3）**：`-Clouds status`、`-Hop status`、`Heavenly status`。
- **物品（49）**：`potion of brilliance`、`potion of enlightenment`、`potion of moonshine`、`potion of attraction`、`scroll of acquirement`、`scroll of butterflies`、`scroll of fear`、`scroll of fog`、`scroll of identify`、`scroll of poison`、`scroll of revelation`、`scroll of torment`、`ring of dexterity`、`ring of evasion`、`ring of flight`、`ring of intelligence`、`ring of magical power`、`ring of poison resistance`、`ring of protection`、`ring of protection from cold`、`ring of protection from fire`、`ring of willpower`、`ring of strength`、`ring of wizardry`、`amulet of chemistry`、`amulet of dissipation`、`amulet of wildshape`、`blade talisman`、`dragon-coil talisman`、`eel talisman`、`fortress talisman`、`granite talisman`、`hive talisman`、`inkwell talisman`、`lupine talisman`、`maw talisman`、`medusa talisman`、`protean talisman`、`quill talisman`、`riddle talisman`、`rimehorn talisman`、`sanguine talisman`、`scarab talisman`、`serpent talisman`、`spider talisman`、`spore talisman`、`storm talisman`、`talisman of death`、`wellspring talisman`。
- **状态描述键（29）**：`Fast status`、`Invis status`、`Tele status`、`Fly status`、`Brill status`、`Might status`、`Regen status`、`MPRegen status`、`Berserk status`、`Pois status`、`Conf status`、`Contam status`、`Drain status`、`Slow status`、`Para status`、`Sleep status`、`Held status`、`Constr status`、`Mesm status`、`Fear status`、`Mark status`、`Fire status`、`Barbs status`、`Vertigo status`、`Corr status`、`Weak status`、`Will/2 status`、`Hero status`、`Finesse status`。

## 校验结果

- 原始 744 个英文键和顺序保留，未覆盖原 CSV；新增英文键按忽略大小写检查，无重复。
- 所有输出行均为五列，Term/Translation 非空；跨行备注和引号可完整往返解析。
- 29 个技能、26 个背景、35 个种族名称与 26 个现行神祇均有对应术语。
- 校对表通过 Artifact Tool 的值检查和代表性预览；最终 CSV 另用独立 CSV 解析器复核。
- 没有运行游戏内译文测试：本次仅交付术语表和阶段 0 校对记录。

输入 SHA-256：`3d38be62d8309269d6876262f0d71d7b543152f72574603d982b684004670ba1`。
