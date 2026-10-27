#pragma once

#include "s-display.h"

inline ui_arg t_clause_arg(const std::string &key)
{
    return key.empty() ? ui_literal_arg("") : ui_text_arg("t.clause", key);
}

// Only a canonical projectile key, never a rendered sentence.
inline ui_arg t_projectile_arg(const std::string &key)
{
    ui_arg result{"", false};
    result.available = try_ui_projectile_name(key, result.text);
    return result;
}

ui_arg t_harlequin_arg(const std::string &selected);
ui_arg t_visible_monsters_arg(const std::vector<monster*> &monsters);
struct monster_info;
ui_arg t_monster_info_arg(const monster_info &mon);
