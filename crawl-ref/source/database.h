/**
 * @file
 * database.h
**/

#pragma once

#include <list>
#include <vector>

using std::vector;

#ifdef DB_NDBM
extern "C" {
#   include <ndbm.h>
}
#elif defined(DB_DBH)
extern "C" {
#   define DB_DBM_HSEARCH 1
#   include <db.h>
}
#elif defined(USE_SQLITE_DBM)
#   include "sqldbm.h"
#else
#   error DBM interfaces unavailable!
#endif

#define DPTR_COERCE char *

void databaseSystemInit();
void databaseSystemShutdown();

// Read-only TextDB resolution, including aliases and embedded references.
// A missing translation keeps the whole fallback expansion in English.
string query_textdb_entry(DBM *english, DBM *translated, string key,
                         bool canonicalise_key, bool run_lua,
                         bool untranslated = false, bool *used_translation = nullptr);
string query_weighted_textdb_entry(DBM *english, DBM *translated,
                                  const string &key, const string &suffix = "",
                                  int fixed_weight = -1, bool untranslated = false);

typedef bool (*db_find_filter)(string key, string body);

string getQuoteString(const string &key);
string getLongDescription(const string &key);

vector<string> getLongDescKeysByRegex(const string &regex,
                                      db_find_filter filter = nullptr);
vector<string> getLongDescBodiesByRegex(const string &regex,
                                        db_find_filter filter = nullptr);

string getGameStartDescription(const string &key);

string getShoutString(const string &monst, const string &suffix = "");
string getSpeakString(const string &key, bool untranslated = false);
// Select and expand canonical speech once; a parallel translation uses the
// same weights and recursive choices. Never reroll to construct a fallback.
struct textdb_speech
{
    textdb_speech() = default;
    textdb_speech(const string &source, const string &display, bool available)
        : english(source), translated(display), complete(available) {}
    string english;
    string translated;
    bool complete = false;
    void replace(const string &marker, const string &original,
                 const string &display, bool available);
    string text() const;
    void pick_substrings();
};
textdb_speech query_paired_speech(DBM *english, DBM *translated, const string &key,
                                  bool preserve_names = false);
textdb_speech getPairedSpeakString(const string &key);
textdb_speech getPairedCastString(const string &key);
textdb_speech getPairedShoutString(const string &key, const string &suffix = "");
string getRandMonNameString(const string &montype);
string getRandNameString(const string &itemtype, const string &suffix = "");
string getHelpString(const string &topic);
// untranslated also applies to recursive references; useful when constructing
// the original English fallback before attempting a whole-message translation.
string getMiscString(const string &misc, const string &suffix = "",
                     bool untranslated = false);
string getHintString(const string &key, bool untranslated = false,
                     bool *used_translation = nullptr);
string getEgoString(const string &key);

vector<string> getAllFAQKeys();
string getFAQ_Question(const string &key);
string getFAQ_Answer(const string &question);
