/**
 * @file
 * @brief Notetaking stuff
**/

#include "AppHdr.h"
#include "i18n.h"

#include "notes.h"

#include <iomanip>
#include <sstream>
#include <vector>

#include "branch.h"
#include "directn.h"
#include "describe.h"
#include "env.h"
#include "items.h"
#include "item-name.h"
#include "shopping.h"
#include "tags.h"
#include "english.h"
#include "hiscores.h"
#include "jobs.h"
#include "species.h"
#include "item-prop.h" // gem_adj
#include "message.h"
#include "mon-util.h"
#include "mutation.h"
#include "options.h"
#include "religion.h"
#include "skills.h"
#include "spl-util.h"
#include "state.h"
#include "stringutil.h"
#include "tag-version.h"
#include "unicode.h"
#include "view.h"

#define NOTES_VERSION_NUMBER 1002

vector<Note> note_list;
int last_screen_turn = -1;

static bool _is_highest_skill(int skill)
{
    for (int i = 0; i < NUM_SKILLS; ++i)
    {
        if (i == skill)
            continue;
        if (you.skills[i] >= you.skills[skill])
            return false;
    }
    return true;
}

static bool _is_noteworthy_hp(int hp, int maxhp)
{
    return hp > 0 && Options.note_hp_percent
           && hp <= (maxhp * Options.note_hp_percent) / 100;
}

static int _dungeon_branch_depth(uint8_t branch)
{
    if (branch >= NUM_BRANCHES)
        return -1;
    return brdepth[branch];
}

static bool _is_noteworthy_dlevel(level_id place)
{
    branch_type branch = place.branch;
    int lev = place.depth;

    // Entering the Abyss is noted a different way, since we care mostly about
    // the cause.
    if (branch == BRANCH_ABYSS)
        return lev == _dungeon_branch_depth(branch);

    // These get their note in the .des files.
    if (branch == BRANCH_WIZLAB || branch == BRANCH_TROVE)
        return false;

    // Other portal levels are always interesting.
    if (!is_connected_branch(branch))
        return true;

    return lev == _dungeon_branch_depth(branch)
           || branch == BRANCH_DUNGEON && (lev % 5) == 0
           || branch != BRANCH_DUNGEON && lev == 1;
}

// Is a note worth taking?
// This function assumes that game state has not changed since
// the note was taken, e.g. you.* is valid.
static bool _is_noteworthy(const Note& note)
{
    // Always noteworthy.
    if (note.type == NOTE_XP_LEVEL_CHANGE
        || note.type == NOTE_LEARN_SPELL
        || note.type == NOTE_GET_GOD
        || note.type == NOTE_GOD_GIFT
        || note.type == NOTE_GET_MUTATION
        || note.type == NOTE_LOSE_MUTATION
        || note.type == NOTE_PERM_MUTATION
        || note.type == NOTE_GET_ITEM
        || note.type == NOTE_ID_ITEM
        || note.type == NOTE_ACQUIRE_ITEM
        || note.type == NOTE_BUY_ITEM
        || note.type == NOTE_DONATE_MONEY
        || note.type == NOTE_SEEN_MONSTER
        || note.type == NOTE_DEFEAT_MONSTER
        || note.type == NOTE_POLY_MONSTER
        || note.type == NOTE_USER_NOTE
        || note.type == NOTE_MESSAGE
        || note.type == NOTE_LOSE_GOD
        || note.type == NOTE_PENANCE
        || note.type == NOTE_MOLLIFY_GOD
        || note.type == NOTE_DEATH
        || note.type == NOTE_XOM_REVIVAL
        || note.type == NOTE_SEEN_FEAT
        || note.type == NOTE_PARALYSIS
        || note.type == NOTE_VEXED
        || note.type == NOTE_RECRUITED_APOSTLE
        || note.type == NOTE_ALLY_DEATH
        || note.type == NOTE_FEAT_MIMIC
        || note.type == NOTE_OFFERED_SPELL
        || note.type == NOTE_ANCESTOR_TYPE
        || note.type == NOTE_FOUND_UNRAND
        || note.type == NOTE_ZOT_TOUCHED
        || note.type == NOTE_DREAMSHARD
        || note.type == NOTE_GEM_LOST
        || note.type == NOTE_GAIN_LIFE
        || note.type == NOTE_LOSE_LIFE
        || note.type == NOTE_FLED_CHALLENGE
        || note.type == NOTE_INFERNAL_MARK
        || note.type == NOTE_GET_BANE
        || note.type == NOTE_LOSE_BANE
        || note.type == NOTE_TESSERACT_ACTIVATED)
    {
        return true;
    }

    // Never noteworthy, hooked up for fun or future use.
    if (note.type == NOTE_MP_CHANGE
        || note.type == NOTE_MAXHP_CHANGE
        || note.type == NOTE_MAXMP_CHANGE)
    {
        return false;
    }

    // Xom effects are only noteworthy if the option is true.
    if (note.type == NOTE_XOM_EFFECT)
        return Options.note_xom_effects;

    // HP noteworthiness is handled in its own function.
    if (note.type == NOTE_HP_CHANGE
        && !_is_noteworthy_hp(note.first, note.second))
    {
        return false;
    }

    // Skills are always noteworthy in order to construct the skill_gains table
    // in the chardump. The options to control display are used in
    // Note::hidden().
    if (note.type == NOTE_GAIN_SKILL || note.type == NOTE_LOSE_SKILL)
        return true;

    if (note.type == NOTE_DUNGEON_LEVEL_CHANGE)
        return _is_noteworthy_dlevel(note.place);

    for (const Note &oldnote : note_list)
    {
        if (oldnote.type != note.type)
            continue;

        const Note& rnote(oldnote);
        switch (note.type)
        {
        case NOTE_PIETY_RANK:
            if (rnote.first == note.first && rnote.second == note.second)
                return false;
            break;

        case NOTE_HP_CHANGE:
            // Not if we have a recent warning
            // unless we've lost half our HP since then.
            if (note.turn - rnote.turn < 5
                && note.first * 2 >= rnote.first)
            {
                return false;
            }
            break;

        default:
            mpr("Buggy note passed: unknown note type");
            // Return now, rather than give a "Buggy note passed" message
            // for each note of the matching type in the note list.
            return true;
        }
    }
    return true;
}

Note make_starting_note(const string &name, species_type race, job_type job)
{
    const string original = name + " the " + species::name(race) + " "
        + get_job_name(job) + " began the quest for the Orb.";
    return Note(NOTE_MESSAGE, race, job, original, "morgue-start-v1\n" + name);
}

Note make_xp_level_note(int level, int hp, int maxhp, int mp, int maxmp)
{
    xlog_fields fields;
    fields.add_field("hp", "%d", hp); fields.add_field("mhp", "%d", maxhp);
    fields.add_field("mp", "%d", mp); fields.add_field("mmp", "%d", maxmp);
    return Note(NOTE_XP_LEVEL_CHANGE, level, 0,
        make_stringf("HP: %d/%d MP: %d/%d", hp, maxhp, mp, maxmp),
        "morgue-xp-v1\n" + fields.xlog_line());
}

Note make_version_note(const string &before, const string &after)
{
    xlog_fields fields;
    fields.add_field("before", "%s", before.c_str());
    fields.add_field("after", "%s", after.c_str());
    return Note(NOTE_MESSAGE, 0, 0,
        make_stringf("Upgraded the game from %s to %s", before.c_str(), after.c_str()),
        "morgue-version-v1\n" + fields.xlog_line());
}

static string _item_note_marker()
{
    return "morgue-item-v1:" + to_string(TAG_MAJOR_VERSION) + ":"
        + to_string(TAG_MINOR_VERSION) + "\n";
}

Note make_identified_item_note(const item_def &item, const string &description)
{
    Note note(NOTE_ID_ITEM, 0, 0, item.name(DESC_A), description);
    item_def snapshot = item;
    snapshot.pos = ITEM_IN_INVENTORY; snapshot.link = 0; snapshot.slot = 'a';
    if (!snapshot.is_valid(true)) return note;
    vector<unsigned char> bytes; auto output = writer(&bytes);
    marshallItem(output, snapshot, true);
    // screen is an existing short-string field; only user notes display it.
    if (bytes.size() > 15000) return note;
    const char *digits = "0123456789abcdef";
    note.screen = _item_note_marker();
    for (auto byte : bytes)
    { note.screen += digits[byte >> 4]; note.screen += digits[byte & 15]; }
    return note;
}

static bool _item_from_note(const Note &note, item_def &item)
{
    const string marker = _item_note_marker();
    if (!starts_with(note.screen, marker)) return false;
    const string hex = note.screen.substr(marker.size());
    if (hex.empty() || hex.size() % 2 || hex.size() > 30000) return false;
    vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        const auto digit = [](char c) -> int {
            return c >= '0' && c <= '9' ? c - '0'
                 : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
        };
        const int a = digit(hex[i]), b = digit(hex[i + 1]);
        if (a < 0 || b < 0) return false;
        bytes.push_back((a << 4) | b);
    }
    try
    {
        auto input = reader(bytes, TAG_MINOR_VERSION); input.set_safe_read(true);
        unmarshallItem(input, item);
        return item.is_valid(true) && item.name(DESC_A) == note.name;
    }
    catch (const short_read_exception &) { return false; }
}

Note make_obtained_item_note(const item_def &item, int time_left)
{
    Note note = make_identified_item_note(item, origin_desc(item));
    note.type = NOTE_GET_ITEM;
    note.first = time_left;
    return note;
}

static string _location_note_english(const string &kind, const string &name)
{
    if (name.empty()) return "";
    if (kind == "portal") return "Entered " + name;
    if (kind == "realm") return "Entered the realm of " + name + ".";
    return "";
}

Note make_location_note(const string &original, const string &kind, const string &name)
{
    Note note(NOTE_MESSAGE, 0, 0, original);
    if (original != _location_note_english(kind, name)) return note;
    xlog_fields fields;
    fields.add_field("kind", "%s", kind.c_str());
    fields.add_field("name", "%s", name.c_str());
    note.desc = "morgue-location-v1\n" + fields.xlog_line();
    return note;
}

static string _location_note_display(const string &kind, const string &name)
{
    if (kind == "portal")
        return ui_render("n.note", "Entered %s", "",
                         {ui_text_arg("lua.portal.name", name)});
    if (kind == "realm")
        return ui_render("n.note", "Entered the realm of %s.", "",
                         {ui_text_arg("display.monster", name)});
    return "";
}

// Old collectible notes have no item snapshot. Match the full canonical name
// against this finite set of typed collectibles; never parse an English noun.
static ui_arg _legacy_collectible_note_name(const string &name)
{
    for (auto base : {OBJ_RUNES, OBJ_GEMS, OBJ_ORBS})
    {
        const int count = base == OBJ_RUNES ? NUM_RUNE_TYPES
                        : base == OBJ_GEMS ? NUM_GEM_TYPES : 1;
        for (int sub = 0; sub < count; ++sub)
        {
            item_def item;
            item.base_type = base; item.sub_type = sub; item.quantity = 1;
            item.flags = ISFLAG_IDENTIFIED;
            if (item.name(DESC_A) == name) return ui_item_arg(item);
        }
    }
    return {"", false};
}

Note make_feature_note(const coord_def &where)
{
    const auto feat = env.map_knowledge(where).feat();
    const auto trap = env.map_knowledge(where).trap();
    xlog_fields fields;
    fields.add_field("raw", "%s", feature_description_at(where, false, DESC_PLAIN).c_str());
    if (feat == DNGN_ENTER_SHOP)
    {
        const shop_struct *shop = shop_at(where, true);
        if (shop)
        {
            fields.add_field("type", "%d", shop->type);
            fields.add_field("x", "%d", shop->pos.x); fields.add_field("y", "%d", shop->pos.y);
            fields.add_field("owner", "%s", shop->shop_name.c_str());
            fields.add_field("typename", "%s", shop->shop_type_name.c_str());
            fields.add_field("suffix", "%s", shop->shop_suffix_name.c_str());
            for (int i = 0; i < 3; ++i)
                fields.add_field("seed" + to_string(i), "%d", shop->keeper_name[i]);
        }
    }
    return Note(NOTE_SEEN_FEAT, feat, trap, feature_description_at(where, false, DESC_A) + ".",
                "morgue-feature-v1\n" + fields.xlog_line());
}

// Exact compatibility matches for portal notes written before typed metadata.
// Only English identities are listed here; translations stay in lua.portal.name.
static string _legacy_location_note_display(const string &original)
{
    for (const char *name : {
        "The Chambers of the Cloud Mage",
        "The Hall of the Hellbinder",
        "Doroklohe's Tomb",
        "Cigotuvi's Fleshworks",
        "Iskenderun's Mystic Tower",
        "Zonguldrok's Shrine",
        "Wucad Mu's Monastery",
        "The Roulette of Golubria",
        "Lehudib's Moon Base",
        "Eringya's Formal Garden",
        "Maxwell's Workshop",
        "Yara's Duelist Academy",
        "Borgnjor's Mausoleum",
        "Tukima's Studio",
        "Alistair's Party Mansion",
        "The Name-Rending Infernalists' Reservoir",
        "Ozocubu's Refrigerator",
        "The Storm Queen's Palace Crash-Site",
        "Leda's Sunken Stockpile",
        "Eringya's Secret Bog",
        "Rutra's Hidden Sanctum",
        "The Dread Knight's Derelict Chapel",
        "The First Justicar's Armoury",
        "The Octopus King's Forgotten Garden",
        "The Lost Hoard of Erebora",
        "a devouring Rift",
        "an orange crystal hatchery"})
        if (original == _location_note_english("portal", name))
            return _location_note_display("portal", name);
    for (const char *name : {"Mnoleg", "Lom Lobon", "Cerebov", "Gloorx Vloq"})
        if (original == _location_note_english("realm", name))
            return _location_note_display("realm", name);
    return "";
}

static ui_arg _feature_note_name(const Note &note)
{
    const string marker = "morgue-feature-v1\n";
    if (!starts_with(note.desc, marker) || note.first < 0 || note.first >= NUM_FEATURES
        || (note.second != TRAP_UNASSIGNED
            && (note.second < 0 || note.second >= NUM_TRAPS))) return {"", false};
    const auto feat = static_cast<dungeon_feature_type>(note.first);
    const auto trap = static_cast<trap_type>(note.second);
    const xlog_fields fields(note.desc.substr(marker.size()));
    const string raw = fields.str_field("raw");
    if (raw.empty()) return {"", false};
    if (feat == DNGN_ENTER_SHOP)
    {
        for (const string &key : {string("type"), string("x"), string("y"), string("seed0"), string("seed1"), string("seed2")})
            if (fields.str_field(key) != to_string(fields.int_field(key))) return {"", false};
        const int type = fields.int_field("type"), x = fields.int_field("x"), y = fields.int_field("y");
        if (type < 0 || type >= NUM_SHOPS || x < 0 || y < 0 || x >= GXM || y >= GYM) return {"", false};
        shop_struct shop; shop.type = static_cast<shop_type>(type); shop.pos = coord_def(x, y);
        shop.shop_name = fields.str_field("owner"); shop.shop_type_name = fields.str_field("typename");
        shop.shop_suffix_name = fields.str_field("suffix");
        for (int i = 0; i < 3; ++i)
        {
            const int seed = fields.int_field("seed" + to_string(i));
            if (seed < 0 || seed > 255) return {"", false};
            shop.keeper_name[i] = seed;
        }
        ui_arg shown;
        const string canonical = shop_name(shop, &shown);
        if (raw != canonical || note.name != canonical + ".") return {"", false};
        return shown;
    }
    const auto dtype = feat == DNGN_FLOOR ? DESC_THE : DESC_A;
    if (note.name != thing_do_grammar(dtype, raw, feat == DNGN_TRAP_ZOT) + ".") return {"", false};
    const ui_arg exact = ui_text_arg("d.feature.name", raw);
    if (exact.available) return exact;
    if (raw != feature_description(feat, trap, "", DESC_PLAIN)) return {"", false};
    string shown; const bool available = try_feature_display_name(feat, trap, shown);
    return {shown, available};
}

string Note::describe(bool when, bool where, bool what) const
{
    ostringstream result;

    if (when)
        result << setw(6) << turn << " ";

    if (where)
    {
        result << "| "
               << chop_string(place.describe(), MAX_NOTE_PLACE_LEN)
               << " | ";
    }

    if (what)
    {
        switch (type)
        {
        case NOTE_HP_CHANGE:
            // [ds] Shortened HP change note from "Had X hitpoints" to
            // accommodate the cause for the loss of hitpoints.
            result << "HP: " << first << "/" << second
                   << " [" << name << "]";
            break;
        case NOTE_XOM_REVIVAL:
            result << "Xom revived you";
            break;
        case NOTE_MP_CHANGE:
            result << "Magic: " << first << "/" << second;
            break;
        case NOTE_MAXHP_CHANGE:
            result << "Reached " << first << " max health";
            break;
        case NOTE_MAXMP_CHANGE:
            result << "Reached " << first << " max magic points";
            break;
        case NOTE_XP_LEVEL_CHANGE:
            result << "Reached XP level " << first << ". " << name;
            break;
        case NOTE_DUNGEON_LEVEL_CHANGE:
            if (!desc.empty())
                result << desc;
            else
                result << "Entered "
                       << place.describe(true, true);
            break;
        case NOTE_LEARN_SPELL:
            result << "Learned a level "
                   << spell_difficulty(static_cast<spell_type>(first))
                   << " spell: "
                   << spell_title(static_cast<spell_type>(first));
            break;
        case NOTE_GET_GOD:
            result << "Became a worshipper of "
                   << god_name(static_cast<god_type>(first), true);
            break;
        case NOTE_LOSE_GOD:
            result << "Fell from the grace of "
                   << god_name(static_cast<god_type>(first));
            break;
        case NOTE_PENANCE:
            result << "Was placed under penance by "
                   << god_name(static_cast<god_type>(first));
            break;
        case NOTE_MOLLIFY_GOD:
            result << "Was forgiven by "
                   << god_name(static_cast<god_type>(first));
            break;
        case NOTE_GOD_GIFT:
            result << "Received a gift from "
                   << god_name(static_cast<god_type>(first));
            if (!name.empty())
                result << " (" << name << ")";
            break;
        case NOTE_ID_ITEM:
            result << "Identified " << name;
            if (!desc.empty())
                result << " (" << desc << ")";
            break;
        case NOTE_GET_ITEM:
            result << "Got " << name;
            if (first != 0) // gems
            {
                const int turns = (first + 9) / 10;
                result << " with " << turns << " turn"
                       << (first == 1 ? "" : "s") << " to spare";
            }
            break;
        case NOTE_ACQUIRE_ITEM:
            result << "Acquired " << name;
            break;
        case NOTE_BUY_ITEM:
            result << "Bought " << name << " for " << first << " gold piece"
                   << (first == 1 ? "" : "s");
            break;
        case NOTE_DONATE_MONEY:
            result << "Donated " << first << " gold piece"
                   << (first == 1 ? "" : "s") << " to Zin";
            break;
        case NOTE_GAIN_SKILL:
            result << "Reached skill level " << second
                   << " in " << skill_name(static_cast<skill_type>(first));
            break;
        case NOTE_LOSE_SKILL:
            result << "Reduced skill "
                   << skill_name(static_cast<skill_type>(first))
                   << " to level " << second;
            break;
        case NOTE_SEEN_MONSTER:
            result << "Encountered " << name;
            break;
        case NOTE_DEFEAT_MONSTER:
            if (second)
                result << name << " (ally) was " << desc;
            else
                result << uppercase_first(desc) << " " << name;
            break;
        case NOTE_POLY_MONSTER:
            result << name << " changed into " << desc;
            break;
        case NOTE_PIETY_RANK:
            result << "Reached "
                   << string(second, '*')
                   << " piety under "
                   << god_name(static_cast<god_type>(first));
            break;
        case NOTE_GET_MUTATION:
            result << "Gained mutation: "
                   << mutation_desc(static_cast<mutation_type>(first),
                                    second == 0 ? 1 : second);
            if (!name.empty())
                result << " [" << name << "]";
            break;
        case NOTE_LOSE_MUTATION:
            result << "Lost mutation: "
                   << mutation_desc(static_cast<mutation_type>(first),
                                    second == 3 ? 3 : second+1);
            if (!name.empty())
                result << " [" << name << "]";
            break;
        case NOTE_PERM_MUTATION:
            result << "Mutation became permanent: "
                   << mutation_desc(static_cast<mutation_type>(first),
                                    second == 0 ? 1 : second);
            if (!name.empty())
                result << " [" << name << "]";
            break;
        case NOTE_GET_BANE:
            result << "Gained bane: "
                   << bane_name(static_cast<bane_type>(first));
            if (!name.empty())
                result << " [" << name << "]";
            break;
        case NOTE_LOSE_BANE:
            result << "Lost bane: "
                   << bane_name(static_cast<bane_type>(first));
            break;
        case NOTE_DEATH:
            result << name;
            break;
        case NOTE_USER_NOTE:
            result << Options.user_note_prefix << name;
            break;
        case NOTE_MESSAGE:
            result << name;
            break;
        case NOTE_SEEN_FEAT:
            result << "Found " << name;
            break;
        case NOTE_FEAT_MIMIC:
            result << name <<" was a mimic.";
            break;
        case NOTE_XOM_EFFECT:
            result << "XOM: " << name;
#if defined(DEBUG_XOM) || defined(NOTE_DEBUG_XOM)
            // If debugging, also take note of piety and tension.
            result << " (piety: " << first;
            if (second >= 0)
                result << ", tension: " << second;
            result << ")";
#endif
            break;
        case NOTE_PARALYSIS:
        {
            const float turns = first / 10.0;
            result << "Paralysed by " << name << " for " << setprecision(2) << turns << " turns";
        }
            break;
        case NOTE_VEXED:
            result << "Vexed by " << name << " for " << first << " turns";
            break;
        case NOTE_RECRUITED_APOSTLE:
            result << "Anointed " << name << " the " << desc << " as your apostle";
            break;
        case NOTE_ALLY_DEATH:
            result << "Your ally " << name << " died";
            break;
        case NOTE_OFFERED_SPELL:
            result << "Offered knowledge of "
                   << spell_title(static_cast<spell_type>(first))
                   << " by Vehumet.";
            break;
        case NOTE_ANCESTOR_TYPE:
            result << "Remembered your ancestor " << hepliaklqana_ally_name()
                   << " as " << name;
            break;
#if TAG_MAJOR_VERSION == 34
        case NOTE_ANCESTOR_SPECIALIZATION:
            result << "Remembered your ancestor " << hepliaklqana_ally_name()
                   << " " << name;
            break;
        case NOTE_ANCESTOR_DEATH:
            result << "Remembered your ancestor "
                   << apostrophise(hepliaklqana_ally_name())
                   << " " << name << " death";
            break;
#endif
          case NOTE_FOUND_UNRAND:
            result << "Found " << name;
            break;
        case NOTE_ZOT_TOUCHED:
            result << "Touched by the power of Zot (MHP " << first << " -> " << second << ")";
            break;
        case NOTE_DREAMSHARD:
            result << "Saved by the dreamshard amulet";
            break;
        case NOTE_GEM_LOST:
            result << "Lost the "
                   << gem_adj(static_cast<gem_type>(first))
                   << " gem through the power of Zot.";
            break;
        case NOTE_GAIN_LIFE:
            result << "Gained a life (" << first << (first == 1 ? " life " : " lives ") << "remaining)";
            break;
        case NOTE_LOSE_LIFE:
            result << "Lost a life ("   << first << (first == 1 ? " life " : " lives ") << "remaining)";
            break;
        case NOTE_FLED_CHALLENGE:
            result << "Fled from a divine trial";
            break;
        case NOTE_INFERNAL_MARK:
            result << "Branded self with the " << name;
            break;
        case NOTE_TESSERACT_ACTIVATED:
            result << "Activated a boundless tesseract";
            break;
        default:
            result << "Buggy note description: unknown note type";
            break;
        }
    }

    if (type == NOTE_SEEN_MONSTER || type == NOTE_DEFEAT_MONSTER)
    {
        if (what && first == MONS_PANDEMONIUM_LORD)
            result << " the pandemonium lord";
    }
    return result.str();
}

// Historical note fields are opaque canonical snapshots. Only exact reviewed
// keys may be translated; never infer an entity by parsing rendered English.
static ui_arg _n_note_text(const string& text)
{
    if (text.empty()) return {"", true};
    for (const char* context : {"n.note.text", "display.monster", "display.item",
                               "display.god", "a.mutation.name", "b.bane.name",
                               "display.skill", "display.spell", "display.creation",
                               "m.fakemut", "m.mutation", "d.monster.state"})
    {
        const ui_arg shown = ui_text_arg(context, text);
        if (shown.available) return shown;
    }
    for (int i = 0; i < NUM_MONSTERS; ++i)
    {
        if (invalid_monster_index(i)) continue;
        const auto type = static_cast<monster_type>(i);
        if (text == mons_type_name(type, DESC_A) || text == mons_type_name(type, DESC_THE))
            return ui_text_arg("display.monster", mons_type_name(type, DESC_PLAIN));
    }
    return {"", false};
}

static ui_arg _n_note_source(const string &text)
{
    const auto known = _n_note_text(text);
    if (known.available) return known;
    const auto literal = ui_text_arg("n.note.reason", text);
    if (literal.available) return literal;
    // Rebuild only finite canonical productions, never strip English prefixes.
    for (auto race : species::get_all_species())
        if (text == species::name(race) + " growth")
        {
            ui_arg name = ui_text_arg("display.creation", species::name(race));
            if (!name.available)
                name = ui_text_arg("d.god.title.component", species::name(race));
            const string shown = ui_render("n.note.reason", "%s growth", "", {name});
            return {shown, !shown.empty()};
        }
    for (int god = GOD_NO_GOD + 1; god < NUM_GODS; ++god)
    for (const char *pattern : {"the wrath of %s", "the malice of %s", "the anger of %s",
         "the capriciousness of %s", "the fury of %s", "the will of %s", "the fiery rage of %s",
         "the displeasure of %s", "the touch of %s", "the all-consuming vengeance of %s",
         "the enmity of %s", "the meddling of %s", "the doom of %s", "the darkness of %s",
         "the greed of %s", "the adversity of %s", "the disappointment of %s", "the progress of %s",
         "the memory of %s", "the rancor of %s", "the fiery vengeance of %s"})
    for (bool full : {false, true})
        if (text == make_stringf(pattern, god_name(static_cast<god_type>(god), full).c_str()))
        {
            const string shown = ui_render("n.note.reason", pattern, "",
                {ui_text_arg("display.god", god_name(static_cast<god_type>(god)))});
            return {shown, !shown.empty()};
        }
    // Historical disabling and mutation notes only stored the source name.
    // An exact identity match against a typed encounter supplies its role.
    for (const auto &note : note_list)
        if ((note.type == NOTE_SEEN_MONSTER || note.type == NOTE_DEFEAT_MONSTER)
            && note.first == MONS_PANDEMONIUM_LORD && note.name == text)
        {
            const string shown = ui_render("n.note", "%s the pandemonium lord", "",
                                           {ui_generated_name_arg(text)});
            return {shown, !shown.empty()};
        }
    return {"", false};
}

static ui_arg _n_disabled_source(const Note &note)
{
    const string marker = "morgue-pan-source-v1\n";
    if (starts_with(note.desc, marker))
    {
        if (note.name.empty() || note.desc.substr(marker.size()) != note.name)
            return {"", false};
        const string shown = ui_render("n.note", "%s the pandemonium lord", "",
                                       {ui_generated_name_arg(note.name)});
        return {shown, !shown.empty()};
    }
    return _n_note_source(note.name);
}

string Note::display_describe(bool when, bool where, bool what) const
{
    const string original = describe(when, where, what);
    if (!what || !ui_translation_group({{"n.note", "Xom revived you"}}).available())
        return original;
    ui_arg n = _n_note_text(name);
    // The note type supplies the identity of a randomly named Pan lord.
    // Its saved name is already a literal proper name, not a rendered clause.
    if ((type == NOTE_SEEN_MONSTER || type == NOTE_DEFEAT_MONSTER)
        && first == MONS_PANDEMONIUM_LORD && !name.empty())
        n = ui_generated_name_arg(name);
    if ((type == NOTE_SEEN_MONSTER || type == NOTE_DEFEAT_MONSTER)
        && first == MONS_PANDEMONIUM_LORD)
    {
        const string qualified = ui_render("n.note", "%s the pandemonium lord", "", {n});
        n = {qualified, !qualified.empty()};
    }
    const ui_arg d = _n_note_text(desc);
    const ui_arg f = ui_literal_arg(to_string(first));
    const ui_arg s = ui_literal_arg(to_string(second));
    const auto render = [&](const char* source, std::initializer_list<ui_arg> args) {
        return ui_render("n.note", source, "", args);
    };
    string shown;
    switch (type)
    {
    case NOTE_HP_CHANGE:
    {
        ui_arg reason = n;
        const string marker = "morgue-damage-v1\n";
        const string unseen = "morgue-damage-unseen-v1\n";
        if (starts_with(desc, marker))
        {
            scorefile_entry score;
            const string raw = desc.substr(marker.size());
            if (raw.empty() || !score.parse(raw)
                || score.death_description(scorefile_entry::DDV_TERSE) != name)
                return original;
            const string translated = score.display_damage_description();
            if (translated == name) return original;
            reason = ui_literal_arg(translated);
        }
        else if (starts_with(desc, unseen))
        {
            const string number = desc.substr(unseen.size());
            const xlog_fields fields("dam=" + number);
            if (number != to_string(fields.int_field("dam"))
                || name != "something (" + number + ")") return original;
            const string text = render("something (%s)", {ui_literal_arg(number)});
            reason = {text, !text.empty()};
        }
        shown = render("HP: %s/%s [%s]", {f, s, reason});
        break;
    }
    case NOTE_XOM_REVIVAL:
        shown = render("Xom revived you", {});
        break;
    case NOTE_MP_CHANGE:
        shown = render("Magic: %s/%s", {f, s});
        break;
    case NOTE_MAXHP_CHANGE:
        shown = render("Reached %s max health", {f});
        break;
    case NOTE_MAXMP_CHANGE:
        shown = render("Reached %s max magic points", {f});
        break;
    case NOTE_XP_LEVEL_CHANGE:
    {
        ui_arg stats = n;
        const string marker = "morgue-xp-v1\n";
        if (starts_with(desc, marker))
        {
            const xlog_fields fields(desc.substr(marker.size()));
            for (const char *key : {"hp", "mhp", "mp", "mmp"})
                if (fields.str_field(key) != to_string(fields.int_field(key)))
                    return original;
            const int hp = fields.int_field("hp"), maxhp = fields.int_field("mhp");
            const int mp = fields.int_field("mp"), maxmp = fields.int_field("mmp");
            if (name != make_stringf("HP: %d/%d MP: %d/%d", hp, maxhp, mp, maxmp))
                return original;
            const string text = render("HP: %s/%s MP: %s/%s",
                {ui_literal_arg(to_string(hp)), ui_literal_arg(to_string(maxhp)),
                 ui_literal_arg(to_string(mp)), ui_literal_arg(to_string(maxmp))});
            stats = {text, !text.empty()};
        }
        shown = render("Reached XP level %s. %s", {f, stats});
        break;
    }
    case NOTE_DUNGEON_LEVEL_CHANGE:
        if (!desc.empty()) shown = d.available ? d.text : "";
        else if (place.is_valid())
            shown = render("Entered %s, level %s",
                {ui_text_arg("b.branch.name", branches[place.branch].longname),
                 ui_literal_arg(to_string(place.depth))});
        break;
    case NOTE_LEARN_SPELL:
        shown = render("Learned a level %s spell: %s", {ui_literal_arg(to_string(spell_difficulty(static_cast<spell_type>(first)))), ui_text_arg("display.spell", spell_title(static_cast<spell_type>(first)))});
        break;
    case NOTE_GET_GOD:
        shown = render("Became a worshipper of %s", {ui_text_arg("display.god", god_name(static_cast<god_type>(first)))});
        break;
    case NOTE_LOSE_GOD:
        shown = render("Fell from the grace of %s", {ui_text_arg("display.god", god_name(static_cast<god_type>(first)))});
        break;
    case NOTE_PENANCE:
        shown = render("Was placed under penance by %s", {ui_text_arg("display.god", god_name(static_cast<god_type>(first)))});
        break;
    case NOTE_MOLLIFY_GOD:
        shown = render("Was forgiven by %s", {ui_text_arg("display.god", god_name(static_cast<god_type>(first)))});
        break;
    case NOTE_GOD_GIFT:
        shown = name.empty() ? render("Received a gift from %s%s",
            {ui_text_arg("display.god", god_name(static_cast<god_type>(first))), n})
            : render("Received a gift from %s (%s)",
                {ui_text_arg("display.god", god_name(static_cast<god_type>(first))), n});
        break;
    case NOTE_ID_ITEM:
    {
        ui_arg item_name = n, origin = d;
        if (starts_with(screen, "morgue-item-v1:"))
        {
            item_def item;
            if (!_item_from_note(*this, item)) return original;
            item_name = ui_item_arg(item);
            if (!desc.empty())
            {
                if (desc != origin_desc(item)) return original;
                string translated = item_origin_description(item);
                if (translated == desc + ".") return original;
                if (ends_with(translated, "。")) translated.resize(translated.size() - string("。").size());
                origin = ui_literal_arg(translated);
            }
        }
        shown = desc.empty() ? render("Identified %s", {item_name})
            : render("Identified %s (%s)", {item_name, origin});
        break;
    }
    case NOTE_GET_ITEM:
    {
        ui_arg item_name = n;
        if (starts_with(screen, "morgue-item-v1:"))
        {
            item_def item;
            if (!_item_from_note(*this, item)) return original;
            item_name = ui_item_arg(item);
        }
        else if (!item_name.available)
            item_name = _legacy_collectible_note_name(name);
        shown = first == 0 ? render("Got %s", {item_name})
            : render("Got %s with %s turns to spare", {item_name, ui_literal_arg(to_string((first + 9) / 10))});
        break;
    }
    case NOTE_ACQUIRE_ITEM:
        shown = render("Acquired %s", {n});
        break;
    case NOTE_BUY_ITEM:
        shown = render("Bought %s for %s gold pieces", {n, f});
        break;
    case NOTE_DONATE_MONEY:
        shown = render("Donated %s gold pieces to Zin", {f});
        break;
    case NOTE_GAIN_SKILL:
        shown = render("Reached skill level %s in %s", {s, ui_text_arg("display.skill", skill_name(static_cast<skill_type>(first)))});
        break;
    case NOTE_LOSE_SKILL:
        shown = render("Reduced skill %s to level %s", {ui_text_arg("display.skill", skill_name(static_cast<skill_type>(first))), s});
        break;
    case NOTE_SEEN_MONSTER:
        shown = render("Encountered %s", {n});
        break;
    case NOTE_DEFEAT_MONSTER:
        shown = second ? render("%s (ally) was %s", {n, d})
            : render("%s %s", {d, n});
        break;
    case NOTE_POLY_MONSTER:
        shown = render("%s changed into %s", {n, d});
        break;
    case NOTE_PIETY_RANK:
        shown = render("Reached %s piety under %s", {ui_literal_arg(string(second, '*')), ui_text_arg("display.god", god_name(static_cast<god_type>(first)))});
        break;
    case NOTE_GET_MUTATION:
    case NOTE_LOSE_MUTATION:
    case NOTE_PERM_MUTATION:
    {
        const int level = type == NOTE_LOSE_MUTATION
            ? (second == 3 ? 3 : second + 1) : (second == 0 ? 1 : second);
        bool available = false;
        const string mutation = mutation_display_desc(static_cast<mutation_type>(first),
            level, false, false, &available);
        const char* key = type == NOTE_GET_MUTATION ? "Gained mutation: %s%s"
            : type == NOTE_LOSE_MUTATION ? "Lost mutation: %s%s"
            : "Mutation became permanent: %s%s";
        ui_arg reason = _n_note_source(name);
        if (!name.empty() && reason.available) reason.text = "［" + reason.text + "］";
        shown = render(key, {{mutation, available}, reason});
        break;
    }
    case NOTE_GET_BANE:
        shown = render("Gained bane: %s%s", {bane_display_arg(static_cast<bane_type>(first)), _n_note_source(name)});
        break;
    case NOTE_LOSE_BANE:
        shown = render("Lost bane: %s", {bane_display_arg(static_cast<bane_type>(first))});
        break;
    case NOTE_SEEN_FEAT:
    {
        const ui_arg feature = starts_with(desc, "morgue-feature-v1\n") ? _feature_note_name(*this) : n;
        shown = starts_with(desc, "morgue-feature-v1\n")
            ? render("Found %s.", {feature}) : render("Found %s", {feature});
        break;
    }
    case NOTE_FEAT_MIMIC:
        shown = render("%s was a mimic.", {n});
        break;
    case NOTE_XOM_EFFECT:
        shown = render("XOM: %s", {n});
        break;
    case NOTE_PARALYSIS:
    {
        ostringstream turns;
        turns << setprecision(2) << first / 10.0f;
        shown = render("Paralysed by %s for %s turns", {_n_disabled_source(*this), ui_literal_arg(turns.str())});
        break;
    }
    case NOTE_VEXED:
        shown = render("Vexed by %s for %s turns", {_n_disabled_source(*this), f});
        break;
    case NOTE_RECRUITED_APOSTLE:
        shown = render("Anointed %s the %s as your apostle", {n, d});
        break;
    case NOTE_ALLY_DEATH:
        shown = render("Your ally %s died", {n});
        break;
    case NOTE_OFFERED_SPELL:
        shown = render("Offered knowledge of %s by Vehumet.", {ui_text_arg("display.spell", spell_title(static_cast<spell_type>(first)))});
        break;
    case NOTE_ANCESTOR_TYPE:
        shown = render("Remembered your ancestor %s as %s", {ui_literal_arg(hepliaklqana_ally_name()), n});
        break;
#if TAG_MAJOR_VERSION == 34
    case NOTE_ANCESTOR_SPECIALIZATION:
        shown = render("Remembered your ancestor %s %s", {ui_literal_arg(hepliaklqana_ally_name()), n});
        break;
    case NOTE_ANCESTOR_DEATH:
        shown = render("Remembered your ancestor %s's %s death", {ui_literal_arg(hepliaklqana_ally_name()), n});
        break;
#endif
    case NOTE_FOUND_UNRAND:
        shown = render("Found the artefact %s", {n});
        break;
    case NOTE_ZOT_TOUCHED:
        shown = render("Touched by the power of Zot (MHP %s -> %s)", {f, s});
        break;
    case NOTE_DREAMSHARD:
        shown = render("Saved by the dreamshard amulet", {});
        break;
    case NOTE_GEM_LOST:
        shown = render("Lost the %s gem through the power of Zot.", {ui_text_arg("display.item", string(gem_adj(static_cast<gem_type>(first))) + " gem")});
        break;
    case NOTE_GAIN_LIFE:
        shown = render("Gained a life (%s lives remaining)", {f});
        break;
    case NOTE_LOSE_LIFE:
        shown = render("Lost a life (%s lives remaining)", {f});
        break;
    case NOTE_FLED_CHALLENGE:
        shown = render("Fled from a divine trial", {});
        break;
    case NOTE_INFERNAL_MARK:
        shown = render("Branded self with the %s", {ui_text_arg("a.mutation.name", name)});
        break;
    case NOTE_TESSERACT_ACTIVATED:
        shown = render("Activated a boundless tesseract", {});
        break;
    case NOTE_DEATH:
    {
        const string normal = "morgue-score-normal-v1\n";
        const string verbose = "morgue-score-verbose-v1\n";
        const bool full = starts_with(desc, verbose);
        if (!full && !starts_with(desc, normal))
        {
            shown = ui_text_arg("n.note.text", name).text;
            break;
        }
        scorefile_entry score;
        string raw = desc.substr(full ? verbose.size() : normal.size());
        while (!raw.empty() && (raw.back() == '\n' || raw.back() == '\r'))
            raw.pop_back();
        if (raw.empty() || !score.parse(raw)) return original;
        const auto verbosity = full ? scorefile_entry::DDV_VERBOSE
                                    : scorefile_entry::DDV_NORMAL;
        // Metadata must reproduce the complete original snapshot before use.
        if (score.death_description(verbosity) != name) return original;
        shown = score.display_death_description(verbosity);
        break;
    }
    case NOTE_MESSAGE:
    {
        const string start = "morgue-start-v1\n";
        const string version = "morgue-version-v1\n";
        if (starts_with(desc, start))
        {
            const auto race = static_cast<species_type>(first);
            if (!species::is_valid(race) || second < 0 || second >= NUM_JOBS)
                return original;
            const string player = desc.substr(start.size());
            const string species_name = species::name(race);
            const string job = get_job_name(static_cast<job_type>(second));
            if (name != player + " the " + species_name + " " + job
                + " began the quest for the Orb.") return original;
            shown = render("%s the %s %s began the quest for the Orb.",
                {ui_literal_arg(player), ui_text_arg("display.creation", species_name),
                 ui_text_arg("display.creation", job)});
        }
        else if (starts_with(desc, version))
        {
            const xlog_fields fields(desc.substr(version.size()));
            const string before = fields.str_field("before"), after = fields.str_field("after");
            if (before.empty() || after.empty() || name != make_stringf(
                "Upgraded the game from %s to %s", before.c_str(), after.c_str()))
                return original;
            shown = render("Upgraded the game from %s to %s",
                {ui_literal_arg(before), ui_literal_arg(after)});
        }
        else if (starts_with(desc, "morgue-location-v1\n"))
        {
            const xlog_fields fields(desc.substr(string("morgue-location-v1\n").size()));
            const string kind = fields.str_field("kind"), place_name = fields.str_field("name");
            if (name != _location_note_english(kind, place_name)) return original;
            shown = _location_note_display(kind, place_name);
        }
        else
        {
            const ui_arg text = ui_text_arg("n.note.message", name);
            if (text.available) shown = text.text;
            else shown = _legacy_location_note_display(name);
        }
        break;
    }
    case NOTE_USER_NOTE:
    default:
        return original;
    }
    if (shown.empty()) return original;
    return describe(when, where, false) + shown;
}

bool Note::hidden() const
{
    // Hide skill gains that are not enabled by options.
    if (type == NOTE_GAIN_SKILL || type == NOTE_LOSE_SKILL)
    {
        return !(Options.note_all_skill_levels
                 || second <= 27 && Options.note_skill_levels[second]
                 || Options.note_skill_max && _is_highest_skill(first));
    }
    // Hide gems being shattered by default.
    if (type == NOTE_GEM_LOST)
        return !Options.more_gem_info;
    return false;
}

void Note::check_milestone() const
{
    if (crawl_state.game_is_arena())
        return;

    if (type == NOTE_DUNGEON_LEVEL_CHANGE)
    {
        const int br = place.branch,
                 dep = place.depth;

        // Wizlabs report their milestones on their own.
        if (br != -1 && br != BRANCH_WIZLAB)
        {
            ASSERT_RANGE(br, 0, NUM_BRANCHES);
            string branch = place.describe(true, false);

            if (starts_with(branch, "The "))
                branch[0] = tolower_safe(branch[0]);

            if (dep == 1)
            {
                mark_milestone(br == BRANCH_ZIGGURAT ? "zig.enter" : "br.enter",
                               "entered " + branch + ".", "parent");
            }
            else if (dep == _dungeon_branch_depth(br)
                     || br == BRANCH_ZIGGURAT)
            {
                string level = place.describe(true, true);
                if (starts_with(level, "Level "))
                    level[0] = tolower_safe(level[0]);

                mark_milestone(br == BRANCH_ZIGGURAT ? "zig" : "br.end",
                               "reached " + level + ".");
            }
        }
    }
}

void Note::save(writer& outf) const
{
    marshallInt(outf, type);
    marshallInt(outf, turn);
    place.save(outf);
    marshallInt(outf, first);
    marshallInt(outf, second);
    marshallString4(outf, name);
    marshallString4(outf, desc);
    marshallString(outf, screen);
}

void Note::load(reader& inf)
{
    type = static_cast<NOTE_TYPES>(unmarshallInt(inf));
    turn = unmarshallInt(inf);
#if TAG_MAJOR_VERSION == 34
    if (inf.getMinorVersion() < TAG_MINOR_PLACE_UNPACK)
        place = level_id::from_packed_place(unmarshallShort(inf));
    else
#endif
    place.load(inf);
    first  = unmarshallInt(inf);
    second = unmarshallInt(inf);
    unmarshallString4(inf, name);
    unmarshallString4(inf, desc);
#if TAG_MAJOR_VERSION == 34
    if (inf.getMinorVersion() >= TAG_MINOR_MORGUE_SCREENSHOTS)
        screen = unmarshallString(inf);
#else
    screen = unmarshallString(inf);
#endif

}

static bool notes_active = false;

bool notes_are_active()
{
    return notes_active;
}

void take_note(const Note& note, bool force)
{
    if (notes_active && (force || _is_noteworthy(note)))
    {
        note_list.push_back(note);
        note.check_milestone();
    }
}

void activate_notes(bool active)
{
    notes_active = active;
}

void save_notes(writer& outf)
{
    marshallInt(outf, NOTES_VERSION_NUMBER);
    marshallInt(outf, note_list.size());
    for (const Note &note : note_list)
        note.save(outf);
}

void load_notes(reader& inf)
{
    if (unmarshallInt(inf) != NOTES_VERSION_NUMBER)
        return;

    const int num_notes = unmarshallInt(inf);
    for (int i = 0; i < num_notes; ++i)
    {
        Note new_note;
        new_note.load(inf);
        note_list.push_back(new_note);
    }
}

void make_user_note()
{
    char buf[400];
    bool validline = !msgwin_get_line(ui_translate("n.ui", "Enter note: "), buf, sizeof(buf));
    if (!validline || (!*buf))
        return;
    Note unote(NOTE_USER_NOTE);
    unote.name = buf;
    // Only one screenshot a turn allowed
    if (last_screen_turn != unote.turn)
    {
        last_screen_turn = unote.turn;
        unote.screen = screenshot();
    }
    take_note(unote);
}
