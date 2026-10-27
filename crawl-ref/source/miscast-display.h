#pragma once

#include "database.h"

class actor;

// Render a single already-selected passage, retaining its canonical fallback.
string render_miscast_message(actor &target, textdb_speech speech, int damage);
