/**
 * @file
 * @brief Functions used when starting a new game.
**/

#pragma once

#include <vector>

#include "item-prop-enum.h"
#include "job-type.h"
#include "species-type.h"

using std::vector;

class UINewGameMenu;
struct menu_letter;
struct newgame_def;
struct player_save_info;

string newgame_char_description(const newgame_def& ng);
string newgame_char_display_description(const newgame_def& ng);
struct ui_arg;
ui_arg newgame_char_display_arg(const newgame_def& ng);
string save_character_display(const player_save_info &save, bool short_form = false,
                               bool use_qualifier = true);
string character_welcome_display(bool game_start, const string &name,
                                  species_type species, job_type job);

void choose_tutorial_character(newgame_def& ng_choice);

bool choose_game(newgame_def& ng, newgame_def& choice,
                 const newgame_def& defaults);

weapon_type starting_weapon_upgrade(weapon_type wp, job_type job,
                                            species_type species);

string newgame_random_name();

/*
 * A structure for grouping backgrounds by category.
 */
struct job_group
{
    const char* name;   ///< Name of the group.
    coord_def position; ///< Relative coordinates of the title.
    int width;          ///< Column width.
    vector<job_type> jobs; ///< List of jobs in the group.

    /// A method to attach the group to a freeform.
    void attach(const newgame_def& ng, const newgame_def& defaults,
                UINewGameMenu* menu, menu_letter &letter);
};

struct species_group
{
    const char* name;   ///< Name of the group.
    coord_def position; ///< Relative coordinates of the title.
    int width;          ///< Column width.
    vector<species_type> species_list; ///< List of species in the group.

    /// A method to attach the group to a freeform.
    void attach(const newgame_def& ng, const newgame_def& defaults,
                UINewGameMenu* menu, menu_letter &letter);
};
