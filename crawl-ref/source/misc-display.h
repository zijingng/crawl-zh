#pragma once

#include "database.h"

// Select once from canonical MiscDB and keep a complete English fallback.
textdb_speech getPairedMiscString(const string &key, const string &suffix = "");

// Translate recognized canonical gizmo names without drawing or changing saves.
bool try_gizmo_display_name(const string &canonical, string &display);

// Resolve a saved English appearance using reviewed TextDB; no name generation.
bool try_randart_appearance(const string &key, const string &canonical, string &display);

// Match complete reviewed title forms; leave unknown proper names intact.
bool try_randart_title(const string &canonical, string &display,
                       const string &player_name = "");

// Complete reviewed book themes; unknown owners or modifiers retain English.
bool try_randbook_title(const string &canonical, string &display,
                        const string &player_name = "", bool has_typed_owner = false);
