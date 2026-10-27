/**
 * @file
 * @brief Wizard mode command handling.
**/

#pragma once

#include <string>

#ifdef WIZARD
class WizardEntry;
WizardEntry wizard_display_entry(const std::string &title, const WizardEntry &original);
std::string wizard_display_choice(const std::string &title, const std::string &canonical);
int list_wizard_commands(bool do_redraw_screen = false);
void handle_wizard_command();
void enter_explore_mode();
#endif
