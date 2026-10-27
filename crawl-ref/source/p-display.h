#pragma once

#include "i18n.h"

ui_arg p_hand_arg();
ui_arg p_arm_arg();
ui_arg p_foot_arg();

// A canonical noun or state clause, never a parsed rendered English sentence.
// Empty optional clauses are complete and need no catalogue entry.
inline ui_arg p_clause_arg(const std::string &canonical)
{
    return canonical.empty() ? ui_arg{"", true}
                             : ui_text_arg("p.clause", canonical);
}

inline ui_arg p_duration_arg(const std::string &canonical)
{
    const ui_arg specific = ui_text_arg("p.duration.message", canonical);
    return specific.available ? specific : ui_text_arg("d.duration", canonical);
}

template<typename Range>
inline ui_arg p_item_list_arg(const Range &items)
{
    ui_arg result{"", true};
    for (const auto *item : items)
    {
        const ui_arg name = ui_item_arg(*item);
        result.available = result.available && name.available;
        if (!result.text.empty())
            result.text += "、";
        result.text += name.text;
    }
    return result;
}

template<typename Range>
inline ui_arg p_clause_list_arg(const Range &clauses)
{
    ui_arg result{"", true};
    for (const auto &clause : clauses)
    {
        const ui_arg part = p_clause_arg(clause);
        result.available = result.available && part.available;
        if (!result.text.empty())
            result.text += "、";
        result.text += part.text;
    }
    return result;
}
