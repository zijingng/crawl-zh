# x 查看地形的显示接入（2026-10-25）

`direction_chooser::print_floor_description` 原先直接打印 `cell_floor_description` 的规范英文结果，未接入已有 `try_feature_display_at` 显示接口。鼠标方格提示 `get_square_desc` 也使用了同一英文结果。

本轮在两个 UI 调用点接入已有地形名称、门状态和覆盖物译文。`x` 的短描述使用现有 `%s.` → `%s。` 模板；方格提示直接使用无标点的显示名。沿用已有“岩壁”“石墙”“关闭的门”“地面”等目录，不新增重复译文。术语查询见 [记录](look-feature-01-preflight.json)。

`cell_floor_description`、`feature_description_at` 与地形查询键仍使用原英文；不解析英文冠词，不改变地形、视野或搜索行为。未知自定义地形在方格提示中连同 `Here:` 前缀整行回退英文，避免中文前缀与英文名称混杂。

原描述已经求值过的 Lua 标记属性不会为了翻译再调用一次；这类不透明自定义描述保留完整英文。已有独立的地形长说明显示逻辑不变。

回归覆盖实际 `get_square_desc`、岩壁/石墙/门/楼梯、普通地面过滤、规范英文名称不变、英文切换、未知标记整行回退和随机状态不变。源文件依赖复核见 [记录](../../localization/manifests/look-feature-01.json)。

`crawl.exe` 已重新编译。128 项本地化测试通过（192,327 个断言），`git diff --check` 通过。
