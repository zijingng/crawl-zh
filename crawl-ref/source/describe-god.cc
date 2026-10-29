/**
 * @file
 * @brief Functions used to print information about gods.
 **/

#include "AppHdr.h"
#include "i18n.h"

#include "describe-god.h"

#include <iomanip>

#include "act-iter.h"
#include "ability.h"
#include "branch.h"
#include "cio.h"
#include "database.h"
#include "decks.h"
#include "describe.h"
#include "english.h"
#include "env.h"
#include "god-abil.h"
#include "god-companions.h"
#include "god-conduct.h"
#include "god-passive.h"
#include "god-type.h"
#include "items.h"
#include "item-name.h"
#include "libutil.h"
#include "menu.h"
#include "message.h"
#include "religion.h"
#include "skills.h"
#include "spl-util.h"
#include "stringutil.h"
#include "tag-version.h"
#include "terrain.h"
#include "tilepick.h"
#include "unicode.h"
#include "xom.h"

using namespace ui;

static int _piety_level(int piety)
{
    return (piety >= piety_breakpoint(5)) ? 7 :
           (piety >= piety_breakpoint(4)) ? 6 :
           (piety >= piety_breakpoint(3)) ? 5 :
           (piety >= piety_breakpoint(2)) ? 4 :
           (piety >= piety_breakpoint(1)) ? 3 :
           (piety >= piety_breakpoint(0)) ? 2 :
           (piety >                    0) ? 1
                                          : 0;
}

static int _gold_level()
{
    return (you.gold >= 50000) ? 7 :
           (you.gold >= 10000) ? 6 :
           (you.gold >=  5000) ? 5 :
           (you.gold >=  1000) ? 4 :
           (you.gold >=   500) ? 3 :
           (you.gold >=   100) ? 2 : 1;
}

static int _invocations_level()
{
    int invo = you.skills[SK_INVOCATIONS];
    return (invo == 27) ? 7 :
           (invo >= 24) ? 6 :
           (invo >= 20) ? 5 :
           (invo >= 16) ? 4 :
           (invo >= 12) ? 3 :
           (invo >= 8)  ? 2
                        : 1;
}

int god_favour_rank(god_type which_god)
{
    if (which_god == GOD_GOZAG)
        return _gold_level();
    else if (which_god == GOD_USKAYAW)
        return _invocations_level();
    else
        return _piety_level(you.raw_piety);
}

static string _describe_favour(god_type which_god)
{
    if (player_under_penance())
    {
        const int penance = you.penance[which_god];
        return (penance >= 50) ? ui_translate("d.god", "Godly wrath is upon you!") :
               (penance >= 20) ? ui_translate("d.god", "You've transgressed heavily! Be penitent!") :
               (penance >=  5) ? ui_translate("d.god", "You are under penance.")
                               : ui_translate("d.god", "You should show more discipline.");
    }

    if (which_god == GOD_XOM)
        return uppercase_first(ui_translate("d.god.xom.favour", describe_xom_favour().c_str()));


    const string godname = god_name(which_god);
    switch (god_favour_rank(which_god))
    {
        case 7:  return ui_render("d.god", "A prized avatar of %s.",
                    "A prized avatar of " + godname + ".", {ui_text_arg("display.god", godname)});
        case 6:  return ui_render("d.god", "A favoured servant of %s.",
                    "A favoured servant of " + godname + ".", {ui_text_arg("display.god", godname)});
        case 5:

            if (you_worship(GOD_DITHMENOS))
                return ui_render("d.god", "A glorious shadow in the eyes of %s.",
                    "A glorious shadow in the eyes of " + godname + ".", {ui_text_arg("display.god", godname)});
            else
                return ui_render("d.god", "A shining star in the eyes of %s.",
                    "A shining star in the eyes of " + godname + ".", {ui_text_arg("display.god", godname)});

        case 4:

            if (you_worship(GOD_DITHMENOS))
                return ui_render("d.god", "A rising shadow in the eyes of %s.",
                    "A rising shadow in the eyes of " + godname + ".", {ui_text_arg("display.god", godname)});
            else
                return ui_render("d.god", "A rising star in the eyes of %s.",
                    "A rising star in the eyes of " + godname + ".", {ui_text_arg("display.god", godname)});

        case 3:  return ui_render("d.god", "%s is pleased with you.",
                    uppercase_first(godname) + " is pleased with you.", {ui_text_arg("display.god", godname)});
        case 2:  return ui_render("d.god", "%s is aware of your devotion.",
                    uppercase_first(godname) + " is aware of your devotion.", {ui_text_arg("display.god", godname)});
        default: return ui_render("d.god", "%s is noncommittal.",
                    uppercase_first(godname) + " is noncommittal.", {ui_text_arg("display.god", godname)});
    }
}

// The various titles granted by the god of your choice. Note that Xom
// doesn't use piety the same way as the other gods, so these are just
// placeholders.
static const char *divine_title[][8] =
{
    // No god.
    {"Buglet",             "Firebug",               "Bogeybug",                 "Bugger",
        "Bugbear",            "Bugged One",            "Giant Bug",                "Lord of the Bugs"},

    // Zin.
    {"Blasphemer",         "Anchorite",             "Apologist",                "Pious",
        "Devout",             "Orthodox",              "Immaculate",               "Bringer of Law"},

    // The Shining One.
    {"Honourless",         "Acolyte",               "Righteous",                "Unflinching",
        "Holy Warrior",       "Exorcist",              "Demon Slayer",             "Bringer of Light"},

    // Kikubaaqudgha -- death scholar theme.
    {"Tormented",          "Purveyor of Pain",       "Pupil of Sorrows",        "Merchant of Misery",
     "Scholar of Souls",   "Artisan of Death",       "Demagogue of Despair",    "Lord of Darkness"},

    // Yredelemnul -- fervent death knight theme.
    {"Traitor",            "Torchbearer",            "Despoiler",               "Black Crusader",
     "Fallen @Genus@",     "Harbinger of Doom",      "Inexorable Tide",         "Bringer of Blasphemy"},

    // Xom.
    {"Toy",                "Toy",                   "Toy",                      "Toy",
        "Toy",                "Toy",                   "Toy",                      "Toy"},

    // Vehumet -- battle mage theme.
    {"Meek",               "Sorcerer's Apprentice", "Scholar of Destruction",   "Caster of Ruination",
        "Traumaturge",        "Battlemage",            "Warlock",                  "Luminary of Lethal Lore"},

    // Okawaru -- battle theme.
    {"Coward",             "Struggler",             "Combatant",                "@Genus@-At-Arms",
        "Knight",             "Myrmidon",             "Warmonger",                "Victor of a Thousand Battles"},

    // Makhleb -- chaos theme.
    {"Orderly",            "Spawn of Chaos",        "Disciple of Destruction",  "Fanfare of Bloodshed",
        "Fiendish",           "Demolition @Genus@",    "Pandemonic",               "Champion of Chaos"},

    // Sif Muna -- generalist scholarly theme.
    {"Ignorant",           "Disciple",              "Student",                  "Adept",
        "Scribe",             "Scholar",               "Sage",                     "Genius of the Arcane"},

    // Trog -- anger theme.
    {"Puny",               "Troglodyte",            "Angry Troglodyte",         "Frenzied",
        "@Genus@ of Prey",    "Rampant",               "Wild @Genus@",             "Bane of Scribes"},

    // Nemelex Xobeh -- alluding to Tarot and cards.
    {"Unlucky @Genus@",    "Pannier",               "Jester",                   "Fortune-Teller",
        "Soothsayer",         "Magus",                 "Cardsharp",                "Hand of Fortune"},

    // Elyvilon.
    {"Sinner",                "Practitioner",       "Comforter",             "Caregiver",
        "Mender",           "Pacifist",                "Purifying @Genus@",        "Bringer of Life"},

    // Lugonu -- distortion theme.
    {"Pure",               "Abyss-Baptised",        "Unweaver",                 "Distorting @Genus@",
        "Agent of Entropy",   "Schismatic",            "Envoy of Void",            "Corrupter of Planes"},

    // Beogh -- messiah theme.
    {"Apostate",           "Convert",               "Proselytiser",             "Priest",
        "Missionary",         "Evangelist",            "Unifier",                  "Messiah"},

    // Jiyva -- slime and jelly theme.
    {"Scum",               "Squelcher",             "Ooze",                     "Jelly",
        "Slime Creature",     "Dissolving @Genus@",    "Blob",                     "Royal Jelly"},

    // Fedhas Madash -- nature theme.
    {"@Walking@ Fertiliser", "Fungal",              "Green @Genus@",            "Cultivator",
        "Fruitful",           "Photosynthesist",       "Green Death",              "Force of Nature"},

    // Cheibriados -- slow theme
    {"Hasty",              "Sluggish @Genus@",      "Deliberate",               "Unhurried",
     "Contemplative",         "Epochal",               "Timeless",                 "@Adj@ Aeon"},

    // Ashenzari -- divination theme
    {"Star-crossed",       "Cursed",                "Initiated",                "Seer",
        "Oracle",            "Illuminatus",            "Prince of Secrets",        "Omniscient"},

    // Dithmenos -- darkness theme
    {"Conspicuous",         "Nocturnal",            "Bump in the Night",        "Thespian",
        "Tenebrous",          "Puppetmaster",          "@Walking@ Midnight",       "Who Hides the Stars"},

    // Gozag -- entrepreneur theme
    {"Profligate",         "Pauper",                "Entrepreneur",             "Capitalist",
        "Rich",               "Opulent",               "Tycoon",                   "Plutocrat"},

    // Qazlal -- natural disaster theme
    {"Unspoiled",          "@Adj@ Mishap",          "Lightning Rod",            "@Adj@ Disaster",
        "Eye of the Storm",   "@Adj@ Catastrophe",     "@Adj@ Cataclysm",          "End of an Era"},

    // Ru -- enlightenment theme
    {"Sleeper",           "Questioner",             "Initiate",                 "Seeker of Truth",
        "@Walker@ of the Path","Lifter of the Veil",     "Transcendent",     "Drop of Water"},

#if TAG_MAJOR_VERSION == 34
    // Pakellas -- inventor theme
    {"Reactionary",       "Apprentice",             "Inquisitive",              "Experimenter",
        "Inventor",           "Pioneer",               "Brilliant",                "Grand Gadgeteer"},
#endif

    // Uskayaw -- reveler theme
    {"Prude",             "Wallflower",             "Party-goer",              "Dancer",
        "Impassioned",        "Rapturous",             "Ecstatic",                "Rhythm of Life and Death"},

    // Hepliaklqana -- memory/ancestry theme
    {"Damnatio Memoriae",       "Hazy",             "@Adj@ @Child@",              "Storyteller",
        "Brooding",           "Anamnesiscian",               "Grand Scion",                "Unforgettable"},

    // Wu Jian -- animal/chinese martial arts monk theme
    {"Wooden Rat",          "Young Dog",             "Young Crane",              "Young Tiger",
        "Young Dragon",     "Red Sash",               "Golden Sash",              "Sifu"},

    // Ignis -- fire/candles theme
    {"Extinguished",          "Last Ember",             "Glowing Coal",              "Thurifer",
        "Hearthfire",     "Furnace",               "Raging Flame",              "Inferno"},
};
COMPILE_CHECK(ARRAYSZ(divine_title) == NUM_GODS);

string god_title(god_type which_god, species_type which_species, int piety)
{
    string title;
    if (player_under_penance(which_god))
        title = divine_title[which_god][0];
    else if (which_god == GOD_USKAYAW)
        title = divine_title[which_god][_invocations_level()];
    else if (which_god == GOD_GOZAG)
        title = divine_title[which_god][_gold_level()];
    else
        title = divine_title[which_god][_piety_level(piety)];

    const map<string, string> replacements =
    {
        { "Adj", species::name(which_species, species::SPNAME_ADJ) },
        { "Genus", species::name(which_species, species::SPNAME_GENUS) },
        { "Walking", species::walking_title(which_species) + "ing" },
        { "Walker", species::walking_title(which_species) + "er" },
        { "Child", species::child_name(which_species) },
        { "Orc", species::orc_name(which_species) },
    };

    return replace_keys(title, replacements);
}


// Canonical titles remain available for dumps, identifiers and English mode.
string god_display_title(god_type god, species_type sp, int piety)
{
    const string original = god_title(god, sp, piety);
    const int rank = player_under_penance(god) ? 0
                   : god == GOD_USKAYAW ? _invocations_level()
                   : god == GOD_GOZAG ? _gold_level() : _piety_level(piety);
    const char *source = divine_title[god][rank];
    ui_translation_group group({{"d.god.title", source}});
    const map<string, string> keys = {
        {"Adj", species::name(sp, species::SPNAME_ADJ)},
        {"Genus", species::name(sp, species::SPNAME_GENUS)},
        {"Walking", species::walking_title(sp)},
        {"Walker", species::walking_title(sp)},
        {"Child", species::child_name(sp)},
        {"Orc", species::orc_name(sp)},
    };
    for (const auto &entry : keys)
        if (string(source).find("@" + entry.first + "@") != string::npos)
            group.require("d.god.title.component", entry.second.c_str());
    if (!group.available())
        return original;
    map<string, string> values;
    for (const auto &entry : keys)
        values[entry.first] = group.get("d.god.title.component", entry.second.c_str());
    return replace_keys(group.get("d.god.title", source), values);
}

static string _describe_item_curse(const item_def& item)
{
    if (!item.props.exists(CURSE_KNOWLEDGE_KEY))
        return ui_translate("d.god", "None");

    const CrawlVector &curses = item.props[CURSE_KNOWLEDGE_KEY].get_vector();

    if (curses.empty())
        return ui_translate("d.god", "None");

    const string original = comma_separated_fn(curses.begin(), curses.end(),
            curse_name, ", ", ", ");
    vector<string> names;
    for (const auto &curse : curses)
    {
        const ui_arg name = ui_text_arg("a.curse.name", curse_name(curse));
        if (!name.available) return original;
        names.push_back(name.text);
    }
    return join_strings(names.begin(), names.end(), "、");
}

static string _describe_ash_skill_boost()
{
    ostringstream desc;
    desc.setf(ios::left);
    desc << "<white>";
    const auto column = [] (const string &text, int width) {
        return text + string(max(0, width - strwidth(text)), ' ');
    };
    desc << column(ui_translate("d.god", "Bound item"), 40);
    desc << column(ui_translate("d.god", "Curse bonuses"), 30);
    desc << "</white>\n";

    vector<item_def*> eq = you.equipment.get_slot_items(SLOT_ALL_EQUIPMENT, true);
    for (item_def* item : eq)
    {
        if (item->cursed())
        {
            const bool meld = item_is_melded(*item);
            desc << (meld ? "<darkgrey>" : "<lightred>");
            string name;
            if (!try_item_display_name(*item, name, false, false))
                name = item->name(DESC_QUALNAME, true, false, false);
            desc << column(name, 40);
            desc << column(meld ? ui_translate("d.god", "melded") : _describe_item_curse(*item), 30);
            desc << (meld ? "</darkgrey>" : "</lightred>");
            desc << "\n";
        }
    }

    return desc.str();
}

typedef pair<int, string> ancestor_upgrade;

static const map<monster_type, vector<ancestor_upgrade> > ancestor_data =
{
    { MONS_ANCESTOR_KNIGHT,
      { { 1,  "Flail" },
        { 1,  "Shield" },
        { 1,  "Chain mail (+AC)" },
        { 15, "Broad axe (flame)" },
        { 19, "Tower shield (reflect)" },
        { 19, "Haste" },
        { 24, "Broad axe (speed)" },
      }
    },
    { MONS_ANCESTOR_BATTLEMAGE,
      { { 1,  "Quarterstaff" },
        { 1,  "Throw Frost" },
        { 1,  "Stone Arrow" },
        { 1,  "Increased melee damage" },
        { 15, "Bolt of Magma" },
        { 19, "Lajatang (freeze)" },
        { 19, "Haste" },
        { 24, "Lehudib's Crystal Spear" },
      }
    },
    { MONS_ANCESTOR_HEXER,
      { { 1,  "Dagger (drain)" },
        { 1,  "Slow" },
        { 1,  "Confuse" },
        { 15, "Paralyse" },
        { 19, "Mass Confusion" },
        { 19, "Haste" },
        { 24, "Quick blade (antimagic)" },
      }
    },
};

/// Build & return a table of Hep's upgrades for your chosen ancestor type.
static string _describe_ancestor_upgrades()
{
    if (!you.props.exists(HEPLIAKLQANA_ALLY_TYPE_KEY))
        return "";

    string desc;
    const monster_type ancestor =
        static_cast<monster_type>(you.props[HEPLIAKLQANA_ALLY_TYPE_KEY].get_int());
    const vector<ancestor_upgrade> *upgrades = map_find(ancestor_data,
                                                        ancestor);

    if (upgrades)
    {
        desc = ui_translate("d.god", "Ancestor Upgrades:\n\n<white>XL              Upgrade\n</white>");
        for (auto &entry : *upgrades)
        {
            desc += make_stringf("%s%2d              %s%s\n",
                                 you.experience_level < entry.first
                                     ? "<darkgrey>" : "",
                                 entry.first,
                                 ui_translate("d.ancestor.upgrade", entry.second.c_str()),
                                 you.experience_level < entry.first
                                     ? "</darkgrey>" : "");
        }
    }

    // XXX: maybe it'd be nice to let you see other ancestor types'...?
    return desc;
}

// from dgn-overview.cc
extern map<branch_type, set<level_id> > stair_level;

/**
 * Populate a provided vector with a list of bribable branches which are known
 * to the player.
 *
 * @param[out] targets      A list of bribable branches.
 */
static void _list_bribable_branches(vector<branch_type> &targets)
{
    for (branch_iterator it; it; ++it)
    {
        const branch_type br = it->id;
        if (!gozag_branch_bribable(br))
            continue;

        // Only list the Hells once.
        if (is_hell_subbranch(br))
            continue;

        // If you don't know the branch exists, don't list it;
        // this mainly plugs info leaks about Lair branch structure.
        if (!stair_level.count(br))
            continue;

        targets.push_back(br);
    }
}

/**
 * Describe the current options for Gozag's bribe branch ability.
 *
 * @return      A description of branches' bribe status.
 */
static string _describe_branch_bribability()
{
    string ret = ui_translate("d.god", "You can bribe the following branches of the dungeon:\n");
    vector<branch_type> targets;
    _list_bribable_branches(targets);

    size_t width = 0;
    for (branch_type br : targets)
        width = max(width, static_cast<size_t>(strwidth(branch_display_name(br))));

    for (branch_type br : targets)
    {
        string line = " ";
        line += branch_display_name(br);
        line += string(width + 3 - strwidth(line), ' ');

        if (!branch_bribe[br])
            line += ui_translate("d.god", "not bribed");
        else
            line += make_stringf("$%d", branch_bribe[br]);

        ret += line + "\n";
    }

    return ret;
}

static inline void _add_par(formatted_string &desc, const string &str)
{
    if (!str.empty())
        desc += formatted_string::parse_string(trimmed_string(str) + "\n\n");
}

/**
 * Describe the causes of the given god's wrath.
 *
 * @param which_god     The god in question.
 * @return              A description of the actions that cause this god's
 *                      wrath.
 */
static string _describe_god_wrath_causes_english(god_type which_god)
{
    if (which_god == GOD_RU)
        return ""; // no wrath
    vector<god_type> evil_gods;
    vector<god_type> chaotic_gods;
    for (god_iterator it; it; ++it)
    {
        const god_type god = *it;
        if (is_evil_god(god))
            evil_gods.push_back(god);
        else if (is_chaotic_god(god)) // intentionally not including evil!
            chaotic_gods.push_back(god);
        // XXX: refactor this if any god hates chaotic but not evil gods
    }

    switch (which_god)
    {
        case GOD_SHINING_ONE:
        case GOD_ELYVILON:
            return uppercase_first(god_name(which_god)) +
                   " forgives followers for abandonment; however, those who"
                   " later take up the worship of an evil god will be"
                   " punished. (" +
                   comma_separated_fn(begin(evil_gods), end(evil_gods),
                                      bind(god_name, placeholders::_1, false)) +
                   " are evil gods.)";

        case GOD_ZIN:
            return uppercase_first(god_name(which_god)) +
                   " forgives followers for abandonment; however, those who"
                   " later take up the worship of an evil or chaotic god will"
                   " be scourged. (" +
                   comma_separated_fn(begin(evil_gods), end(evil_gods),
                                      bind(god_name, placeholders::_1, false)) +
                   " are evil, and " +
                   comma_separated_fn(begin(chaotic_gods), end(chaotic_gods),
                                      bind(god_name, placeholders::_1, false)) +
                   " are chaotic.)";
        default:
            return uppercase_first(god_name(which_god)) +
                   " does not appreciate abandonment, and will call down"
                   " fearful punishments on disloyal followers!";
    }
}

/**
 * Print a description of the given god's dislikes & wrath effects.
 *
 * @param which_god     The god in question.
 */
static string _describe_god_wrath_causes(god_type which_god)
{
    const string original = _describe_god_wrath_causes_english(which_god);
    ui_translation_group group;
    if (!group.available() || which_god == GOD_RU)
        return original;
    ui_arg evil = {"", true}, chaotic = {"", true};
    for (god_iterator it; it; ++it)
    {
        const god_type god = *it;
        ui_arg *list = is_evil_god(god) ? &evil
                       : is_chaotic_god(god) ? &chaotic : nullptr;
        if (!list) continue;
        const ui_arg name = ui_text_arg("display.god", god_name(god));
        list->available = list->available && name.available;
        if (!list->text.empty()) list->text += "、";
        list->text += name.text;
    }
    const ui_arg name = ui_text_arg("display.god", god_name(which_god));
    if (which_god == GOD_SHINING_ONE || which_god == GOD_ELYVILON)
        return ui_render("d.god", "%s forgives followers for abandonment; however, those who later take up the worship of an evil god will be punished. (%s are evil gods.)",
                         original, {name, evil});
    if (which_god == GOD_ZIN)
        return ui_render("d.god", "%s forgives followers for abandonment; however, those who later take up the worship of an evil or chaotic god will be scourged. (%s are evil, and %s are chaotic.)",
                         original, {name, evil, chaotic});
    return ui_render("d.god", "%s does not appreciate abandonment, and will call down fearful punishments on disloyal followers!",
                     original, {name});
}

static formatted_string _god_wrath_description(god_type which_god)
{
    formatted_string desc;

    _add_par(desc, get_god_dislikes(which_god, true));
    _add_par(desc, _describe_god_wrath_causes(which_god));
    _add_par(desc, getLongDescription(god_name(which_god) + " wrath"));

    if (which_god != GOD_RU) // Permanent wrath.
    {
        const bool long_wrath = initial_wrath_penance_for(which_god) > 30;
        _add_par(desc, ui_render("d.god", long_wrath
            ? "%s's wrath lasts for a relatively long duration."
            : "%s's wrath lasts for a relatively short duration.",
            apostrophise(uppercase_first(god_name(which_god)))
                + " wrath lasts for a relatively "
                + (long_wrath ? "long" : "short") + " duration.",
            {ui_text_arg("display.god", god_name(which_god))}));
    }

    return desc;
}

static formatted_string _beogh_extra_description()
{
    formatted_string desc;

    for (int i = 1; i <= get_num_apostles(); ++i)
        _add_par(desc, apostle_short_description(i));

    if (you.duration[DUR_BEOGH_CAN_RECRUIT])
    {
        _add_par(desc, "-----------------------------------\n");
        _add_par(desc, apostle_short_description(0));
    }

    return desc;
}

static string _describe_deck_summary()
{
    ostringstream desc;
    desc << ui_translate("d.god", "Decks of power:\n");
    for (int i = FIRST_PLAYER_DECK; i <= LAST_PLAYER_DECK; i++)
        desc << " " << deck_display_status((deck_type) i) << "\n";

    string stack = stack_display_contents();
    if (!stack.empty())
        desc << ui_translate("d.god", "\n stacked deck: ") << stack << "\n";

    return desc.str();
}

static formatted_string _god_extra_description(god_type which_god)
{
    formatted_string desc;

    switch (which_god)
    {
        case GOD_ASHENZARI:
            desc = formatted_string::parse_string(
                       getLongDescription(god_name(which_god) + " extra"));
            if (have_passive(passive_t::bondage_skill_boost))
            {
                desc.cprintf("\n");
                _add_par(desc, ui_translate("d.god", "Ashenzari supports the following skill groups because of your curses:"));
                _add_par(desc,  _describe_ash_skill_boost());
            }
            break;
        case GOD_BEOGH:
            if (you_worship(GOD_BEOGH))
                desc = _beogh_extra_description();
            break;
        case GOD_GOZAG:
            if (you_worship(GOD_GOZAG))
                _add_par(desc, _describe_branch_bribability());
            break;
        case GOD_HEPLIAKLQANA:
            if (you_worship(GOD_HEPLIAKLQANA))
                desc = formatted_string::parse_string(_describe_ancestor_upgrades());
            break;
        case GOD_NEMELEX_XOBEH:
            if (you_worship(GOD_NEMELEX_XOBEH))
                _add_par(desc, _describe_deck_summary());
            break;
        case GOD_WU_JIAN:
            _add_par(desc, ui_translate("d.god", "Martial attacks:"));
            desc += formatted_string::parse_string(
                        getLongDescription(god_name(which_god) + " extra"));
            break;
        default:
            break;
    }

    return desc;
}

/**
 * Describe miscellaneous information about the given god.
 *
 * @param which_god     The god in question.
 * @return              Info about gods which isn't covered by their powers,
 *                      likes, or dislikes.
 */
static string _get_god_misc_info_english(god_type which_god)
{
    string info = "";
    skill_type skill = invo_skill(which_god);

    switch (skill)
    {
        case SK_INVOCATIONS:
            break;
        case SK_NONE:
            info += uppercase_first(apostrophise(god_name(which_god))) +
                " powers are not affected by the Invocations skill.";
            break;
        default:
            info += uppercase_first(apostrophise(god_name(which_god))) +
                    " powers are based on " + skill_name(skill) + " instead"
                    " of Invocations skill.";
            break;
    }

    if (!info.empty())
        info += "\n\n";

    return info;
}

/**
 * Print a detailed description of the given god's likes and powers.
 *
 * @param god       The god in question.
 */
static string _get_god_misc_info(god_type god)
{
    const string original = _get_god_misc_info_english(god);
    const skill_type skill = invo_skill(god);
    if (skill == SK_INVOCATIONS)
        return original;
    const ui_arg name = ui_text_arg("display.god", god_name(god));
    if (skill == SK_NONE)
        return ui_render("d.god", "%s's powers are not affected by the Invocations skill.\n\n",
                         original, {name});
    return ui_render("d.god", "%s's powers are based on %s instead of Invocations skill.\n\n",
                     original, {name, ui_text_arg("display.skill", skill_name(skill))});
}

static formatted_string _detailed_god_description(god_type which_god)
{
    formatted_string desc;
    _add_par(desc, getLongDescription(god_name(which_god) + " powers"));
    _add_par(desc, get_god_likes(which_god, true));
    _add_par(desc, _get_god_misc_info(which_god));
    return desc;
}

/**
 * Describe the given god's level of irritation at the player.
 *
 * Player may or may not be currently under penance.
 *
 * @param which_god     The god in question.
 * @return              A format string, describing the god's ire (or lack of).
 */
static string _raw_penance_message(god_type which_god)
{
    const int penance = you.penance[which_god];

    // Give more appropriate message for the good gods.
    if (penance > 0 && is_good_god(which_god))
    {
        if (is_good_god(you.religion))
            return "%s is ambivalent towards you.";
        if (!god_hates_your_god(which_god))
        {
            return "%s is almost ready to forgive your sins.";
                 // == "Come back to the one true church!"
        }
    }

    const int initial_penance = initial_wrath_penance_for(which_god);
    // could do some math tricks to turn this into a table, but it seems fiddly
    if (penance > initial_penance * 3 / 4)
        return "%s's wrath is upon you!";
    if (penance > initial_penance / 2)
        return "%s well remembers your sins.";
    if (penance > initial_penance / 4)
        return "%s's wrath is beginning to fade.";
    if (penance > 0)
    {
        if (which_god == GOD_IGNIS)
            return "%s' wrath will not burn much longer.";
        return "%s is almost ready to forgive your sins.";
    }
    return "%s is neutral towards you.";
}

/**
 * Describe the given god's level of irritation at the player.
 *
 * Player may or may not be currently under penance.
 *
 * @param which_god     The god in question.
 * @return              A description of the god's ire (or lack thereof).
 */
static string _god_penance_message(god_type which_god)
{
    const string message = _raw_penance_message(which_god);
    return ui_render("d.god", message.c_str(),
        make_stringf(message.c_str(), uppercase_first(god_name(which_god)).c_str()),
        {ui_text_arg("display.god", god_name(which_god))});
}

/**
 * Print a description of the powers & abilities granted to the player by the
 * given god. If player worships the god, the currently available powers are
 * highlighted.
 *
 * @param which_god     The god in question.
 */
static formatted_string _describe_god_powers(god_type which_god)
{
    formatted_string desc;

    int piety = you_worship(which_god) ? you.piety() : 0;

    desc.textcolour(LIGHTGREY);
    const char *header = ui_translate("d.god", "Granted powers:");
    const char *cost   = ui_translate("d.god", "(Cost)");
    desc.cprintf("\n\n%s%*s%s\n", header,
            80 - strwidth(header) - strwidth(cost),
            "", cost);

    bool have_any = false;

    // set default color here, so we don't have to set in multiple places for
    // always available passive abilities
    if (!you_worship(which_god))
        desc.textcolour(DARKGREY);
    else
        desc.textcolour(god_colour(which_god));

    // mv: Some gods can protect you from harm.
    // The god isn't really protecting the player - only sometimes saving
    // their life.
    if (god_gives_passive(which_god, passive_t::protect_from_harm)
        || god_gives_passive(which_god, passive_t::lifesaving))
    {
        have_any = true;

        const char *how = "";

        if (god_gives_passive(which_god, passive_t::lifesaving))
        {
            how = (piety >= piety_breakpoint(5)) ? "carefully " :
                  (piety >= piety_breakpoint(3)) ? "often " :
                  (piety >= piety_breakpoint(1)) ? "sometimes "
                                                 : "occasionally ";
        }
        else
            how = (piety >= piety_breakpoint(5)) ? "sometimes "
                                                 : "occasionally ";

        desc.cprintf("%s", ui_render("d.god", "%s %sguards your life.\n",
            make_stringf("%s %sguards your life.\n",
                uppercase_first(god_name(which_god)).c_str(),
                how), {ui_text_arg("display.god", god_name(which_god)), ui_text_arg("d.god.degree", how)}).c_str());
    }

    switch (which_god)
    {
    case GOD_BEOGH:
    {
        have_any = true;
        if (have_passive(passive_t::convert_orcs))
            desc.textcolour(god_colour(which_god));
        else
            desc.textcolour(DARKGREY);

        if (piety >= piety_breakpoint(5))
            desc.cprintf("%s", ui_translate("d.god", "Orcs frequently recognise you as Beogh's chosen one.\n"));
        else
            desc.cprintf("%s", ui_translate("d.god", "Orcs sometimes recognise you as one of their own.\n"));
    }
    break;

    case GOD_ZIN:
    {
        have_any = true;
        const char *how =
            (piety >= piety_breakpoint(5)) ? "always" :
            (piety >= piety_breakpoint(3)) ? "often" :
            (piety >= piety_breakpoint(1)) ? "sometimes" :
                                             "occasionally";

        desc.cprintf("%s", ui_render("d.god", "%s %s shields you from chaos.\n",
            make_stringf("%s %s shields you from chaos.\n",
                uppercase_first(god_name(which_god)).c_str(), how), {ui_text_arg("display.god", god_name(which_god)), ui_text_arg("d.god.degree", how)}).c_str());

        how =
            (piety >= piety_breakpoint(5)) ? "often" :
            (piety >= piety_breakpoint(3)) ? "sometimes" :
            (piety >= piety_breakpoint(1)) ? "occasionally" :
                                             "rarely";
        desc.cprintf("%s", ui_render("d.god", "%s %s shields you from Hell.\n",
            make_stringf("%s %s shields you from Hell.\n",
                uppercase_first(god_name(which_god)).c_str(), how), {ui_text_arg("display.god", god_name(which_god)), ui_text_arg("d.god.degree", how)}).c_str());
        break;
    }

    case GOD_SHINING_ONE:
    {
        have_any = true;
        desc.cprintf("%s", ui_render("d.god", "%s prevents you from stabbing unaware foes.\n",
            make_stringf("%s prevents you from stabbing unaware foes.\n",
                uppercase_first(god_name(which_god)).c_str()), {ui_text_arg("display.god", god_name(which_god))}).c_str());

        const int halo_size = you_worship(which_god) ? you.halo_radius() : -1;
        if (halo_size < 0)
            desc.textcolour(DARKGREY);
        else
            desc.textcolour(god_colour(which_god));
        desc.cprintf("%s", ui_render("d.god",
            halo_size > 5 ? "You radiate a large righteous aura, and foes within it are easier to hit.\n"
            : halo_size > 3 ? "You radiate a righteous aura, and foes within it are easier to hit.\n"
                            : "You radiate a small righteous aura, and foes within it are easier to hit.\n",
            make_stringf("You radiate a%s righteous aura, and foes within it are easier to hit.\n",
                halo_size > 5 ? " large" : halo_size > 3 ? "" : " small"), {}).c_str());

        if (piety >= piety_breakpoint(1))
            desc.textcolour(god_colour(which_god));
        else
            desc.textcolour(DARKGREY);
        const char *how =
            (piety >= piety_breakpoint(5)) ? "completely" :
            (piety >= piety_breakpoint(3)) ? "mostly" :
                                             "partially";
        desc.cprintf("%s", ui_render("d.god", "%s %s shields you from negative energy.\n",
            make_stringf("%s %s shields you from negative energy.\n",
                uppercase_first(god_name(which_god)).c_str(), how), {ui_text_arg("display.god", god_name(which_god)), ui_text_arg("d.god.degree", how)}).c_str());
        break;
    }

    case GOD_JIYVA:
        have_any = true;
        desc.cprintf("%s", ui_translate("d.god", "Jellies are peaceful and will consume items off the floor.\n"));
        desc.cprintf("%s", ui_translate("d.god", "Jiyva prevents you from harming jellies.\n"));

        if (have_passive(passive_t::jelly_regen))
            desc.textcolour(god_colour(which_god));
        else
            desc.textcolour(DARKGREY);
        desc.cprintf("%s", ui_render("d.god",
            piety >= piety_breakpoint(5) ? "Your health and magic regeneration is very greatly accelerated.\n"
            : piety >= piety_breakpoint(3) ? "Your health and magic regeneration is greatly accelerated.\n"
                                         : "Your health and magic regeneration is accelerated.\n",
            make_stringf("Your health and magic regeneration is %saccelerated.\n",
                     piety >= piety_breakpoint(5) ? "very greatly " :
                     piety >= piety_breakpoint(3) ? "greatly " : ""), {}).c_str());
        break;

    case GOD_CHEIBRIADOS:
        have_any = true;
        if (have_passive(passive_t::stat_boost))
            desc.textcolour(god_colour(which_god));
        else
            desc.textcolour(DARKGREY);
        desc.cprintf("%s", ui_render("d.god",
            piety >= piety_breakpoint(5) ? "%s greatly slows your movement.\n"
            : piety >= piety_breakpoint(2) ? "%s slows your movement.\n"
                                         : "%s slightly slows your movement.\n",
            make_stringf("%s %sslows your movement.\n",
                uppercase_first(god_name(which_god)).c_str(),
                piety >= piety_breakpoint(5) ? "greatly " :
                piety >= piety_breakpoint(2) ? "" : "slightly "),
            {ui_text_arg("display.god", god_name(which_god))}).c_str());
        desc.cprintf("%s", ui_render("d.god", "%s supports your attributes. (+%s)\n",
            make_stringf("%s supports your attributes. (+%d)\n",
                uppercase_first(god_name(which_god)).c_str(), chei_stat_boost(piety)),
            {ui_text_arg("display.god", god_name(which_god)),
             ui_literal_arg(to_string(chei_stat_boost(piety)))}).c_str());
        break;

    case GOD_VEHUMET:
        have_any = true;
        if (const int numoffers = you.vehumet_gifts.size())
        {
            const char* offer = numoffers == 1
                               ? spell_title(*you.vehumet_gifts.begin())
                               : "some of Vehumet's most lethal spells";
            const string original = make_stringf("You can memorise %s.\n", offer);
            const string message = numoffers == 1
                ? ui_render("d.god", "You can memorise %s.\n", original,
                    {ui_text_arg("display.spell", spell_title(*you.vehumet_gifts.begin()))})
                : ui_render("d.god", "You can memorise some of Vehumet's most lethal spells.\n",
                    original, {});
            desc.cprintf("%s", message.c_str());
        }
        else if (!you.has_mutation(MUT_INNATE_CASTER))
        {
            desc.textcolour(DARKGREY);
            desc.cprintf("%s", ui_translate("d.god", "You can memorise some of Vehumet's spells.\n"));
        }
        break;

    case GOD_YREDELEMNUL:
        // TODO: Vary the text depending on the size of the umbra.
        desc.cprintf("%s", ui_translate("d.god", "You are surrounded by an umbra.\n"
                     "Foes that die within your umbra may be raised as undead servants.\n"));
        break;

    case GOD_HEPLIAKLQANA:
        // Frailty occurs even under penance post-abandonment, so we can't put
        // this in the usual god_powers block.
        have_any = true;
    {
        const auto textcol = have_passive(passive_t::frail) ? god_colour(which_god) : DARKGREY;
        // We need to set textcolour before each line so that it'll display
        // correctly in webtiles. (It works fine locally regardless.)
        // Feature request: not this.
        desc.textcolour(textcol);
        desc.cprintf(ui_translate("d.god", "Your life essence is reduced. (-10%% HP)\n"));
    }
        break;

    default:
        break;
    }

    for (const auto& power : get_god_powers(which_god))
    {
        // hack: don't mention the necronomicon alone unless it
        // wasn't already mentioned by the other description
        if (power.abil == ABIL_KIKU_GIFT_CAPSTONE_SPELLS
            && !you.has_mutation(MUT_NO_GRASPING))
        {
            continue;
        }
        // Skip over Makhleb's brand options after the first one, since
        // only the first one has an associated god ability.
        if (power.abil == ABIL_MAKHLEB_BRAND_SELF_2
            || power.abil == ABIL_MAKHLEB_BRAND_SELF_3)
        {
            continue;
        }
        have_any = true;

        if (you_worship(which_god)
            && (power.rank <= 0
                || power.rank == 7 && can_do_capstone_ability(which_god)
                || piety_rank(piety) >= power.rank)
            && (!player_under_penance()
                || power.rank == -1))
        {
            desc.textcolour(god_colour(which_god));
        }
        else
            desc.textcolour(DARKGREY);

        // XXX: I don't like this, but there's no other obvious way to
        //      slot the destruction upgrade mutations in at the right piety
        //      point in the list for them.
        if (which_god == GOD_MAKHLEB && power.rank == 4)
        {
            desc.textcolour(god_colour(which_god));
            if (you.has_mutation(MUT_MAKHLEB_DESTRUCTION_GEH))
                desc.cprintf("%s", ui_translate("d.god", "Your Destruction is augmented by the power of Gehenna.\n"));
            else if (you.has_mutation(MUT_MAKHLEB_DESTRUCTION_COC))
                desc.cprintf("%s", ui_translate("d.god", "Your Destruction is augmented by the power of Cocytus.\n"));
            else if (you.has_mutation(MUT_MAKHLEB_DESTRUCTION_TAR))
                desc.cprintf("%s", ui_translate("d.god", "Your Destruction is augmented by the power of Tartarus.\n"));
            else if (you.has_mutation(MUT_MAKHLEB_DESTRUCTION_DIS))
                desc.cprintf("%s", ui_translate("d.god", "Your Destruction is augmented by the power of Dis.\n"));
            else
            {
                desc.textcolour(DARKGREY);
                desc.cprintf("%s", ui_translate("d.god", "Your Destruction will be augmented by one of the Four Hells.\n"));
            }

            continue;
        }

        if (power.abil == ABIL_MAKHLEB_BRAND_SELF_1
            && !makhleb_mark_name().empty())
        {
                desc.textcolour(god_colour(which_god));
                desc.cprintf("%s", ui_render("d.god", "You have been branded with the %s.\n",
                    make_stringf("You have been branded with the %s.\n", makhleb_mark_name().c_str()),
                    {ui_text_arg("a.mutation.name", makhleb_mark_name())}).c_str());
                continue;
        }

        string buf = power.general;

        // Skip listing powers with no description (they are intended to be hidden)
        if (buf.length() == 0)
        {
            have_any = false;
            continue;
        }

        if (!isupper(buf[0])) // Complete sentence given?
            buf = "You can " + buf + ".";
        // The canonical power record selects a complete Chinese sentence.
        // Keep the original English grammar and the religion table unchanged.
        ui_translation_group power_text({{"d.god.power", power.general}});
        if (power_text.available())
            buf = power_text.get("d.god.power", power.general);
        const int desc_len = strwidth(buf);

        string abil_cost = "(" + make_cost_description(power.abil) + ")";
        if (abil_cost == "(None)")
            abil_cost = "";

        desc.cprintf("%s%*s%s\n", buf.c_str(), 80 - desc_len - strwidth(abil_cost),
                "", abil_cost.c_str());
    }

    if (!have_any)
        desc.cprintf("%s", ui_translate("d.god", "None.\n"));

    // Show Jiyva's opening of the Slime Pits at the bottom of the list
    // We want this to stay green permanently once the player hits 6*
    if (which_god == GOD_JIYVA)
    {
        if (you.one_time_ability_used[which_god])
            desc.textcolour(god_colour(which_god));
        else
            desc.textcolour(DARKGREY);
        desc.cprintf("%s", ui_translate("d.god", "Jiyva will unlock the Slime Pits vaults.\n"));
    }

    return desc;
}

static formatted_string _god_overview_description(god_type which_god)
{
    formatted_string desc;

    // Print god's description.
    const string god_desc = getLongDescription(god_name(which_god));
    desc += trimmed_string(god_desc) + "\n";

    // Title only shown for our own god.
    if (you_worship(which_god))
    {
        // Print title based on piety.
        desc.cprintf(ui_translate("phase4.describe-god", "\nTitle  - "));
        desc.textcolour(god_colour(which_god));

        string title = god_display_title(which_god, you.species, you.raw_piety);
        desc.cprintf("%s", title.c_str());
    }

    // mv: Now let's print favour as Brent suggested.
    // I know these messages aren't perfect so if you can think up
    // something better, do it.

    desc.textcolour(LIGHTGREY);
    desc.cprintf(ui_translate("phase4.describe-god", "\nFavour - "));
    desc.textcolour(god_colour(which_god));

    if (!you_worship(which_god))
        desc.cprintf("%s", _god_penance_message(which_god).c_str());
    else
        desc.cprintf("%s", _describe_favour(which_god).c_str());
    desc += _describe_god_powers(which_god);
    desc.cprintf("\n\n");

    return desc;
}

static void build_partial_god_ui(god_type which_god, shared_ptr<ui::Popup>& popup, shared_ptr<Switcher>& desc_sw, shared_ptr<Switcher>& more_sw)
{
    formatted_string topline;
    topline.textcolour(god_colour(which_god));
    const string canonical = god_name(which_god);
    const string display = ui_translate("display.god", canonical.c_str());
    topline += formatted_string(display == canonical
        ? uppercase_first(god_name(which_god, true))
        : display + " (" + uppercase_first(god_name(which_god, true)) + ")");

    auto vbox = make_shared<Box>(Widget::VERT);
    vbox->set_cross_alignment(Widget::STRETCH);
    auto title_hbox = make_shared<Box>(Widget::HORZ);

#ifdef USE_TILE
    auto icon = make_shared<Image>();
    const tileidx_t idx = tileidx_feature_base(altar_for_god(which_god));
    icon->set_tile(tile_def(idx));
    title_hbox->add_child(std::move(icon));
#endif

    auto title = make_shared<Text>(topline.trim());
    title->set_margin_for_sdl(0, 0, 0, 16);
    title_hbox->add_child(std::move(title));

    title_hbox->set_main_alignment(Widget::CENTER);
    title_hbox->set_cross_alignment(Widget::CENTER);
    vbox->add_child(std::move(title_hbox));

    desc_sw = make_shared<Switcher>();
    more_sw = make_shared<Switcher>();
    desc_sw->current() = 0;
    more_sw->current() = 0;

    const formatted_string descs[4] = {
        _god_overview_description(which_god),
        _detailed_god_description(which_god),
        _god_wrath_description(which_god),
        _god_extra_description(which_god)
    };

    int mores_index = descs[3].empty() ? 0 : 1;
    const char* mores[2][4] =
    {
        {
            ui_translate("phase4.describe-god", "[<w>!</w>]: <w>Overview</w>|Powers|Wrath"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|<w>Powers</w>|Wrath"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|Powers|<w>Wrath</w>"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|Powers|Wrath")
        },
        {
            ui_translate("phase4.describe-god", "[<w>!</w>]: <w>Overview</w>|Powers|Wrath|Extra"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|<w>Powers</w>|Wrath|Extra"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|Powers|<w>Wrath</w>|Extra"),
            ui_translate("phase4.describe-god", "[<w>!</w>]: Overview|Powers|Wrath|<w>Extra</w>")
        }
    };

    for (int i = 0; i < 4; i++)
    {
        const auto &desc = descs[i];
        if (desc.empty())
            continue;

        auto scroller = make_shared<Scroller>();
        auto text = make_shared<Text>(desc.trim());
        text->set_wrap_text(true);
        scroller->set_child(text);
        desc_sw->add_child(std::move(scroller));

        more_sw->add_child(make_shared<Text>(
                formatted_string::parse_string(mores[mores_index][i])));
    }

    desc_sw->set_margin_for_sdl(20, 0);
    desc_sw->set_margin_for_crt(1, 0);
    desc_sw->expand_h = false;
#ifdef USE_TILE_LOCAL
    desc_sw->max_size().width = tiles.get_crt_font()->char_width()*80;
#endif
    vbox->add_child(desc_sw);

    vbox->add_child(more_sw);

    popup = make_shared<ui::Popup>(vbox);
}

static const string _god_service_fee_description(god_type which_god)
{
    const int fee = (which_god == GOD_GOZAG) ? gozag_service_fee() : 0;

    if (which_god == GOD_GOZAG)
    {
        if (fee == 0)
        {
            const string original = string(" (no fee if you ")
                          + random_choose("act now", "join today") + ")";
            return ui_translate("d.god", original.c_str());
        }
        else
            return make_stringf(ui_translate("d.god", " (%d gold; you have %d)"), fee, you.gold);
    }

    return "";
}

#ifdef USE_TILE_WEB
static void _send_god_ui(god_type god, bool is_altar)
{
    tiles.json_open_object();

    const tileidx_t idx = tileidx_feature_base(altar_for_god(god));
    tiles.json_open_object("tile");
    tiles.json_write_int("t", idx);
    tiles.json_write_int("tex", get_tile_texture(idx));
    tiles.json_close_object();

    tiles.json_write_int("colour", god_colour(god));
    const string canonical = god_name(god);
    const string display = ui_translate("display.god", canonical.c_str());
    tiles.json_write_string("name", display == canonical
        ? god_name(god, true) : display + " (" + god_name(god, true) + ")");
    tiles.json_write_bool("is_altar", is_altar);

    tiles.json_write_string("description", getLongDescription(god_name(god)));
    if (you_worship(god))
        tiles.json_write_string("title", god_display_title(god, you.species, you.piety()));
    tiles.json_write_string("favour", you_worship(god) ?
            _describe_favour(god) : _god_penance_message(god));
    tiles.json_write_string("powers_list",
            _describe_god_powers(god).to_colour_string(LIGHTGREY));
    tiles.json_write_string("info_table", "");

    tiles.json_write_string("powers",
            _detailed_god_description(god).to_colour_string(LIGHTGREY));
    tiles.json_write_string("wrath",
            _god_wrath_description(god).to_colour_string(LIGHTGREY));
    tiles.json_write_string("extra",
            _god_extra_description(god).to_colour_string(LIGHTGREY));
    tiles.json_write_string("service_fee",
            _god_service_fee_description(god));
    tiles.push_ui_layout("describe-god", 1);
}
#endif

void describe_god(god_type which_god)
{
    if (which_god == GOD_NO_GOD) //mv: No god -> say it and go away.
    {
        mpr(ui_translate("d.god", "You are not religious."));
        return;
    }

    shared_ptr<ui::Popup> popup;
    shared_ptr<Switcher> desc_sw;
    shared_ptr<Switcher> more_sw;
    build_partial_god_ui(which_god, popup, desc_sw, more_sw);

    bool done = false;
    popup->on_keydown_event([&](const KeyEvent& ev) {
        const auto key = ev.key();
        if (key == '!' || key == '^')
        {
            int n = (desc_sw->current() + 1) % desc_sw->num_children();
            desc_sw->current() = more_sw->current() = n;
#ifdef USE_TILE_WEB
                tiles.json_open_object();
                tiles.json_write_int("pane", n);
                tiles.ui_state_change("describe-god", 0);
#endif
            return true;
        }
        if (desc_sw->current_widget()->on_event(ev))
            return true;
        return done = ui::key_exits_popup(key, false);
    });

#ifdef USE_TILE_WEB
    _send_god_ui(which_god, false);
    popup->on_layout_pop([](){ tiles.pop_ui_layout(); });
#endif

    ui::run_layout(popup, done);
}

bool describe_god_with_join(god_type which_god)
{
    const string service_fee = _god_service_fee_description(which_god);

    shared_ptr<ui::Popup> popup;
    shared_ptr<Switcher> desc_sw;
    shared_ptr<Switcher> more_sw;
    build_partial_god_ui(which_god, popup, desc_sw, more_sw);

    for (auto& child : *more_sw)
    {
        Text* label = static_cast<Text*>(child.get());
        formatted_string text = label->get_text();
        text += formatted_string::parse_string(ui_translate("d.god", "  [<w>J</w>/<w>Enter</w>]: "
                                               "join"));

        // We assume that a player who has enough gold such that
        // the join fee plus accumulated gold overflows knows what this menu
        // does.
        if (text.width() + service_fee.length() + 9 <= MIN_COLS)
            text += " religion";
        if (!service_fee.empty())
            text += service_fee;
        label->set_text(text);
    }

    // States for the state machine
    enum join_step_type {
        SHOW = -1, // Show the usual god UI
        ABANDON, // Ask whether to abandon god, if applicable
    };

    // Add separate text widgets the possible abandon-god prompts;
    // then when a different prompt needs to be shown, we switch to that prompt.
    // This is somewhat brittle, but ensures that the UI doesn't resize when
    // switching between prompts.
    const string abandon_prompt =
        ui_render("d.god", "Are you sure you want to abandon %s?",
            make_stringf("Are you sure you want to abandon %s?", god_name(you.religion).c_str()),
            {ui_text_arg("display.god", god_name(you.religion))});
    formatted_string prompt_fs(abandon_prompt, channel_to_colour(MSGCH_PROMPT));

    more_sw->add_child(make_shared<Text>(prompt_fs));

    prompt_fs.cprintf("%s", ui_translate("d.god", " [Y]es or [n]o only, please."));
    more_sw->add_child(make_shared<Text>(prompt_fs));

    join_step_type step = SHOW;
    bool yesno_only = false;
    bool done = false, join = false;

    // The join-god UI state machine transition function
    popup->on_keydown_event([&](const KeyEvent& ev) {
        const auto keyin = ev.key();

        // Always handle escape and pane-switching keys the same way
        if (ui::key_exits_popup(keyin, false))
            return done = true;
        if (keyin == '!' || keyin == '^')
        {
            int n = (desc_sw->current() + 1) % desc_sw->num_children();
            desc_sw->current() = n;
#ifdef USE_TILE_WEB
            tiles.json_open_object();
            tiles.json_write_int("pane", n);
            tiles.ui_state_change("describe-god", 0);
#endif
            if (step == SHOW)
            {
                more_sw->current() = n;
                return true;
            }
            else
                yesno_only = false;
        }

        // Next, allow child widgets to handle scrolling keys
        // NOTE: these key exceptions are also specified in ui-layouts.js
        if (keyin != 'J' && keyin != CK_ENTER)
            if (desc_sw->current_widget()->on_event(ev))
                return true;

        if (step == ABANDON)
        {
            if (keyin != 'Y' && toupper_safe(keyin) != 'N')
                yesno_only = true;
            else
                yesno_only = false;

            if (toupper_safe(keyin) == 'N')
            {
                canned_msg(MSG_OK);
                return done = true;
            }

            if (keyin == 'Y')
                return done = join = true;
        }
        else if ((keyin == 'J' || keyin == CK_ENTER) && step == SHOW)
        {
            if (you_worship(GOD_NO_GOD))
                return done = join = true;

            step = ABANDON;
        }
        else
            return done = true;

#ifdef USE_TILE_WEB
        tiles.json_open_object();
        string prompt = abandon_prompt + (yesno_only ? ui_translate("d.god", " [Y]es or [n]o only, please.") : "");
        tiles.json_write_string("prompt", prompt);
        tiles.json_write_int("pane", desc_sw->current());
        tiles.ui_state_change("describe-god", 0);
#endif
        if (step == ABANDON)
            more_sw->current() = desc_sw->num_children() + step*2 + yesno_only;
        return true;
    });

#ifdef USE_TILE_WEB
    _send_god_ui(which_god, true);
    popup->on_layout_pop([](){ tiles.pop_ui_layout(); });
#endif

    ui::run_layout(popup, done);

    return join;
}
