/**
 * @file
 * @brief Let the player search for descriptions of monsters, items, etc.
 **/

#pragma once

#include <string>

#include "command-type.h"
#include "branch-type.h"
#include "lookup-help-type.h"

using std::string;

void keyhelp_query_descriptions(command_type where_from=CMD_NO_CMD);

string lookup_help_type_name(lookup_help_type lht);
char lookup_help_type_shortcut(lookup_help_type lht);
bool find_description_of_type(lookup_help_type lht);

// Display-only adapters; canonical search keys and entry text stay English.
bool try_lookup_help_display_name(const string &key, string &output);
string lookup_branch_extra_info(branch_type br);
