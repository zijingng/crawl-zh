#pragma once

#include "database.h"
#include "i18n.h"

void do_decor_name_replacements(textdb_speech &speech);

// Caller selects inline choices at the original point before reading anatomy.
// Opaque anatomy keeps the whole original line; personal names reuse the
// canonical generated spelling in both display languages.
string render_decor_line(textdb_speech speech, const string &weapon,
                         const string &hands,
                         const ui_arg &weapon_display = {"", false},
                         const ui_arg &hands_display = {"", false});
