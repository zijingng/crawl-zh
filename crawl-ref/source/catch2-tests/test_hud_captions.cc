#include "AppHdr.h"
#include "catch_amalgamated.hpp"
#include "i18n.h"
#include "options.h"
#include "unwind.h"
#include "unicode.h"

TEST_CASE("HUD full Chinese captions retain English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const struct { const char *context; const char *source; const char *display; } labels[] = {
        {"o.label", "AC:", "护甲:"},
        {"o.label", "EV:", "闪避:"},
        {"o.label", "SH:", "格挡:"},
        {"o.label", "XL: ", "等级: "},
        {"phase3.output", "Str:", "力量: "},
        {"phase3.output", "Int:", "智力: "},
        {"phase3.output", "Dex:", "敏捷: "},
    };
    for (const auto &label : labels)
    {
        Options.language = lang_t::ZH;
        CHECK(string(ui_translate(label.context, label.source)) == label.display);
        CHECK(strwidth(label.display) <= 6);
        Options.language = lang_t::EN;
        CHECK(string(ui_translate(label.context, label.source)) == label.source);
    }
}
