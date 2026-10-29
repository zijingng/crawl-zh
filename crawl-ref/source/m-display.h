#pragma once

#include "i18n.h"
#include "stringutil.h"
#include "mon-info.h"
#include "directn.h"
#include "message.h"
#include "actor.h"

// The caller keeps the complete canonical warning for missing components.
inline string m_confused_move_prompt(const string &original, bool stationary,
                                     const ui_arg &nearby, const ui_arg &expiry,
                                     bool penance)
{
    return ui_render("m.confused", "While confused and next to %s%s, are you sure you want to %s?%s",
        original, {nearby, expiry,
            ui_text_arg("m.confused", stationary ? "swing wildly" : "stumble around"),
            penance ? ui_text_arg("p.clause", " This could place you under penance!")
                    : ui_literal_arg("")});
}

// Display-only helpers. Canonical formatting is evaluated once by the caller.
inline string m_ice_move_prompt(const vector<string> &effects, const string &original)
{
    string names;
    bool complete = true;
    for (const string &effect : effects)
    {
        const ui_arg name = ui_text_arg("d.duration", effect);
        complete &= name.available;
        if (!names.empty()) names += "和";
        names += name.text;
    }
    return ui_render("m.move.confirm", "Your %s will break if you move. Continue?",
        original, {{names, complete}});
}

inline string m_play_time_display(const string &elapsed, int turns)
{
    const string count = to_string(turns);
    return ui_render("m.ui", "Play time: %s (%s turns).",
        "Play time: " + elapsed + " (" + count + " turns).",
        {ui_literal_arg(elapsed), ui_literal_arg(count)});
}

inline bool m_display_active(const char *context, const char *source)
{
    return ui_translation_group({{context, source}}).available();
}

inline ui_arg m_actor_arg(const actor &who)
{
    return ui_actor_arg(who);
}

inline ui_arg m_clause_arg(const string &canonical)
{
    return canonical.empty() ? ui_arg{"", true}
                             : ui_text_arg("m.clause", canonical);
}

inline ui_arg m_sentence_arg(const char *source, initializer_list<ui_arg> args)
{
    if (!m_display_active("m.clause", source)) return {"", false};
    for (const auto &arg : args)
        if (!arg.available) return {"", false};
    return {ui_render("m.clause", source, "", args), true};
}

inline bool m_monster_event(const monster &mons, const string &event,
                            const char *source, initializer_list<ui_arg> args,
                            bool possessive = false,
                            msg_channel_type channel = MSGCH_PLAIN, int param = 0)
{
    const ui_arg shown = m_sentence_arg(source, args);
    return simple_monster_message(mons, event.c_str(), possessive, channel,
                                  param, DESC_THE, shown.text);
}

inline ui_arg m_resist_arg(const actor &who, int margin)
{
    if (!ui_translation_group().available()) return {"", false};
    if (who.willpower() == WILL_INVULN)
        return ui_text_arg("m.resist", "unaffected");
    static const char *phrases[] = {"barely resist", "struggle to resist",
        "resist with significant effort", "resist with some effort",
        "easily resist", "resist with almost no effort"};
    const int index = std::max(0, std::min(5, (margin + 45) / 15));
    return ui_text_arg("m.resist", phrases[index]);
}

inline ui_arg m_actor_arg(const actor *who)
{
    return who ? ui_actor_arg(*who) : ui_arg{"", false};
}

inline ui_arg m_monster_type_arg(monster_type type)
{
    string shown;
    const bool available = try_monster_type_display_name(type, shown);
    return {shown, available};
}

inline ui_arg m_feature_arg(const coord_def &position)
{
    string shown;
    const bool available = try_feature_display_at(position, false, shown);
    return {shown, available};
}

inline ui_arg m_feature_type_arg(dungeon_feature_type type)
{
    string shown;
    const bool available = try_feature_display_name(type, NUM_TRAPS, shown);
    return {shown, available};
}

inline ui_arg m_join_args(const vector<ui_arg> &parts, const string &separator = "、")
{
    string result;
    for (const auto &part : parts)
    {
        if (!part.available) return {"", false};
        if (part.text.empty()) continue;
        if (!result.empty()) result += separator;
        result += part.text;
    }
    return {result, true};
}
