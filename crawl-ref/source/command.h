/**
 * @file
 * @brief Misc commands, and functions for working with commands.
**/

#pragma once

#include <string>
#include <map>
#include <vector>

#include "cio.h"
#include "command-type.h"
#include "enum.h"
#include "format.h"

void list_armour();
void list_jewellery();

void show_specific_helps(const vector<string> keys);
void show_specific_help(const string& key);
void show_levelmap_help();
void show_targeting_help();
void show_interlevel_travel_branch_help();
void show_interlevel_travel_depth_help();
void show_interlevel_travel_altar_help();
void show_annotate_help();
void show_stash_search_help();
void show_skill_menu_help();
void show_spell_library_help();

void show_help(int section = CK_HOME, string highlight_string = "");

// Localized help retains line-sized operations for viewport glyph batching.
formatted_string help_document_display(const formatted_string &original);

// Zero-based section positions in the document actually being displayed.
std::map<int, int> help_document_hotkeys(const string &text);

int show_keyhelp_menu(const vector<formatted_string> &lines);

// XXX: Actually defined in main.cc; we may want to move this to command.cc.
void process_command(command_type cmd, command_type prev_cmd = CMD_NO_CMD);

// This is an ugly hack to return a command where a keycode is expected
// e.g. when returning mouse input from the getch_ck function
int encode_command_as_key(command_type cmd) noexcept;
