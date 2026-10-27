# 铭文、冒险者与 HUD 状态用词统一

- 物品 inscription 统一译为“铭文”，操作 inscribe 使用“添加铭文”。同步帮助目录、物品菜单、无可用物品提示、输入询问、手册目录、正文及附录。自动铭文和安全铭文沿用同一名称。
- 排行榜标题中的“地下城探险者”改为“地下城冒险者”。
- `-Scroll` 的显示简称由“禁卷”改为“禁读”，同步已有的即将结束形式。
- HUD `Will/2` 补为“意志减半”，与术语表中的 `Will/2 status` 一致。原状态键及机制保持不变。

“铭刻武器”对应 Imprint Weapon，是将武器构造赋予白金典范的独立能力；铭刻束缚符印也是施法动作。这些不属于物品 inscription 功能，保留原有语境用词。

当前 [术语表](../../localization/glossary.csv) 保持 Paratranz 五列结构，补充 inscription、inscribe、-Scroll、Will/2、Crawler、Best Crawlers 六条术语。原有 2,824 条术语逐字段保持不变。CSV 通过捆绑的 Artifact Tool 修改，并重新读取核对，没有生成额外工作簿。

`ui.po` 是手册译文的维护来源；构建时重新导出 `crawl-ref/docs/zh/crawl_manual.txt`。本批只更改中文译文和术语数据，没有修改英文原文、手册按键示例、rc 配置键或玩家自行填写的铭文。

验证包含中文手册章节跳转、Tiles UTF-8 逐行显示、HUD 实际状态入口 `DUR_LOWERED_WL`／`STATUS_NO_SCROLL`、颜色、规范状态值、英文回退与 RNG。不改写既有 morgue 文件。结果见 [检查记录](wording-01-checks.json)。

Windows Tiles `crawl.exe` 编译通过；5 项原生测试（245 条断言）、33 项 Python 检查及 `git diff --check` 通过。
