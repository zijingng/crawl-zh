#include "AppHdr.h"

#include "i18n.h"

#include <cstring>

#include "i18n-catalogue.h"
#include "options.h"
#include "actor.h"
#include "item-name.h"
#include "item-def.h"
#include "mon-info.h"
#include "player.h"
#include "stringutil.h"

ui_arg ui_generated_name_arg(const string &canonical)
{
    if (Options.language != lang_t::ZH || canonical.empty() || canonical.size() > 96)
        return {canonical, false};
    string lower = canonical;
    for (char &c : lower)
    {
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        if (!(c >= 'a' && c <= 'z') && c != ' ' && c != '-' && c != '\'')
            return {canonical, false};
    }
    if (lower.front() < 'a' || lower.front() > 'z'
        || lower.back() < 'a' || lower.back() > 'z')
        return {canonical, false};

    // Approximate spelling-to-sound rules for invented names, not a dictionary
    // of English words. Longest digraphs win; every input byte is consumed.
    static const struct { const char *letters; int vowel; const char *alone; } vowels[] = {
        {"ae",1,"埃"},{"ai",1,"艾"},{"ay",1,"艾"},{"au",3,"奥"},
        {"aw",3,"奥"},{"ea",2,"伊"},{"ee",2,"伊"},{"ei",1,"埃"},
        {"ey",1,"埃"},{"ie",2,"伊"},{"oa",3,"欧"},{"oe",3,"欧"},
        {"oi",3,"奥伊"},{"oo",4,"乌"},{"ou",3,"欧"},{"ow",3,"奥"},
        {"ue",4,"乌"},{"ui",4,"乌伊"},
        {"a",0,"阿"},{"e",1,"埃"},{"i",2,"伊"},{"o",3,"奥"},{"u",4,"乌"}
    };
    static const struct {
        const char *letters; const char *final; const char *syllables[5];
    } consonants[] = {
        {"ch","奇",{"查","切","奇","乔","楚"}},
        {"sh","什",{"沙","舍","希","绍","舒"}},
        {"th","斯",{"萨","瑟","西","索","苏"}},
        {"ph","夫",{"法","费","菲","福","夫"}},
        {"kh","赫",{"哈","赫","希","霍","胡"}},
        {"gh","格",{"加","格","吉","戈","古"}},
        {"qu","克",{"夸","奎","奎","阔","库"}},
        {"ck","克",{"卡","凯","基","科","库"}},
        {"ng","恩格",{"恩加","恩格","恩吉","恩戈","恩古"}},
        {"b","布",{"巴","贝","比","波","布"}},
        {"c","克",{"卡","塞","西","科","库"}},
        {"d","德",{"达","德","迪","多","杜"}},
        {"f","夫",{"法","费","菲","福","富"}},
        {"g","格",{"加","格","吉","戈","古"}},
        {"h","赫",{"哈","赫","希","霍","胡"}},
        {"j","吉",{"贾","杰","吉","乔","朱"}},
        {"k","克",{"卡","凯","基","科","库"}},
        {"l","尔",{"拉","勒","利","洛","卢"}},
        {"m","姆",{"马","梅","米","莫","穆"}},
        {"n","恩",{"纳","内","尼","诺","努"}},
        {"p","普",{"帕","佩","皮","珀","普"}},
        {"q","克",{"卡","凯","基","科","库"}},
        {"r","尔",{"拉","雷","里","罗","鲁"}},
        {"s","斯",{"萨","瑟","西","索","苏"}},
        {"t","特",{"塔","特","蒂","托","图"}},
        {"v","夫",{"瓦","维","维","沃","武"}},
        {"w","乌",{"瓦","韦","维","沃","乌"}},
        {"x","克斯",{"克萨","克塞","克西","克索","克苏"}},
        {"y","伊",{"亚","耶","伊","约","尤"}},
        {"z","兹",{"扎","泽","齐","佐","祖"}}
    };
    string shown;
    size_t pos = 0;
    int words = 1;
    while (pos < lower.size())
    {
        if (lower[pos] == ' ' || lower[pos] == '-' || lower[pos] == '\'')
        {
            if (++words > 8 || pos + 1 == lower.size()
                || lower[pos + 1] < 'a' || lower[pos + 1] > 'z')
                return {canonical, false};
            shown += "·"; ++pos; continue;
        }
        bool matched = false;
        for (const auto &c : consonants)
        {
            const size_t length = strlen(c.letters);
            if (lower.compare(pos, length, c.letters)) continue;
            pos += length;
            // Doubled consonants in invented words represent one sound.
            if (length == 1 && pos < lower.size() && lower[pos] == c.letters[0])
                ++pos;
            const char *syllable = c.final;
            for (const auto &v : vowels)
                if (!lower.compare(pos, strlen(v.letters), v.letters))
                {
                    syllable = c.syllables[v.vowel];
                    pos += strlen(v.letters); break;
                }
            shown += syllable; matched = true; break;
        }
        if (matched) continue;
        for (const auto &v : vowels)
            if (!lower.compare(pos, strlen(v.letters), v.letters))
            {
                shown += v.alone; pos += strlen(v.letters); matched = true; break;
            }
        if (!matched) return {canonical, false};
    }
    return {shown, true};
}

static const char *find_ui_translation(const char *context, const char *english)
{
    if (Options.language == lang_t::ZH)
        for (const auto &entry : zh_ui_catalogue)
            if (!strcmp(context, entry.context)
                && !strcmp(english, entry.english))
                return entry.chinese;

    return nullptr;
}

const char *ui_translate(const char *context, const char *english)
{
    const char *translated = find_ui_translation(context, english);
    // Matching the English source as well as the context makes a changed
    // upstream sentence fall back until its translation has been reviewed.
    return translated ? translated : english;
}

ui_translation_group::ui_translation_group(std::initializer_list<ui_message> required)
    : m_available(Options.language == lang_t::ZH)
{
    for (const auto &message : required)
        require(message.context, message.english);
}

void ui_translation_group::require(const char *context, const char *english)
{
    m_available = m_available && find_ui_translation(context, english);
}

const char *ui_translation_group::get(const char *context, const char *english) const
{
    return m_available ? ui_translate(context, english) : english;
}

bool try_ui_actor_name(const actor &who, string &output)
{
    ui_translation_group group;
    if (!group.available())
        return false;
    if (who.is_player() || !you.can_see(who))
    {
        const char *source = who.is_player() ? "you" : "something";
        group.require("log.actor", source);
        if (!group.available())
            return false;
        output = group.get("log.actor", source);
        return true;
    }
    // Names do not need threat/XP estimates or other full-info calculations.
    return monster_info(who.as_monster(), MILEV_NAME).try_display_name(output);
}

bool try_ui_item_log(const char *english, const item_def &item,
                     string &output, int quantity)
{
    ui_translation_group group({{"log.item", english}});
    if (!group.available())
        return false;
    item_def shown = item;
    if (quantity >= 0)
        shown.quantity = quantity;
    string name;
    if (!try_item_display_name(shown, name, false, false))
        return false;
    output = make_stringf(group.get("log.item", english), name.c_str());
    return true;
}

bool try_ui_actor_log(const char *context, const char *english,
                      const actor &who, string &output)
{
    ui_translation_group group({{context, english}});
    string name;
    if (!group.available() || !try_ui_actor_name(who, name))
        return false;
    output = make_stringf(group.get(context, english), name.c_str());
    return true;
}

bool try_ui_projectile_name(const string &english, string &output)
{
    for (const char *context : {"log.projectile.name", "display.spell"})
    {
        ui_translation_group group({{context, english.c_str()}});
        if (group.available())
        {
            output = group.get(context, english.c_str());
            return true;
        }
    }
    return false;
}

ui_arg ui_actor_arg(const actor &who)
{
    ui_arg result;
    result.available = try_ui_actor_name(who, result.text);
    return result;
}

ui_arg ui_knockback_cause_arg(const string &canonical)
{
    if (canonical.empty()) return ui_literal_arg("");
    const ui_arg exact = ui_text_arg("a.knockback.cause", canonical);
    if (exact.available) return exact;
    ui_arg noun = ui_text_arg("a.knockback.source", canonical);
    if (!noun.available)
        noun.available = try_ui_projectile_name(canonical, noun.text);
    const string shown = ui_render("a.actor.cause", " by the %s", "", {noun});
    return {shown, !shown.empty()};
}

ui_arg ui_corrode_source_arg(const actor *source, const string &canonical)
{
    for (const char *context : {"o.corrode.source", "m.clause"})
    {
        const ui_arg fixed = ui_text_arg(context, canonical);
        if (fixed.available) return fixed;
    }
    if (source)
    {
        const string name = source->name(DESC_THE);
        if (canonical == name || canonical == uppercase_first(name))
            return ui_actor_arg(*source);
    }
    return {"", false};
}

ui_arg ui_item_arg(const item_def &item)
{
    ui_arg result;
    result.available = Options.language == lang_t::ZH
        && try_item_display_name(item, result.text, false, false);
    return result;
}

ui_arg ui_target_arg(const actor &who, const actor &subject)
{
    return &who == &subject ? ui_text_arg("a.noun", "itself") : ui_actor_arg(who);
}

ui_arg ui_text_arg(const char *context, const string &canonical)
{
    ui_translation_group group({{context, canonical.c_str()}});
    return {group.get(context, canonical.c_str()), group.available()};
}

ui_arg ui_literal_arg(const string &text)
{
    return {text, true};
}

string ui_render(const char *context, const char *source, const string &original,
                 initializer_list<ui_arg> args)
{
    ui_translation_group group({{context, source}});
    if (!group.available() || args.size() > 6)
        return original;
    const char *values[6] = {"", "", "", "", "", ""};
    unsigned i = 0;
    for (const auto &arg : args)
    {
        if (!arg.available)
            return original;
        values[i++] = arg.text.c_str();
    }
    return make_stringf(group.get(context, source), values[0], values[1],
                        values[2], values[3], values[4], values[5]);
}

bool try_ui_melee_miss(const char *source, const actor &attacker, const actor &defender,
                       const string &adverb, const item_def *weapon,
                       string &output)
{
    const bool player = attacker.is_player();
    ui_translation_group group({{"log.melee", source}});
    if (!adverb.empty())
        group.require("log.miss.adverb", adverb.c_str());
    if (weapon)
        group.require("log.attack.detail", " with %s");
    string subject, target, weapon_name;
    if (!group.available() || !try_ui_actor_name(defender, target)
        || (!player && !try_ui_actor_name(attacker, subject))
        || (weapon && !try_item_display_name(*weapon, weapon_name, false, false)))
        return false;
    const string manner = adverb.empty() ? ""
        : group.get("log.miss.adverb", adverb.c_str());
    const string tool = weapon
        ? make_stringf(group.get("log.attack.detail", " with %s"), weapon_name.c_str())
        : "";
    output = player
        ? make_stringf(group.get("log.melee", source), manner.c_str(), target.c_str(), tool.c_str())
        : make_stringf(group.get("log.melee", source), subject.c_str(), manner.c_str(), target.c_str());
    return true;
}
