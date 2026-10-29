# WebTiles 怪物栏与物品动作按键

怪物栏原先始终读取 `name`／`plural`，忽略游戏已有的 `name_display`。现复用 `options.display`：中文模式显示已有怪物译名，分组显示“数量 × 中文名”；英文模式、缺少译名或旧服务端缺字段时，保留原英文单复数。排序、分组和类型判断继续使用原规范名称。

物品描述页的 `clickify_actions` 原先只按英文逗号加空格分割；中文列表使用逗号“，”，整串因此被包进第一个动作的点击区域。现兼容中英文逗号，并保留原分隔符；“或”与中文句号在动作链接之外，每个动作独立绑定自己的 `data-hotkey`。放下武器、丢弃、调整字母、训练目标、铭文及其他共用动作列表的按键不改写。

仅修改 `game_data/static/monster_list.js`、`game_data/static/ui-layouts.js`，无需改 C++ 或协议，不需重新编译游戏程序。部署时须同步到 WebTiles 实际使用的游戏静态目录，再强制刷新浏览器缓存；只替换 crawl 游戏程序不会更新这两份前端脚本。

`node tools/localization/test_webtiles_monster_actions.js` 通过，直接执行生产函数，检查中文列表的 u/d/=/s/i 五个独立热键、英文和混合分隔符、e(v)oke 等标记形式、显示文本保持不变、怪物单只与分组显示、英文切换和旧字段回退。两份修改后的 JS 语法检查及 `git diff --check` 通过。未连接实际 WebTiles 服务器做浏览器端测试。
