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
