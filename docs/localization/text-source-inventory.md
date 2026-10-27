# 阶段 0：文本来源清单

基线：DCSS `0.34.1`，提交 `1eebc1a2892e1c89776a0d7a10691f8dac8d9796`。盘点日期：2026-10-20。首期交付后端：Windows 本地 Tiles。

本清单按文本来源与接入方式分类；不是全仓每个字符串的完整抽取目录。TextDB 文件清单由当前注册表实际枚举，代码和脚本部分列主要入口及后续抽取范围，不把候选扫描结果当成待翻译全集。

## 1. TextDB 数据文本

当前注册 11 个逻辑数据库，共 46 个不同的输入文件；其中 31 个有同名中文文件。一个输入文件可被多个数据库复用，计数不重复。

入口：[database.cc](../../crawl-ref/source/database.cc)。英文在 `dat/descript/` 或 `dat/database/`，中文在对应 `zh/` 子目录。查询首先尝试中文，缺少条目后查英文；部分调用显式要求 untranslated。说明内还可能执行 Lua、引用其他条目及进行随机加权选择。

现有回退只判断查询结果，不会自动发现“旧译文仍有同键但机制已变化”；来源哈希和复核状态仍须在后续构建层接入。

| 英文输入文件（相对 source/dat） | 逻辑数据库 | 主要内容 | 英文键数 | 中文文件 | 优先级 |
| --- | --- | --- | ---: | --- | --- |
| `descript/features.txt` | descriptions | 地形、门、楼梯、陷阱和入口说明 | 290 | 有 | P1 |
| `descript/items.txt` | descriptions | 常规物品、药水、卷轴、饰品和护符说明 | 307 | 有 | P0 |
| `descript/unident.txt` | descriptions | 未鉴定外观说明 | 7 | 有 | P1 |
| `descript/unrand.txt` | descriptions | 固定神器说明 | 140 | 有 | P1 |
| `descript/monsters.txt` | descriptions | 怪物说明 | 724 | 有 | P1 |
| `descript/spells.txt` | descriptions | 法术说明 | 402 | 有 | P0 |
| `descript/gods.txt` | descriptions | 神祇、能力和神罚说明 | 82 | 有 | P0 |
| `descript/branches.txt` | descriptions | 分支和特殊区域说明 | 36 | 有 | P1 |
| `descript/skills.txt` | descriptions | 技能说明 | 29 | 有 | P0 |
| `descript/ability.txt` | descriptions | 主动能力说明 | 154 | 有 | P0 |
| `descript/cards.txt` | descriptions | 卡牌说明 | 24 | 有 | P1 |
| `descript/commands.txt` | descriptions | 命令说明 | 95 | 有 | P1 |
| `descript/clouds.txt` | descriptions | 云雾说明 | 39 | 有 | P1 |
| `descript/status.txt` | descriptions | 玩家状态描述 | 237 | 有 | P0 |
| `descript/monstatus.txt` | descriptions | 怪物状态描述 | 139 | 无 | P0 |
| `descript/mutations.txt` | descriptions | 变异说明 | 251 | 有 | P0 |
| `descript/passives.txt` | descriptions | 被动能力说明 | 20 | 有 | P0 |
| `descript/species.txt` | gamestart | 角色创建的种族说明 | 27 | 有 | P0 |
| `descript/backgrounds.txt` | gamestart | 角色创建的背景说明 | 26 | 有 | P0 |
| `database/randname.txt` | randart | 随机名称片段 | 33 | 无 | P1 |
| `database/rand_wpn.txt` | randart | 随机武器名称片段 | 45 | 无 | P1 |
| `database/rand_arm.txt` | randart | 随机防具名称片段 | 19 | 无 | P1 |
| `database/rand_all.txt` | randart | 随机饰品等名称片段 | 18 | 无 | P1 |
| `database/randbook.txt` | randart | 随机法术书名称片段 | 72 | 无 | P1 |
| `database/monspeak.txt` | speak | 怪物对白 | 731 | 无 | P1 |
| `database/monspell.txt` | speak | 怪物施法对白 | 262 | 有 | P1 |
| `database/monflee.txt` | speak | 怪物逃跑对白 | 1 | 有 | P1 |
| `database/wpnnoise.txt` | speak | 武器发声文本 | 65 | 有 | P1 |
| `database/insult.txt` | speak, shout | 嘲讽文本 | 33 | 无 | P1 |
| `database/godspeak.txt` | speak | 神祇对白 | 193 | 有 | P1 |
| `database/monname.txt` | speak, misc | 使徒、先祖等命名数据 | 52 | 无 | P1 |
| `database/colourname.txt` | speak, misc | 颜色与图案命名 | 9 | 无 | P1 |
| `database/graffiti.txt` | speak, misc | 涂鸦文本与作者 | 58 | 无 | P1 |
| `database/miscast.txt` | speak | 施法失误文本 | 33 | 无 | P1 |
| `database/shout.txt` | shout | 叫喊文本 | 93 | 有 | P1 |
| `database/miscname.txt` | misc | 杂项名称 | 10 | 有 | P1 |
| `database/godname.txt` | misc | 神祇称谓/后缀 | 23 | 有 | P1 |
| `database/montitle.txt` | misc | 怪物称号 | 86 | 有 | P1 |
| `database/decorlines.txt` | misc | 装饰地形消息 | 132 | 无 | P1 |
| `database/gizmo.txt` | misc | 装置名称组合 | 3 | 无 | P1 |
| `descript/quotes.txt` | quotes | 引言 | 465 | 有 | P1 |
| `database/help.txt` | help | 帮助文本 | 13 | 有 | P1 |
| `database/FAQ.txt` | FAQ | 常见问题 | 38 | 有 | P1 |
| `descript/hints.txt` | hints | 提示模式文本 | 116 | 有 | P0 |
| `descript/tutorial.txt` | hints | 教程文本 | 88 | 有 | P0 |
| `descript/egos.txt` | egos | 印记和防具特殊效果说明 | 80 | 无 | P0 |

键数是按当前 TextDB 文本解析方式得到的有效键数，重复键以最后一条为准。它不是可见 UI 字符串数，也不是有效中文覆盖率。

## 2. C++ 界面、消息和名称

以下文件均以 `crawl-ref/source/` 为根。P0 优先跑通角色创建、HUD、背包、描述和代表性战斗消息；其余按模块渐进覆盖。

| 模块 | 主要源码入口 | 文本形态与接入点 | 优先级 |
| --- | --- | --- | --- |
| 创建角色与主菜单 | `newgame.cc`, `ng-setup.cc`, `outer-menu.cc` | 菜单标题、种族/背景选项、帮助和选择提示。名称数据也来自 YAML。 | P0 |
| HUD 与状态 | `output.cc`, `status.cc`, `duration-data.h`, `tilereg-stat.cc` | 属性、状态灯号和短描述。保留快捷缩写和消息通道；检查显示宽度。 | P0 |
| 背包与物品交互 | `invent.cc`, `item-use.cc`, `item-name.cc`, `shopping.cc` | 物品名称组合、装备、使用、拾取和商店提示；不能只翻译长说明。 | P0 |
| 物品/怪物/法术说明界面 | `describe.cc`, `describe-spells.cc`, `describe-god.cc`, `mon-info.cc` | 从 TextDB 取文本后仍有 C++ 生成的数值、属性、标题和菜单。 | P0 |
| 技能菜单 | `skills.cc`, `skill-menu.cc` | 技能名、称号、训练开关、等级和资质。区分旧兼容技能。 | P0 |
| 法术与能力 | `spl-cast.cc`, `ability.cc`, `god-abil.cc` | 法术/能力菜单、失败原因、成本、确认和效果消息。 | P0 |
| 近战与远程战斗 | `attack.cc`, `melee-attack.cc`, `ranged-attack.cc`, `beam.cc` | 完整句子、mprf 参数、名称/代词及 conj_verb 等英文语法组合。保留英文生成路径。 | P0 |
| 神祇系统 | `religion.cc`, `god-menu.cc`, `god-passive.cc`, `god-wrath.cc` | 神名、称号、虔诚、神罚和能力提示；TextDB 只是其中一部分。 | P1 |
| 怪物与名称 | `monster.cc`, `mon-util.cc`, `mon-info.cc` | 可见性、匿名名称、种类/状态/所有格和复数，禁止泄露未识别信息。 | P0 |
| 名称和静态表 | `mon-data.h`, `spl-data.h`, `item-prop.cc`, `feature-data.h` | 怪物/法术/物品/地形的短名与属性；独立于 TextDB 长说明。 | P0 |
| 英文语法与既有语言钩子 | `english.cc`, `lang-fake.cc` | 冠词、复数、代词及现有趣味语言转换；先评估再接入，不把趣味转换当作完整中文国际化。 | P0 |
| 变异与变形 | `mutation.cc`, `mutation-data.h`, `transform.cc` | 突变等级、变形限制、状态和装备融合消息。 | P1 |
| 消息和菜单基础层 | `message.cc`, `menu.cc`, `format.cc`, `stringutil.cc` | 通道、颜色/格式标记、日志历史、截断与格式化；不在此对所有最终英文正则替换。 | P0 |
| 结束、存档与记录 | `hiscores.cc`, `chardump.cc`, `files.cc` | 死亡原因、保存/读取错误、morgue 与排名；机器字段和文件格式不直接翻译。 | P1 |
| 教程与帮助补充 | `hints.cc`, `command.cc` | 按键提示和动态教学；与 TextDB 教程共同审查。 | P1 |
| Tiles 字体与窗口 | `tilefont.cc`, `tilesdl.cc`, `tilereg-text.cc`, `menu.cc` | 字体、测量、换行、窗口和菜单布局；属于显示适配而非词条翻译。 | P0 |

后续抽取须同时检查 `mpr/mprf`、格式化函数、菜单构造、`+`/`+=`/流拼接、名称辅助函数及错误输出。某处使用 `mprf` 不等于已经具备国际化。

## 3. 结构化定义、Lua 和地图脚本

| 来源 | 文本及风险 | 处理方式 |
| --- | --- | --- |
| `source/dat/species/*.yaml`、`dat/jobs/*.yaml` | 种族/背景名称、形容词和称谓；包含 deprecated 历史定义 | 本地化显示名与内部 enum/配置键分离；已在术语校对中对应现行名称 |
| `source/dat/forms/*.yaml`、`dat/mons/` | 形态和怪物结构化定义 | 先识别显示字段，禁止翻译程序引用或生成器标识 |
| `source/dat/descript/*.txt` 内 `{{ ... }}` | 根据种族、神祇等生成文本的内嵌 Lua | 翻译返回文字，保留判断字面量、Lua 语法和变量；单列结构审查 |
| `source/dat/clua/` | autofight 等客户端辅助脚本的玩家提示 | `crawl.mpr` 等输出是候选，调试日志与程序键排除 |
| `source/dat/dlua/` | 地图/教程/门户逻辑及其消息 | 提示文本进入目录；调试消息、函数名和地图标签保持原样 |
| `source/dat/des/**/*.des` | 关卡、门户和 Sprint 的嵌入 Lua、欢迎文字和特殊名称 | 保留 NAME/TAGS/地图符号/marker 等结构；按地图功能验证 |
| `source/dat/defaults/` | 默认配置及说明性文本 | 选项名和枚举不改；面向玩家的说明可单独翻译 |
| YAML 对应的生成头文件及 `util/*-gen.py` | 部分名称和表由生成器输出 | 修改上游定义或显示适配层，不把生成头文件当作人工译文来源重复维护 |

## 4. 帮助文档、设置与 WebTiles

| 来源 | 首期范围 | 注意事项 |
| --- | --- | --- |
| `crawl-ref/docs/crawl_manual.rst`、派生 `.txt/.html`、`options_guide.txt` | 常用帮助可后续分批接入 | 明确生成源；不同时手改多个生成格式；保留按键和选项 |
| `crawl-ref/settings/` | 示例配置说明，低于核心游玩优先级 | 配置键、命令和脚本匹配规则保持英文兼容 |
| `source/webserver/templates/`、`game_data/templates/`、`static/scripts/`、`webtiles/` | WebTiles 不在首期交付 | 浏览器登录/大厅/客户端提示另建组件，第三方 JS 不进入游戏翻译目录 |
| `source/tileweb*.cc` | 后续 WebTiles 引擎桥接 | 网络协议键和前端显示文本分开，不能在序列化层翻译键名 |
| `source/contrib/`、`docs/license/` | 不作为常规游戏汉化对象 | 第三方源码、许可原文和程序日志保留；字体许可随包携带 |
| `source/rltiles/`、`source/dat/tiles/`、`contrib/fonts/` | 图片和字体资产 | 图片文件名/图块索引不翻译；如发现图片内文字另建资产任务，CJK 字体另做许可与排版验证 |

## 5. 后续抽取记录字段

每个候选至少记录：组件、文件和行号、原文或原始键、正文哈希、调用/生成方式、参数契约、格式标记、语言回退、玩家可见性、优先级、接入状态。行号只是定位信息，不能当永久翻译标识。

优先级：P0 表示进入最小双语样例或影响机制/安全提示；P1 表示后续内容覆盖。它是实施顺序，不是“译文已合格”。

完整文件和键差异、来源指纹见 [阶段 0 清单快照](../../localization/manifests/phase-0-text-audit.json)；旧中文逐文件审查见 [旧译文审查清单](legacy-translation-review-checklist.md)。
