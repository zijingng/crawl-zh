/**
 * @file
 * @brief Functions to handle speaking monsters
**/

#pragma once

#include "mpr.h"

void maybe_mons_speaks(monster* mons);
bool mons_speaks(monster* mons);
struct textdb_speech;
bool mons_speaks_msg(monster* mons, const string &msg,
                     const msg_channel_type def_chan = MSGCH_TALK,
                     const bool silence = false, const string &display = "",
                     textdb_speech *speech = nullptr);
bool invalid_msg(const monster &mon, string msg);
