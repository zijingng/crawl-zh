#pragma once

#include <string>
#include <vector>
#include "enchant-type.h"

using std::string;
using std::vector;

class monster;
// Pure display helpers retain their complete original sentence on any miss.
string xom_scenery_message(const string &original, int fountains, int snacks,
                           int trees, int opened, int closed);
string xom_buff_message(const monster &mon, const vector<enchant_type> &effects,
                        bool healed, const string &original);
