# 怪物跳跃提示漏译（2026-10-25）

报告原文：`The boulder beetle leaps!`

`monster::blink_to` 在 `teleport.cc` 中先选取 `leap`、`hop` 或 `blink`，再调用原有 `conj_verb` 和 `simple_monster_message`。已有显示端会用完整的 `log.monster` 事件模板和原始怪物对象生成译文；本次漏译是目录未收录这三种由动词变形生成的模板。

查阅术语表后采用“巨石甲虫”“闪烁蛙”及位移语境的“闪烁”。[术语查询记录](teleport-fix-01-preflight.json)保存当前术语表哈希与命中条目。

补入三条独立目录项：

| 原文模板 | 中文模板 |
| --- | --- |
| `%s leaps!` | `%s跃起！` |
| `%s hops!` | `%s蹦跳起来！` |
| `%s blinks!` | `%s闪烁了一下！` |

无需修改游戏逻辑或英文构造。原有可见性、友方消息频道、英文冠词与动词变形保留；名称或模板缺译时整句回退英文。没有对渲染完成的英文进行替换。

本轮[译文清单](../../localization/manifests/teleport-fix-01.json)记录模板和相关源文件哈希。回归测试使用实际巨石甲虫、闪烁蛙和兽人对象检查中文、英文、名称缺译回退、规范名称不变和随机状态不变。
