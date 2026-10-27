#pragma once

#include "i18n.h"
#include "dungeon-feature-type.h"
#include "spell-type.h"

class monster;
struct coord_def;

ui_arg s_known_actor_arg(const actor &who);
ui_arg s_feature_arg(dungeon_feature_type feature);
ui_arg s_feature_at_arg(const coord_def &where);
ui_arg s_actor_list_arg(const std::vector<monster*> &actors);
ui_arg s_spell_list_arg(const std::vector<spell_type> &spells);

// These are canonical clauses and anatomy nouns, never rendered English logs.
inline ui_arg s_clause_arg(const std::string &canonical)
{
    return canonical.empty() ? ui_literal_arg("") : ui_text_arg("s.clause", canonical);
}

inline ui_arg s_anatomy_arg(const std::string &canonical)
{
    return ui_text_arg("s.anatomy", canonical);
}

ui_arg s_spell_schools_arg(spell_type spell);
