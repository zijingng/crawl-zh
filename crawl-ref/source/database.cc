/**
 * @file
 * database.cc
**/

#include "AppHdr.h"
#include "i18n.h"

#include "database.h"
#include "misc-display.h"

#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(UNIX) || defined(TARGET_COMPILER_MINGW)
#include <unistd.h>
#endif

#include "clua.h"
#include "end.h"
#include "files.h"
#include "libutil.h"
#include "options.h"
#include "random.h"
#include "species.h"
#include "stringutil.h"
#include "syscalls.h"
#include "unicode.h"

// TextDB handles dependency checking the db vs text files, creating the
// db, loading, and destroying the DB.
class TextDB
{
public:
    // db_name is the savedir-relative name of the db file,
    // minus the "db" extension.
    TextDB(const char* db_name, const char* dir, vector<string> files);
    TextDB(TextDB *parent);
    ~TextDB() { shutdown(true); delete translation; }
    void init();
    void shutdown(bool recursive = false);
    DBM* get() { return _db; }

    // Make it easier to migrate from raw DBM* to TextDB
    operator bool() const { return _db != 0; }
    operator DBM*() const { return _db; }

 private:
    bool _needs_update() const;
    void _regenerate_db();

 private:
    bool open_db();
    const char* const _db_name;
    string _directory;
    vector<string> _input_files;
    DBM* _db;
    string timestamp;
    TextDB *_parent;
    const char* lang() { return _parent ? Options.lang_name : 0; }
public:
    TextDB *translation;
};

// Convenience functions for (read-only) access to generic
// berkeley DB databases.
static void _store_text_db(const string &in, DBM *db);

static string _query_database(TextDB &db, string key, bool canonicalise_key,
                              bool run_lua, bool untranslated = false);
static void _add_entry(DBM *db, const string &k, string &v);

static TextDB AllDBs[] =
{
    TextDB("descriptions", "descript/",
          { "features.txt",
            "items.txt",
            "unident.txt",
            "unrand.txt",
            "monsters.txt",
            "spells.txt",
            "gods.txt",
            "branches.txt",
            "skills.txt",
            "ability.txt",
            "cards.txt",
            "commands.txt",
            "clouds.txt",
            "status.txt",
            "monstatus.txt",
            "mutations.txt",
            "passives.txt", }),

    TextDB("gamestart", "descript/",
          { "species.txt",
            "backgrounds.txt" }),

    TextDB("randart", "database/",
          { "randname.txt",
            "rand_wpn.txt", // mostly weapons
            "rand_arm.txt", // mostly armour
            "rand_all.txt", // jewellery and general
            "randbook.txt", // artefact books
            }),

    TextDB("speak", "database/",
          { "monspeak.txt", // monster speech
            "monspell.txt", // monster spellcasting speech
            "monflee.txt",  // monster fleeing speech
            "wpnnoise.txt", // noisy weapon speech
            "insult.txt",   // imp/demon taunts
            "godspeak.txt", // god speech
            "monname.txt",  // names for Beogh apostles and Hep ancestors
                            // and weapon spirits, plus graffiti authors
            "colourname.txt", // colour/colour pattern names
            "graffiti.txt", // graffiti
            "miscast.txt",  // spell miscasts
            }),

    TextDB("shout", "database/",
          { "shout.txt",
            "insult.txt"    // imp/demon taunts, again
            }),

    TextDB("misc", "database/",
          { "miscname.txt", // names for miscellaneous things
            "godname.txt",  // god-related names (mostly His Xomminess)
            "montitle.txt", // titles for monsters (i.e. uniques)
            "decorlines.txt", // miscellaneous lines for walking on decoration
            "monname.txt",  // names for Beogh apostles and Hep ancestors
                            // and weapon spirits, plus graffiti authors, again
            "colourname.txt", // colour/colour pattern names, again
            "graffiti.txt", // graffiti, again
            "gizmo.txt",    // name-assembling for gizmos
            }),

    TextDB("quotes", "descript/",
          { "quotes.txt"    // quotes for items and monsters
            }),

    TextDB("help", "database/",
          { "help.txt"      // database for outsourced help texts
            }),

    TextDB("FAQ", "database/",
          { "FAQ.txt",      // database for Frequently Asked Questions
            }),

    TextDB("hints", "descript/",
          { "hints.txt",    // hints mode
            "tutorial.txt", // tutorial mode
            }),

    TextDB("egos", "descript/",
          { "egos.txt",     // weapon/armour/missile egos
            }),
};

static TextDB& DescriptionDB = AllDBs[0];
static TextDB& GameStartDB   = AllDBs[1];
static TextDB& RandartDB     = AllDBs[2];
static TextDB& SpeakDB       = AllDBs[3];
static TextDB& ShoutDB       = AllDBs[4];
static TextDB& MiscDB        = AllDBs[5];
static TextDB& QuotesDB      = AllDBs[6];
static TextDB& HelpDB        = AllDBs[7];
static TextDB& FAQDB         = AllDBs[8];
static TextDB& HintsDB       = AllDBs[9];
static TextDB& EgosDB        = AllDBs[10];

static string _db_cache_path(string db, const char *lang)
{
    if (lang)
        db = db + "." + lang;
    return savedir_versioned_path("db/" + db);
}

// ----------------------------------------------------------------------
// TextDB
// ----------------------------------------------------------------------

TextDB::TextDB(const char* db_name, const char* dir, vector<string> files)
    : _db_name(db_name), _directory(dir), _input_files(files),
      _db(nullptr), timestamp(""), _parent(0), translation(0)
{
}

TextDB::TextDB(TextDB *parent)
    : _db_name(parent->_db_name),
      _directory(parent->_directory + Options.lang_name + "/"),
      _input_files(parent->_input_files), // FIXME: pointless copy
      _db(nullptr), timestamp(""), _parent(parent), translation(nullptr)
{
    // Chinese files are reviewed exports. Paired callers preserve canonical
    // selection and delay actor/species substitutions until display.
}

bool TextDB::open_db()
{
    if (_db)
        return true;

    const string full_db_path = _db_cache_path(_db_name, lang());
    _db = dbm_open(full_db_path.c_str(), O_RDONLY, 0660);
    if (!_db)
        return false;

    timestamp = _query_database(*this, "TIMESTAMP", false, false, true);
    if (timestamp.empty())
        return false;

    return true;
}

void TextDB::init()
{
    if (Options.lang_name && !_parent)
    {
        translation = new TextDB(this);
        translation->init();
    }

    open_db();

    if (!_needs_update())
        return;
    _regenerate_db();

    if (!open_db())
    {
        end(1, true, "Failed to open DB: %s",
            _db_cache_path(_db_name, lang()).c_str());
    }
}

void TextDB::shutdown(bool recursive)
{
    if (_db)
    {
        dbm_close(_db);
        _db = nullptr;
    }
    if (recursive && translation)
        translation->shutdown(recursive);
}

bool TextDB::_needs_update() const
{
    string ts;
    bool no_files = true;

    for (const string &file : _input_files)
    {
        string full_input_path = _directory + file;
        full_input_path = datafile_path(full_input_path, !_parent);
        // packagers who mess with mtime beware: you shouldn't put the db in
        // a shared folder, as a fixed mtime will break this check.
        time_t mtime = file_modtime(full_input_path);
        const bool exists = file_exists(full_input_path);
        if (exists || !_parent)
        {
            if (exists)
                no_files = false;
            char buf[20];
            snprintf(buf, sizeof(buf), ":%" PRId64, (int64_t)mtime);
            ts += buf;
        }
    }

    if (no_files)
    {
        // No point in empty databases, although for simplicity keep ones
        // for disappeared translations for now.
        ASSERTM(_parent,
            "No readable database files in `%s` (internal error).",
            _directory.c_str());
        TextDB *en = _parent;
        delete en->translation; // ie, ourself
        en->translation = 0;
        return false;
    }

    return ts != timestamp;
}

void TextDB::_regenerate_db()
{
    shutdown();
    if (_parent)
    {
#if defined(DEBUG_DIAGNOSTICS) && !(defined(TARGET_COMPILER_VC) && defined(USE_TILE))
        printf("Regenerating db: %s [%s]\n", _db_name, Options.lang_name);
#endif
        mprf(MSGCH_PLAIN, ui_translate("d.database", "Regenerating db: %s [%s]"), _db_name, Options.lang_name);
    }
    else
    {
#if defined(DEBUG_DIAGNOSTICS) && !(defined(TARGET_COMPILER_VC) && defined(USE_TILE))
        printf("Regenerating db: %s\n", _db_name);
#endif
        mprf(MSGCH_PLAIN, ui_translate("d.database", "Regenerating db: %s"), _db_name);
    }

    string db_path = _db_cache_path(_db_name, lang());
    string full_db_path = db_path + ".db";

    {
        string output_dir = get_parent_directory(db_path);
        if (!check_mkdir("DB directory", &output_dir))
            end(1, false, "Cannot create db directory '%s'.", output_dir.c_str());
    }

    file_lock lock(db_path + ".lk", "wb");
#ifndef DGL_REWRITE_PROTECT_DB_FILES
    unlink_u(full_db_path.c_str());
#endif

    string ts;
    if (!(_db = dbm_open(db_path.c_str(), O_RDWR | O_CREAT, 0660)))
        end(1, true, "Unable to open DB: %s", db_path.c_str());
    for (const string &file : _input_files)
    {
        string full_input_path = _directory + file;
        full_input_path = datafile_path(full_input_path, !_parent);
        char buf[20];
        time_t mtime = file_modtime(full_input_path);
        if (file_exists(full_input_path)
            || !_parent) // english is mandatory
        {
            snprintf(buf, sizeof(buf), ":%" PRId64, (int64_t)mtime);
            ts += buf;
            _store_text_db(full_input_path, _db);
        }
    }
    _add_entry(_db, "TIMESTAMP", ts);

    dbm_close(_db);
    _db = 0;
}

// ----------------------------------------------------------------------
// DB system
// ----------------------------------------------------------------------

#define NUM_DB ARRAYSZ(AllDBs)

// Display-only dictionary caches; canonical names remain in save properties.
static map<string,string> gizmo_adjectives, gizmo_nouns, gizmo_modifiers;
static bool gizmo_names_loaded = false;
static map<string,map<string,string>> appearance_words;
static map<string,set<string>> book_subjects;
static map<string,set<string>> book_modifiers, book_owners;

void databaseSystemInit()
{
    for (unsigned int i = 0; i < NUM_DB; i++)
        AllDBs[i].init();
}

void databaseSystemShutdown()
{
    appearance_words.clear();
    book_subjects.clear();
    book_modifiers.clear();book_owners.clear();
    gizmo_names_loaded = false;
    gizmo_adjectives.clear(); gizmo_nouns.clear(); gizmo_modifiers.clear();
    for (unsigned int i = 0; i < NUM_DB; i++)
        AllDBs[i].shutdown(true);
}

////////////////////////////////////////////////////////////////////////////
// Main DB functions

static datum _database_fetch(DBM *database, const string &key)
{
    datum result;
    result.dptr = nullptr;
    result.dsize = 0;
    datum dbKey;

    dbKey.dptr = (DPTR_COERCE) key.c_str();
    dbKey.dsize = key.length();

    // Don't use the database if called from "monster".
    if (database)
        result = dbm_fetch(database, dbKey);

    return result;
}

static vector<string> _database_find_keys(DBM *database,
                                          const string &regex,
                                          bool ignore_case,
                                          db_find_filter filter = nullptr)
{
    text_pattern             tpat(regex, ignore_case);
    vector<string> matches;

    datum dbKey = dbm_firstkey(database);

    while (dbKey.dptr != nullptr)
    {
        string key((const char *)dbKey.dptr, dbKey.dsize);

        if (tpat.matches(key)
            && key.find("__") == string::npos
            && (filter == nullptr || !(*filter)(key, "")))
        {
            matches.push_back(key);
        }

        dbKey = dbm_nextkey(database);
    }

    return matches;
}

static vector<string> _database_find_bodies(DBM *database,
                                            const string &regex,
                                            bool ignore_case,
                                            db_find_filter filter = nullptr)
{
    text_pattern             tpat(regex, ignore_case);
    vector<string> matches;

    datum dbKey = dbm_firstkey(database);

    while (dbKey.dptr != nullptr)
    {
        string key((const char *)dbKey.dptr, dbKey.dsize);

        datum dbBody = dbm_fetch(database, dbKey);
        string body((const char *)dbBody.dptr, dbBody.dsize);

        if (tpat.matches(body)
            && key.find("__") == string::npos
            && (filter == nullptr || !(*filter)(key, body)))
        {
            matches.push_back(key);
        }

        dbKey = dbm_nextkey(database);
    }

    return matches;
}

///////////////////////////////////////////////////////////////////////////
// Internal DB utility functions
static void _execute_embedded_lua(string &str,
    vector<pair<string, string>> *results = nullptr)
{
    // Execute any lua code found between "{{" and "}}". The lua code
    // is expected to return a string, with which the lua code and
    // braces will be replaced.
    string::size_type pos = str.find("{{");
    while (pos != string::npos)
    {
        string::size_type end = str.find("}}", pos + 2);
        if (end == string::npos)
        {
            mprf(MSGCH_DIAGNOSTICS, "Unbalanced {{, bailing.");
            break;
        }

        string lua_full = str.substr(pos, end - pos + 2);
        string lua      = str.substr(pos + 2, end - pos - 2);

        if (clua.execstring(lua.c_str(), "db_embedded_lua", 1))
        {
            string err = "{{" + clua.error + "}}";
            str.replace(pos, lua_full.length(), err);
            return;
        }

        string result;
        clua.fnreturns(">s", &result);

        if (results)
            results->emplace_back(lua_full, result);
        str.replace(pos, lua_full.length(), result);

        pos = str.find("{{", pos + result.length());
    }
}

static void _substitute_descriptions(DBM *english, DBM *translated, string &str,
                                     bool canonicalise_key, bool run_lua,
                                     bool untranslated)
{
    // Replace all keys found between "[[" and "]]" with corresponding
    // descriptions from the database.
    string::size_type pos = str.find("[[");
    while (pos != string::npos)
    {
        string::size_type end = str.find("]]", pos + 2);
        if (end == string::npos)
        {
            mprf(MSGCH_DIAGNOSTICS, "Unbalanced [[, bailing.");
            break;
        }

        string key = str.substr(pos + 2, end - pos - 2);
        string result = query_textdb_entry(english, translated, key, canonicalise_key,
                                          run_lua, untranslated);
        str.replace(pos, key.length() + 4, trim_string_right(result));

        pos = str.find("[[", pos + result.length());
    }
}

static void _trim_leading_newlines(string &s)
{
    s.erase(0, s.find_first_not_of("\n"));
}

static void _add_entry(DBM *db, const string &k, string &v)
{
    _trim_leading_newlines(v);
    datum key, value;
    key.dptr = (char *) k.c_str();
    key.dsize = k.length();

    value.dptr = (char *) v.c_str();
    value.dsize = v.length();

    if (dbm_store(db, key, value, DBM_REPLACE))
        end(1, true, "Error storing %s", k.c_str());
}

static void _parse_text_db(LineInput &inf, DBM *db)
{
    string key;
    string value;

    bool in_entry = false;
    while (!inf.eof())
    {
        string line = inf.get_line();

        if (!line.empty() && line[0] == '#')
            continue;

        if (!line.compare(0, 4, "%%%%"))
        {
            if (!key.empty())
                _add_entry(db, key, value);
            key.clear();
            value.clear();
            in_entry = true;
            continue;
        }

        if (!in_entry)
            continue;

        if (key.empty())
        {
            key = line;
            trim_string(key);
            lowercase(key);
        }
        else
        {
            trim_string_right(line);
            value += line + "\n";
        }
    }

    if (!key.empty())
        _add_entry(db, key, value);
}

static void _store_text_db(const string &in, DBM *db)
{
    UTF8FileLineInput inf(in.c_str());
    if (inf.error())
        end(1, true, "Unable to open input file: %s", in.c_str());

    _parse_text_db(inf, db);
}

static string _chooseStrByWeight(const string &entry, int fixed_weight = -1,
                                int *chosen_weight = nullptr,
                                vector<int> *shape = nullptr)
{
    vector<string> parts;
    vector<int>    weights;

    vector<string> lines = split_string("\n", entry, false, true);

    int total_weight = 0;
    for (int i = 0, size = lines.size(); i < size; i++)
    {
        // Skip over multiple blank lines, and leading and trailing
        // blank lines.
        while (i < size && lines[i].empty())
            i++;

        if (i == size)
            break;

        int         weight;
        string part = "";

        if (sscanf(lines[i].c_str(), "w:%d", &weight))
        {
            i++;
            if (i == size)
                return "BUG, WEIGHT AT END OF ENTRY";
        }
        else
            weight = 10;

        total_weight += weight;

        while (i < size && !lines[i].empty())
        {
            part += lines[i++];
            part += "\n";
        }
        trim_string(part);

        parts.push_back(part);
        weights.push_back(total_weight);
    }

    if (parts.empty())
        return "BUG, EMPTY ENTRY";

    int choice = 0;
    if (fixed_weight != -1)
        choice = fixed_weight % total_weight;
    else
        choice = random2(total_weight);

    if (chosen_weight)
        *chosen_weight = choice;
    if (shape)
        *shape = weights;

    for (int i = 0, size = parts.size(); i < size; i++)
        if (choice < weights[i])
            return parts[i];

    return "BUG, NO STRING CHOSEN";
}

#define MAX_RECURSION_DEPTH 10
#define MAX_REPLACEMENTS    100

string query_weighted_textdb_entry(DBM *english, DBM *translated,
                                  const string &key, const string &suffix,
                                  int fixed_weight, bool untranslated)
{
    // We have to canonicalise the key (in case the user typed it
    // in and got the case wrong.)
    string canonical_key = key + suffix;
    lowercase(canonical_key);

    // Query the DB.
    datum result{};

    if (translated && !untranslated)
        result = _database_fetch(translated, canonical_key);
    if (result.dsize <= 0)
        result = _database_fetch(english, canonical_key);

    if (result.dsize <= 0)
    {
        // Try ignoring the suffix.
        canonical_key = key;
        lowercase(canonical_key);

        // Query the DB.
        if (translated && !untranslated)
            result = _database_fetch(translated, canonical_key);
        if (result.dsize <= 0)
            result = _database_fetch(english, canonical_key);

        if (result.dsize <= 0)
            return "";
    }

    // Cons up a (C++) string to return. The caller must release it.
    string str = string((const char *)result.dptr, result.dsize);

    return _chooseStrByWeight(str, fixed_weight);
}

static string _getWeightedString(TextDB &db, const string &key,
                                 const string &suffix, int fixed_weight = -1,
                                 bool untranslated = false)
{
    return query_weighted_textdb_entry(db.get(),
        db.translation ? db.translation->get() : nullptr,
        key, suffix, fixed_weight, untranslated || Options.language == lang_t::EN);
}

static void _call_recursive_replacement(string &str, TextDB &db,
                                        const string &suffix,
                                        int &num_replacements,
                                        int recursion_depth = 0,
                                        bool untranslated = false);

static string _getRandomisedStr(TextDB &db, const string &key,
                                const string &suffix,
                                int &num_replacements,
                                int recursion_depth = 0,
                                bool untranslated = false)
{
    recursion_depth++;
    if (recursion_depth > MAX_RECURSION_DEPTH)
    {
        mprf(MSGCH_DIAGNOSTICS, "Too many nested replacements, bailing.");

        return "TOO MUCH RECURSION";
    }

    string str = _getWeightedString(db, key, suffix, -1, untranslated);

    _call_recursive_replacement(str, db, suffix, num_replacements,
                                recursion_depth, untranslated);

    return str;
}

// Replace any "@foo@" markers that can be found in this database.
// Those that can't be found are left alone for the caller to deal with.
static void _call_recursive_replacement(string &str, TextDB &db,
                                        const string &suffix,
                                        int &num_replacements,
                                        int recursion_depth,
                                        bool untranslated)
{
    string::size_type pos = str.find("@");
    while (pos != string::npos)
    {
        num_replacements++;
        if (num_replacements > MAX_REPLACEMENTS)
        {
            mprf(MSGCH_DIAGNOSTICS, "Too many string replacements, bailing.");
            return;
        }

        string::size_type end = str.find("@", pos + 1);
        if (end == string::npos)
        {
            mprf(MSGCH_DIAGNOSTICS, "Unbalanced @, bailing.");
            break;
        }

        string marker_full = str.substr(pos, end - pos + 1);
        string marker      = str.substr(pos + 1, end - pos - 1);

        string replacement =
            _getRandomisedStr(db, marker, suffix, num_replacements,
                              recursion_depth, untranslated);

        if (replacement.empty())
        {
            // Nothing in database, leave it alone and go onto next @foo@
            pos = str.find("@", end + 1);
        }
        else
        {
            str.replace(pos, marker_full.length(), replacement);

            // Start search from pos rather than end + 1, so that if
            // the replacement contains its own @foo@, we can replace
            // that too.
            pos = str.find("@", pos);
        }
    } // while (pos != string::npos)
}

static string _query_database(TextDB &db, string key, bool canonicalise_key,
                              bool run_lua, bool untranslated)
{
    return query_textdb_entry(db.get(), db.translation ? db.translation->get() : nullptr,
                             key, canonicalise_key, run_lua, untranslated);
}

string query_textdb_entry(DBM *english, DBM *translated, string key,
                         bool canonicalise_key, bool run_lua, bool untranslated,
                         bool *used_translation)
{
    if (used_translation)
        *used_translation = false;
    if (canonicalise_key)
    {
        // We have to canonicalise the key (in case the user typed it
        // in and got the case wrong.)
        lowercase(key);
    }

    // Query the DB.
    datum result;

    if (translated && !untranslated)
        result = _database_fetch(translated, key);
    if (result.dsize <= 0)
    {
        result = _database_fetch(english, key);
        // Do not splice translated children into a fallback English parent.
        untranslated = true;
    }

    if (result.dsize <= 0)
        return "";

    if (used_translation)
        *used_translation = !untranslated;
    string str((const char *)result.dptr, result.dsize);

    // <foo> is an alias to key foo
    if (str[0] == '<' && str[str.size() - 2] == '>'
        && str.find('<', 1) == str.npos
        && str.find('\n') == str.size() - 1)
    {
        return query_textdb_entry(english, translated, str.substr(1, str.size() - 3),
                                 canonicalise_key, run_lua, untranslated, used_translation);
    }

    _substitute_descriptions(english, translated, str, canonicalise_key, run_lua, untranslated);

    if (run_lua)
        _execute_embedded_lua(str);

    return str;
}

/////////////////////////////////////////////////////////////////////////////
// Quote DB specific functions.

string getQuoteString(const string &key)
{
    return unwrap_desc(_query_database(QuotesDB, key, true, true));
}

/////////////////////////////////////////////////////////////////////////////
// Description DB specific functions.

string getLongDescription(const string &key)
{
    return unwrap_desc(_query_database(DescriptionDB, key, true, true));
}

vector<string> getLongDescKeysByRegex(const string &regex,
                                      db_find_filter filter)
{
    if (!DescriptionDB.get())
    {
        vector<string> empty;
        return empty;
    }

    // FIXME: need to match regex against translated keys, which can't
    // be done by db only.
    return _database_find_keys(DescriptionDB.get(), regex, true, filter);
}

vector<string> getLongDescBodiesByRegex(const string &regex,
                                        db_find_filter filter)
{
    if (!DescriptionDB.get())
    {
        vector<string> empty;
        return empty;
    }

    // On partial translations, this will match only translated descriptions.
    // Not good, but otherwise we'd have to check hundreds of keys, with
    // two queries for each.
    // SQL can do this in one go, DBM can't.
    DBM *database = DescriptionDB.translation ?
        DescriptionDB.translation->get() : DescriptionDB.get();
    return _database_find_bodies(database, regex, true, filter);
}

/////////////////////////////////////////////////////////////////////////////
// GameStart DB specific functions.
string getGameStartDescription(const string &key)
{
    return _query_database(GameStartDB, key, true, true);
}

/////////////////////////////////////////////////////////////////////////////
// Shout DB specific functions.
string getShoutString(const string &monst, const string &suffix)
{
    if (Options.language == lang_t::ZH)
        return getPairedShoutString(monst, suffix).text();
    int num_replacements = 0;

    return _getRandomisedStr(ShoutDB, monst, suffix, num_replacements);
}

/////////////////////////////////////////////////////////////////////////////
// Speak DB specific functions.
string getSpeakString(const string &key, bool untranslated)
{
    // All translated speech is selected from canonical English once. The
    // exporter, rather than a second UI catalogue, owns review filtering.
    if (!untranslated && Options.language == lang_t::ZH)
        return getPairedCastString(key).text();
    int num_replacements = 0;

#ifdef DEBUG_MONSPEAK
    dprf(DIAG_SPEECH, "monster speech lookup for %s", key.c_str());
#endif
    string txt = _getRandomisedStr(SpeakDB, key, "", num_replacements, 0, untranslated);
    _execute_embedded_lua(txt);

    return txt;
}

static bool _speech_is_alias_chain(const string &text)
{
    if (text.empty())
        return false;
    size_t pos = 0;
    while (pos < text.size())
    {
        if (text[pos] != '@')
            return false;
        const size_t end = text.find('@', pos + 1);
        if (end == string::npos || end == pos + 1)
            return false;
        pos = end + 1;
    }
    return true;
}

static textdb_speech _paired_speech(DBM *english, DBM *translated,
                                   string key, int &replacements, int depth,
                                   const string &suffix = "",
                                   bool preserve_names = false, bool literal_name = false)
{
    if (++depth > MAX_RECURSION_DEPTH)
        return {"TOO MUCH RECURSION", "", false};
    lowercase(key);
    // This database namespace contains proper names, not display prose.
    literal_name = literal_name || (preserve_names
        && (key == "orc name" || key == "any orc name"
            || key == "ancestor name" || key == "bland name"));
    string resolved_key = lowercase_string(key + suffix);
    datum en = _database_fetch(english, resolved_key);
    if (en.dsize <= 0)
    {
        resolved_key = key;
        en = _database_fetch(english, resolved_key);
    }
    if (en.dsize <= 0)
        return {};
    int choice = 0;
    vector<int> en_shape, zh_shape;
    textdb_speech result;
    const string source((const char*)en.dptr, en.dsize);
    result.english = _chooseStrByWeight(source, -1, &choice, &en_shape);
    datum chinese = _database_fetch(translated, resolved_key);
    if (literal_name)
    {
        result.translated = result.english;
        result.complete = true;
    }
    else if (chinese.dsize > 0)
    {
        result.translated = _chooseStrByWeight(
            string((const char *)chinese.dptr, chinese.dsize), choice, nullptr, &zh_shape);
        result.complete = en_shape == zh_shape;
    }
    else if (_speech_is_alias_chain(result.english))
    {
        // A pure alias chain has no prose to translate. Every selected child
        // must still be complete before the parent can use the translation.
        result.translated = result.english;
        result.complete = true;
    }
    size_t pos = result.english.find('@');
    while (pos != string::npos)
    {
        if (++replacements > MAX_REPLACEMENTS)
        {
            result.complete = false;
            break;
        }
        const size_t end = result.english.find('@', pos + 1);
        if (end == string::npos)
        {
            result.complete = false;
            break;
        }
        const string marker = result.english.substr(pos, end - pos + 1);
        const auto child = _paired_speech(english, translated,
            marker.substr(1, marker.size() - 2), replacements, depth, suffix,
            preserve_names, literal_name);
        if (child.english.empty())
            pos = result.english.find('@', end + 1);
        else
        {
            const size_t lua_start = result.english.rfind("{{", pos);
            const size_t lua_end = result.english.rfind("}}", pos);
            const bool inside_lua = lua_start != string::npos
                && (lua_end == string::npos || lua_start > lua_end);
            result.english.replace(pos, marker.size(), child.english);
            const size_t zh_pos = result.translated.find(marker);
            result.complete = result.complete && child.complete
                              && zh_pos != string::npos;
            if (zh_pos != string::npos)
                // Never splice Chinese prose into a canonical Lua program.
                // Reviewed result mappings supply the selected display later.
                result.translated.replace(zh_pos, marker.size(),
                    inside_lua ? child.english : child.translated);
            pos = result.english.find('@', pos);
        }
    }
    return result;
}

textdb_speech query_paired_speech(DBM *english, DBM *translated, const string &key,
                                  bool preserve_names)
{
    int replacements = 0;
    return _paired_speech(english, translated, key, replacements, 0, "", preserve_names);
}

// Execute canonical Lua once. Prefer a reviewed whole result; otherwise replace
// identical canonical blocks in the display template with reviewed fragments.
static void _execute_paired_lua(textdb_speech &result, DBM *translated)
{
    if (result.english.find("{{") == string::npos)
        return;
    const bool available = result.complete;
    const bool identical = result.translated == result.english;
    vector<pair<string, string>> choices;
    _execute_embedded_lua(result.english, &choices);
    const datum shown = _database_fetch(translated,
        "__lua_display__" + lowercase_string(result.english));
    if (available && identical && shown.dsize > 0)
    {
        result.translated = string((const char *)shown.dptr, shown.dsize);
        trim_string_right(result.translated);
        return;
    }
    result.complete = available && !choices.empty();
    for (const auto &choice : choices)
    {
        const size_t pos = result.translated.find(choice.first);
        const datum fragment = _database_fetch(translated,
            "__lua_fragment__" + lowercase_string(choice.second));
        if (pos == string::npos || fragment.dsize <= 0)
        {
            result.complete = false;
            break;
        }
        string display((const char *)fragment.dptr, fragment.dsize);
        trim_string_right(display);
        result.translated.replace(pos, choice.first.length(), display);
    }
    result.complete = result.complete
        && result.translated.find("{{") == string::npos
        && result.english.find("{{") == string::npos;
}

textdb_speech getPairedSpeakString(const string &key)
{
    auto result = query_paired_speech(SpeakDB.get(),
        SpeakDB.translation ? SpeakDB.translation->get() : nullptr, key, true);
    _execute_paired_lua(result, SpeakDB.translation
        ? SpeakDB.translation->get() : nullptr);
    return result;
}

textdb_speech getPairedCastString(const string &key)
{
    auto result = query_paired_speech(SpeakDB.get(),
        SpeakDB.translation ? SpeakDB.translation->get() : nullptr, key, true);
    _execute_paired_lua(result, SpeakDB.translation
        ? SpeakDB.translation->get() : nullptr);
    return result;
}

textdb_speech getPairedShoutString(const string &key, const string &suffix)
{
    int replacements = 0;
    // Shouts use the reviewed source body. A stale bundled overlay must not
    // restore a translation rejected by the current catalogue.
    return _paired_speech(ShoutDB.get(),
        ShoutDB.translation ? ShoutDB.translation->get() : nullptr,
        key, replacements, 0, suffix, true);
}

void textdb_speech::replace(const string &marker, const string &original,
                           const string &display, bool available)
{
    if (english.find(marker) != string::npos
        || translated.find(marker) != string::npos)
        complete = complete && available;
    english = replace_all(english, marker, original);
    translated = replace_all(translated, marker, display);
}

string textdb_speech::text() const
{
    return Options.language == lang_t::ZH && complete
                    && translated.find('@') == string::npos
                    && translated.find("{{") == string::npos
         ? translated : english;
}

void textdb_speech::pick_substrings()
{
    size_t start = 0;
    size_t zh_start = 0;
    while ((start = english.find('[', start)) != string::npos)
    {
        const size_t end = english.find(']', start);
        if (end == string::npos)
            break;
        const auto choices = split_string("|", english.substr(start + 1, end - start - 1), false, true);
        const int index = random2(choices.size());
        english.replace(start, end + 1 - start, choices[index]);
        zh_start = translated.find('[', zh_start);
        const size_t zh_end = zh_start == string::npos ? string::npos : translated.find(']', zh_start);
        if (zh_end == string::npos)
            complete = false;
        else
        {
            const auto zh_choices = split_string("|", translated.substr(zh_start + 1, zh_end - zh_start - 1), false, true);
            if (zh_choices.size() != choices.size())
                complete = false;
            else
                translated.replace(zh_start, zh_end + 1 - zh_start, zh_choices[index]);
        }
    }
    if (translated.find('[') != string::npos)
        complete = false;
}

string getRandMonNameString(const string &montype)
{
    int num_replacements = 0;

    return _getRandomisedStr(SpeakDB, montype, " name", num_replacements);
}

/////////////////////////////////////////////////////////////////////////////
// Randname DB specific functions.
string getRandNameString(const string &itemtype, const string &suffix)
{
    int num_replacements = 0;

    // Random names are save identities, irrespective of display language.
    return _getRandomisedStr(RandartDB, itemtype, suffix, num_replacements, 0, true);
}


namespace
{
// Expand only paired, reviewed naming candidates. Never execute Lua or draw RNG.
bool _appearance_dictionary(const string &requested, map<string,string> &out, int depth)
{
    string key=requested;lowercase(key);
    if (depth > 8 || !RandartDB.translation) return false;
    if (key=="player_species")
    {
        // This marker is a typed species genus, not an unreviewed TextDB key.
        for (const auto sp:species::get_all_species())
        {
            const string english=species::name(sp,species::SPNAME_GENUS);
            string lower=english;lowercase(lower);
            for (const char *context:{"display.monster","display.creation","d.god.title.component"})
            {
                const string &word=string(context)=="display.monster" ? lower : english;
                ui_translation_group text;text.require(context,word.c_str());
                if (text.available()) {out.emplace(english,text.get(context,word.c_str()));break;}
            }
        }
        return !out.empty();
    }
    const datum en = _database_fetch(RandartDB.get(), key.c_str());
    const datum zh = _database_fetch(RandartDB.translation->get(), key.c_str());
    if (en.dsize <= 0 || zh.dsize <= 0) return false;
    const auto left = split_string("\n\n", string((const char *)en.dptr,en.dsize));
    const auto right = split_string("\n\n", string((const char *)zh.dptr,zh.dsize));
    if (left.empty() || left.size() != right.size()) return false;
    for (size_t i=0; i<left.size(); ++i)
    {
        string a=left[i], b=right[i];
        if (starts_with(a,"w:"))
        {
            const size_t line=a.find('\n');
            if (line==string::npos || b.substr(0,line+1)!=a.substr(0,line+1)) return false;
            a.erase(0,line+1); b.erase(0,line+1);
        }
        if (a.empty() || b.empty() || a.find_first_of("\n\r{}[]")!=string::npos
            || b.find_first_of("\n\r{}[]")!=string::npos) return false;
        vector<pair<string,string>> expanded{{a,b}};
        while (!expanded.empty() && expanded.front().first.find('@')!=string::npos)
        {
            const string pattern=expanded.front().first;
            const size_t start=pattern.find('@'), end=pattern.find('@',start+1);
            if (end==string::npos) return false;
            const string marker=pattern.substr(start,end-start+1);
            map<string,string> children;
            if (!_appearance_dictionary(pattern.substr(start+1,end-start-1),children,depth+1)) return false;
            vector<pair<string,string>> next;
            for (const auto &parent:expanded)
            {
                if (parent.second.find(marker)==string::npos) return false;
                for (const auto &child:children)
                {
                    next.emplace_back(replace_all(parent.first,marker,child.first),
                                      replace_all(parent.second,marker,child.second));
                    if (next.size()>10000) return false;
                }
            }
            expanded=std::move(next);
        }
        for (const auto &word:expanded)
        {
            if (word.second.find('@')!=string::npos) return false;
            auto found=out.emplace(word);
            if (!found.second && found.first->second!=word.second) return false;
        }
    }
    return !out.empty();
}
}

bool try_randart_appearance(const string &key, const string &canonical, string &display)
{
    if (Options.language!=lang_t::ZH) return false;
    // Restrict the grammar to the reviewed appearance namespaces.
    static const set<string> roots={"general appearance","weapon appearance",
        "robe appearance","body armour appearance","armour appearance",
        "ring appearance","amulet appearance","book appearance"};
    if (!roots.count(key)) return false;
    auto found=appearance_words.find(key);
    if (found==appearance_words.end())
    {
        map<string,string> words;
        if (!_appearance_dictionary(key,words,0)) return false;
        found=appearance_words.emplace(key,std::move(words)).first;
    }
    const auto word=found->second.find(canonical);
    if (word==found->second.end()) return false;
    display=word->second;
    return true;
}



namespace
{
// Match the saved canonical title against the reviewed Ashenzari program.
// Expand only words compatible with its literal prefix, without generating
// names again or materialising the full adjective/object Cartesian product.
bool _ashenzari_display_title(const string &canonical, string &display)
{
    if (!RandartDB.translation) return false;
    const datum en=_database_fetch(RandartDB.get(),"ashenzari");
    const datum zh=_database_fetch(RandartDB.translation->get(),"ashenzari");
    if(en.dsize<=0 || zh.dsize<=0)return false;
    const auto source=split_string("\n\n",string((const char *)en.dptr,en.dsize));
    const auto target=split_string("\n\n",string((const char *)zh.dptr,zh.dsize));
    if(source.size()!=target.size())return false;
    set<string> matches;
    std::function<void(string,string,int)> match=[&](string pattern,string shown,int depth) {
        if(depth>8)return;
        const size_t start=pattern.find('@');
        if(start==string::npos) {
            if(pattern==canonical && !shown.empty() && shown.find_first_of("@{}[]\n\r")==string::npos)
                matches.insert(shown);
            return;
        }
        if(!starts_with(canonical,pattern.substr(0,start)))return;
        const size_t end=pattern.find('@',start+1);if(end==string::npos)return;
        const string marker=pattern.substr(start,end-start+1);
        string key=pattern.substr(start+1,end-start-1);lowercase(key);
        if(shown.find(marker)==string::npos)return;
        map<string,string> typed;
        const map<string,string> *words=nullptr;
        if(key=="god_name" || key=="god_name_possessive") {
            // This program names an Ashenzari curse, not an arbitrary gift.
            ui_translation_group god;god.require("display.god","Ashenzari");
            if(!god.available())return;
            typed.emplace(key=="god_name" ? "Ashenzari" : "Ashenzari's",
                          god.get("display.god","Ashenzari"));words=&typed;
        } else {
            const set<string> roots={"_ashenzari_adjective_","_ashenzari_object_",
                "_ashenzari_epithet_","_ashenzari_bound_adjective_",
                "_ashenzari_knowledge_adjective_","_ashenzari_celestial_adjective_",
                "_ashenzari_heavens_","_celestial_bodies_"};
            if(!roots.count(key))return;
            auto found=appearance_words.find(key);
            if(found==appearance_words.end()) {
                map<string,string> paired;
                if(!_appearance_dictionary(key,paired,0))return;
                found=appearance_words.emplace(key,std::move(paired)).first;
            }
            words=&found->second;
        }
        for(const auto &word:*words) {
            const string expanded=replace_all(pattern,marker,word.first);
            const size_t next=expanded.find('@');
            if(starts_with(canonical,expanded.substr(0,next)))
                match(expanded,replace_all(shown,marker,word.second),depth+1);
        }
    };
    for(size_t i=0;i<source.size();++i) {
        string pattern=source[i],shown=target[i];
        if(starts_with(pattern,"w:")) {
            const size_t line=pattern.find('\n');
            if(line==string::npos || shown.substr(0,line+1)!=pattern.substr(0,line+1))return false;
            pattern.erase(0,line+1);shown.erase(0,line+1);
        }
        if(pattern.find_first_of("\n\r{}[]")!=string::npos || shown.find_first_of("\n\r{}[]")!=string::npos)
            return false;
        if(starts_with(pattern,"of "))pattern.erase(0,3);
        else if(pattern.size()>=2 && pattern.front()=='"' && pattern.back()=='"')
            pattern=pattern.substr(1,pattern.size()-2);
        match(pattern,shown,0);
    }
    if(matches.size()!=1)return false;
    display=*matches.begin();return true;
}
}

// Translate complete supported title forms, never individual words in a name.
bool try_randart_title(const string &canonical, string &display, const string &player_name)
{
    if (Options.language!=lang_t::ZH) return false;
    for (const char *key:{"_jewellery_animal_","_jewellery_special_name_",
         "_suspicion_no_conflict_name_","_time_name_","_virtue_name_","_vice_name_",
         "_destiny_name_","_number_or_qualifier_","_thing_names_","_verbing_",
         "_adjective_","_fruit_name_","_suspicion_name_","divine_esteem","_xom_esteem_",
         "_sky_or_light_","_virtue_or_vice_","_power_or_anger_","_battle_or_war_",
         "death_or_doom","_strategy_or_justice_","_diviner_name_","_evil_stuff_",
         "_catastrophe_name_","_chaos_name_","_balance_or_order_","_economics_or_politics_",
         "_place_name_","_discomfort_weapon_","_philosophy_name_","_science_name_",
         "_magic_name_","_substance_name_","_people_name_","_weapon_animal_","_plant_name_",
         "_item_name_","other_weapon_name","short_monster_name_","_monster_name_",
         "_short_killer_name_","killer_name","_long_killer_name_","_real_weapon_name_",
         "_joke_weapon_name_","_flower_name_","_critter_name_","_discomfort_and_inaction_",
         "_good_or_divine_","_armour_name_shielding_","_armour_property_name_",
         "_wacky_armour_name_","_celestial_bodies_","_instrument_name_","_evil_being_",
         "_weather_name_","_profession_name_","_game_name_","_politics_name_armour_",
         "_other_armour_name_","_plain_armour_name_","_armour_animal_",
         "_valour_name_","_anger_quality_"})
    {
        if (appearance_words.count(key)) continue;
        map<string,string> words;
        if (!_appearance_dictionary(key,words,0)) return false;
        appearance_words.emplace(key,std::move(words));
    }
    set<string> matches;
    const auto literal=[&](const char *key,const string &text,const string &prefix="") {
        const auto &words=appearance_words.at(key);const auto found=words.find(text);
        if (found!=words.end()) matches.insert(prefix+found->second);
    };
    for (const char *key:{"_jewellery_special_name_","_suspicion_no_conflict_name_",
                         "_time_name_","_virtue_name_","_vice_name_","_destiny_name_",
                         "_suspicion_name_","_sky_or_light_","_virtue_or_vice_",
                         "_power_or_anger_","_battle_or_war_","death_or_doom",
                         "_strategy_or_justice_","_evil_stuff_","_catastrophe_name_",
                         "_chaos_name_","_balance_or_order_","_economics_or_politics_",
                         "_place_name_","_discomfort_weapon_","_philosophy_name_",
                         "_science_name_","_magic_name_","_substance_name_","other_weapon_name",
                         "_real_weapon_name_","_joke_weapon_name_","_flower_name_",
                         "_discomfort_and_inaction_","_good_or_divine_","_armour_name_shielding_",
                         "_armour_property_name_","_wacky_armour_name_","_instrument_name_",
                         "_evil_being_","_weather_name_","_game_name_","_politics_name_armour_",
                         "_other_armour_name_","_plain_armour_name_"})
        literal(key,canonical);
    // A reviewed whole phrase (Quick Death, the People) takes precedence over
    // a coincidentally matching adjective or definite-article construction.
    if (!matches.empty())
    {
        if (matches.size()!=1) return false;
        display=*matches.begin();return true;
    }
    if (starts_with(canonical,"the "))
    {
        const string rest=canonical.substr(4);
        literal("_jewellery_animal_",rest);literal("_fruit_name_",rest);
        for (const char *key:{"_people_name_","_weapon_animal_","_plant_name_","_item_name_",
                             "_critter_name_","_profession_name_","_celestial_bodies_","_armour_animal_"})
            literal(key,rest);
        if (ends_with(rest," Gods"))
        {
            const auto &words=appearance_words.at("_verbing_");
            const auto found=words.find(rest.substr(0,rest.size()-5));
            if (found!=words.end()) matches.insert(found->second+"的诸神");
        }
        // An adjective is only meaningful with its complete noun suffix.
        if (ends_with(rest," God"))
        {
            const auto &words=appearance_words.at("_adjective_");
            const auto found=words.find(rest.substr(0,rest.size()-4));
            if (found!=words.end()) matches.insert(found->second+"神");
        }
        for (const auto &adj:appearance_words.at("_adjective_"))
            if (starts_with(rest,adj.first+" "))
                for (const char *key:{"_fruit_name_","_critter_name_","_profession_name_","_armour_animal_"})
                    literal(key,rest.substr(adj.first.size()+1),adj.second);
    }

    // These are complete official naming forms, with their noun dictionaries.
    for (const auto &adj:appearance_words.at("_adjective_"))
        if (starts_with(canonical,adj.first+" "))
            for (const char *key:{"_strategy_or_justice_","_power_or_anger_","death_or_doom","_good_or_divine_"})
                literal(key,canonical.substr(adj.first.size()+1),adj.second);
    for (const auto &verb:appearance_words.at("_verbing_"))
    {
        if (starts_with(canonical,verb.first+" "))
            literal("death_or_doom",canonical.substr(verb.first.size()+1),verb.second+"的");
        const string prefix="the "+verb.first+" ";
        if (starts_with(canonical,prefix))
            for (const char *key:{"_diviner_name_","_battle_or_war_","_people_name_","_weapon_animal_","_fruit_name_"})
                literal(key,canonical.substr(prefix.size()),verb.second+"的");
    }

    for (const auto &monster:appearance_words.at("short_monster_name_"))
        for (const auto &ending:appearance_words.at("_short_killer_name_"))
            if (canonical==monster.first+ending.first)
                matches.insert(monster.second+ending.second);
    for (const char *key:{"killer_name","_long_killer_name_"})
        for (const auto &ending:appearance_words.at(key))
        {
            if (!ends_with(canonical,ending.first)) continue;
            const string owner=canonical.substr(0,canonical.size()-ending.first.size());
            if (owner.empty()) continue;
            if (!player_name.empty() && owner==player_name)
                matches.insert(owner+ending.second); // preserve the literal player's name
            else
            {
                const auto &monsters=appearance_words.at("_monster_name_");
                const auto found=monsters.find(owner);
                if (found!=monsters.end()) matches.insert(found->second+ending.second);
            }
        }

    // A god name is a typed display parameter, not free text to translate.
    const auto god_display=[](const string &english,string &translated) {
        ui_translation_group god;
        god.require("display.god",english.c_str());
        if (!god.available()) return false;
        translated=god.get("display.god",english.c_str());return true;
    };
    size_t owner=canonical.find("'s ");size_t suffix=3;
    if (owner==string::npos) {owner=canonical.find("' ");suffix=2;}
    if (owner!=string::npos)
    {
        string god;const string english=canonical.substr(0,owner);
        if (god_display(english,god))
        {
            const string rest=canonical.substr(owner+suffix);
            literal("divine_esteem",rest,god+"之");
            if (english=="Xom") literal("_xom_esteem_",rest,god+"之");
            if (english=="Trog") literal("_anger_quality_",rest,god+"之");
        }
        if (!player_name.empty() && english==player_name && matches.empty())
        {
            const string rest=canonical.substr(owner+suffix);
            literal("_valour_name_",rest,player_name+"之");
            literal("death_or_doom",rest,player_name+"之");
        }
    }
    const size_t comma=canonical.find(", ");
    if (comma!=string::npos)
    {
        string god;
        if (god_display(canonical.substr(0,comma),god))
            literal("_verbing_",canonical.substr(comma+2),god+"之");
    }

    // Xom can choose two independent verbs, including repeated words.
    const auto &verbs=appearance_words.at("_verbing_");
    if (starts_with(canonical,"Xom, "))
    {
        const string rest=canonical.substr(5);size_t join=rest.find(" and ");
        while (join!=string::npos)
        {
            const auto left=verbs.find(rest.substr(0,join));
            const auto right=verbs.find(rest.substr(join+5));
            string xom;
            if (left!=verbs.end() && right!=verbs.end() && god_display("Xom",xom))
                matches.insert(xom+"之"+left->second+"与"+right->second);
            join=rest.find(" and ",join+1);
        }
    }
    if (ends_with(canonical," Xom"))
    {
        const string head=canonical.substr(0,canonical.size()-4);string xom;
        if (god_display("Xom",xom))
        {
            const auto verb=verbs.find(head);
            if (verb!=verbs.end()) matches.insert(verb->second+"的"+xom);
            const auto &adjectives=appearance_words.at("_adjective_");
            const auto adjective=adjectives.find(head);
            if (adjective!=adjectives.end()) matches.insert(adjective->second+xom);
        }
    }

    for (const auto &number:appearance_words.at("_number_or_qualifier_"))
    {
        // Prefer the explicit deity/fruit forms over a bare definite article.
        if (number.first=="the" && !matches.empty()) continue;
        if (!starts_with(canonical,number.first+" ")) continue;
        const string rest=canonical.substr(number.first.size()+1);
        literal("_thing_names_",rest,number.second);
        for (const auto &verb:appearance_words.at("_verbing_"))
            if (starts_with(rest,verb.first+" "))
                literal("_thing_names_",rest.substr(verb.first.size()+1),number.second+verb.second+"的");
    }
    if (matches.empty())
    {
        string named_god;
        if (god_display(canonical,named_god)) matches.insert(named_god);
        const auto branch=ui_text_arg("b.branch.name",canonical);
        if (branch.available) matches.insert(branch.text);
    }
    if (matches.empty()) return _ashenzari_display_title(canonical,display);
    if (matches.size()!=1) return false;
    display=*matches.begin();return true;
}



namespace
{
// Interpret reviewed fixed-level templates against an existing saved name.
// Matching never calls the name generator or changes the random stream.
bool _fixed_level_book_title(const string &canonical, string &display,
                             const string &player_name, bool has_typed_owner)
{
    if(!RandartDB.translation)return false;
    vector<pair<string,string>> forms{{canonical,""}};
    size_t possessive=canonical.find("'s ");size_t width=3;
    if(possessive==string::npos){possessive=canonical.find("' ");width=2;}
    if(possessive!=string::npos)
    {
        const string who=canonical.substr(0,possessive);string named;
        if(!player_name.empty() && who==player_name)named=who;
        else {
            const auto author=book_owners.find(who);
            if(author!=book_owners.end() && author->second.size()==1)named=*author->second.begin();
            else for(const char *context:{"display.god","display.monster"}) {
                const string key=string(context)=="display.monster" ? lowercase_string(who) : who;
                const auto value=ui_text_arg(context,key);
                if(value.available){named=value.text;break;}
            }
        }
        if(named.empty() && has_typed_owner && !who.empty()
            && who.find_first_of("@{}[]\n\r") == string::npos) named=who;
        if(!named.empty()) {
            const string title=canonical.substr(possessive+width);
            for(const char *article:{"","The ","A ","An "})
                forms.emplace_back(string(article)+title,named+"的");
        }
    }
    set<string> matches;
    for(const auto &form:forms)
    {
        std::function<void(string,string,int)> match=[&](string pattern,string shown,int depth) {
            if(depth>8)return;
            pattern=uppercase_first(pattern);
            const size_t start=pattern.find('@');
            if(start==string::npos) {
                if(pattern==form.first && !shown.empty() && shown.find_first_of("@{}[]\n\r")==string::npos)
                    matches.insert(form.second+shown);
                return;
            }
            if(!starts_with(form.first,pattern.substr(0,start)))return;
            const size_t end=pattern.find('@',start+1);if(end==string::npos)return;
            const string marker=pattern.substr(start,end-start+1);
            const size_t displayed=shown.find(marker);if(displayed==string::npos)return;
            string key=pattern.substr(start+1,end-start-1);lowercase(key);
            map<string,string> typed;const map<string,string> *words=nullptr;
            if(key=="level") {
                const char *english[]={"One","Two","Three","Four","Five","Six","Seven","Eight","Nine"};
                const char *chinese[]={"一","二","三","四","五","六","七","八","九"};
                for(int i=0;i<9;++i)typed.emplace(english[i],chinese[i]);
                words=&typed;
            } else if(key=="player_name") {
                if(player_name.empty())return;
                typed.emplace(player_name,player_name);words=&typed;
            } else {
                const set<string> roots={"_the_book_magic_","_book_magic_","_beginners_",
                    "book_name","book_noun","_book_noun_on_","_the_powerful_caster_",
                    "_last_or_lost_","_testament_noun_","_occult_adjective_"};
                if(!roots.count(key))return;
                auto found=appearance_words.find(key);
                if(found==appearance_words.end()) {
                    map<string,string> paired;
                    if(!_appearance_dictionary(key,paired,0))return;
                    found=appearance_words.emplace(key,std::move(paired)).first;
                }
                words=&found->second;
            }
            for(const auto &word:*words) {
                string expanded=pattern;expanded.replace(start,marker.size(),word.first);
                expanded=uppercase_first(expanded);const size_t next=expanded.find('@');
                if(!starts_with(form.first,expanded.substr(0,next)))continue;
                string translated=shown;translated.replace(displayed,marker.size(),word.second);
                match(expanded,translated,depth+1);
            }
        };
        for(const char *key:{"starting level book","easy level book","moderate level book",
                             "_difficult level book owner_","difficult level book"})
        {
            const datum en=_database_fetch(RandartDB.get(),key);
            const datum zh=_database_fetch(RandartDB.translation->get(),key);
            if(en.dsize<=0 || zh.dsize<=0)continue;
            const auto source=split_string("\n\n",string((const char *)en.dptr,en.dsize));
            const auto target=split_string("\n\n",string((const char *)zh.dptr,zh.dsize));
            if(source.size()!=target.size())continue;
            for(size_t i=0;i<source.size();++i) {
                string pattern=source[i],shown=target[i];
                if(starts_with(pattern,"w:")) {
                    const size_t line=pattern.find('\n');
                    if(line==string::npos || shown.substr(0,line+1)!=pattern.substr(0,line+1))continue;
                    pattern.erase(0,line+1);shown.erase(0,line+1);
                }
                if(pattern=="@_difficult level book owner_@")continue; // separately interpreted above
                if(pattern.find_first_of("\n\r{}[]")!=string::npos || shown.find_first_of("\n\r{}[]")!=string::npos)continue;
                match(pattern,shown,0);
            }
        }
    }
    if(matches.size()!=1)return false;
    display=*matches.begin();return true;
}
}


namespace
{
// Match a reviewed random-subject program without generating a second subject.
// Ambiguous individual names fall back without disabling unrelated candidates.
bool _random_book_subject(const string &canonical, string &display,
                          const string &player_name, bool xom)
{
    if(!RandartDB.translation)return false;
    set<string> matches;size_t work=0;bool exhausted=false;
    std::function<void(string,string,int)> match=[&](string pattern,string shown,int depth) {
        if(++work>20000){exhausted=true;return;}
        if(depth>16){exhausted=true;return;}
        const size_t start=pattern.find('@');
        if(start==string::npos) {
            if(pattern==canonical && !shown.empty() && shown.find_first_of("@{}[]\n\r")==string::npos)
                matches.insert(shown);
            return;
        }
        if(!starts_with(canonical,pattern.substr(0,start)))return;
        const size_t end=pattern.find('@',start+1);if(end==string::npos)return;
        const string marker=pattern.substr(start,end-start+1);
        const size_t translated_pos=shown.find(marker);if(translated_pos==string::npos)return;
        string key=pattern.substr(start+1,end-start-1);lowercase(key);
        vector<pair<string,string>> words;
        if(key=="player_death") {
            if(player_name.empty())return;
            // This is exactly the canonical player-death production. The
            // reviewed suffix supplies its Chinese words; the name stays literal.
            words.emplace_back(player_name+"@killer_name@",player_name+"@killer_name@");
        } else {
            const datum en=_database_fetch(RandartDB.get(),key);
            const datum zh=_database_fetch(RandartDB.translation->get(),key);
            if(en.dsize<=0 || zh.dsize<=0)return;
            const auto left=split_string("\n\n",string((const char *)en.dptr,en.dsize));
            const auto right=split_string("\n\n",string((const char *)zh.dptr,zh.dsize));
            if(left.size()!=right.size())return;
            for(size_t i=0;i<left.size();++i) {
                string a=left[i],b=right[i];
                if(starts_with(a,"w:")) {
                    const size_t line=a.find('\n');
                    if(line==string::npos || b.substr(0,line+1)!=a.substr(0,line+1))return;
                    a.erase(0,line+1);b.erase(0,line+1);
                }
                if(a.empty() || b.empty() || a.find_first_of("\n\r{}[]")!=string::npos
                    || b.find_first_of("\n\r{}[]")!=string::npos)return;
                words.emplace_back(a,b);
            }
        }
        for(const auto &word:words) {
            string expanded=pattern;expanded.replace(start,marker.size(),word.first);
            const size_t next=expanded.find('@');
            if(!starts_with(canonical,expanded.substr(0,next)))continue;
            string translated=shown;translated.replace(translated_pos,marker.size(),word.second);
            match(expanded,translated,depth+1);
            if(exhausted)return;
        }
    };
    match(xom ? "@Xom_book_title@" : "@random_book_title@",
          xom ? "@Xom_book_title@" : "@random_book_title@",0);
    if(exhausted || matches.size()!=1)return false;
    display=*matches.begin();return true;
}
}

// Random books have their own cached full title, without an item-base prefix.
bool try_randbook_title(const string &canonical, string &display, const string &player_name, bool has_typed_owner)
{
    if (Options.language!=lang_t::ZH) return false;
    const auto load=[](const char *key) {
        if (appearance_words.count(key)) return true;
        map<string,string> words;
        if (!_appearance_dictionary(key,words,0)) return false;
        appearance_words.emplace(key,std::move(words));return true;
    };
    for (const char *key:{"book_name","book_noun","_book_noun_on_","air","conjuration",
         "earth","fire","hexes","ice","necromancy","alchemy","summoning","translocation","forgecraft"})
        if (!load(key)) return false;
    if (book_subjects.empty())
        for (const char *key:{"air","conjuration","earth","fire","hexes","ice",
                             "necromancy","alchemy","summoning","translocation","forgecraft"})
            for (const auto &word:appearance_words.at(key))book_subjects[word.first].insert(word.second);

    for (const char *key:{"air adj","conjuration adj","earth adj","fire adj","hexes adj","ice adj",
                         "necromancy adj","alchemy adj","summoning adj","translocation adj","forgecraft adj"})
        if(!load(key)) return false;
    if(book_modifiers.empty())
        for (const char *key:{"air adj","conjuration adj","earth adj","fire adj","hexes adj","ice adj",
                             "necromancy adj","alchemy adj","summoning adj","translocation adj","forgecraft adj"})
            for(const auto &word:appearance_words.at(key))book_modifiers[word.first].insert(word.second);
    for (const char *key:{"_the_powerful_caster_","highlevel conjuration earth owner","conjuration earth owner",
         "highlevel conjuration fire owner","highlevel conjuration necromancy owner","highlevel conjuration summoning owner",
         "alchemy earth owner","highlevel earth summoning owner","highlevel fire summoning owner","fire ice owner",
         "highlevel alchemy necromancy owner","alchemy necromancy owner","highlevel conjuration owner","conjuration owner",
         "highlevel earth owner","earth owner","highlevel fire owner","ice owner","air owner","highlevel necromancy owner",
         "necromancy owner","highlevel alchemy owner","alchemy owner","highlevel summoning owner","summoning owner",
         "highlevel translocation owner","translocation owner","hexes owner"})
        if(!load(key)) return false;
    if(book_owners.empty())
        for (const char *key:{"_the_powerful_caster_","highlevel conjuration earth owner","conjuration earth owner",
             "highlevel conjuration fire owner","highlevel conjuration necromancy owner","highlevel conjuration summoning owner",
             "alchemy earth owner","highlevel earth summoning owner","highlevel fire summoning owner","fire ice owner",
             "highlevel alchemy necromancy owner","alchemy necromancy owner","highlevel conjuration owner","conjuration owner",
             "highlevel earth owner","earth owner","highlevel fire owner","ice owner","air owner","highlevel necromancy owner",
             "necromancy owner","highlevel alchemy owner","alchemy owner","highlevel summoning owner","summoning owner",
             "highlevel translocation owner","translocation owner","hexes owner"})
            for(const auto &word:appearance_words.at(key))
                if(word.first!="__NONE")book_owners[word.first].insert(word.second);

    const auto &subjects=book_subjects;
    const auto subject=[&](const string &english,string &translated,bool random,bool xom) {
        auto found=subjects.find(english);
        if (found!=subjects.end()) {
            if (found->second.size()!=1) return false;
            translated=*found->second.begin();return true;
        }

        set<string> matches;
        const bool article=starts_with(english,"the ");
        const string rest=article ? english.substr(4) : english;
        for(const auto &modifier:book_modifiers)
        {
            if(modifier.second.size()!=1 || !starts_with(rest,modifier.first+" "))continue;
            const string suffix=rest.substr(modifier.first.size()+1);
            if(!article && starts_with(suffix,"the "))continue;
            const string tail=(article ? "the " : "")+suffix;
            const auto noun=subjects.find(tail);
            if(noun!=subjects.end() && noun->second.size()==1)
                matches.insert(*modifier.second.begin()+*noun->second.begin());
        }
        size_t pos=english.find(" and ");
        while(pos!=string::npos) {
            const auto a=subjects.find(english.substr(0,pos));
            const auto b=subjects.find(english.substr(pos+5));
            if(a!=subjects.end() && b!=subjects.end() && a->second.size()==1 && b->second.size()==1)
                matches.insert(*a->second.begin()+"与"+*b->second.begin());
            pos=english.find(" and ",pos+1);
        }
        if(matches.empty() && random)
            return _random_book_subject(english,translated,player_name,xom);
        if(matches.size()!=1) return false;
        translated=*matches.begin();return true;
    };
    const auto owner=[&](string english,string &translated) {
        if(ends_with(english,"'s"))english.erase(english.size()-2);
        else if(ends_with(english,"'"))english.pop_back();
        else return false;
        if(english.empty()) return false;
        if(!player_name.empty() && english==player_name) {translated=english;return true;}
        const auto reviewed=book_owners.find(english);
        if(reviewed!=book_owners.end()) {
            if(reviewed->second.size()!=1)return false;
            translated=*reviewed->second.begin();return true;
        }
        for(const char *context:{"display.god","display.monster"}) {
            string key=english;if(string(context)=="display.monster")lowercase(key);
            ui_translation_group text;text.require(context,key.c_str());
            if(text.available()) {translated=text.get(context,key.c_str());return true;}
        }
        // The item's existing is_named property supplies the author role.
        // Preserve an opaque proper name only inside a reviewed title grammar.
        if(has_typed_owner && english.find_first_of("@{}[]\n\r")==string::npos)
        { translated=english;return true; }
        return false;
    };
    set<string> matches;
    for (const char *key:{"book_noun","_book_noun_on_"})
    {
        const string link=string(key)=="book_noun" ? " of " : " on ";
        for(const auto &noun:appearance_words.at(key))
        {
            const string prefix=noun.first+link;
            // Verify that this prefix is part of the reviewed naming grammar.
            const auto pattern=appearance_words.at("book_name").find(noun.first+link.substr(0,link.size()-1));
            if(pattern==appearance_words.at("book_name").end())continue;
            const string &ending=pattern->second;
            if(starts_with(canonical,prefix)) {
                string theme;
                if(subject(canonical.substr(prefix.size()),theme,string(key)=="book_noun",false))matches.insert(theme+ending);
            }
            const string marker=" "+prefix;
            const size_t pos=canonical.find(marker);
            if(pos!=string::npos) {
                string who,theme;
                if(owner(canonical.substr(0,pos),who) && subject(canonical.substr(pos+marker.size()),theme,string(key)=="book_noun",canonical.substr(0,pos)=="Xom's"))
                    matches.insert(who+"的"+theme+ending);
            }
        }
    }
    if(matches.empty()) return _fixed_level_book_title(canonical,display,player_name,has_typed_owner);
    if(matches.size()!=1) return false;
    display=*matches.begin();return true;
}

/////////////////////////////////////////////////////////////////////////////
// Help DB specific functions.

string getHelpString(const string &topic)
{
    string help = _query_database(HelpDB, topic, false, true);
    if (help.empty())
        help = ui_render("d.database", "Error! The help for %s is missing!",
            "Error! The help for \"" + topic + "\" is missing!", {ui_literal_arg(topic)});
    return help;
}

/////////////////////////////////////////////////////////////////////////////
// FAQ DB specific functions.
vector<string> getAllFAQKeys()
{
    if (!FAQDB.get())
    {
        vector<string> empty;
        return empty;
    }

    return _database_find_keys(FAQDB.get(), "^q.+", false);
}

string getFAQ_Question(const string &key)
{
    return _query_database(FAQDB, key, false, true);
}

string getFAQ_Answer(const string &question)
{
    string key = "a" + question.substr(1, question.length()-1);
    string val = unwrap_desc(_query_database(FAQDB, key, false, true));

    // Remove blank lines between items on a bulleted list, for small
    // terminals' sake. Far easier to store them as separated paragraphs
    // in the source.
    // Also, use a nicer bullet as we're already here.
    val = replace_all(val, "\n\n*", "\n•");

    return val;
}

/////////////////////////////////////////////////////////////////////////////
// Miscellaneous DB specific functions.

textdb_speech getPairedMiscString(const string &key, const string &suffix)
{
    int replacements = 0;
    auto result = _paired_speech(MiscDB.get(),
        MiscDB.translation ? MiscDB.translation->get() : nullptr, key, replacements, 0,
                                 suffix, true);
    _execute_paired_lua(result, MiscDB.translation
        ? MiscDB.translation->get() : nullptr);
    return result;
}

namespace
{
bool _gizmo_pairs(const char *key, vector<pair<string,string>> &words)
{
    const datum en = _database_fetch(MiscDB.get(), key);
    const datum zh = _database_fetch(MiscDB.translation ? MiscDB.translation->get() : nullptr, key);
    if (en.dsize <= 0 || zh.dsize <= 0) return false;
    const auto left = split_string("\n\n", string((const char *)en.dptr, en.dsize));
    const auto right = split_string("\n\n", string((const char *)zh.dptr, zh.dsize));
    if (left.empty() || left.size() != right.size()) return false;
    for (size_t i=0;i<left.size();++i) {
        // Current naming candidates are single literals or one modifier include.
        // Unknown upstream grammar must keep its full English name.
        if (left[i].find('\n') != string::npos || right[i].find('\n') != string::npos)
            return false;
        words.emplace_back(left[i],right[i]);
    }
    return true;
}

bool _gizmo_dictionary(const vector<pair<string,string>> &templates,
                       map<string,string> &dictionary)
{
    const string marker="@gizmo_modifier@";
    // Prefer an actual word (automatic, transformer) over a coincidentally
    // identical prefix construction. This needs no saved generation metadata.
    for (const auto &word:templates)
        if (word.first.find('@') == string::npos) {
            if (word.second.empty() || word.second.find('@') != string::npos) return false;
            dictionary.emplace(word.first,word.second);
        }
    for (const auto &word:templates) {
        if (word.first.find('@') == string::npos) continue;
        if (word.first.find(marker) == string::npos || word.second.find(marker) == string::npos)
            return false;
        for (const auto &prefix:gizmo_modifiers) {
            const string key=replace_all(word.first,marker,prefix.first);
            const string value=replace_all(word.second,marker,prefix.second);
            if (key.find('@') != string::npos || value.find('@') != string::npos) return false;
            dictionary.emplace(key,value);
        }
    }
    return !dictionary.empty();
}

bool _load_gizmo_names()
{
    if (gizmo_names_loaded) return !gizmo_adjectives.empty() && !gizmo_nouns.empty()
                                  && !gizmo_modifiers.empty();
    gizmo_names_loaded=true;
    vector<pair<string,string>> modifiers,adjectives,nouns;
    if (!_gizmo_pairs("gizmo_modifier",modifiers) || !_gizmo_pairs("gizmo_adjective",adjectives)
        || !_gizmo_pairs("gizmo_noun",nouns)) return false;
    for (const auto &word:modifiers) {
        if (word.first.empty() || word.first.find('@') != string::npos
            || word.second.empty() || word.second.find('@') != string::npos) return false;
        gizmo_modifiers.emplace(word.first,word.second);
    }
    if (!_gizmo_dictionary(adjectives,gizmo_adjectives) || !_gizmo_dictionary(nouns,gizmo_nouns)) {
        gizmo_adjectives.clear();gizmo_nouns.clear();gizmo_modifiers.clear();return false;
    }
    return true;
}

// The integer records the optional outer prefix chosen by acquire.cc. Inner
// prefixes already expanded from TextDB templates do not count toward it.
set<pair<string,int>> _gizmo_token(const string &token, const map<string,string> &words)
{
    set<pair<string,int>> matches;
    const auto plain=words.find(token);
    if (plain!=words.end()) matches.emplace(plain->second,0);
    for (const auto &prefix:gizmo_modifiers) {
        if (!starts_with(token,prefix.first)) continue;
        const auto part=words.find(token.substr(prefix.first.size()));
        if (part!=words.end()) matches.emplace(prefix.second+part->second,1);
    }
    return matches;
}

bool _gizmo_serial(const string &text, bool short_form)
{
    size_t pos=0;
    if (short_form && (starts_with(text,"Mk.") || starts_with(text,"Ver."))) {
        pos=starts_with(text,"Mk.")?3:4;
        if (text.size()-pos<1 || text.size()-pos>2) return false;
    } else {
        while(pos<text.size() && text[pos]>='A' && text[pos]<='Z') ++pos;
        if (pos<1 || pos>2 || pos>=text.size() || text[pos++]!='-') return false;
        const size_t digits=text.size()-pos;
        if (digits!=2 && digits!=4) return false;
    }
    for (;pos<text.size();++pos) if(text[pos]<'0' || text[pos]>'9') return false;
    return true;
}
}

bool try_gizmo_display_name(const string &canonical, string &display)
{
    if (Options.language != lang_t::ZH || !_load_gizmo_names()) return false;
    const size_t space=canonical.find(' ');
    if (space==string::npos || canonical.find(' ',space+1)!=string::npos) return false;
    const string left=canonical.substr(0,space),right=canonical.substr(space+1);
    set<string> matches;
    if (_gizmo_serial(left,false)) {
        for (const auto &noun:_gizmo_token(right,gizmo_nouns))
            if (noun.second==1) matches.insert(left+" "+noun.first);
    } else if (_gizmo_serial(right,true)) {
        for (const auto &noun:_gizmo_token(left,gizmo_nouns))
            if (noun.second==1) matches.insert(noun.first+" "+right);
    } else {
        for (const auto &adj:_gizmo_token(left,gizmo_adjectives))
        for (const auto &noun:_gizmo_token(right,gizmo_nouns))
            if (adj.second+noun.second<=1) matches.insert(adj.first+noun.first);
    }
    if (matches.size()!=1) return false;
    display=*matches.begin();return true;
}

string getMiscString(const string &misc, const string &suffix, bool untranslated)

{
    if (!untranslated && Options.language == lang_t::ZH)
        return getPairedMiscString(misc, suffix).text();
    int num_replacements = 0;

    string txt = _getRandomisedStr(MiscDB, misc, suffix, num_replacements, 0, untranslated);
    _execute_embedded_lua(txt);

    return txt;
}

/////////////////////////////////////////////////////////////////////////////
// Hints DB specific functions.

string getHintString(const string &key, bool untranslated, bool *used_translation)
{
    return unwrap_desc(query_textdb_entry(HintsDB.get(),
        HintsDB.translation ? HintsDB.translation->get() : nullptr,
        key, true, true, untranslated || Options.language == lang_t::EN,
        used_translation));
}

/////////////////////////////////////////////////////////////////////////////
// Egos DB specific functions.

string getEgoString(const string &key)
{
    return unwrap_desc(_query_database(EgosDB, key, true, true));
}
