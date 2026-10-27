# Windows Tiles 最小双语样例

基线：`stone_soup-0.34.1-zh`，官方 `0.34.1`。本次实现并验证一个局部双语样例，用于继续扩展汉化接口和中文显示。

## 1. 直接运行

在 MSYS2 **MINGW64** 终端，从仓库根目录执行：

```bash
pacman -S --needed gettext
cd crawl-ref/source
make -j4 TILES=y
./crawl.exe -rc ../settings/zh-local-tiles.rc
```

`gettext` 只在构建和翻译目录维护时使用；本机已具备 `msgfmt` 和 `xgettext`。游戏运行时不需要 gettext DLL、Python 或翻译服务。

启动后创建或读取角色，按 **F1 或 `~`** 打开游戏菜单，Esc 返回。可以从背包 `i` 中选择物品，观察已有中文说明和英文属性的混排。

切回英文并重新启动：

```bash
./crawl.exe -rc ../settings/zh-local-tiles.rc -extra-opt-last 'language = en'
```

此命令保留样例中显式选择的等宽主字体与微软雅黑后备字体；要观察原默认字体，使用自己的英文配置或去掉五组主字体和后备字体覆盖。更改语言或字体后重新启动，首期不承诺运行中热切换。

样例配置不指定角色名、背景、自动按键或存档目录；验证时使用的自动打开菜单和背包的 Lua 钩子仅存在于独立临时测试配置中。

## 2. 复用的原有配置

语言仍为上游 `language`：

```text
language = zh
```

原解析器同样接受 `zh_CN`、`zh-CN`；本次没有新增语言选项。`language = en` 保留英文，其他语言不会套用中文 UI 目录。TextDB 仍使用上游的 `zh` 子目录。

保留上游五组主字体与字号配置，各新增可选的 `tile_font_*_fallback_file`：

```text
tile_font_crt_file = C:/Windows/Fonts/consola.ttf
tile_font_msg_file = C:/Windows/Fonts/consola.ttf
tile_font_stat_file = C:/Windows/Fonts/consola.ttf
tile_font_tip_file = C:/Windows/Fonts/consola.ttf
tile_font_lbl_file = C:/Windows/Fonts/consola.ttf
tile_font_crt_fallback_file = C:/Windows/Fonts/msyh.ttc
tile_font_msg_fallback_file = C:/Windows/Fonts/msyh.ttc
tile_font_stat_fallback_file = C:/Windows/Fonts/msyh.ttc
tile_font_tip_fallback_file = C:/Windows/Fonts/msyh.ttc
tile_font_lbl_fallback_file = C:/Windows/Fonts/msyh.ttc
```

字号沿用各自的 `tile_font_*_size`，样例设为 18。可以换成自己安装的 CJK 字体文件；终端网格区域优先使用拉丁半宽、汉字双宽的字体。比例字体的像素文字与终端网格是不同路径，不能只凭某个字符能显示就认定所有布局都适合。

样例使用本机的 Consolas 作为等宽主字体，缺失字形由本机微软雅黑提供。五组后备字体可分别配置，留空时保持原行为；两个字体都缺字时仍使用主字体的缺字标记。后备字体沿用该组字号，共用基线，行高容纳两个字体；后备字形按 Unicode 显示列数推进，汉字占两格。字体缓存包含后备字体路径，DPI/字号变化会同时刷新两个字体。没有复制或分发 Windows 字体，也不自动搜寻系统字体；路径需按本机调整。

## 3. 本次接入范围

新 UI PO 目录包含 11 个上下文条目：

- 游戏菜单的返回、两种保存项、角色记录、图块编辑、宏编辑、帮助和查询，共 8 项。
- 启动后的命令帮助提示，保留 `<w>?</w>` 标记。
- 普通升级消息，保留 `%d` 等级参数；最高等级等其他句子仍沿用英文。
- 恐惧导致攻击退缩的消息，保留原目标名参数 `%s`、随机判断和战斗逻辑。目标名称仍走上游可见性与英文命名路径，尚未接入中文实体名称系统。

游戏菜单中尚未接入的退出项仍显示英文，作为局部覆盖/回退的实际例子。角色创建、HUD、物品名、机制属性说明和大部分战斗消息仍可能是英文。

现有 TextDB 的中文描述继续通过原查询机制加载。本次用连枷和鳞甲说明观察字形、长段落换行及数值混排，没有把旧译文全量标记为已复核，也没有实现 TextDB 到 PO 的全量导入导出。

## 4. 目录、生成与回退

```text
localization/templates/ui.pot         从接入调用点抽取的英文模板
localization/zh_CN/ui.po              人工编辑的中文 UI 译文
localization/manifests/ui-sample.json  样例消息来源和审核状态
tools/localization/build_catalogue.py 构建时校验与生成
crawl-ref/source/i18n.h/.cc           薄查询接口
crawl-ref/source/i18n-catalogue.h     自动生成，已忽略，不人工编辑
```

`msgfmt --check --check-format` 解析 PO、排除 fuzzy 条目并检查已标记的格式参数；生成器另核对 printf 规格及颜色标签，防止编辑器删除格式标记后绕过检查。Python 使用标准 `gettext.GNUTranslations` 读取 MO，没有自行解析 PO 语法。

只有清单中标为 `reviewed` 且 PO 有可用译文的条目进入只读 C++ 表。查询同时匹配上下文和完整英文原文：上游改了句子但没有重新审核时，旧译文不会命中。未接入、缺译、fuzzy 和未复核条目走英文；结构错误的译文阻止构建，要求修复后再发布。

译文先选定，再由原 `mprf` 格式化，消息通道、参数、按键和业务逻辑保持原行为。没有对最终输出的英文句子做全局替换。

重新抽取模板，从 `crawl-ref/source` 执行：

```bash
xgettext --language=C++ --keyword=ui_translate:1c,2 --from-code=UTF-8 \
  --package-name=DCSS --package-version=0.34.1 \
  --output=../../localization/templates/ui.pot main.cc player.cc melee-attack.cc
msgmerge --update ../../localization/zh_CN/ui.po ../../localization/templates/ui.pot
```

用 Poedit 或 VS Code 复核修改项，保留参数和颜色标记。新增/变化消息还要更新 `ui-sample.json` 的来源和审核状态；不要为了消除 fuzzy 提示直接批准未复核内容。之后 `make TILES=y` 自动重新生成并编译目录。

## 5. 中文显示修复

- `TextRegion` 按 `wcwidth` 写终端列，汉字占两列，第二列为 continuation；覆盖或擦除任意半格时清理旧字。
- 右边缘放不下完整汉字时不绘制半个字；换行、光标和连续长字符串按显示列计数。
- FreeType 终端网格的单格宽度与字体的最宽字形分离；CJK 字体用拉丁 `M` 的 advance 定义单格，汉字字形纹理仍保留其完整宽度。像素字符串仍使用原字体的逐字 advance。
- 图集由 256 扩到 1024 个槽，并修复大于 255 的槽位在 LRU 查询时被截成 8 位的问题。
- 字号/DPI 重配置时清理图集 LRU 状态；glyph 度量使用整型，避免高字号下 8 位溢出。

这里不是完整 Unicode 排版引擎。终端网格尚未实现组合字符/emoji 字素聚合，零宽字符不额外占列；超大量独特字形、其他字体和不同 DPI 仍应继续专项验证。

## 6. 验证记录

2026-10-20，本机 Windows、MINGW64：

- Windows Tiles 编译成功。
- Catch2：2 个定向用例、41 项断言通过，包括现有语言配置、缺译/原文变化回退、英文及其他语言、数值与目标名格式化、汉字双列、覆盖半字、擦除、右边界、换行和超过 1024 字符的输入。
- Python：7 项目录测试通过，包括已审、未审、缺译、fuzzy、英文来源变化、参数类型与颜色标签校验。
- 实际窗口：中文游戏菜单的标签和原快捷键可见；未覆盖的退出项保持英文。中文启动消息、连枷说明、鳞甲连续中文段落的换行以及英文数值/属性混排已观察。
- 实际窗口：英文配置与原默认字体启动成功，启动提示和目标说明保持英文。

测试命令，从 `crawl-ref/source` 执行：

```bash
make -j4 TILES=y catch2-tests-executable
./catch2-tests-executable.exe '[localization]'
python3 ../../tools/localization/test_catalogue.py
```

测试对象使用 Catch2 所需的 C++14，游戏对象保留 C++11。原 `make catch2-tests` 的全量/覆盖率目标仍可使用。

尚未通过实际游玩触发升级和恐惧退缩消息；其模板格式和语言选择已做自动验证。没有声称通过全部字体、缩放、长消息、保存/读档语言切换或其他平台的回归测试。

后备字体新增验证：Windows Tiles 重新编译与现有 localization 回归测试；实际混排外观仍需在目标显示器上检查。

阶段 3 已扩展此样例，当前覆盖以 [阶段 3 记录](phase-3-static-ui.md) 为准。上面的 11 条消息和早期窗口验证仅描述阶段 1 样例。
