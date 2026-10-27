#pragma once

#include <string>

class actor;
struct bolt;

// Display only: consume canonical beam state without rerunning an attack.
// Missing components leave output untouched and require whole English fallback.
bool try_beam_hit_display(const char *source, const bolt &beam,
                          const actor &target, bool no_damage,
                          const std::string &punctuation, std::string &output);
