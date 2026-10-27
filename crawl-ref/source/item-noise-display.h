#pragma once

#include "database.h"
#include "message.h"

struct item_def;
class actor;

bool try_item_noise_display_name(const item_def &item, string &output);

// Select canonical speech once, then render both languages with the same RNG.
string render_item_noise(const item_def &item, textdb_speech speech,
                         msg_channel_type &channel);
void paired_item_noise(const item_def &item, actor &act,
                       textdb_speech speech, int loudness);
