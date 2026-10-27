# Tiles 悬停显示与状态术语修正

本批处理反馈第 7–11 项。翻译前查询记录见 `tiles-hover-01-preflight.json`；指定术语修改、完整提示条目及源码依赖复核见 `../../localization/manifests/tiles-hover-01.json`。

- 施法栏顶部的 MP 改为“魔力”，例如“1 魔力    召唤小型哺乳动物”。
- 施法、能力的鼠标提示先组装规范英文，再按完整提示查询译文，最后调用原有 `insert_commands` 插入当前快捷键。覆盖可用和不可用两种状态；缺少完整提示译文时整段回退英文。
- 怪物栏顶部的悬停名称使用现有 `monster_info` 显示名称适配。缺译、不可适配的自定义名字以及英文模式继续使用原有 `proper_name(DESC_A)`；不改变怪物规范名称。
- `Clear terrain to floor` 统一为“清除地形为地面”。
- `-Breath status` 保留“吐息冷却”；其余为“吐息”“吐息+”“吐息++”。
- Destr 系列统一为“毁灭增长”“毁灭增长+”“毁灭增长++”“毁灭增长+++”。

同步修改术语表 Translation 和 Note 中的界面名称。查询界面实际使用小写规范键（例如 `breath status`），译文按该键维护；英文键、原始英文模板与游戏逻辑保留。

新增本地化测试检查完整鼠标提示、快捷键占位符、未知提示整段英文回退、MP 行与术语显示，以及切回英文。构建与测试日志保存在工作区 `.term-audit/tiles-hover-01-build.log` 和 `tiles-hover-01-tests.log`。尚未进行实际窗口中的鼠标悬停验证。

验证结果：Windows Tiles `crawl.exe` 编译成功；135 个本地化测试全部通过，共 253,341 条断言。
