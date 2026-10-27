# 球蛛蓄力提示与抗性简写现状（2026-10-25）

## 球蛛蓄力

报告原文：`The orb spider begins to weave its threads into a brightly glowing ball.`

该句来自 `dat/database/monspeak.txt` 的 `orb spider charge`，同一条目有三种随机候选。本轮逐句审校：

| 原文 | 中文 |
| --- | --- |
| `@The_monster@ spins a strand of pure energy.` | `@The_monster@纺出一缕纯粹的能量。` |
| `@The_monster@ pulsates with a strange energy.` | `@The_monster@涌动着奇异的能量。` |
| `@The_monster@ begins to weave its threads into a brightly glowing ball.` | `@The_monster@开始将丝线编织成一个明亮发光的球体。` |

术语表的 `orb spider` 为“球蛛”。译前命中和术语表哈希见 [查询记录](orb-charge-01-preflight.json)。

`_spell_charged` 改用已有配对语音接口，同一次英文加权选择决定中英文候选。原英文只执行一次 `do_mon_str_replacements`；中文使用原怪物对象替换原始模板标记，不再查找渲染完成的英文句子。可见性、蓄力状态和友方/敌方频道保留。缺译、未知标记或英文模式均使用完整原句。

`fix.charge` 目录绑定整个英文候选条目的内容与相关代码哈希；上游条目变化后不会重新使用旧语音数据库中的同名译文。[目录及依赖记录](../../localization/manifests/orb-charge-01.json)只刷新本轮重新审核过的共享依赖。

## 抗性简写现状

| 规范键 | 角色抗性面板 | 术语表 |
| --- | --- | --- |
| `rF`（面板键 `rFire`） | 火焰抗性 | `rF`：火抗；完整名称 `fire resistance`：火焰抗性 |
| `rC`（面板键 `rCold`） | 寒冷抗性 | `rC`：冰抗；完整名称 `cold resistance`：寒冷抗性 |
| `rCorr` | 腐蚀抗性 | `rCorr`：腐蚀抗性 |
| `rNeg`（物品键 `rN`） | 负能抗性 | 没有独立 `rNeg` 条目；已有 `rN` 与 `negative energy resistance`：负能量抗性 |

物品紧凑属性仍保留 `rF`、`rC`、`rN`、`rCorr` 以及原有 `+/-` 等级符号。此处记录当前实现，不擅自统一术语或改变缩写显示；本轮没有修改术语表。

回归测试从实际英文数据文件读取该条目，跨 32 个固定随机种子验证同一次选择及相同后续随机状态，覆盖全部三种中文候选、英文模式、组件不完整以及上游原文变化后的整句英文回退。

`crawl.exe` 已重新编译。120 项本地化测试通过，共 192,126 个断言；`git diff --check` 通过。
