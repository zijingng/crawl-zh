# Lua 显示文本与未鉴定鞋靴外观（2026-10-25）

本轮按 README 的英文回退方针处理 Lua 显示端，保留 `crawl.mpr`、英文名称接口、搜索匹配、配置键和序列化内容。译前术语查询记录见 [lua-01-preflight.json](lua-01-preflight.json)；采用靴子、手套、魔力值、法术、秘藏、虔诚、放逐术影响等现有术语。

## 发现刺绣靴子

`Found %s.` 原本已有译文。漏译来自外观目录键的词序：旧目录为 `embroidered pair of boots`，实际名称为 `pair of embroidered boots`。本轮补齐靴子、手套在刻符文、发光、刺绣三类可见外观下的实际英文键及受诅咒变体，共 12 条。没有改变原物品名称生成，也不会泄露未鉴定物品的强化值或印记。

实际日志为“发现刺绣的靴子。”；切换英文仍生成 `Found a pair of embroidered boots.`。

## Lua 显示接口

- `crawl.ui_translate(context, source)`：显式翻译完整静态原文，未收录时原样返回。
- `crawl.ui_name(context, canonical_key)`：从规范名称键取得显示组件，缺译或英文模式返回 `nil`。
- `crawl.ui_format(context, source, original, ...)`：只接受 `%s`、`%%` 的显示模板。模板、组件或参数数量不完整时返回调用点保存的整句英文；插入值不会被再次解释为格式串。
- `crawl.ui_feature_at(x, y)`：从地形坐标获取显示名，仅注册在地下城 Lua 中。
- `crawl.ui_bane_name`、`crawl.ui_bane_description`：诅咒的独立显示接口。描述同时要求正文和持续时间译文完整。原 `crawl.bane_description` 现在明确使用英文生成路径，作为回退原文。

自动战斗、自动施法、秘藏的物品要求/提示/入口说明/概览、计时传送门消失及声音提示、runrest 配置错误、巫师伤害统计均已接入。伤害统计的任一可选列缺译会使整行回退英文。

秘藏的中文物品要求直接读取原始请求表的类别、子类、数量与强化值，另建显示名，不解析 `self:item_name()` 的英文结果。原有物品匹配、消耗、英文笔记及保存逻辑保留。

## 地图消息与覆盖边界

`lm_fog.lua`、`lm_monst.lua`、`lm_door.lua`、`lm_trig.lua`、`tutorial.lua` 的地图消息出口支持显式的 `lua.map` 原文目录。计时消息和消失消息通过 `util.display_message` 在展开 `$F` 前查找原始模板；实体名称缺译或模板未知时整句使用原英文。没有改写通用 `print`、`crawl.message` 或 `crawl.mpr`。

已审校十种计时入口的最后提示及声音构造所需的节奏、距离、声源组件。其余 `.des` 文件提供的剧情文本、任意用户回调返回的文本没有被自动翻译；未经审校的原文继续完整显示英文，不能算作汉化完成。

[逐文件清单](lua-01-inventory.json) 覆盖 `crawl-ref/source` 下全部 109 个项目自有 `.lua` 文件。第三方 `contrib` 不纳入游戏文本翻译。搜索注记、生成器参数、测试断言、机器输出和内部开发诊断仍保留规范英文；这些不能作为玩家提示直接替换。已有击杀清单继续使用 `kills.display_*` 接口。

## 上游变更与验证

[本轮目录清单](../../localization/manifests/lua-01.json) 记录 116 条新审校条目及源文件依赖。主运行时清单仅刷新本轮重新审核的依赖；英文模板或相关源文件变化时，未重新审校的译文不会继续进入生成目录。

验证覆盖实际 Lua 调用点，包含未知法术、未知地图实体、缺失模板/参数、英文切换、规范物品名称、伤害统计整行回退，以及未鉴定鞋靴不泄露强化值。另对 109 个 Lua 文件使用游戏所带 Lua 执行语法检查，并运行本地化 C++ 测试和 Python 工具测试。没有以图形方式逐张进入地图测试。

Windows Tiles 的 `crawl.exe` 已重新生成。118 项本地化测试通过（191,871 个断言），28 项 Python 工具测试通过，109 个 Lua 文件语法检查通过。
