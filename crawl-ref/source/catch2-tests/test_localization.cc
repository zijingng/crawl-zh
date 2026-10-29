#include "AppHdr.h"
#include "viewgeom.h"
#include "mapmark.h"
#include <memory>
#include "wizard.h"
#include "prompt.h"
#include "viewmap.h"
#include "version.h"
#include "t-display.h"
#include "target.h"
#include "catch_amalgamated.hpp"

#include "format.h"
#include "files.h"
#include "scroller.h"
#include "command.h"
#include "stash.h"
#include "pattern.h"
#include "i18n.h"
#include "i18n-catalogue.h"
#include "options.h"
#include "stringutil.h"
#include "unwind.h"
#include "unicode.h"
#include "unicode-wrap.h"
#include "skills.h"
#include "spl-util.h"
#include "database.h"
#include "misc-display.h"
#include "syscalls.h"
#include "item-name.h"
#include "item-use.h"
#include "player-notices.h"
#include "ghost.h"
#include "corpse.h"
#include "item-prop.h"
#include "item-status-flag-type.h"
#include "items.h"
#include "invent.h"
#include "potion-type.h"
#include "mon-info.h"
#include "mon-info-flag-name.h"
#include "mon-util.h"
#include "mon-speak.h"
#include "artefact.h"
#include "art-enum.h"
#include "macro.h"
#include "ability.h"
#include "decks.h"
#include "describe.h"
#include "branch.h"
#include "branch-data-json.h"
#include "mutation.h"
#include "cloud.h"
#include "chardump.h"
#include "state.h"
#include "describe-god.h"
#include "status.h"
#include "religion.h"
#include "book-type.h"
#include "randbook.h"
#include "describe-spells.h"
#include "directn.h"
#include "feature.h"
#include "spl-book.h"
#include "spl-goditem.h"
#include "orb-type.h"
#include "evoke.h"
#include "exclude.h"
#include "dgn-overview.h"
#include "transform.h"
#include "god-abil.h"
#include "god-conduct.h"
#include "hints.h"
#include "hiscores.h"
#include "i18n-item.h"
#include "nearby-danger.h"
#include "env.h"
#include "species.h"
#include "kills.h"
#include "monster.h"
#include "clua.h"
#include "l-libs.h"
#include "lookup-help.h"
#include "m-display.h"
#include "newgame.h"
#include "newgame-def.h"
#include "notes.h"
#include "output.h"
#include "melee-attack.h"
#include "map-cell.h"
#include "tags.h"
#include "p-display.h"
#include "player-stats.h"
#include "quiver.h"
#include "spl-damage.h"
#include "spl-selfench.h"
#include "shopping.h"
#include "s-display.h"
#include "x-display.h"
#include "zot.h"
#include "player-save-info.h"
#include "shout.h"
#include "item-noise-display.h"
#include "decor-display.h"
#include "english.h"
#include "attack.h"
#include "fight.h"
#include "miscast-display.h"
#include "initfile.h"
#include "misc-display.h"
#include "artefact-speech.h"
#include "terrain.h"
#include "dungeon.h"
#include "delay.h"
#include "beam.h"
#include "beam-display.h"
#include "target-display.h"
#include "travel.h"
#include "explore-stop-options.h"
#include <fstream>

TEST_CASE("Reported UI save summaries and welcome text translate typed roles without renaming saves", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    player_save_info saved;
    saved.name = "Ceneedgh";
    saved.filename = "Ceneedgh.cs";
    saved.species = SP_MINOTAUR;
    saved.job = JOB_FIGHTER;
    saved.species_name = "Minotaur";
    saved.class_name = "Fighter";
    saved.experience_level = 3;
    saved.saved_game_type = GAME_TYPE_NORMAL;
    const string original = saved.short_desc();
    const string short_original = saved.really_short_desc();
    const auto random_state = rng::peek_uint64();
    CHECK(save_character_display(saved) == original);
    CHECK(save_character_display(saved, true) == short_original);
    CHECK(character_welcome_display(false, saved.name, saved.species, saved.job).empty());
    Options.language = lang_t::ZH;
    CHECK(save_character_display(saved) == "Ceneedgh，3级牛头人战士");
    CHECK(save_character_display(saved, true) == "Ceneedgh（牛头人战士）");
    CHECK(character_welcome_display(false, saved.name, saved.species, saved.job)
        == "<yellow>欢迎回来，Ceneedgh（牛头人战士）。</yellow>");
    CHECK(character_welcome_display(true, saved.name, saved.species, saved.job)
        == "<yellow>欢迎，Ceneedgh（牛头人战士）。</yellow>");
    saved.saved_game_type = GAME_TYPE_SPRINT;
    CHECK(save_character_display(saved).find("[地下城冲刺]") == 0);
    CHECK(save_character_display(saved, false, false) == "Ceneedgh，3级牛头人战士");
    saved.religion = GOD_ZIN;
    saved.god_name = "new upstream deity";
    CHECK(save_character_display(saved) == saved.short_desc());
    saved.religion = GOD_NO_GOD;
    saved.species_name = "Historical Minotaur";
    CHECK(save_character_display(saved) == saved.short_desc());
    CHECK(save_character_display(saved, true) == saved.really_short_desc());
    CHECK(saved.name == "Ceneedgh");
    CHECK(saved.filename == "Ceneedgh.cs");
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Reported UI terrain lookup cannot misclassify arbitrary feature keys as Fighting", "[localization]")
{
    init_show_table();
    init_feat_desc_cache();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto random_state = rng::peek_uint64();
    string shown;
    CHECK(str_to_skill("rock wall") == SK_NONE);
    for (const string name : {"a rock wall", "a stone wall", "the floor", "a shop"})
    {
        INFO(name);
        REQUIRE(try_lookup_help_display_name(name, shown));
        CHECK(shown != "战斗");
        CHECK(shown != name);
    }
    REQUIRE(try_lookup_help_display_name("Fighting", shown));
    CHECK(shown == "战斗");
    CHECK_FALSE(try_lookup_help_display_name("unknown upstream terrain", shown));
    Options.language = lang_t::EN;
    CHECK_FALSE(try_lookup_help_display_name("a rock wall", shown));
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Reported UI actual map hover uses remembered terrain display and preserves its English", "[localization]")
{
#ifdef USE_TILE
    init_show_table();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<bool> on_level(you.on_current_level, false);
    const coord_def cell(10, 10);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<dungeon_feature_type> terrain(env.grid(cell), DNGN_ROCK_WALL);
    unwind_var<unsigned int> vault_id(env.level_map_ids(cell), INVALID_MAP_INDEX);
    env.map_knowledge(cell).set_feature(DNGN_ROCK_WALL);
    env.map_knowledge(cell).flags = MAP_SEEN_FLAG;
    const auto random_state = rng::peek_uint64();
    const string original = get_cell_mouseover_tag(cell);
    CHECK(original.find("rock wall") != string::npos);
    Options.language = lang_t::ZH;
    const string shown = get_cell_mouseover_tag(cell);
    CHECK(shown.front() == '[');
    CHECK(shown.back() == ']');
    CHECK(shown != original);
    CHECK(shown.find("rock wall") == string::npos);
    CHECK(env.map_knowledge(cell).feat() == DNGN_ROCK_WALL);
    CHECK(rng::peek_uint64() == random_state);
    Options.language = lang_t::EN;
    CHECK(get_cell_mouseover_tag(cell) == original);
#endif
}

TEST_CASE("Look mode square descriptions translate terrain while canonical feature grammar stays English", "[localization]")
{
    init_show_table();
    init_feat_desc_cache();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<bool> on_level(you.on_current_level, false);
    const coord_def cell(10, 10);
    unwind_var<int> floor_item(env.igrid(cell), NON_ITEM);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<dungeon_feature_type> terrain(env.grid(cell));
    unwind_var<unsigned int> vault_id(env.level_map_ids(cell), INVALID_MAP_INDEX);
    REQUIRE(env.markers.find(cell) == nullptr);
    const auto state = rng::peek_uint64();
    for (auto feature : {DNGN_ROCK_WALL, DNGN_STONE_WALL, DNGN_CLOSED_DOOR, DNGN_STONE_STAIRS_DOWN_I})
    {
        env.grid(cell) = feature;
        env.map_knowledge(cell) = map_cell();
        env.map_knowledge(cell).set_feature(feature);
        env.map_knowledge(cell).flags = MAP_SEEN_FLAG;
        const string canonical = cell_floor_description(cell, true);
        const string original = get_square_desc(cell);
        CHECK(original.find(canonical) != string::npos);
        Options.language = lang_t::ZH;
        string name;
        REQUIRE(try_feature_display_at(cell, true, name));
        const string shown = get_square_desc(cell);
        CHECK(shown.find(string(ui_translate("d.target", "<cyan>Here:</cyan> ")) + name) == 0);
        CHECK(shown.find(canonical) == string::npos);
        CHECK(cell_floor_description(cell, true) == canonical);
        CHECK(ui_render("d.action", "%s.", canonical + ".", {{name, true}}) == name + "。");
        CHECK(ui_render("d.action", "%s.", canonical + ".", {{"", false}}) == canonical + ".");
        Options.language = lang_t::EN;
        CHECK(get_square_desc(cell) == original);
    }
    env.grid(cell) = DNGN_FLOOR;
    env.map_knowledge(cell).set_feature(DNGN_FLOOR);
    CHECK(cell_floor_description(cell, false).empty());
    CHECK(cell_floor_description(cell, true) == "the floor");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Unknown authored look terrain keeps the entire row English", "[localization]")
{
    init_show_table();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> on_level(you.on_current_level, false);
    const coord_def cell(10, 10);
    unwind_var<int> floor_item(env.igrid(cell), NON_ITEM);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<dungeon_feature_type> terrain(env.grid(cell), DNGN_ROCK_WALL);
    unwind_var<unsigned int> vault_id(env.level_map_ids(cell), INVALID_MAP_INDEX);
    env.map_knowledge(cell) = map_cell();
    env.map_knowledge(cell).set_feature(DNGN_ROCK_WALL);
    env.map_knowledge(cell).flags = MAP_SEEN_FLAG;
    REQUIRE(env.markers.find(cell) == nullptr);
    auto *marker = new map_wiz_props_marker(cell);
    marker->set_property("feature_description", "mysterious upstream terrain");
    env.markers.add(marker);
    struct cleanup { map_marker *p; ~cleanup() { env.markers.remove(p); } } reset{marker};
    const string canonical = cell_floor_description(cell, true);
    const auto state = rng::peek_uint64();
    CHECK(get_square_desc(cell).find("<cyan>Here:</cyan> " + canonical) == 0);
    CHECK(cell_floor_description(cell, true) == canonical);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Message separators preserve English and Latin fallback punctuation", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    for (lang_t mode : {lang_t::EN, lang_t::ZH})
    {
        Options.language = mode;
        for (const char *text : {"A hit.", "A hit!", "A hit?", "Some gold,", "Gold;", "Warning:", "You die..."})
        {
            CHECK(message_join_separator(text) == " ");
            CHECK(message_join_separator(text, 2) == "; ");
        }
        for (const char *text : {"", "Some gold", "A hit\n", "The orc says, \"Stop!\"", "a wizard's hat"})
            CHECK(message_join_separator(text) == "; ");
    }
    Options.language = lang_t::EN;
    CHECK(message_join_separator("你命中了！") == "; ");
    CHECK(message_join_separator("金币") == "; ");
}

TEST_CASE("Chinese message separators recognise UTF8 punctuation without stripping it", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    for (const char *text : {"命中。", "命中！", "命中？", "命中，", "命中；", "注意：", "金币、", "你死了……",
             "怪物说：“停！”", "怪物说：\"停！\"", "提示（完成。）"})
        CHECK(message_join_separator(text) == "");
    CHECK(message_join_separator("你命中了!") == " ");
    CHECK(message_join_separator("金币") == "；");
    CHECK(message_join_separator("披风「Kittixae」") == "；");
    CHECK(message_join_separator("你闪避了！", 2) == "；");
    CHECK(strwidth(message_join_separator("金币")) == 2);
    const string tagged = "<red>你命中了！</red>";
    const string plain = formatted_string::parse_string(tagged).tostring();
    CHECK(message_join_separator(plain) == "");
    CHECK(tagged + message_join_separator(plain) + "兽人倒下了。"
        == "<red>你命中了！</red>兽人倒下了。");
    CHECK(message_join_separator(string("broken ") + char(0xe3)) == "; ");
}

TEST_CASE("Real message history joins Chinese particles and respects rendered line width", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<bool> level(you.on_current_level, false);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type, GAME_TYPE_NORMAL);
    unwind_var<bool> repeats(Options.msg_condense_repeats, true);
    unwind_var<bool> short_messages(Options.msg_condense_short, true);
    unwind_var<bool> clear(Options.clear_messages, true);
    unwind_var<int> width(crawl_view.msgsz.x, 80);
    struct cleanup { ~cleanup() { clear_message_store(); } } reset;
    const auto state = rng::peek_uint64();
    auto history = [](const vector<string> &messages) {
        clear_message_store();
        for (const string &message : messages) mpr(message);
        return get_last_messages(10, true);
    };
    CHECK(history({"你击中了兽人。", "兽人倒下了！"}) == "你击中了兽人。兽人倒下了！\n\n");
    CHECK(history({"金币", "药水"}) == "金币；药水\n\n");
    CHECK(history({"你闪避了！", "你闪避了！", "你命中了。"}) == "你闪避了！ x2；你命中了。\n\n");
    CHECK(history({"<red>你命中了！</red>", "兽人倒下了。"}) == "你命中了！兽人倒下了。\n\n");
    CHECK(history({"怪物说：“停！”", "你退后了。"}) == "怪物说：“停！”你退后了。\n\n");
    const string first = "甲甲甲甲甲甲甲。", second = "乙乙乙乙乙。", third = "丙丙。";
    crawl_view.msgsz.x = 38;
    CHECK(history({first, first, second, third}) == first + " x2；" + second + "\n" + third + "\n\n");
    crawl_view.msgsz.x = 39;
    CHECK(history({first, first, second, third}) == first + " x2；" + second + third + "\n\n");
    crawl_view.msgsz.x = 80;
    CHECK(history({"Some gold", "A potion"}) == "Some gold; A potion\n\n");
    Options.language = lang_t::EN;
    CHECK(history({"You hit the orc.", "The orc dies!"}) == "You hit the orc. The orc dies!\n\n");
    CHECK(history({"Some gold", "A potion"}) == "Some gold; A potion\n\n");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Reported UI automatic target Lua translates explicit templates and leaves unknown source English", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    cluaopen_crawl(clua.state());
    const auto random_state = rng::peek_uint64();
    REQUIRE(clua.execstring("assert(crawl.ui_translate('fix.autotarget', 'No target in view!') == '视野内没有目标！')") == 0);
    REQUIRE(clua.execstring("assert(crawl.ui_translate('fix.autotarget', 'No reachable target in view!') == '视野内没有可接近的目标！')") == 0);
    REQUIRE(clua.execstring("assert(crawl.ui_translate('fix.autotarget', 'A new target message.') == 'A new target message.')") == 0);
    Options.language = lang_t::EN;
    REQUIRE(clua.execstring("assert(crawl.ui_translate('fix.autotarget', 'No target in view!') == 'No target in view!')") == 0);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Generated monster leap hop and blink events use canonical actor names", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    struct example { monster_type type; const char *source; const char *english; const char *chinese; };
    for (const auto &e : {
        example{MONS_BOULDER_BEETLE, "%s leaps!", "The boulder beetle leaps!", "巨石甲虫跃起！"},
        example{MONS_BLINK_FROG, "%s hops!", "The blink frog hops!", "闪烁蛙蹦跳起来！"},
        example{MONS_ORC, "%s blinks!", "The orc blinks!", "兽人闪烁了一下！"}})
    {
        monster mons;
        mons.type = mons.base_monster = e.type;
        mons.hit_points = mons.max_hit_points = 10;
        mons.set_position(coord_def(10, 10));
        const auto random_state = rng::peek_uint64();
        const string canonical_name = mons.name(DESC_THE);
        const ui_arg name = ui_actor_arg(mons);
        REQUIRE(name.available);
        CHECK(ui_render("log.monster", e.source, e.english, {name}) == e.chinese);
        CHECK(ui_render("log.monster", e.source, e.english, {{"unknown", false}}) == e.english);
        Options.language = lang_t::EN;
        CHECK(ui_render("log.monster", e.source, e.english, {name}) == e.english);
        Options.language = lang_t::ZH;
        CHECK(mons.name(DESC_THE) == canonical_name);
        CHECK(rng::peek_uint64() == random_state);
    }
}

TEST_CASE("Lua display call sites preserve canonical names and complete English fallbacks", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), lua_close);
    REQUIRE(state);
    lua_State *ls = state.get();
    luaL_openlibs(ls);
    cluaopen_crawl(ls);
    const auto random_state = rng::peek_uint64();
    REQUIRE(luaL_loadfile(ls, "../../tools/localization/lua_ui_checks.lua") == 0);
    const int result = lua_pcall(ls, 0, 0, 0);
    INFO((result ? lua_tostring(ls, -1) : "Lua UI checks passed"));
    REQUIRE(result == 0);
    Options.language = lang_t::EN;
    REQUIRE(luaL_dostring(ls,
        "assert(crawl.ui_format('lua.ui', 'No spell in slot %s!', 'No spell in slot a!', 'a') == 'No spell in slot a!'); "
        "assert(crawl.ui_name('display.spell', 'Magic Dart') == nil); "
        "assert(util.display_message('portcullis', '$F{The} has been lowered almost to the ground.') == 'the portcullis has been lowered almost to the ground.')") == 0);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Cosmetic boots and gloves use actual visible English appearance keys", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto random_state = rng::peek_uint64();
    for (int subtype : {ARM_BOOTS, ARM_GLOVES})
    {
        item_def item;
        item.base_type = OBJ_ARMOUR;
        item.sub_type = subtype;
        item.quantity = 1;
        item.plus = 5;
        item.flags = ISFLAG_EMBROIDERED_SHINY;
        const string original = item.name(DESC_PLAIN, false, false, false);
        CHECK(original == (subtype == ARM_BOOTS ? "pair of embroidered boots" : "pair of embroidered gloves"));
        string shown;
        REQUIRE(try_item_display_name(item, shown, false, false));
        CHECK(shown == (subtype == ARM_BOOTS ? "刺绣的靴子" : "刺绣的手套"));
        CHECK(shown.find("+5") == string::npos);
        CHECK(ui_render("t.discovery", "Found %s.", "Found a " + original + ".", {{shown, true}})
            == (subtype == ARM_BOOTS ? "发现刺绣的靴子。" : "发现刺绣的手套。"));
        CHECK(item.name(DESC_PLAIN, false, false, false) == original);
        Options.language = lang_t::EN;
        CHECK_FALSE(try_item_display_name(item, shown, false, false));
        CHECK(ui_render("t.discovery", "Found %s.", "Found a " + original + ".", {{shown, true}}) == "Found a " + original + ".");
        Options.language = lang_t::ZH;
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Reported UI monster shout articles and possessives use raw actor names with whole sentence fallback", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    monster spider;
    spider.type = MONS_TARANTELLA;
    spider.base_monster = MONS_TARANTELLA;
    spider.hit_points = spider.max_hit_points = 10;
    spider.set_position(coord_def(10, 10));
    const string original = "The tarantella's many legs skitter faintly.";
    textdb_speech speech("SOUND:@The_monster_possessive@ many legs skitter faintly.",
        "SOUND:@The_monster_possessive@多条腿轻轻窸窣作响。", true);
    const auto random_state = rng::peek_uint64();
    CHECK(monster_shout_display(speech, spider, original) == "塔兰泰拉的多条腿轻轻窸窣作响。");
    textdb_speech article("SOUND:@The_monster@ hisses.", "SOUND:@The_monster@嘶嘶作响。", true);
    CHECK(monster_shout_display(article, spider, "The tarantella hisses.") == "塔兰泰拉嘶嘶作响。");
    auto missing = speech;
    missing.complete = false;
    CHECK(monster_shout_display(missing, spider, original) == original);
    auto unknown = speech;
    unknown.translated += "@unknown_body_part@";
    CHECK(monster_shout_display(unknown, spider, original) == original);
    Options.language = lang_t::EN;
    CHECK(monster_shout_display(speech, spider, original) == original);
    CHECK(spider.type == MONS_TARANTELLA);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Z gem countdown uses typed gem names and preserves English rounding and state", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<bool> show(Options.more_gem_info, true);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type, GAME_TYPE_NORMAL);
    unwind_var<decltype(you.chapter)> chapter(you.chapter, CHAPTER_ORB_HUNTING);
    unwind_var<branch_type> branch(you.where_are_you, BRANCH_LAIR);
    unwind_var<decltype(you.gems_found)> found(you.gems_found);
    unwind_var<decltype(you.gems_shattered)> shattered(you.gems_shattered);
    unwind_var<decltype(you.gem_time_spent)> times(you.gem_time_spent);
    you.gems_found = decltype(you.gems_found)();
    you.gems_shattered = decltype(you.gems_shattered)();
    const gem_type gem = gem_for_branch(BRANCH_LAIR);
    REQUIRE(gem != NUM_GEM_TYPES);
    you.gems_found.set(gem);
    const auto random_state = rng::peek_uint64();
    for (int remaining : {1, 10, 11, 20, 21})
    {
        you.gem_time_spent[gem] = gem_time_limit(gem) - remaining;
        const int turns = (remaining + 9) / 10;
        const string original = make_stringf("If you linger in this branch for another %d turns, the power of Zot will shatter your %s gem.\n", turns, gem_adj(gem));
        Options.language = lang_t::EN;
        CHECK(gem_status() == original);
        Options.language = lang_t::ZH;
        const auto name = ui_text_arg("display.item", string(gem_adj(gem)) + " gem");
        REQUIRE(name.available);
        CHECK(gem_status() == "如果你继续在此分支逗留" + to_string(turns) + "回合，佐特的力量就会震碎你的" + name.text + "。\n");
        CHECK(you.gem_time_spent[gem] == gem_time_limit(gem) - remaining);
    }
    CHECK(ui_render("z.log", "With a frightful flash, the power of Zot shatters your %s gem into ten thousand fragments!", "Original full English.", {{"unknown gem", false}}) == "Original full English.");
    CHECK(ui_render("z.log", "Upstream changed the gem warning: %s", "Original changed English.", {ui_literal_arg("name")}) == "Original changed English.");
    Options.more_gem_info = false;
    CHECK(gem_status().empty());
    Options.more_gem_info = true;
    you.gems_shattered.set(gem);
    CHECK(gem_status().empty());
    CHECK(rng::peek_uint64() == random_state);
    Options.language = lang_t::ZH;
    string name;
    CHECK(try_ui_projectile_name("crystal spear", name));
    CHECK(name == "水晶矛");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_projectile_name("crystal spear", name));
}

TEST_CASE("X speech tokens keep complete English and share substring choices", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    textdb_speech message("Your @item@ glows.", "你的@item@发光了。", true);
    const auto state = rng::peek_uint64();
    message.replace("@unused@", "unknown", "", false);
    CHECK(message.complete);
    message.replace("@item@", "axe", "斧", true);
    CHECK(message.text() == "你的斧发光了。");
    Options.language = lang_t::EN;
    CHECK(message.text() == "Your axe glows.");
    Options.language = lang_t::ZH;
    textdb_speech missing("Your @item@ glows.", "你的@item@发光了。", true);
    missing.replace("@item@", "untranslated relic", "", false);
    CHECK(missing.text() == "Your untranslated relic glows.");
    textdb_speech unresolved("Original", "仍有@missing@", true);
    CHECK(unresolved.text() == "Original");
    CHECK(rng::peek_uint64() == state);
    for (int seed = 0; seed < 24; ++seed)
    {
        string original;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            original = maybe_pick_random_substring("[Smoke|Steam] pours out!");
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        textdb_speech choice("[Smoke|Steam] pours out!", "[烟|蒸汽]涌出！", true);
        choice.pick_substrings();
        CHECK(choice.english == original);
        CHECK(choice.text() == (original == "Smoke pours out!" ? "烟涌出！" : "蒸汽涌出！"));
        CHECK(rng::peek_uint64() == next);
    }
}

TEST_CASE("X scenery and enchant summaries preserve canonical sentences on a miss", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    const string original = "A fountain starts gushing blood, and some trees are warped by chaos!";
    const auto state = rng::peek_uint64();
    CHECK(xom_scenery_message(original, 1, 0, 3, 0, 0) == original);
    Options.language = lang_t::ZH;
    CHECK(xom_scenery_message(original, 1, 0, 3, 0, 0) == "1座喷泉开始涌出鲜血；3棵树被混沌扭曲了！");
    CHECK(xom_scenery_message("Two doors burst open!", 0, 0, 0, 2, 0) == "2扇门猛然敞开！");
    CHECK(xom_scenery_message("unchanged", 0, 0, 0, 0, 0) == "unchanged");
    monster mon;
    mon.type = MONS_ORC;
    mon.base_monster = MONS_ORC;
    mon.hit_points = mon.max_hit_points = 10;
    const string buff = "The orc suddenly looks fast, and looks braver for a moment!";
    CHECK(xom_buff_message(mon, {ENCH_HASTE}, false, buff) != buff);
    CHECK(xom_buff_message(mon, {ENCH_HASTE, ENCH_NONE}, false, buff) == buff);
    Options.language = lang_t::EN;
    CHECK(xom_buff_message(mon, {ENCH_HASTE}, false, buff) == buff);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("M mutation display keeps canonical descriptions and RNG intact", "[localization]")
{
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    const auto random_state = rng::peek_uint64();
    const auto removed = get_removed_mutations();
    for (int index = 0; index < NUM_MUTATIONS; ++index)
    {
        const auto mut = static_cast<mutation_type>(index);
        if (std::find(removed.begin(), removed.end(), mut) != removed.end()) continue;
        const string canonical_name = mutation_name(mut);
        for (int level = 1; level <= mutation_max_levels(mut); ++level)
        {
            Options.language = lang_t::EN;
            const string original = mutation_desc(mut, level);
            CHECK(mutation_display_desc(mut, level) == original);
            Options.language = lang_t::ZH;
            bool available = false;
            const string shown = mutation_display_desc(mut, level, false, false, &available);
            INFO(mutation_name(mut));
            INFO(level);
            CHECK(available);
            CHECK(shown != original);
            CHECK(mutation_desc(mut, level) == original);
            CHECK(mutation_name(mut) == canonical_name);
            CHECK(formatted_string::parse_string(mutation_display_desc(mut, level, true)).tostring() == shown);
        }
    }
    CHECK(mutation_from_name("horns", false) == MUT_HORNS);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("M typed clauses fall back as a complete English message", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const string original = "The air twists around and strikes an orc!";
    const char *source = "%s and strikes %s%s";
    const auto args = {m_clause_arg("The air twists around"), ui_arg{"兽人", true}, ui_arg{"!", true}};
    CHECK(ui_render("m.log.023", source, original, args) == "空气扭转，击中了兽人!");
    CHECK(ui_render("m.log.023", source, original,
                    {m_clause_arg("The air twists around"), {"", false}, {"!", true}}) == original);
    CHECK(ui_render("m.log.023", source, original,
                    {m_clause_arg("unknown custom air effect"), {"兽人", true}, {"!", true}}) == original);
    CHECK(m_clause_arg("").available);
    Options.language = lang_t::EN;
    CHECK(ui_render("m.log.023", source, original, args) == original);
}

TEST_CASE("M auxiliary descriptions localize only when requested", "[localization]")
{
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    const auto random_state = rng::peek_uint64();
    const string original = mut_aux_attack_desc(MUT_HORNS);
    CHECK_FALSE(original.empty());
    CHECK(mut_aux_attack_desc(MUT_HORNS, true) == original);
    Options.language = lang_t::ZH;
    CHECK(mut_aux_attack_desc(MUT_HORNS) == original);
    CHECK(mut_aux_attack_desc(MUT_HORNS, true).find("触发") != string::npos);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("M monster pane localizes flags without changing canonical summaries", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    monster_info orc(MONS_ORC);
    orc.mb.set(MB_HASTED);
    vector<monster_info> monsters{orc, monster_info(MONS_ORC)};
    string original;
    mons_conditions_string(original, monsters, 0, 2, false);
    CHECK(original == " (1 fast)");
    Options.language = lang_t::ZH;
    string canonical, shown;
    bool available = false;
    mons_conditions_string(canonical, monsters, 0, 2, false);
    CHECK(canonical == original);
    mons_conditions_string(shown, monsters, 0, 2, false, true, &available);
    CHECK(available);
    CHECK(shown == "（1个疾速）");
    int colour = 0;
    orc.to_string(1, shown, colour, true, nullptr, true);
    CHECK(shown.find("兽人") != string::npos);
    CHECK(shown.find("疾速") != string::npos);
    orc.to_string(1, canonical, colour, true, nullptr, true, false);
    CHECK(canonical.find("orc") != string::npos);
    CHECK(canonical.find("fast") != string::npos);
}

TEST_CASE("M monster details use raw status and saved constriction inputs", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    monster_info orc(MONS_ORC);
    orc.mb.set(MB_BREATH_WEAPON);
    orc.mb.set(MB_PAIN_BOND);
    const auto canonical = orc.attributes();
    const auto translated = orc.attribute_display_args();
    REQUIRE(translated.size() == canonical.size());
    REQUIRE(translated.size() == 2);
    for (const auto &part : translated) CHECK(part.available);
    CHECK(orc.attributes() == canonical);
    orc.dam = MDAM_HEAVILY_DAMAGED;
    orc.holi = MH_NATURAL;
    const ui_arg wound = orc.wounds_display_arg(true);
    REQUIRE(wound.available);
    CHECK(formatted_string::parse_string(wound.text).tostring().find("伤") != string::npos);
    orc.constrictor_name = "constricted by an orc";
    CHECK_FALSE(orc.constriction_display_arg().available);
    orc.constrictor_display = m_sentence_arg("constricted by %s", {{"兽人", true}});
    REQUIRE(orc.constriction_display_arg().available);
    CHECK(orc.constriction_display_arg().text == "被兽人缠绕");
    monster_info copy = orc;
    CHECK(copy.constriction_display_arg().text == orc.constriction_display_arg().text);
    copy.constricting_name.push_back("constricting an unknown creature");
    CHECK_FALSE(copy.constriction_display_arg().available);
    Options.language = lang_t::EN;
    CHECK_FALSE(orc.constriction_display_arg().available);
    CHECK(orc.constriction_description() == "constricted by an orc");
}

TEST_CASE("M monster display snapshots do not enter saved map knowledge", "[localization]")
{
    void marshallMapCell(writer &, const map_cell &);
    void unmarshallMapCell(reader &, map_cell &);
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    monster_info orc(MONS_ORC);
    map_cell cell;
    cell.set_monster(orc);
    vector<unsigned char> canonical;
    auto first = writer(&canonical);
    marshallMapCell(first, cell);
    orc.constrictor_display = {"被兽人缠绕", true};
    orc.constricting_display.push_back({"正在缠绕你", true});
    cell.set_monster(orc);
    vector<unsigned char> displayed;
    auto second = writer(&displayed);
    marshallMapCell(second, cell);
    CHECK(displayed == canonical);
    auto input = reader(displayed, TAG_MINOR_VERSION);
    map_cell loaded;
    unmarshallMapCell(input, loaded);
    REQUIRE(loaded.monsterinfo());
    CHECK(loaded.monsterinfo()->type == MONS_ORC);
    CHECK_FALSE(loaded.monsterinfo()->constrictor_display.available);
    CHECK(loaded.monsterinfo()->constricting_display.empty());
    CHECK_FALSE(input.valid());
}

TEST_CASE("L lookup display preserves canonical keys and unavailable output", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    init_monsters();
    init_spell_descs();
    init_spell_name_cache();
    const auto random_state = rng::peek_uint64();
    string shown;
    CHECK(try_lookup_help_display_name("Draw Stack ability", shown));
    CHECK(shown == "抽取叠牌");
    CHECK(try_lookup_help_display_name("Searing Ray spell", shown));
    CHECK(shown == "灼热射线");
    CHECK(try_lookup_help_display_name("Fast status", shown));
    CHECK(shown == "疾速");
    CHECK(try_lookup_help_display_name("-GClaw status", shown));
    CHECK(shown.find("坟墓之爪") != string::npos);
    CHECK(try_lookup_help_display_name("guardian spirit passive", shown));
    CHECK(shown == "守护之灵");
    CHECK(try_lookup_help_display_name("Dungeon", shown));
    CHECK(shown == "地下城");
    shown = "untouched";
    CHECK_FALSE(try_lookup_help_display_name("unknown custom entry status", shown));
    CHECK(shown == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_lookup_help_display_name("Fast status", shown));
    CHECK(shown == "untouched");
    CHECK(ability_name(ABIL_NEMELEX_DRAW_STACK) == "Draw Stack");
    CHECK(string(spell_title(SPELL_SEARING_RAY)) == "Searing Ray");
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("L branch lookup details retain English grammar and typed numeric state", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<int> skill_cost(you.skill_cost_level, 14);
    const auto random_state = rng::peek_uint64();
    const string original = lookup_branch_extra_info(BRANCH_ZOT);
    CHECK(original.find("entered while carrying at least 3 runes of Zot") != string::npos);
    CHECK(original.find("5 levels deep") != string::npos);
    Options.language = lang_t::ZH;
    const string translated = lookup_branch_extra_info(BRANCH_ZOT);
    CHECK(translated.find("3枚佐特符文") != string::npos);
    CHECK(translated.find("5层") != string::npos);
    CHECK(lookup_branch_extra_info(BRANCH_VAULTS).find("1枚佐特符文") != string::npos);
    CHECK(lookup_branch_extra_info(BRANCH_ABYSS).find("深渊第2层") != string::npos);
    CHECK(ui_render("l.branch", "\n\nThe entrance to this branch is in %s.",
        "original entrance paragraph", {{"", false}}) == "original entrance paragraph");
    Options.language = lang_t::EN;
    CHECK(lookup_branch_extra_info(BRANCH_ZOT) == original);
    CHECK(you.skill_cost_level == 14);
    CHECK(rng::peek_uint64() == random_state);
}

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

extern map<level_id, string> level_exclusions;

TEST_CASE("K kill records retain canonical descriptions and typed display qualifiers", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<kill_dump_options> place_option(Options.dump_kill_places, KDO_NO_PLACES);
    init_monsters();
    const auto random_state = rng::peek_uint64();
    kill_def record;
    record.kills = 3;
    kill_monster_desc kind;
    kind.monnum = MONS_ORC;
    kind.modifier = kill_monster_desc::M_NORMAL;
    const kill_exp ordinary(record, kind);
    CHECK(ordinary.desc == "3 orcs");
    CHECK(ordinary.display_description() == "3个兽人");
    for (auto modifier : {kill_monster_desc::M_ZOMBIE, kill_monster_desc::M_DRAUGR,
                          kill_monster_desc::M_SIMULACRUM, kill_monster_desc::M_SPECTRE,
                          kill_monster_desc::M_SHAPESHIFTER})
    {
        kind.modifier = modifier;
        const kill_exp changed(record, kind);
        CHECK(changed.display_description() != changed.desc);
        Options.language = lang_t::EN;
        CHECK(changed.display_description() == changed.desc);
        Options.language = lang_t::ZH;
    }
    kind.monnum = MONS_SIGMUND;
    kind.modifier = kill_monster_desc::M_NORMAL;
    CHECK(kill_exp(record, kind).display_description().find("3次") != string::npos);
    kind.monnum = MONS_ORC;
    record.places = {level_id(BRANCH_DUNGEON, 3)};
    Options.dump_kill_places = KDO_ALL_PLACES;
    CHECK(kill_exp(record, kind).display_description().find("地下城:3") != string::npos);
    kill_exp missing(record, kind);
    missing.modifier = -1;
    CHECK(missing.display_description() == missing.desc);
    missing.monnum = MONS_PROGRAM_BUG;
    missing.desc = "The ghost of an old recorded character (D:3)";
    CHECK(missing.display_description() == missing.desc);
    CHECK(ui_render("k.kills", "Vanquished Creatures (%s)",
        "Vanquished Creatures (custom category)",
        {ui_text_arg("k.category", "custom category")})
        == "Vanquished Creatures (custom category)");
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("K native kill display and shipped Lua preserve original English accessors", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<kill_dump_options> place_option(Options.dump_kill_places, KDO_NO_PLACES);
    init_monsters();
    lua_State *ls = clua.state();
    lua_stack_cleaner stack(ls);
    cluaopen_kills(ls);
    // Preserve the callback/options even when a REQUIRE exits this test early.
    struct restore_globals
    {
        lua_State *ls;
        vector<pair<string, int>> values;
        restore_globals(lua_State *state) : ls(state)
        {
            for (const char *name : {"c_kill_list", "DUMP_KILL_BREAKDOWNS", "chk_lua_option"})
            {
                lua_getglobal(ls, name);
                values.emplace_back(name, luaL_ref(ls, LUA_REGISTRYINDEX));
            }
        }
        ~restore_globals()
        {
            for (const auto &entry : values)
            {
                lua_rawgeti(ls, LUA_REGISTRYINDEX, entry.second);
                lua_setglobal(ls, entry.first.c_str());
                luaL_unref(ls, LUA_REGISTRYINDEX, entry.second);
            }
        }
    } restore(ls);
    REQUIRE(clua.execstring("chk_lua_option = {}; function c_kill_list() return false end") == 0);
    monster mon;
    mon.type = MONS_ORC;
    mon.exp = 1;
    KillMaster records;
    records.record_kill(&mon, KILL_YOU, false);
    records.record_kill(&mon, KILL_MON, true);
    const auto random_state = rng::peek_uint64();
    const string canonical = records.kill_info();
    REQUIRE(canonical.find("Vanquished Creatures") != string::npos);
    const string shown = records.kill_info(true);
    CHECK(shown.find("击杀的生物") != string::npos);
    CHECK(shown.find("兽人") != string::npos);
    CHECK(shown.find("总计：击杀了2个生物") != string::npos);
    CHECK(records.kill_info() == canonical);
    Options.language = lang_t::EN;
    CHECK(records.kill_info(true) == canonical);
    // Exercise the actual bundled script, including symbol/nature/count groups.
    REQUIRE(luaL_loadfile(ls, "dat/clua/kills.lua") == 0);
    REQUIRE(lua_pcall(ls, 0, 0, 0) == 0);
    REQUIRE(clua.execstring("DUMP_KILL_BREAKDOWNS = true") == 0);
    const string detailed = records.kill_info();
    CHECK(records.kill_info(true) == detailed);
    Options.language = lang_t::ZH;
    const string detailed_shown = records.kill_info(true);
    CHECK(detailed_shown.find("怪物符号") != string::npos);
    CHECK(detailed_shown.find("怪物本质") != string::npos);
    CHECK(detailed_shown.find("兽人") != string::npos);
    CHECK(detailed_shown.find("总计：击杀了2个生物") != string::npos);
    CHECK(records.kill_info() == detailed);
    // Custom scripts still receive the original category and description.
    REQUIRE(clua.execstring("function c_kill_list(a, who) if not a then return false end; "
        "assert(who == nil or who == 'collateral kills'); "
        "assert(kills.desc(a[1]) == 'An orc'); "
        "assert(kills.summary(a) == '1 creature vanquished.'); return false end") == 0);
    CHECK(records.kill_info(true).find("Lua error:") == string::npos);
    CHECK(records.total_kills() == 2);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("I single item display keeps stack identity and inventory metadata", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> known(you.type_ids[OBJ_POTIONS][POT_CURING], false);
    item_def item;
    item.base_type = OBJ_POTIONS;
    item.sub_type = POT_CURING;
    item.subtype_rnd = 0;
    item.quantity = 7;
    item.slot = 'c';
    item.pos = ITEM_IN_INVENTORY;
    item.link = 0;
    item.inscription = "keep";
    const string canonical = item.name(DESC_INVENTORY);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::ZH;
    const ui_arg single = ui_single_item_arg(item);
    REQUIRE(single.available);
    item_def one = item;
    one.quantity = 1;
    CHECK(single.text == ui_item_arg(one).text);
    CHECK(single.text.find("治疗") == string::npos);
    const ui_arg inventory = ui_inventory_item_arg(item);
    REQUIRE(inventory.available);
    CHECK(inventory.text.find("c - 7 × ") == 0);
    CHECK(inventory.text.find("keep") != string::npos);
    CHECK(item.quantity == 7);
    CHECK(item.name(DESC_INVENTORY) == canonical);
    Options.language = lang_t::EN;
    CHECK_FALSE(ui_single_item_arg(item).available);
    CHECK(item.name(DESC_INVENTORY) == canonical);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("I coloured names expose display separately from canonical output", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> known(you.type_ids[OBJ_SCROLLS][SCR_IDENTIFY], true);
    item_def item;
    item.base_type = OBJ_SCROLLS;
    item.sub_type = SCR_IDENTIFY;
    item.quantity = 4;
    item.slot = 'd';
    item.pos = ITEM_IN_INVENTORY;
    item.link = 1;
    item.flags = ISFLAG_IDENTIFIED;
    Options.language = lang_t::EN;
    const string original = menu_colour_item_name(item, DESC_INVENTORY);
    const auto random_state = rng::peek_uint64();
    ui_arg display;
    CHECK(menu_colour_item_name(item, DESC_INVENTORY, &display) == original);
    CHECK_FALSE(display.available);
    Options.language = lang_t::ZH;
    CHECK(menu_colour_item_name(item, DESC_INVENTORY) == original);
    CHECK(menu_colour_item_name(item, DESC_INVENTORY, &display) == original);
    REQUIRE(display.available);
    CHECK(display.text.find("d - 4 × ") != string::npos);
    CHECK(display.text.find("鉴定卷轴") != string::npos);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("I inventory reasons and all current scroll warning actions are reviewed", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    for (int selector : vector<int>{OSEL_ANY, OBJ_WEAPONS, OBJ_POTIONS, OBJ_SCROLLS, OSEL_UNIDENT})
    {
        Options.language = lang_t::EN;
        const string original = no_selectables_message(selector);
        Options.language = lang_t::ZH;
        CHECK(no_selectables_message(selector) == original);
        CHECK(ui_text_arg("i.inventory.empty", original).available);
    }
    for (int subtype : all_item_subtypes(OBJ_SCROLLS))
    {
        item_def scroll;
        scroll.base_type = OBJ_SCROLLS;
        scroll.sub_type = subtype;
        scroll.quantity = 1;
        scroll.flags = ISFLAG_IDENTIFIED;
        const string action = "read the " + scroll.name(DESC_DBNAME);
        INFO(action);
        CHECK(ui_text_arg("f.attack.action", action).available);
    }
    for (int species_index = 0; species_index < NUM_SPECIES; ++species_index)
        CHECK(ui_text_arg("i.skin", species::skin_name(static_cast<species_type>(species_index))).available);
}

TEST_CASE("I equipment safety display reuses the original hazard result", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<decltype(env.cloud)> clouds(env.cloud);
    env.cloud.clear();
    unwind_var<int> health(you.hp, 1000);
    unwind_var<int> poison(you.duration[DUR_POISONING], 0);
    unwind_var<int> contamination(you.magic_contamination, 0);
    unwind_var<int> fire(you.duration[DUR_STICKY_FLAME], 1);
    const auto random_state = rng::peek_uint64();
    string reason;
    ui_arg display;
    CHECK_FALSE(i_feel_safe(false, false, false, true, -1, &reason, &display));
    CHECK(reason == "you are on fire");
    REQUIRE(display.available);
    CHECK(display.text == "你身上正在燃烧");
    const string original = "Spend multiple turns changing equipment while you are on fire?";
    CHECK(ui_render("i.use", "Spend multiple turns changing equipment while %s?", original,
                    {display}).find("你身上正在燃烧") != string::npos);
    CHECK(ui_render("i.use", "Spend multiple turns changing equipment while %s?", original,
                    {{"", false}}) == original);
    Options.language = lang_t::EN;
    CHECK_FALSE(i_feel_safe(false, false, false, true, -1, &reason, &display));
    CHECK(reason == "you are on fire");
    CHECK_FALSE(display.available);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("H item hints preserve English, command bindings and RNG", "[localization]")
{
    init_properties();
    init_mut_index();
    unwind_var<lang_t> language(Options.language);
    unwind_var<hints_state> hints(Hints);
    const auto random_state = rng::peek_uint64();
    item_def potion;
    potion.base_type = OBJ_POTIONS;
    potion.sub_type = POT_CURING;
    potion.quantity = 1;
    Options.language = lang_t::EN;
    const string english = hints_describe_item(potion);
    const auto events = Hints.hints_events;
    CHECK(english.find("quaff this potion") != string::npos);
    Options.language = lang_t::ZH;
    const string chinese = hints_describe_item(potion);
    CHECK(chinese.find("饮用这瓶药水") != string::npos);
    CHECK(chinese.find("quaff") == string::npos);
    for (int i = 0; i < HINT_EVENTS_NUM; ++i)
        CHECK(Hints.hints_events[i] == events[i]);
    string key = "%";
    insert_commands(key, {CMD_QUAFF});
    CHECK(chinese.find("<w>" + key + "</w>") != string::npos);
    Options.language = lang_t::EN;
    CHECK(hints_describe_item(potion) == english);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("H complete paragraphs reject unavailable names", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const string source = "That monster is %s. You can learn about any monster by hovering your mouse over it, and read its description by <w>right-clicking</w> on it.";
    const string english = "That monster is an unknown creature. You can learn about any monster by hovering your mouse over it, and read its description by <w>right-clicking</w> on it.";
    CHECK(ui_render("h.hints.first", source.c_str(), english,
                    {{"", false}}) == english);
    CHECK(ui_render("h.hints.first", source.c_str(), english,
                    {ui_literal_arg("兽人")}).find("那只怪物是兽人") == 0);
    ui_translation_group group({{"h.hints", "This monster appears to be really dangerous!\n"},
                                {"h.missing", "new upstream paragraph"}});
    CHECK_FALSE(group.available());
    CHECK(string(group.get("h.hints", "This monster appears to be really dangerous!\n"))
          == "This monster appears to be really dangerous!\n");
}

TEST_CASE("H scores opt into display without changing stored records", "[localization]")
{
    init_properties();
    init_monsters();
    init_mut_index();
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> paralysis(you.duration[DUR_PARALYSIS], 0);
    unwind_var<int> sleep(you.duration[DUR_SLEEP], 0);
    unwind_var<int> vexed(you.duration[DUR_VEXED], 0);
    scorefile_entry record;
    REQUIRE(record.parse("v=0.34.1:sc=42:name=Tester:race=Human:cls=Fighter:char=HuFi:xl=5:title=Fighter:ktyp=quitting:br=D:lvl=2:start=20261023080000S:end=20261023090000S:dur=60:turn=10:hp=4:mhp=10:mmhp=10\n"));
    Options.language = lang_t::EN;
    const string stored = record.raw_string();
    const string original = hiscores_format_single_long(record, true);
    CHECK(hiscores_format_single_long(record, true, true) == original);
    Options.language = lang_t::ZH;
    CHECK(hiscores_format_single_long(record, true) == original);
    const string display = hiscores_format_single_long(record, true, true);
    CHECK(display.find("退出了游戏") != string::npos);
    CHECK(display.find("本局历时") != string::npos);
    CHECK(record.raw_string() == stored);
    scorefile_entry historical;
    REQUIRE(historical.parse("v=0.34.1:sc=42:name=Tester:race=Human:cls=Fighter:char=HuFi:xl=5:title=Fighter:ktyp=mon:killer=an orc:kaux=a short sword:dam=8:br=D:lvl=2:start=20261023080000S:end=20261023090000S:dur=60:turn=10:hp=-4:mhp=10:mmhp=10\n"));
    const string canonical_death = historical.death_description(scorefile_entry::DDV_VERBOSE);
    CHECK(hiscores_format_single_long(historical, true, true).find("兽人") != string::npos);
    CHECK(hiscores_format_single_long(historical, true, true).find("短剑") != string::npos);
    CHECK(hiscores_format_single_long(historical, true).find(canonical_death) != string::npos);
    scorefile_entry winner;
    REQUIRE(winner.parse("v=0.34.1:sc=100:name=Tester:race=Human:cls=Fighter:char=HuFi:xl=27:title=Fighter:ktyp=winning:br=D:lvl=1:nrune=3:fgem=2:igem=1:start=20261023080000S:end=20261023090000S:dur=60:turn=10:hp=4:mhp=10:mmhp=10\n"));
    unwind_var<bool> gems(Options.more_gem_info, true);
    const string win = hiscores_format_single_long(winner, true, true);
    CHECK(win.find("3 枚符文") != string::npos);
    CHECK(win.find("2 颗宝石") != string::npos);
    CHECK(win.find("1 颗完好") != string::npos);
    Options.language = lang_t::EN;
    CHECK(hiscores_format_single_long(record, true, true) == original);
}

TEST_CASE("E-source exclusion annotations opt into display without changing saved text", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language);
    unwind_var<exclude_set> exclusions(curr_excludes);
    unwind_var<decltype(level_exclusions)> annotations(level_exclusions);
    curr_excludes.clear();
    travel_exclude record;
    record.pos = coord_def(1, 1);
    record.radius = 0;
    record.desc = "orc";
    curr_excludes.add_exclude(record);
    const level_id level = level_id::current();
    const string canonical = curr_excludes.get_exclusion_desc();
    set_level_exclusion_annotation(canonical, level);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    REQUIRE(get_level_annotation(level, false, true) == canonical);
    Options.language = lang_t::ZH;
    REQUIRE(get_level_annotation(level, false, true) == canonical);
    REQUIRE(get_level_annotation(level, false, true, false, LIGHTRED, true) == "移动禁区：兽人");
    REQUIRE(level_exclusions.at(level) == canonical);
    REQUIRE(curr_excludes.get_exclude_root(coord_def(1, 1))->desc == "orc");
    Options.language = lang_t::EN;
    REQUIRE(get_level_annotation(level, false, true, false, LIGHTRED, true) == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("E-source manual display preserves canonical status and study points", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<decltype(you.skill_manual_points)> manuals(you.skill_manual_points);
    for (int skill = 0; skill < NUM_SKILLS; ++skill) you.skill_manual_points[skill] = 0;
    you.skill_manual_points[SK_FIGHTING] = 100;
    you.skill_manual_points[SK_EVOCATIONS] = 250;
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    ui_arg english;
    const string names = manual_skill_names(false, &english);
    REQUIRE_FALSE(english.available);
    REQUIRE(manual_skill_names(true) == "2 skills");
    status_info canonical;
    REQUIRE(fill_status_info(STATUS_MANUAL, canonical));
    Options.language = lang_t::ZH;
    ui_arg chinese;
    REQUIRE(manual_skill_names(false, &chinese) == names);
    REQUIRE(chinese.available);
    REQUIRE(chinese.text.find(ui_text_arg("display.skill", skill_name(SK_FIGHTING)).text) != string::npos);
    REQUIRE(manual_skill_names(true, &chinese) == "2 skills");
    REQUIRE(chinese.text == "2项技能");
    status_info raw, shown;
    REQUIRE(fill_status_info(STATUS_MANUAL, raw));
    REQUIRE(raw.short_text == canonical.short_text);
    REQUIRE(raw.long_text == canonical.long_text);
    REQUIRE(fill_status_display_info(STATUS_MANUAL, shown));
    REQUIRE(shown.short_text == "研读2项技能");
    REQUIRE(shown.long_text.find("你正在研读") == 0);
    REQUIRE(you.skill_manual_points[SK_FIGHTING] == 100);
    REQUIRE(you.skill_manual_points[SK_EVOCATIONS] == 250);
    Options.language = lang_t::EN;
    REQUIRE(fill_status_display_info(STATUS_MANUAL, shown));
    REQUIRE(shown.long_text == canonical.long_text);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("E-source exclusions retain English metadata and whole-summary fallback", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language);
    exclude_set exclusions;
    const auto append = [&](int x, const string &name) {
        travel_exclude record;
        record.pos = coord_def(x, 1);
        record.radius = 0;
        record.desc = name;
        exclusions.add_exclude(record);
    };
    append(1, "orc");
    append(2, "orc");
    append(3, "orc (detected)");
    append(4, "");
    append(5, "flame cloud");
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = exclusions.get_exclusion_desc();
    string shown = "unchanged";
    REQUIRE_FALSE(exclusions.try_display_desc(level_id::current(), shown));
    REQUIRE(shown == "unchanged");
    Options.language = lang_t::ZH;
    REQUIRE(exclusions.try_display_desc(level_id::current(), shown));
    REQUIRE(shown == "移动禁区：2个兽人、兽人（探测到）、另外1处移动禁区");
    REQUIRE(exclusions.get_exclusion_desc() == canonical);
    REQUIRE(exclusions.get_exclude_root(coord_def(1, 1))->desc == "orc");
    REQUIRE(exclusions.get_exclude_root(coord_def(3, 1))->desc == "orc (detected)");
    REQUIRE(exclusions.get_exclude_root(coord_def(5, 1))->desc == "flame cloud");
    Options.language = lang_t::EN;
    REQUIRE(exclusions.get_exclusion_desc() == canonical);
    Options.language = lang_t::ZH;
    append(6, "Unknown custom exclusion label");
    shown = "unchanged";
    REQUIRE_FALSE(exclusions.try_display_desc(level_id::current(), shown));
    REQUIRE(shown == "unchanged");
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("E-source evocation restrictions keep canonical rule strings", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> berserk(you.duration[DUR_BERSERK], 20);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = cannot_evoke_item_reason();
    REQUIRE(canonical == "You are too berserk!");
    Options.language = lang_t::ZH;
    REQUIRE(cannot_evoke_item_reason() == canonical);
    REQUIRE(ui_text_arg("d.item.reason", canonical).available);
    REQUIRE(you.duration[DUR_BERSERK] == 20);
    Options.language = lang_t::EN;
    REQUIRE(cannot_evoke_item_reason() == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Ghost skill titles preserve canonical species grammar and RNG", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> traitor(you.attribute[ATTR_TRAITOR], 0);
    const auto random_state = rng::peek_uint64();
    for (const auto species : {SP_HUMAN, SP_DEEP_ELF, SP_DEMIGOD, SP_FORMICID,
        SP_ARMATAUR, SP_BARACHI, SP_MUMMY, SP_FELID, SP_COGLIN,
        SP_POLTERGEIST, SP_REVENANT, SP_RED_DRACONIAN, SP_TROLL, SP_ONI})
    {
        for (skill_type skill = SK_FIRST_SKILL; skill < NUM_SKILLS; ++skill)
        {
            if (is_invalid_skill(skill)) continue;
            for (uint8_t rank = 0; rank < 5; ++rank)
            {
                CAPTURE(species, skill, rank);
                Options.language = lang_t::EN;
                ui_arg english;
                const string canonical = skill_title_by_rank(skill, rank,
                    species, 20, 20, 20, GOD_NO_GOD, 0, false, &english);
                REQUIRE_FALSE(english.available);
                Options.language = lang_t::ZH;
                ui_arg chinese;
                REQUIRE(skill_title_by_rank(skill, rank, species,
                    20, 20, 20, GOD_NO_GOD, 0, false, &chinese) == canonical);
                REQUIRE(chinese.available);
                REQUIRE_FALSE(chinese.text.empty());
                REQUIRE(chinese.text.find('@') == string::npos);
            }
        }
    }
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Ghost display retains saved identity and falls back for custom titles", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> traitor(you.attribute[ATTR_TRAITOR], 0);
    monster_info ghost(MONS_PLAYER_GHOST);
    ghost.mname = "Tester";
    ghost.i_ghost.species = SP_HUMAN;
    ghost.i_ghost.job = JOB_FIGHTER;
    ghost.i_ghost.religion = GOD_NO_GOD;
    ghost.i_ghost.best_skill = SK_FIGHTING;
    ghost.i_ghost.best_skill_rank = 2;
    ghost.i_ghost.xl_rank = 3;
    ghost.i_ghost.title = "Warrior";
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = get_ghost_description(ghost);
    string shown = "unchanged";
    REQUIRE_FALSE(try_ghost_description_display(ghost, shown));
    REQUIRE(shown == "unchanged");
    Options.language = lang_t::ZH;
    REQUIRE(try_ghost_description_display(ghost, shown));
    REQUIRE(shown.find("Tester") != string::npos);
    REQUIRE(shown.find("勇士") != string::npos);
    REQUIRE(shown.find("人类") != string::npos);
    REQUIRE(shown.find("战士") != string::npos);
    REQUIRE(get_ghost_description(ghost) == canonical);
    REQUIRE(ghost.i_ghost.title == "Warrior");
    ghost.i_ghost.title = "An unknown custom title";
    shown = "unchanged";
    REQUIRE_FALSE(try_ghost_description_display(ghost, shown));
    REQUIRE(shown == "unchanged");
    ghost.i_ghost.xl_rank = -1;
    REQUIRE_FALSE(try_ghost_description_display(ghost, shown));
    REQUIRE(shown == "unchanged");
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Item prose keeps Lua English and localizes complete use restrictions", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<god_type> religion(you.religion, GOD_TROG);
    unwind_var<bool> known(you.type_ids[OBJ_SCROLLS][SCR_AMNESIA], true);
    item_def item;
    item.base_type = OBJ_SCROLLS;
    item.sub_type = SCR_AMNESIA;
    item.quantity = 1;
    item.flags |= ISFLAG_IDENTIFIED;
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = get_item_description(item, IDM_MONSTER);
    CAPTURE(canonical);
    REQUIRE(canonical.find("Trog doesn't allow you to memorise spells!") != string::npos);
    REQUIRE(get_item_description(item, IDM_MONSTER, true) == canonical);
    Options.language = lang_t::ZH;
    REQUIRE(get_item_description(item, IDM_MONSTER) == canonical);
    const string chinese = get_item_description(item, IDM_MONSTER, true);
    REQUIRE(chinese.find("这种卷轴对你完全无用：特洛戈不允许你记忆法术！") != string::npos);
    REQUIRE(Options.language == lang_t::ZH);
    REQUIRE(item.quantity == 1);
    Options.language = lang_t::EN;
    REQUIRE(get_item_description(item, IDM_MONSTER, true) == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Item origin display preserves stored place giver and English provenance", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> dungeon_depth(brdepth[BRANCH_DUNGEON], 15);
    item_def item;
    item.base_type = OBJ_ORBS;
    item.sub_type = ORB_ZOT;
    item.quantity = 1;
    item.flags |= ISFLAG_IDENTIFIED;
    item.orig_place = level_id(BRANCH_DUNGEON, 3);
    item.orig_monnum = 0;
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = get_item_description(item, IDM_MONSTER);
    CAPTURE(canonical);
    REQUIRE(canonical.find("You found it on level 3 of the Dungeon.") != string::npos);
    Options.language = lang_t::ZH;
    REQUIRE(get_item_description(item, IDM_MONSTER) == canonical);
    const string chinese = get_item_description(item, IDM_MONSTER, true);
    CAPTURE(chinese);
    REQUIRE(chinese.find("发现了它") != string::npos);
    REQUIRE(chinese.find("第3层") != string::npos);
    REQUIRE(item.orig_place == level_id(BRANCH_DUNGEON, 3));
    REQUIRE(item.orig_monnum == 0);
    item.orig_monnum = -GOD_TROG;
    const string gifted = get_item_description(item, IDM_MONSTER, true);
    REQUIRE(gifted.find("特洛戈") != string::npos);
    REQUIRE(gifted.find("赐予了你") != string::npos);
    REQUIRE(item.orig_monnum == -GOD_TROG);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Cancellation prose preserves canonical effects and player state", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> contamination(you.magic_contamination, 2000);
    unwind_var<int> agility(you.duration[DUR_AGILITY], 100);
    init_duration_index();
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    ui_arg english;
    const string canonical = describe_player_cancellation(false, &english);
    REQUIRE_FALSE(english.available);
    REQUIRE(canonical.find("as magically contaminated") != string::npos);
    Options.language = lang_t::ZH;
    ui_arg chinese;
    REQUIRE(describe_player_cancellation(false, &chinese) == canonical);
    REQUIRE(chinese.available);
    REQUIRE(chinese.text.find("魔法污染") != string::npos);
    REQUIRE(chinese.text.find("敏捷") != string::npos);
    REQUIRE(you.magic_contamination == 2000);
    REQUIRE(you.duration[DUR_AGILITY] == 100);
    ui_arg debuffs;
    REQUIRE(describe_player_cancellation(true, &debuffs).find("as magically contaminated") == string::npos);
    REQUIRE(debuffs.text.find("魔法污染") == string::npos);
    Options.language = lang_t::EN;
    REQUIRE(describe_player_cancellation() == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Deck summary preserves canonical counts while translating the whole gift", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<CrawlHashTable> properties(you.props);
    you.props[deck_name(DECK_OF_ESCAPE)] = 1;
    you.props[deck_name(DECK_OF_DESTRUCTION)] = 7;
    you.props[deck_name(DECK_OF_SUMMONING)] = 0;
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    ui_arg english;
    const string canonical = deck_summary(&english);
    REQUIRE(canonical == "1 escape card and 7 destruction cards");
    REQUIRE_FALSE(english.available);
    Options.language = lang_t::ZH;
    ui_arg shown;
    REQUIRE(deck_summary(&shown) == canonical);
    REQUIRE(shown.available);
    REQUIRE(shown.text == "1张逃脱牌和7张毁灭牌");
    REQUIRE(deck_cards(DECK_OF_ESCAPE) == 1);
    REQUIRE(deck_cards(DECK_OF_DESTRUCTION) == 7);
    REQUIRE(deck_cards(DECK_OF_SUMMONING) == 0);
    Options.language = lang_t::EN;
    REQUIRE(deck_summary() == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("Player spell prose keeps Lua canonical and UI language reversible", "[localization]")
{
    init_spell_descs();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> saved(crawl_state.need_save, false);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string canonical = player_spell_desc(SPELL_FIREBALL);
    REQUIRE(canonical.find("Level: 5") != string::npos);
    REQUIRE(player_spell_desc(SPELL_FIREBALL, true) == canonical);
    Options.language = lang_t::ZH;
    REQUIRE(player_spell_desc(SPELL_FIREBALL) == canonical);
    const string translated = player_spell_desc(SPELL_FIREBALL, true);
    REQUIRE(translated.find("等级：5") != string::npos);
    REQUIRE(translated.find("学派：") != string::npos);
    const ui_arg fire_school = ui_text_arg("display.skill", skill_name(SK_FIRE_MAGIC));
    REQUIRE(fire_school.available);
    REQUIRE(translated.find(fire_school.text) != string::npos);
    REQUIRE(Options.language == lang_t::ZH);
    Options.language = lang_t::EN;
    REQUIRE(player_spell_desc(SPELL_FIREBALL, true) == canonical);
    REQUIRE(rng::peek_uint64() == random_state);
}

TEST_CASE("D-source spell lists localize school display and preserve book contents", "[localization]")
{
    init_spell_descs();
    unwind_var<lang_t> language(Options.language);
    item_def book;
    book.base_type = OBJ_BOOKS;
    book.sub_type = BOOK_CONJURATIONS;
    book.quantity = 1;
    book.flags |= ISFLAG_IDENTIFIED;
    const auto contents = spells_in_book(book);
    REQUIRE_FALSE(contents.empty());
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string original = terse_spell_list(book);
    Options.language = lang_t::ZH;
    const string translated = terse_spell_list(book);
    CHECK(translated.find("法术：") == 0);
    CHECK(translated != original);
    CHECK(translated.find("Conjuration") == string::npos);
    CHECK(spells_in_book(book) == contents);
    CHECK(ui_render("d.spells", "Spells: %s", original,
                    {{"unknown spell or school", false}}) == original);
    Options.language = lang_t::EN;
    CHECK(terse_spell_list(book) == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Feature display preserves canonical metadata and English grammar", "[localization]")
{
    init_show_table();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> zot_known(you.zot_orb_monster_known, false);
    const auto next_random = rng::peek_uint64();
    int checked = 0;
    for (int i = 0; i < NUM_FEATURES; ++i)
    {
        const auto feat = static_cast<dungeon_feature_type>(i);
        if (!is_valid_feature_type(feat) || !*get_feature_def(feat).name) continue;
        const string metadata = get_feature_def(feat).name;
        Options.language = lang_t::EN;
        const string english = feature_description(feat);
        string shown = "untouched";
        CHECK_FALSE(try_feature_display_name(feat, NUM_TRAPS, shown));
        CHECK(shown == "untouched");
        Options.language = lang_t::ZH;
        CAPTURE(i, metadata, english);
        REQUIRE(try_feature_display_name(feat, NUM_TRAPS, shown));
        CHECK_FALSE(shown.empty());
        CHECK(shown != metadata);
        CHECK(feature_description(feat) == english);
        CHECK(string(get_feature_def(feat).name) == metadata);
        ++checked;
    }
    CHECK(checked > 150);
    string shown = "untouched";
    CHECK_FALSE(try_feature_display_name(NUM_FEATURES, NUM_TRAPS, shown));
    CHECK(shown == "untouched");
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Monster equipment display keeps identity inventory and full fallback", "[localization]")
{
    init_properties();
    init_monsters();
    unwind_var<lang_t> language(Options.language);
    monster_info mi(MONS_ORC_WARRIOR);
    mi.inv[MSLOT_WEAPON].reset(new item_def());
    item_def &sword = *mi.inv[MSLOT_WEAPON];
    sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    sword.plus = 3;
    sword.flags = ISFLAG_IDENTIFIED;
    mi.mb.set(MB_HASTED);
    const auto next_random = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string english = get_monster_equipment_desc(mi, DESC_FULL, DESC_A, true);
    const string item_name = sword.name(DESC_A);
    string shown = "untouched";
    CHECK_FALSE(try_monster_equipment_display(mi, shown, DESC_FULL, DESC_A, true));
    CHECK(shown == "untouched");
    Options.language = lang_t::ZH;
    REQUIRE(try_monster_equipment_display(mi, shown, DESC_FULL, DESC_A, true));
    CHECK(shown.find("兽人战士") != string::npos);
    CHECK(shown.find("短剑") != string::npos);
    CHECK(shown.find("疾速") != string::npos);
    CHECK(get_monster_equipment_desc(mi, DESC_FULL, DESC_A, true) == english);
    CHECK(sword.name(DESC_A) == item_name);
    CHECK(sword.quantity == 1);
    CHECK(sword.plus == 3);
    monster_info unknown = mi;
    unknown.mname = "unreviewed vault creature";
    unknown.mb.set(MB_NAME_REPLACE);
    shown = "untouched";
    CHECK_FALSE(try_monster_equipment_display(unknown, shown, DESC_FULL, DESC_A, true));
    CHECK(shown == "untouched");
    Options.language = lang_t::EN;
    CHECK(get_monster_equipment_desc(mi, DESC_FULL, DESC_A, true) == english);
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Damage rating display preserves Lua text numeric results and RNG", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    item_def sword;
    sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    sword.flags = ISFLAG_IDENTIFIED;
    sword.plus = 3;
    const string name = sword.name(DESC_A);
    const auto next_random = rng::peek_uint64();
    Options.language = lang_t::EN;
    int english_rating = 0;
    const string english = damage_rating(&sword, &english_rating);
    Options.language = lang_t::ZH;
    string shown;
    int chinese_rating = 0;
    CHECK(damage_rating(&sword, &chinese_rating, &shown) == english);
    CHECK(chinese_rating == english_rating);
    CHECK(damage_rating(&sword) == english);
    CHECK(shown.find("基础") != string::npos);
    CHECK(shown.find("附魔") != string::npos);
    CHECK(shown != english);
    CHECK(sword.name(DESC_A) == name);
    CHECK(sword.plus == 3);
    CHECK(rng::peek_uint64() == next_random);
    CHECK(ui_render("d.equipment", "This weapon falls into the %s category. ", english,
        {{"unreviewed skill", false}}) == english);
    Options.language = lang_t::EN;
    shown = "stale display";
    CHECK(damage_rating(&sword, nullptr, &shown) == english);
    CHECK(shown.empty());
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("D-source hit display reuses canonical calculations without changing English", "[localization]")
{
    init_properties();
    init_monsters();
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<int> blind(you.duration[DUR_BLIND], 0);
    const monster_info target(MONS_GOBLIN);
    const auto random_state = rng::peek_uint64();
    ostringstream english, canonical;
    uint64_t after_english = 0;
    {
        rng::subgenerator sample(123456, 987654);
        Options.language = lang_t::EN;
        describe_to_hit(target, english, nullptr, true, nullptr, 1);
        after_english = rng::peek_uint64();
    }
    string display;
    {
        rng::subgenerator sample(123456, 987654);
        Options.language = lang_t::ZH;
        describe_to_hit(target, canonical, nullptr, true, nullptr, 1, &display);
        CHECK(rng::peek_uint64() == after_english);
    }
    CHECK(canonical.str() == english.str());
    CHECK(display.find("命中率") != string::npos);
    CHECK(rng::peek_uint64() == random_state);
    CHECK(ui_render("d.hit", "about %s%% to hit with %s", english.str(),
        {ui_literal_arg("50"), {"unknown equipment", false}}) == english.str());
    Options.language = lang_t::EN;
    ostringstream restored;
    {
        rng::subgenerator sample(123456, 987654);
        describe_to_hit(target, restored, nullptr, true, nullptr, 1, &display);
        CHECK(rng::peek_uint64() == after_english);
    }
    CHECK(restored.str() == english.str());
    CHECK(display.empty());
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("D-source god power descriptions keep their canonical religion records", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    int checked = 0;
    for (const auto &powers : get_all_god_powers())
        for (const auto &power : powers)
        {
            if (!*power.general)
                continue;
            const string canonical = power.general;
            CAPTURE(canonical);
            Options.language = lang_t::ZH;
            CHECK(ui_translation_group({{"d.god.power", power.general}}).available());
            CHECK(string(ui_translate("d.god.power", power.general)) != canonical);
            CHECK(string(power.general) == canonical);
            Options.language = lang_t::EN;
            CHECK(string(ui_translate("d.god.power", power.general)) == canonical);
            ++checked;
        }
    CHECK(checked > 120);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("D-source artefact explanations switch language without changing inscriptions", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    const auto prior = get_unique_item_status(UNRAND_AUGMENTATION);
    item_def item;
    item.quantity = 1;
    REQUIRE(make_item_unrandart(item, UNRAND_AUGMENTATION));
    set_unique_item_status(item, prior);
    item.flags |= ISFLAG_IDENTIFIED;
    artefact_set_property(item, ARTP_WILLPOWER, -2);
    artefact_set_property(item, ARTP_STEALTH, 2);
    artefact_set_property(item, ARTP_FIRE, 3);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string inscription = artefact_inscription(item);
    vector<string> english, chinese, restored;
    desc_randart_props(item, english);
    Options.language = lang_t::ZH;
    desc_randart_props(item, chinese);
    CHECK(chinese != english);
    CHECK(join_strings(chinese.begin(), chinese.end(), "\n").find("意志力") != string::npos);
    CHECK(join_strings(chinese.begin(), chinese.end(), "\n").find("火焰") != string::npos);
    CHECK(artefact_inscription(item) == inscription);
    Options.language = lang_t::EN;
    desc_randart_props(item, restored);
    CHECK(restored == english);
    CHECK(artefact_inscription(item) == inscription);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("D-source status presentation preserves canonical Lua and cancellation data", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<int> agility(you.duration[DUR_AGILITY], 100);
    const auto random_state = rng::peek_uint64();
    init_duration_index();
    Options.language = lang_t::EN;
    status_info canonical;
    REQUIRE(fill_status_info(DUR_AGILITY, canonical));
    const string name = duration_name(DUR_AGILITY);
    const string end = duration_end_message(DUR_AGILITY);
    Options.language = lang_t::ZH;
    status_info still_canonical, shown;
    REQUIRE(fill_status_info(DUR_AGILITY, still_canonical));
    REQUIRE(fill_status_display_info(DUR_AGILITY, shown));
    CHECK(still_canonical.light_text == canonical.light_text);
    CHECK(still_canonical.short_text == canonical.short_text);
    CHECK(still_canonical.long_text == canonical.long_text);
    CHECK(string(duration_name(DUR_AGILITY)) == name);
    CHECK(duration_by_name(name) == DUR_AGILITY);
    CHECK(shown.light_text == "敏捷");
    CHECK(shown.long_text == "你身手敏捷。");
    CHECK(string(duration_end_message(DUR_AGILITY)) == "你感到不再那么敏捷了。");
    Options.language = lang_t::EN;
    REQUIRE(fill_status_display_info(DUR_AGILITY, shown));
    CHECK(shown.light_text == canonical.light_text);
    CHECK(shown.long_text == canonical.long_text);
    CHECK(string(duration_end_message(DUR_AGILITY)) == end);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("D-source god titles and whole messages retain English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    const string canonical = god_title(GOD_ZIN, SP_HUMAN, 200);
    Options.language = lang_t::ZH;
    CHECK(god_display_title(GOD_ZIN, SP_HUMAN, 200) == "律法使者");
    CHECK(god_title(GOD_ZIN, SP_HUMAN, 200) == canonical);
    const string original = "A forgotten god smites an untranslated monster!";
    CHECK(ui_render("d.curse", "A forgotten god smites %s%s", original,
        {{"unknown", false}, ui_literal_arg("!")}) == original);
    CHECK(ui_render("d.cards", "You have drawn %s.", "You have drawn a new card.",
        {ui_text_arg("a.card.name", "a new card")}) == "You have drawn a new card.");
    CHECK(ui_render("d.cards", "Upstream has changed this message.", original, {}) == original);
    Options.language = lang_t::EN;
    CHECK(god_display_title(GOD_ZIN, SP_HUMAN, 200) == canonical);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Cloud display names preserve canonical Lua and damage names", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    for (int i = CLOUD_NONE + 1; i < NUM_CLOUD_TYPES; ++i)
    {
        const auto type = static_cast<cloud_type>(i);
        for (bool terse : {false, true})
        {
            const string canonical = cloud_type_name(type, terse);
            Options.language = lang_t::EN;
            CHECK(cloud_display_name(type, terse) == canonical);
            string shown = "untouched";
            CHECK_FALSE(try_cloud_display_name(type, terse, shown));
            CHECK(shown == "untouched");
            Options.language = lang_t::ZH;
            REQUIRE(try_cloud_display_name(type, terse, shown));
            CHECK(shown != canonical);
            CHECK(cloud_type_name(type, terse) == canonical);
        }
        const string key = cloud_type_name(type);
        CHECK(cloud_type_name(cloud_name_to_type(key)) == key);
    }
    for (auto type : {CLOUD_NONE, CLOUD_RANDOM, CLOUD_DEBUGGING})
    {
        string shown = "untouched";
        CHECK_FALSE(try_cloud_display_name(type, false, shown));
        CHECK(shown == "untouched");
        CHECK(cloud_display_name(type) == cloud_type_name(type));
    }
    Options.language = lang_t::EN;
    const string original_info = extra_cloud_info(CLOUD_GREY_SMOKE);
    CHECK(original_info.find("This cloud is opaque") != string::npos);
    Options.language = lang_t::ZH;
    CHECK(extra_cloud_info(CLOUD_GREY_SMOKE).find("此云层不透明") != string::npos);
    Options.language = lang_t::EN;
    CHECK(extra_cloud_info(CLOUD_GREY_SMOKE) == original_info);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Cloud effect logs fall back as complete English sentences", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    cloud_struct cloud;
    cloud.type = CLOUD_FIRE;
    Options.language = lang_t::EN;
    const string original = cloud_engulfed_message(cloud, you);
    CHECK(original == "you are engulfed in blazing flames.");
    CHECK(cloud_engulfed_message(cloud, you, true) == "you bask in blazing flames.");
    Options.language = lang_t::ZH;
    CHECK(cloud_engulfed_message(cloud, you) == "你被熊熊烈焰吞没。");
    CHECK(cloud_engulfed_message(cloud, you, true) == "你沐浴在熊熊烈焰中。");
    cloud.type = CLOUD_BATS;
    CHECK(cloud_engulfed_message(cloud, you) == "你被蝙蝠群围攻。");
    cloud.type = CLOUD_RANDOM;
    CHECK(cloud_engulfed_message(cloud, you) == "you are engulfed in buggy goodness.");
    CHECK(ui_render("c.cloud", "%s is engulfed in %s.", original,
        {ui_actor_arg(you), ui_text_arg("c.cloud.name", "new upstream cloud")}) == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Seed display does not alter canonical exported seed description", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    const string canonical = seed_description();
    Options.language = lang_t::EN;
    CHECK(seed_display_description() == canonical);
    Options.language = lang_t::ZH;
    CHECK(seed_display_description().find("游戏种子：") == 0);
    CHECK(seed_description() == canonical);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Character viewer preserves the canonical export and English mode", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<vector<string>> order(Options.dump_order, {"header", "gold"});
    unwind_var<int> found(you.attribute[ATTR_GOLD_FOUND], 123);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::ZH;
    const string canonical = character_dump_text();
    CHECK(canonical.find("You have collected 123 gold pieces.") != string::npos);
    const string shown = character_dump_text(true);
    CHECK(shown.find("版（") != string::npos);
    CHECK(shown.find("你共收集了 123 枚金币。") != string::npos);
    CHECK(character_dump_text() == canonical);
    Options.language = lang_t::EN;
    CHECK(character_dump_text(true) == canonical);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("A-source dynamic sentences require every typed input", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    const auto next_random = rng::peek_uint64();
    const string original = "You blink free of the vines!";
    CHECK(ui_render("a.actor", "%s %s free of %s!", original,
        {ui_actor_arg(you), ui_text_arg("a.escape.verb", "blink"),
         ui_text_arg("a.noun", "vines")}) == "你闪现并摆脱藤蔓的束缚！");
    CHECK(ui_render("a.actor", "%s %s free of %s!", original,
        {ui_actor_arg(you), ui_text_arg("a.escape.verb", "new motion"),
         ui_text_arg("a.noun", "vines")}) == original);
    CHECK(ui_render("a.actor", "Changed %s %s free of %s!", original,
        {ui_actor_arg(you), ui_text_arg("a.escape.verb", "blink"),
         ui_text_arg("a.noun", "vines")}) == original);
    Options.language = lang_t::EN;
    CHECK(ui_render("a.actor", "%s %s free of %s!", original,
        {ui_actor_arg(you), ui_text_arg("a.escape.verb", "blink"),
         ui_text_arg("a.noun", "vines")}) == original);
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Ability display names preserve canonical names and costs", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto next_random = rng::peek_uint64();
    int checked = 0;
    for (const ability_type ability : get_defined_abilities())
    {
        if (ability == ABIL_BEOGH_RECRUIT_APOSTLE
            || (ability >= ABIL_BEOGH_DISMISS_APOSTLE_1
                && ability <= ABIL_BEOGH_DISMISS_APOSTLE_3)
            || (ability >= ABIL_MAKHLEB_BRAND_SELF_1
                && ability <= ABIL_MAKHLEB_BRAND_SELF_3))
        {
            continue;
        }
        const string canonical = ability_name(ability, true);
        string shown = "unchanged";
        Options.language = lang_t::ZH;
        REQUIRE(try_ability_display_name(ability, shown));
        CHECK(shown == ui_translate("display.ability", canonical.c_str()));
        CHECK(ability_name(ability, true) == canonical);
        Options.language = lang_t::EN;
        CHECK(ability_display_name(ability) == canonical);
        shown = "unchanged";
        CHECK_FALSE(try_ability_display_name(ability, shown));
        CHECK(shown == "unchanged");
        ++checked;
    }
    CHECK(checked > 140);
    Options.language = lang_t::EN;
    CHECK(ability_by_name("Spit Poison") == ABIL_SPIT_POISON);
    CHECK(make_cost_description(ABIL_SPIT_POISON) == "Breath");
    Options.language = lang_t::ZH;
    CHECK(ability_display_name(ABIL_SPIT_POISON) == "喷吐毒液");
    CHECK(make_cost_description(ABIL_SPIT_POISON) == "吐息冷却");
    CHECK(ability_by_name("Spit Poison") == ABIL_SPIT_POISON);
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Ability deck descriptions preserve card order counts and RNG", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<CrawlHashTable> properties(you.props);
    you.props[deck_name(DECK_OF_DESTRUCTION)] = 7;
    auto &stack = you.props[NEMELEX_STACK_KEY].new_vector(SV_INT);
    stack.push_back(static_cast<int>(CARD_VELOCITY));
    stack.push_back(static_cast<int>(CARD_TOMB));
    const auto next_random = rng::peek_uint64();
    Options.language = lang_t::ZH;
    string shown;
    REQUIRE(try_deck_ability_display(DECK_OF_DESTRUCTION, shown));
    CHECK(shown.find("目前有 7 张牌，最多可容纳 26 张") != string::npos);
    const string canonical_deck = deck_description(DECK_OF_DESTRUCTION);
    REQUIRE(try_deck_ability_display(DECK_OF_DESTRUCTION, shown, false));
    CHECK(shown.find("一副魔法牌，") == 0);
    CHECK(shown.find("抽取") == string::npos);
    CHECK(which_decks(CARD_TOMB).find("这张牌") != string::npos);
    REQUIRE(try_deck_ability_display(DECK_STACK, shown));
    CHECK(shown.find("剩余的牌：陵墓、迅捷") != string::npos);
    CHECK(stack[0].get_int() == CARD_VELOCITY);
    CHECK(stack[1].get_int() == CARD_TOMB);
    CHECK(deck_cards(DECK_OF_DESTRUCTION) == 7);
    Options.language = lang_t::EN;
    shown = "unchanged";
    CHECK_FALSE(try_deck_ability_display(DECK_STACK, shown));
    CHECK(shown == "unchanged");
    CHECK_FALSE(try_deck_ability_display(DECK_OF_DESTRUCTION, shown, false));
    CHECK(shown == "unchanged");
    CHECK(deck_description(DECK_OF_DESTRUCTION) == canonical_deck);
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Generated artefact display fields preserve canonical inscriptions", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    int fields = 0;
    for (int index = UNRAND_START + 1; index < UNRAND_LAST; ++index)
    {
        const auto entry = get_unrand_entry(index);
        for (const auto text : {entry->inscrip, entry->dbrand, entry->descrip})
        {
            if (!text || !*text)
                continue;
            CAPTURE(index, text);
            Options.language = lang_t::ZH;
            CHECK(string(ui_translate("a.art-data", text)) != text);
            Options.language = lang_t::EN;
            CHECK(string(ui_translate("a.art-data", text)) == text);
            ++fields;
        }
    }
    CHECK(fields == 179);
    init_properties();
    const auto prior = get_unique_item_status(UNRAND_SINGING_SWORD);
    item_def item;
    item.quantity = 1;
    REQUIRE(make_item_unrandart(item, UNRAND_SINGING_SWORD));
    set_unique_item_status(item, prior);
    item.flags |= ISFLAG_IDENTIFIED;
    item.inscription = "!w 用户备注";
    const string canonical = artefact_inscription(item);
    const string original_name = item.name(DESC_A, false);
    const auto next_random = rng::peek_uint64();
    Options.language = lang_t::ZH;
    string shown;
    REQUIRE(try_artefact_display_inscription(item, shown));
    CHECK(shown.find("音波") != string::npos);
    CHECK(artefact_inscription(item) == canonical);
    REQUIRE(try_item_display_name(item, shown, false, false));
    CHECK(shown.find("音波") != string::npos);
    CHECK(shown.find("!w 用户备注") != string::npos);
    CHECK(item.name(DESC_A, false) == original_name);
    CHECK(item.inscription == "!w 用户备注");
    Options.language = lang_t::EN;
    shown = "unchanged";
    CHECK_FALSE(try_artefact_display_inscription(item, shown));
    CHECK(shown == "unchanged");
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Help translates before inserting actual command bindings", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    init_keybindings();
    const char *source = "<w>%</w>/<w>%</w>    : Open/Close door";
    Options.language = lang_t::ZH;
    string chinese = ui_translate("help.interface", source);
    REQUIRE(chinese == "<w>%</w>/<w>%</w>    : 开门／关门");
    insert_commands(chinese, { CMD_OPEN_DOOR, CMD_CLOSE_DOOR });
    CHECK(chinese == "<w>" + command_to_string(CMD_OPEN_DOOR)
          + "</w>/<w>" + command_to_string(CMD_CLOSE_DOOR)
          + "</w>    : 开门／关门");
    // Source changes must never pick up the old translation.
    const char *changed = "<w>%</w>/<w>%</w>    : Open/Close a door";
    CHECK(ui_translate("help.interface", changed) == changed);
    Options.language = lang_t::EN;
    CHECK(ui_translate("help.interface", source) == source);
    string english = ui_translate("help.interface", source);
    string original = source;
    insert_commands(english, { CMD_OPEN_DOOR, CMD_CLOSE_DOOR });
    insert_commands(original, { CMD_OPEN_DOOR, CMD_CLOSE_DOOR });
    CHECK(english == original);
}

TEST_CASE("Pixel text can wrap CJK without splitting UTF-8 or Latin words", "[localization]")
{
    CHECK(cjk_wrap_break("English words here", 10) == 7);
    CHECK(cjk_wrap_break("unbreakable", 6) == -1);
    CHECK(cjk_wrap_break("中文测试", 6) == 6);
    CHECK(cjk_wrap_break("中文，测试", 6) == 3);
    CHECK(cjk_wrap_break("中（文", 6) == 3);
    CHECK(cjk_wrap_break("English中文", 10) == 10);
    const string original = "中文测试";
    const int split = cjk_wrap_break(original.c_str(), 6);
    auto coloured = formatted_string::parse_string("<red>中文</red>测试");
    auto wrapped = coloured;
    REQUIRE(insert_wrap_break(wrapped, split));
    CHECK(wrapped.tostring() == "中文\n测试");
    CHECK(strwidth(wrapped.tostring()) == 8);
    formatted_string background;
    background.textcolour(RED);
    background.textbackground(BLUE);
    background += original;
    const auto operations = background.ops;
    REQUIRE(insert_wrap_break(background, split));
    REQUIRE(background.ops.size() == operations.size());
    for (size_t i = 0; i < operations.size(); ++i)
        if (operations[i].type != FSOP_TEXT)
            CHECK(background.ops[i] == operations[i]);
    CHECK(background.tostring() == "中文\n测试");
}

TEST_CASE("Unidentified talismans cover ordinary and cached random appearances without revealing identity", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    int tested = 0;
    for (int subtype : all_item_subtypes(OBJ_TALISMANS))
    {
        item_def item;
        item.base_type = OBJ_TALISMANS;
        item.sub_type = subtype;
        item.quantity = 1;
        item.flags = 0;
        const string base = talisman_type_name(subtype);
        CAPTURE(base);
        string shown;
        REQUIRE(try_item_display_name(item, shown, false, false));
        CHECK(shown == ui_translate("display.item", base.c_str()));
        CHECK(item.name(DESC_PLAIN, false, false, false) == base);
        item.flags = ISFLAG_RANDART;
        item.props[ARTEFACT_NAME_KEY] = "Never reveal this secret name";
        for (const auto &entry : zh_ui_catalogue)
        {
            if (string(entry.context) != "display.item.appearance")
                continue;
            const string key = entry.english;
            if (key != base && !ends_with(key, " " + base))
                continue;
            // Only this batch's exact suffix is relevant, not other item kinds.
            item.props[ARTEFACT_APPEAR_KEY] = key;
            REQUIRE(try_item_display_name(item, shown, false, false));
            CHECK(shown == entry.chinese);
            CHECK(shown.find("secret") == string::npos);
            CHECK(item.props[ARTEFACT_NAME_KEY].get_string() == "Never reveal this secret name");
            CHECK(item.props[ARTEFACT_APPEAR_KEY].get_string() == key);
            CHECK(item.flags == ISFLAG_RANDART);
            CHECK(item.name(DESC_PLAIN, false, false, false) == key);
            ++tested;
        }
        item.props[ARTEFACT_APPEAR_KEY] = "unreviewed scripted appearance";
        shown = "untouched";
        CHECK_FALSE(try_item_display_name(item, shown, false, false));
        CHECK(shown == "untouched");
        item.props.erase(ARTEFACT_APPEAR_KEY);
        CHECK_FALSE(try_item_display_name(item, shown, false, false));
    }
    CHECK(tested >= 572);
    item_def ink;
    ink.base_type = OBJ_TALISMANS;
    ink.sub_type = TALISMAN_INKWELL;
    ink.quantity = 1;
    ink.flags = 0;
    string shown;
    REQUIRE(try_item_display_name(ink, shown, false, false));
    CHECK(shown == "墨池护符");
    Options.language = lang_t::EN;
    shown = "untouched";
    CHECK_FALSE(try_item_display_name(ink, shown, false, false));
    CHECK(shown == "untouched");
    CHECK(ink.name(DESC_A, false) == "an inkwell talisman");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Equipment restrictions expose Chinese only through the explicit display option", "[localization]")
{
    init_properties();
    init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<species_type> species(you.species,SP_ONI);
    unwind_var<transformation> form(you.form,transformation::none);
    unwind_var<vector<player_equip_entry>> equipment(you.equipment.items,{});
    const auto state=rng::peek_uint64();
    for (species_type race : {SP_ONI,SP_SPRIGGAN})
    {
        you.species=race;
        for (armour_type type : {ARM_GLOVES,ARM_BOOTS})
        {
            item_def item;
            item.base_type=OBJ_ARMOUR;
            item.sub_type=type;
            item.quantity=1;
            item.flags=ISFLAG_IDENTIFIED;
            string english,shown;
            CHECK_FALSE(can_equip_item(item,true,&english));
            CHECK(english.find("Those are too ")==0);
            CHECK_FALSE(can_equip_item(item,true,&shown,true));
            CHECK(shown.find("它们对你的")==0);
            CHECK(shown.find(race==SP_ONI ? "太小了。" : "太大了。")!=string::npos);
            CHECK(shown.find("%s")==string::npos);
            Options.language=lang_t::EN;
            CHECK_FALSE(can_equip_item(item,true,&shown,true));
            CHECK(shown==english);
            Options.language=lang_t::ZH;
        }
    }
    CHECK(rng::peek_uint64()==state);
}

TEST_CASE("Actual delay ticks translate memorisation and dressing without changing action names", "[localization]")
{
    init_properties();
    init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<bool> level(you.on_current_level,false);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type,GAME_TYPE_NORMAL);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    unwind_var<transformation> form(you.form,transformation::none);
    unwind_var<int> width(crawl_view.msgsz.x,160);
    struct cleanup {~cleanup(){clear_message_store();}} reset;
    struct ReadyMemorise : MemoriseDelay {
        ReadyMemorise():MemoriseDelay(3,SPELL_MAGIC_DART){started=true;}
    };
    struct ReadyOn : EquipOnDelay {
        ReadyOn(item_def &item):EquipOnDelay(3,item,SLOT_BODY_ARMOUR){started=true;}
        bool invalidated() override{return false;}
    };
    struct ReadyOff : EquipOffDelay {
        ReadyOff(item_def &item):EquipOffDelay(3,item){started=true;}
        bool invalidated() override{return false;}
    };
    item_def robe;
    robe.base_type=OBJ_ARMOUR;
    robe.sub_type=ARM_ROBE;
    robe.quantity=1;
    robe.flags=ISFLAG_IDENTIFIED;
    const string canonical=robe.name(DESC_YOUR);
    const auto state=rng::peek_uint64();
    ReadyMemorise memo;
    clear_message_store();memo.handle();
    CHECK(get_last_messages(10,true).find("你继续背诵。")!=string::npos);
    CHECK(string(memo.name())=="memorise");
    CHECK(memo.duration==2);
    ReadyOn on(robe);
    clear_message_store();on.handle();
    CHECK(get_last_messages(10,true).find("你继续穿戴")!=string::npos);
    CHECK(get_last_messages(10,true).find("长袍")!=string::npos);
    CHECK(string(on.name())=="equip_on");
    ReadyOff off(robe);
    clear_message_store();off.handle();
    CHECK(get_last_messages(10,true).find("你继续卸下")!=string::npos);
    Options.language=lang_t::EN;
    clear_message_store();on.handle();
    CHECK(get_last_messages(10,true).find("You continue putting on "+canonical+".")!=string::npos);
    clear_message_store();memo.handle();
    CHECK(get_last_messages(10,true).find("You continue memorising.")!=string::npos);
    CHECK(robe.name(DESC_YOUR)==canonical);
    CHECK(rng::peek_uint64()==state);
}

TEST_CASE("Equipment display rows keep inventory letters and equipped state with English fallback", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<vector<player_equip_entry>> equipment(you.equipment.items, {});
    unwind_var<item_def> inventory(you.inv[0]);
    item_def &sword = you.inv[0];
    sword.clear();
    sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    sword.plus = 0;
    sword.brand = SPWPN_NORMAL;
    sword.flags = ISFLAG_IDENTIFIED;
    sword.slot = 'b';
    sword.pos = coord_def(-1, -1);
    sword.link = 0;
    you.equipment.items.emplace_back(0, SLOT_WEAPON, false, false, false);
    const string original = sword.name(DESC_INVENTORY_EQUIP);
    const auto state = rng::peek_uint64();
    string shown;
    REQUIRE(try_item_display_name(sword, shown, true, true));
    CHECK(shown == "b - +0 短剑（主手武器）");
    CHECK(sword.name(DESC_INVENTORY_EQUIP) == original);
    you.equipment.items.clear();
    REQUIRE(try_item_display_name(sword, shown, true, true));
    CHECK(shown == "b - +0 短剑");
    sword.brand = SPWPN_DEBUG_RANDART;
    shown = "untouched";
    CHECK_FALSE(try_item_display_name(sword, shown, true, true));
    CHECK(shown == "untouched");
    sword.brand = SPWPN_NORMAL;
    you.equipment.items.emplace_back(0, SLOT_WEAPON, false, false, false);
    Options.language = lang_t::EN;
    CHECK_FALSE(try_item_display_name(sword, shown, true, true));
    CHECK(sword.name(DESC_INVENTORY_EQUIP) == original);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Attack punctuation is a translated argument and preserves complete English fallbacks", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    for (int damage : {0, HIT_WEAK - 1, HIT_WEAK, HIT_MED, HIT_STRONG, 1024})
    {
        const string original = attack_strength_punctuation(damage);
        const ui_arg translated = ui_attack_punctuation(original);
        REQUIRE(translated.available);
        if (original == ".")
            CHECK(translated.text == "。");
        else
            CHECK(strwidth(translated.text) == int(original.size()) * 2);
        CHECK(attack_strength_punctuation(damage) == original);
        const string english = "The goblin hits you" + original;
        const char *source = "%s %s%s %s%s%s%s%s";
        // The hit renderer uses a translation group for its eight arguments.
        ui_translation_group group({{"log.melee", source}});
        REQUIRE(group.available());
        CHECK(make_stringf(group.get("log.melee", source), "哥布林", "", "击中", "你", "", "", "", translated.text.c_str())
              == "哥布林击中你" + translated.text);
        CHECK(ui_render("unknown.attack", "%s%s", english,
                        {ui_literal_arg("哥布林击中你"), translated}) == english);
        Options.language = lang_t::EN;
        CHECK_FALSE(ui_attack_punctuation(original).available);
        CHECK(attack_strength_punctuation(damage) == original);
        Options.language = lang_t::ZH;
    }
    CHECK_FALSE(ui_attack_punctuation("?unknown").available);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Item display names preserve quantities, metadata, secrecy and English fallback", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> known(you.type_ids[OBJ_POTIONS][POT_CURING], false);
    item_def potion;
    potion.base_type = OBJ_POTIONS;
    potion.sub_type = POT_CURING;
    potion.quantity = 3;
    potion.flags = ISFLAG_IDENTIFIED;
    potion.inscription = "!q 中文备注";
    const string canonical = potion.name(DESC_A, false);
    Options.language = lang_t::ZH;
    string display = "untouched";
    REQUIRE(try_item_display_name(potion, display));
    CHECK(display == "3 × 治疗药水 {!q 中文备注}");
    CHECK(potion.name(DESC_A, false) == canonical);
    CHECK(potion.quantity == 3);
    CHECK(potion.inscription == "!q 中文备注");

    InvEntry row(potion);
    const string sort_key = row.get_fullname();
    CHECK(row.get_text().find("治疗药水") != string::npos);
    CHECK(row.get_filter_text().find("potions of curing") != string::npos);
    CHECK(row.get_filter_text().find("治疗药水") != string::npos);
    CHECK(row.get_fullname() == sort_key);

    potion.flags = 0;
    display = "untouched";
    REQUIRE(try_item_display_name(potion, display));
    CHECK(display.find("治疗") == string::npos);
    CHECK(display.find("药水") != string::npos);
    potion.flags = ISFLAG_IDENTIFIED | ISFLAG_RANDART;
    CHECK_FALSE(try_item_display_name(potion, display));

    item_def sword;
    sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    sword.plus = 2;
    sword.brand = SPWPN_FLAMING;
    sword.flags = ISFLAG_IDENTIFIED;
    REQUIRE(try_item_display_name(sword, display));
    CHECK(display == "+2 短剑（火焰）");
    sword.flags |= ISFLAG_CURSED;
    REQUIRE(try_item_display_name(sword, display));
    CHECK(display.find("诅咒") != string::npos);
    // A known type with an untranslated modifier must not lose the modifier.
    sword.flags = ISFLAG_IDENTIFIED;
    sword.brand = SPWPN_DEBUG_RANDART;
    display = "untouched";
    CHECK_FALSE(try_item_display_name(sword, display));
    CHECK(display == "untouched");

    item_def wand;
    wand.base_type = OBJ_WANDS;
    wand.sub_type = WAND_FLAME;
    wand.quantity = 1;
    wand.charges = 7;
    wand.flags = ISFLAG_IDENTIFIED;
    REQUIRE(try_item_display_name(wand, display));
    CHECK(display == "火焰魔杖 (7)");

    potion.flags = ISFLAG_IDENTIFIED;
    Options.language = lang_t::EN;
    display = "untouched";
    CHECK_FALSE(try_item_display_name(potion, display));
    CHECK(display == "untouched");
    CHECK(potion.name(DESC_A, false) == canonical);
    CHECK(row.get_text() == row.MenuEntry::get_text());
    CHECK(row.get_filter_text() == item_prefix(potion, false) + " " + row.MenuEntry::get_text());
    struct CustomItemRow : InvEntry
    {
        CustomItemRow(const item_def &value) : InvEntry(value) { }
        string get_text() const override { return "custom metadata " + InvEntry::get_text(); }
    };
    const CustomItemRow custom(potion);
    CHECK(custom.get_filter_text() == item_prefix(potion, false) + " " + custom.get_text());
}

TEST_CASE("Every current monster type and ordinary item kind has a display name", "[localization]")
{
    init_properties();
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    for (int i = MONS_0; i < NUM_MONSTERS; ++i)
    {
        const auto type = static_cast<monster_type>(i);
        const auto *data = get_monster_data(type);
        if (!data || data->mc != type || invalid_monster_type(type)
            || mons_class_flag(type, M_UNFINISHED))
            continue;
        const string key = data->name;
        CAPTURE(key, i);
        string display;
        CHECK(try_monster_type_display_name(type, display));
        CHECK(!display.empty());
    }
    for (int i = 0; i < NUM_OBJECT_CLASSES; ++i)
        for (int subtype : all_item_subtypes(static_cast<object_class_type>(i)))
        {
            if (i == OBJ_JEWELLERY && subtype >= NUM_RINGS && subtype < AMU_FIRST_AMULET)
                continue; // unused numeric gap, not an item type
            item_def item;
            item.base_type = static_cast<object_class_type>(i);
            item.sub_type = subtype;
            item.quantity = 1;
            item.plus = 0;
            item.flags = ISFLAG_IDENTIFIED;
            if (item.base_type == OBJ_CORPSES)
                item.mon_type = MONS_RAT;
            const string key = item.name(DESC_DBNAME, true, false, false);
            CAPTURE(key, i, subtype);
            string display = "untouched";
            CHECK(try_item_display_name(item, display, false, false));
            CHECK(display != "untouched");
            CHECK(item.name(DESC_DBNAME, true, false, false) == key);
        }
}

TEST_CASE("Monster display compositions preserve canonical names and whole-name fallback", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    monster_info hydra(MONS_HYDRA);
    hydra.num_heads = 7;
    const string canonical = hydra.full_name(DESC_A);
    string display;
    REQUIRE(hydra.try_display_name(display));
    CHECK(display == "七头海德拉");
    CHECK(hydra.full_name(DESC_A) == canonical);
    monster_info zombie(MONS_ZOMBIE, MONS_ORC);
    REQUIRE(zombie.try_display_name(display));
    CHECK(display == "兽人丧尸");
    monster_info ghost(MONS_PLAYER_GHOST);
    ghost.mname = "Tester 中文";
    REQUIRE(ghost.try_display_name(display));
    CHECK(display == "Tester 中文的幽灵");
    monster_info slime(MONS_SLIME_CREATURE);
    slime.slime_size = 4;
    REQUIRE(slime.try_display_name(display));
    CHECK(display == "巨型史莱姆生物");
    int colour = 0;
    slime.to_string(2, display, colour, false, nullptr, false);
    CHECK(display == "2 × 史莱姆生物");
    monster_info ugly(MONS_UGLY_THING);
    ugly.set_colour(RED);
    ugly.to_string(3, display, colour, true, nullptr, false);
    CHECK(display == "3 × 丑陋之物");
    slime.mb.set(MB_NAME_REPLACE);
    slime.mname = "unreviewed scripted grammar";
    display = "untouched";
    CHECK_FALSE(slime.try_display_name(display));
    CHECK(display == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(hydra.try_display_name(display));
    CHECK(hydra.full_name(DESC_A) == canonical);
}

TEST_CASE("Unidentified item names never use hidden types and artefact names retain metadata", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> curing(you.type_ids[OBJ_POTIONS][POT_CURING], false);
    unwind_var<bool> haste(you.type_ids[OBJ_POTIONS][POT_HASTE], false);
    item_def a;
    a.base_type = OBJ_POTIONS; a.sub_type = POT_CURING;
    a.quantity = 1; a.subtype_rnd = 0;
    item_def b = a; b.sub_type = POT_HASTE;
    string first, second;
    REQUIRE(try_item_display_name(a, first, false, false));
    REQUIRE(try_item_display_name(b, second, false, false));
    CHECK(first == second);
    CHECK(first.find("治疗") == string::npos);
    item_def sword;
    sword.base_type = OBJ_WEAPONS; sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    REQUIRE(make_item_randart(sword, true));
    sword.flags |= ISFLAG_IDENTIFIED;
    sword.plus = 4; sword.inscription = "!w";
    sword.props[ARTEFACT_NAME_KEY] = "short sword of Identity";
    const string original = sword.name(DESC_A, false);
    REQUIRE(try_item_display_name(sword, first, false, false));
    CHECK(first.find("+4 短剑「Identity」") == 0);
    CHECK(first.find("!w") != string::npos);
    CHECK(sword.name(DESC_A, false) == original);
}

TEST_CASE("Randart display titles omit the cached base and naming delimiters without renaming items", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    item_def cloak;
    cloak.base_type = OBJ_ARMOUR;
    cloak.sub_type = ARM_CLOAK;
    cloak.quantity = 1;
    REQUIRE(make_item_randart(cloak, true));
    cloak.flags |= ISFLAG_IDENTIFIED;
    cloak.plus = -1;
    cloak.inscription = "!w";
    for (const auto &entry : vector<pair<string, string>>{
        {"cloak \"Kittixae\"", "Kittixae"},
        {"cloak of Kittixae", "Kittixae"},
        {"cloak of the Clouds", "the Clouds"},
        {"cloak \"of Flames\"", "of Flames"},
        {"cloak Terminus", "Terminus"}})
    {
        set_artefact_name(cloak, entry.first);
        const string original = cloak.name(DESC_A, false);
        const auto state = rng::peek_uint64();
        string shown;
        REQUIRE(try_item_display_name(cloak, shown, false, false));
        CHECK(shown.find("-1 披风「" + entry.second + "」") == 0);
        CHECK(shown.find("!w") != string::npos);
        CHECK(get_artefact_name(cloak) == entry.first);
        CHECK(cloak.name(DESC_A, false) == original);
        Options.language = lang_t::EN;
        shown = "untouched";
        CHECK_FALSE(try_item_display_name(cloak, shown, false, false));
        CHECK(shown == "untouched");
        CHECK(cloak.name(DESC_A, false) == original);
        Options.language = lang_t::ZH;
        CHECK(rng::peek_uint64() == state);
    }
    for (const char *name : {"long sword \"Kittixae\"", "cloak \"Kittixae", "cloak ", "opaque scripted title"})
    {
        set_artefact_name(cloak, name);
        string shown = "untouched";
        CHECK_FALSE(try_item_display_name(cloak, shown, false, false));
        CHECK(shown == "untouched");
    }
    set_artefact_name(cloak, "cloak \"Kittixae\"");
    cloak.flags &= ~ISFLAG_IDENTIFIED;
    cloak.props[ARTEFACT_APPEAR_KEY] = "embroidered cloak";
    string appearance;
    REQUIRE(try_item_display_name(cloak, appearance, false, false));
    CHECK(appearance.find("Kittixae") == string::npos);
    CHECK(appearance.find("-1") == string::npos);
    cloak.flags |= ISFLAG_IDENTIFIED;
    cloak.props.erase(ARTEFACT_NAME_KEY);
    const auto state = rng::peek_uint64();
    CHECK_FALSE(try_item_display_name(cloak, appearance, false, false));
    CHECK_FALSE(cloak.props.exists(ARTEFACT_NAME_KEY));
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("All extant fixed artefacts have translated names without changing their identity", "[localization]")
{
    init_properties();
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    for (int index = UNRAND_START + 1; index < UNRAND_LAST; ++index)
    {
        const auto *entry = get_unrand_entry(index);
        if (entry->flags & UNRAND_FLAG_DELETED)
            continue;
        const string key = entry->name;
        CAPTURE(index, key);
        const auto prior = get_unique_item_status(index);
        unwind_var<uint8_t> octopus(you.octopus_king_rings, 0);
        item_def item;
        item.quantity = 1;
        const bool created = make_item_unrandart(item, index);
        set_unique_item_status(item, prior);
        REQUIRE(created);
        item.flags |= ISFLAG_IDENTIFIED;
        const string canonical = item.name(DESC_A, false);
        string displayed;
        CHECK(try_item_display_name(item, displayed, false, false));
        CHECK(!displayed.empty());
        CHECK(item.name(DESC_A, false) == canonical);
        CHECK(item.unrand_idx == index);
        Options.language = lang_t::EN;
        displayed = "untouched";
        CHECK_FALSE(try_item_display_name(item, displayed, false, false));
        CHECK(displayed == "untouched");
        CHECK(item.name(DESC_A, false) == canonical);
        Options.language = lang_t::ZH;
        if (index == UNRAND_CEREBOV)
        {
            item.flags &= ~ISFLAG_IDENTIFIED;
            REQUIRE_FALSE(item.is_identified());
            CHECK(try_item_display_name(item, displayed, false, false));
            CHECK(displayed.find("瑟雷波夫") == string::npos);
            CHECK(displayed == "巨型蛇形剑");
        }
    }
}

TEST_CASE("Static UI translates display names without changing lookup keys", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    CHECK(string(skill_name(SK_FIGHTING)) == "Fighting");
    CHECK(string(ui_translate("display.skill", skill_name(SK_FIGHTING))) == "战斗");
    CHECK(string(ui_translate("display.creation", "Human")) == "人类");
    CHECK(string(ui_translate("phase3.newgame", "% - List aptitudes")) == "% - 查看天资");
    const string title = chop_string(make_stringf(
        ui_translate("phase3.spl-cast", "Your spells (%s)"),
        ui_translate("spell.action", "describe")), 25);
    CHECK(strwidth(title) == 25);
    CHECK(title.find("查看说明") != string::npos);
    CHECK(strwidth(chop_string(ui_translate("display.skill", "Conjurations"), 15)) == 15);
    Options.language = lang_t::EN;
    CHECK(string(ui_translate("display.skill", skill_name(SK_FIGHTING))) == "Fighting");
    CHECK(string(ui_translate("startup.mode", "Dungeon Crawl")) == "Dungeon Crawl");
}

TEST_CASE("Spell display names preserve canonical English and source fallback", "[localization]")
{
    init_spell_descs();
    init_spell_name_cache();
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    CHECK(string(spell_display_title(SPELL_MAGIC_DART)) == "魔法飞弹");
    CHECK(string(spell_title(SPELL_MAGIC_DART)) == "Magic Dart");
    CHECK(string(spell_display_title(SPELL_FIREBALL)) == "火球术");
    CHECK(spell_by_name("Magic Dart", true) == SPELL_MAGIC_DART);
    CHECK(string(ui_translate("display.spell", "Changed Magic Dart"))
          == "Changed Magic Dart");
    CHECK(string(ui_translate("display.god", "Zin")) == "辛");
    CHECK(string(ui_translate("display.god", "Changed Zin")) == "Changed Zin");
    for (int i = 0; i < NUM_SPELLS; ++i)
    {
        const auto spell = static_cast<spell_type>(i);
        if (!is_valid_spell(spell))
            continue;
        const string canonical = spell_title(spell);
        CHECK(string(spell_display_title(spell)) != canonical);
        Options.language = lang_t::EN;
        CHECK(string(spell_display_title(spell)) == canonical);
        Options.language = lang_t::ZH;
    }
    Options.language = lang_t::EN;
    CHECK(string(ui_translate("display.god", "Zin")) == "Zin");
}

TEST_CASE("Item logs use actual dropped quantities and conceal unknown potion types", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> curing(you.type_ids[OBJ_POTIONS][POT_CURING], false);
    unwind_var<bool> haste(you.type_ids[OBJ_POTIONS][POT_HASTE], false);
    item_def item;
    item.base_type = OBJ_POTIONS;
    item.sub_type = POT_CURING;
    item.quantity = 5;
    item.flags = ISFLAG_IDENTIFIED;
    const string original = item.name(DESC_A);
    const auto next_random = rng::peek_uint64();
    string message;
    REQUIRE(try_ui_item_log("You drop %s.", item, message, 2));
    CHECK(message == "你丢下了2 × 治疗药水。");
    CHECK(item.quantity == 5);
    CHECK(item.name(DESC_A) == original);

    item.flags = 0;
    item.subtype_rnd = 0;
    item_def other = item;
    other.sub_type = POT_HASTE;
    string unknown;
    REQUIRE(try_ui_item_log("You see here %s.", item, message));
    REQUIRE(try_ui_item_log("You see here %s.", other, unknown));
    CHECK(message == unknown);
    CHECK(message.find("治疗") == string::npos);
    CHECK(message.find("加速") == string::npos);

    message = "untouched";
    CHECK_FALSE(try_ui_item_log("You now drop %s.", item, message));
    CHECK(message == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_item_log("You drop %s.", item, message, 2));
    CHECK(message == "untouched");
    CHECK(item.quantity == 5);
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Combat misses require complete templates, adverbs and weapon names", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    string message;
    REQUIRE(try_ui_actor_name(you, message));
    CHECK(message == "你");
    REQUIRE(try_ui_melee_miss("You%s miss %s%s.", you, you, " barely", nullptr, message));
    CHECK(message == "你险些命中，但仍未能击中你。");
    item_def sword;
    sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD;
    sword.quantity = 1;
    sword.flags = ISFLAG_IDENTIFIED;
    REQUIRE(try_ui_melee_miss("You%s miss %s%s.", you, you, "", &sword, message));
    CHECK(message.find("（使用+0 短剑）") != string::npos);
    message = "untouched";
    CHECK_FALSE(try_ui_melee_miss("You%s miss %s%s.", you, you, " newly dodged", &sword, message));
    CHECK(message == "untouched");
    CHECK_FALSE(try_ui_melee_miss("You%s now miss %s%s.", you, you, "", &sword, message));
    CHECK(message == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_melee_miss("You%s miss %s%s.", you, you, " barely", &sword, message));
    CHECK(message == "untouched");
    CHECK_FALSE(try_ui_actor_name(you, message));
    CHECK(message == "untouched");
}

TEST_CASE("Projectile display names leave damage attribution and item identity in English", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    item_def bow;
    bow.base_type = OBJ_WEAPONS;
    bow.sub_type = WPN_SHORTBOW;
    bow.quantity = 1;
    const string canonical = launched_projectile_name(bow);
    REQUIRE(canonical == "arrow");
    const auto next_random = rng::peek_uint64();
    string displayed;
    REQUIRE(try_ui_projectile_name(canonical, displayed));
    CHECK(displayed == "箭矢");
    CHECK(launched_projectile_name(bow) == canonical);
    REQUIRE(try_ui_projectile_name("magic dart", displayed));
    CHECK(displayed == "魔法飞弹");
    REQUIRE(try_ui_projectile_name("Magic Dart", displayed));
    CHECK(displayed == "魔法飞弹");
    displayed = "untouched";
    CHECK_FALSE(try_ui_projectile_name("changed magic dart", displayed));
    CHECK(displayed == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_projectile_name(canonical, displayed));
    CHECK(displayed == "untouched");
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Floor item lists preserve quantities, unknown appearances and whole-list fallback", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> curing(you.type_ids[OBJ_POTIONS][POT_CURING], false);
    unwind_var<bool> haste(you.type_ids[OBJ_POTIONS][POT_HASTE], false);
    item_def potion;
    potion.base_type = OBJ_POTIONS;
    potion.sub_type = POT_CURING;
    potion.subtype_rnd = 0;
    potion.quantity = 3;
    item_def other = potion;
    other.sub_type = POT_HASTE;
    const string canonical = potion.name(DESC_A);
    const auto next_random = rng::peek_uint64();
    string first, second;
    REQUIRE(try_ui_floor_item_list({&potion}, first));
    REQUIRE(try_ui_floor_item_list({&other}, second));
    CHECK(first == second);
    CHECK(first.find("治疗") == string::npos);
    CHECK(first.find("加速") == string::npos);
    REQUIRE(try_ui_floor_item_list({&potion, &other}, first));
    CHECK(formatted_string::parse_string(first).tostring().find("3 × ") != string::npos);
    CHECK(potion.quantity == 3);
    CHECK(potion.name(DESC_A) == canonical);

    item_def unsupported;
    unsupported.base_type = OBJ_WEAPONS;
    unsupported.sub_type = WPN_SHORT_SWORD;
    unsupported.quantity = 1;
    unsupported.flags = ISFLAG_IDENTIFIED;
    unsupported.brand = SPWPN_DEBUG_RANDART;
    first = "untouched";
    CHECK_FALSE(try_ui_floor_item_list({&potion, &unsupported}, first));
    CHECK(first == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_floor_item_list({&potion}, first));
    CHECK(first == "untouched");
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("Encounter and possessive status templates fall back without mixing names", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next_random = rng::peek_uint64();
    string displayed;
    REQUIRE(try_ui_actor_log("log.encounter", "%s comes into view.", you, displayed));
    CHECK(displayed == "你进入了视野。");
    REQUIRE(try_ui_actor_log("log.monster", "%s's wounds heal themselves!", you, displayed));
    CHECK(displayed == "你的伤口自行愈合了！");
    CHECK(string(ui_translate("log.encounter.title", "terence title")) == "terence title"); // Titles live only in TextDB.
    displayed = "untouched";
    CHECK_FALSE(try_ui_actor_log("log.encounter", "%s just came into view.", you, displayed));
    CHECK(displayed == "untouched");
    Options.language = lang_t::EN;
    CHECK_FALSE(try_ui_actor_log("log.encounter", "%s comes into view.", you, displayed));
    CHECK(displayed == "untouched");
    CHECK(rng::peek_uint64() == next_random);
}

TEST_CASE("UI translations reuse language and preserve English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<const char *> code(Options.lang_name);
    unwind_var<string> option(Options.language_option);
    Options.read_option_line("language = zh_CN");
    REQUIRE(Options.language == lang_t::ZH);
    CHECK(string(ui_translate("game_menu.return", "Return to game")) == "返回游戏");
    CHECK(string(ui_translate("missing", "Still English")) == "Still English");
    CHECK(string(ui_translate("game_menu.return", "Return to the game"))
          == "Return to the game");
    CHECK(make_stringf(ui_translate("player.level", "You have reached level %d!"), 2)
          == "你已升至 2 级！");
    CHECK(make_stringf(ui_translate("combat.fear", "You attempt to attack %s, but flinch away in fear!"), "the goblin")
          == "You attempt to attack the goblin, but flinch away in fear!");
    Options.read_option_line("language = en");
    REQUIRE(Options.language == lang_t::EN);
    CHECK(Options.lang_name == nullptr);
    CHECK(string(ui_translate("game_menu.return", "Return to game")) == "Return to game");
    Options.read_option_line("language = fr");
    REQUIRE(Options.language == lang_t::FR);
    CHECK(string(ui_translate("game_menu.return", "Return to game")) == "Return to game");
}

TEST_CASE("Help documents require the complete English source and preserve fallback pointers", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    for (const char *name : {"quickstart.txt", "macros_guide.txt",
                             "tiles_help.txt", "crawl_manual.txt", "aptitudes.txt"})
    {
        // The test runner does not initialize the game's search paths.
        FILE *file = fopen_u(("../docs/" + string(name)).c_str(), "r");
        REQUIRE(file != nullptr);
        formatted_string original;
        char buffer[200];
        while (fgets(buffer, sizeof buffer, file))
            original += string(buffer);
        fclose(file);
        REQUIRE(!original.ops.empty());
        trim_string_right(original.ops.back().text);
        const string english = original.tostring();
        const char *translated = ui_translate("help.document", english.c_str());
        CHECK(translated != english.c_str());
        CHECK((string(translated).find("宏") != string::npos
               || string(translated).find("快速入门") != string::npos
               || string(translated).find("图形版") != string::npos
               || string(translated).find("技能天资表") != string::npos));
        if (string(name) == "crawl_manual.txt")
        {
            const auto english_keys = help_document_hotkeys(english);
            const string chinese(translated);
            const auto chinese_keys = help_document_hotkeys(chinese);
            REQUIRE(english_keys.size() == 20);
            REQUIRE(chinese_keys.size() == 20);
            const auto lines = split_string("\n", chinese, false, true);
            for (const auto &key : english_keys)
            {
                REQUIRE(chinese_keys.count(key.first));
                const int row = chinese_keys.at(key.first);
                REQUIRE(row > 0);
                CHECK(lines[row - 1] == string(72, '-'));
                CHECK(tolower_safe(lines[row][0]) == key.first);
                CHECK(lines[row][1] == '.');
            }
            CHECK(chinese_keys.at('n') != english_keys.at('n'));
            CHECK(chinese.find("选择地图种子") != string::npos);
            CHECK(chinese.find("地下城冒险教程") != string::npos);
            CHECK(chinese.find("提示模式") != string::npos);
            CHECK(chinese.find("按键与命令列表") != string::npos);
            CHECK(chinese.find("备选动作") != string::npos);
        }
    }
    int checked = 0;
    for (const auto &entry : zh_ui_catalogue)
    {
        if (string(entry.context) != "help.document")
            continue;
        ++checked;
        Options.language = lang_t::ZH;
        CHECK(string(ui_translate(entry.context, entry.english)) == entry.chinese);
        const string updated = string(entry.english) + "\nUpstream changed.";
        CHECK(ui_translate(entry.context, updated.c_str()) == updated.c_str());
        CHECK(ui_translate("missing.help.document", entry.english) == entry.english);
        Options.language = lang_t::EN;
        CHECK(ui_translate(entry.context, entry.english) == entry.english);
    }
    CHECK(checked == 5);
}

TEST_CASE("Manual section anchors ignore contents and preserve empty lines", "[localization]")
{
    const string rule(72, '-');
    const string text = "目录\nA. 概述\n\n" + rule + "\nA. 概述\n"
                        + rule + "\n\n正文\n\n" + rule + "\n4. 按键与命令列表\n";
    const auto keys = help_document_hotkeys(text);
    REQUIRE(keys.size() == 2);
    CHECK(keys.at('a') == 4);
    CHECK(keys.at('4') == 10);
    CHECK(help_document_hotkeys("A. no section rule\n").empty());
}

#ifdef USE_SQLITE_DBM
namespace
{
void load_reviewed_textdb(DBM *db);

struct LocalizedTestDB
{
    LocalizedTestDB(bool reviewed = false)
    {
        char name[] = "localization-textdb-XXXXXX";
        const int fd = mkstemp(name);
        if (fd < 0)
            throw runtime_error("Cannot reserve test DB path");
        close(fd);
        path = name;
        db = dbm_open(path.c_str(), O_RDWR | O_CREAT, 0600);
        if (reviewed) load_reviewed_textdb(db);
    }
    ~LocalizedTestDB()
    {
        if (db)
            dbm_close(db);
        std::remove((path + ".db").c_str());
        std::remove(path.c_str());
    }
    void put(const string &key, const string &body)
    {
        const datum k(key), v(body);
        REQUIRE(dbm_store(db, k, v, DBM_REPLACE) == 0);
    }
    void erase(const string &key) { db->remove(key); }
    bool contains(const string &key) { const datum k(key); return dbm_fetch(db,k).dsize>0; }
    string path;
    DBM *db = nullptr;
};
}

namespace
{
map<string, string> read_speech_fixture(const char *path)
{
    std::ifstream input(path);
    REQUIRE(input.good());
    map<string, string> result;
    string key, body, line;
    bool active = false;
    const auto finish = [&]() {
        if (key.empty() || starts_with(key,"__lua_")) return;
        body.erase(0, body.find_first_not_of('\n'));
        result[lowercase_string(key)] = body;
    };
    while (std::getline(input, line))
    {
        trim_string_right(line);
        if (starts_with(line, "#")) continue;
        if (starts_with(line, "%%%%"))
        { finish(); key.clear(); body.clear(); active = true; }
        else if (active && key.empty()) { key = line; trim_string(key); }
        else if (active) body += line + "\n";
    }
    finish();
    return result;
}

void load_reviewed_textdb(DBM *db)
{
    // Fixtures consume only the same reviewed export shipped with the game.
    for (const char *name : {"monspeak","monspell","monflee","wpnnoise","insult",
        "godspeak","colourname","graffiti","miscast","miscname","godname",
        "montitle","decorlines","shout","help","FAQ"}) {
        const string path=string("dat/database/zh/")+name+".txt";
        for (const auto &entry : read_speech_fixture(path.c_str())) {
            const datum key(entry.first),value(entry.second);
            REQUIRE(dbm_store(db,key,value,DBM_REPLACE)==0);
        }
    }
}
}

static string original_item_noise(const item_def &item, string msg, msg_channel_type &channel)
{
    // TODO: messaging for cases where act != you. (This doesn't come up right
    // now.)
    if (is_unrandom_artefact(item))
    {
        // "Your Singing Sword" sounds disrespectful
        // (as if there could be more than one!)
        msg = replace_all(msg, "@Your_weapon@", "@The_weapon@");
        msg = replace_all(msg, "@your_weapon@", "@the_weapon@");
    }

    // Set appropriate channel (will usually be TALK).
    channel = MSGCH_TALK;

    string param;
    const string::size_type pos = msg.find(":");

    if (pos != string::npos)
        param = msg.substr(0, pos);

    if (!param.empty())
    {
        bool match = true;

        if (param == "DANGER")
            channel = MSGCH_DANGER;
        else if (param == "WARN")
            channel = MSGCH_WARN;
        else if (param == "SOUND")
            channel = MSGCH_SOUND;
        else if (param == "PLAIN")
            channel = MSGCH_PLAIN;
        else if (param == "SPELL")
            channel = MSGCH_FRIEND_SPELL;
        else if (param == "ENCHANT")
            channel = MSGCH_FRIEND_ENCHANT;
        else if (param == "VISUAL")
            channel = MSGCH_TALK_VISUAL;
        else if (param != "TALK")
            match = false;

        if (match)
            msg = msg.substr(pos + 1);
    }

    if (msg.empty()) // give default noises
    {
        channel = MSGCH_SOUND;
        msg = "You hear a strange noise.";
    }

    // Replace weapon references. Can't use DESC_THE because that includes
    // pluses etc. and we want just the basename.
    msg = replace_all(msg, "@The_weapon@", "The @weapon@");
    msg = replace_all(msg, "@the_weapon@", "the @weapon@");
    msg = replace_all(msg, "@Your_weapon@", "Your @weapon@");
    msg = replace_all(msg, "@your_weapon@", "your @weapon@");
    msg = replace_all(msg, "@weapon@", item.name(DESC_BASENAME));

    // replace references to player name and god
    msg = replace_all(msg, "@player_name@", you.your_name);
    {
        unwind_var<lang_t> canonical(Options.language, lang_t::EN);
        msg = replace_all(msg, "@player_god@",
                          you_worship(GOD_NO_GOD) ? "atheism"
                          : god_name(you.religion, coinflip()));
    }
    msg = replace_all(msg, "@player_genus@",
                species::name(you.species, species::SPNAME_GENUS));
    msg = replace_all(msg, "@a_player_genus@",
                article_a(species::name(you.species, species::SPNAME_GENUS)));
    msg = replace_all(msg, "@player_genus_plural@",
                pluralise(species::name(you.species, species::SPNAME_GENUS)));

    msg = maybe_pick_random_substring(msg);
    msg = maybe_capitalise_substring(msg);

    return msg;
}


TEST_CASE("Weapon noise uses canonical speech channels substitutions and random state", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    unwind_var<species_type> species_guard(you.species, SP_MINOTAUR);
    unwind_var<string> player_name(you.your_name, "TestHero");
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    item_def blade;
    blade.base_type = OBJ_WEAPONS;
    blade.sub_type = WPN_SHORT_SWORD;
    blade.quantity = 1;
    blade.flags |= ISFLAG_IDENTIFIED;
    int translated = 0, fallback = 0;
    for (god_type god : {GOD_NO_GOD, GOD_TROG, GOD_JIYVA})
    {
        unwind_var<god_type> religion(you.religion, god);
        for (const char *key : {"noisy weapon", "singing sword silenced",
            "singing sword no_tension", "singing sword low_tension",
            "singing sword high_tension", "singing sword SCREAM",
            "frozen axe \"frostbite\"", "trishula \"Condemnation\"",
            "shield of the gong"})
        for (int seed = 0; seed < 128; ++seed)
        {
            CAPTURE(god, key, seed);
            string expected;
            uint64_t next;
            msg_channel_type expected_channel, channel;
            {
                rng::subgenerator isolated(seed, 0);
                expected = original_item_noise(blade, getSpeakString(key, true), expected_channel);
                next = rng::peek_uint64();
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::EN;
                CHECK(render_item_noise(blade, getPairedCastString(key), channel) == expected);
                CHECK(channel == expected_channel);
                CHECK(rng::peek_uint64() == next);
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::ZH;
                const string shown = render_item_noise(blade, getPairedCastString(key), channel);
                CHECK(channel == expected_channel);
                CHECK(rng::peek_uint64() == next);
                if (shown == expected) ++fallback;
                else {
                    ++translated;
                    CHECK(shown.find_first_of("@[]{}") == string::npos);
                    CHECK(shown.find("Your ") == string::npos);
                    CHECK(shown.find("The ") == string::npos);
                }
            }
        }
    }
    CHECK(translated > 0);
    CHECK(fallback > 0);
    msg_channel_type channel;
    Options.language = lang_t::ZH;
    CHECK(render_item_noise(blade, {"@Your_weapon@ sings.", "@Your_weapon@唱歌。", false}, channel)
          == "Your +0 short sword sings.");
    CHECK(render_item_noise(blade, {"VISUAL:quiet", "SOUND:安静", true}, channel) == "quiet");
    CHECK(channel == MSGCH_TALK_VISUAL);
    CHECK(render_item_noise(blade, {"@unknown@ speaks.", "@unknown@说话。", true}, channel)
          == "@unknown@ speaks.");
    CHECK(render_item_noise(blade, {"", "", false}, channel) == "You hear a strange noise.");
    CHECK(channel == MSGCH_SOUND);
    init_properties();
    const auto previous = get_unique_item_status(UNRAND_SINGING_SWORD);
    item_def singing;
    singing.quantity = 1;
    REQUIRE(make_item_unrandart(singing, UNRAND_SINGING_SWORD));
    set_unique_item_status(singing, previous);
    singing.flags |= ISFLAG_IDENTIFIED;
    for (int seed = 0; seed < 128; ++seed)
    {
        string expected;
        uint64_t next;
        msg_channel_type expected_channel;
        {
            rng::subgenerator isolated(seed, 0);
            expected = original_item_noise(singing, getSpeakString("singing sword silenced", true), expected_channel);
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        const string shown = render_item_noise(singing, getPairedCastString("singing sword silenced"), channel);
        CHECK(shown != expected);
        CHECK(shown.find("歌唱之剑") != string::npos);
        CHECK(channel == expected_channel);
        CHECK(rng::peek_uint64() == next);
    }
}

TEST_CASE("Fountain and food decoration preserve canonical choices and opaque component fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    // Reuse the canonical cache from the preceding weapon fixture. TextDB
    // retains its source timestamp across shutdown, even when save_dir changes.
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    const auto entries = read_speech_fixture("dat/database/zh/decorlines.txt");
    CHECK(entries.size() == 132);
    const set<string> opaque = {"default peaceful fountain_blue", "default fountain_blue",
        "default peaceful fountain_sparkling", "default fountain_sparkling", "default fountain_blood",
        "default peaceful dry_fountain", "default dry_fountain", "elyvilon peaceful fountain_blue",
        "gozag fountain_blood", "makhleb peaceful fountain_blood", "okawaru peaceful fountain_blue",
        "trog peaceful fountain_blue"};
    for (const auto &entry : entries)
    for (int seed = 0; seed < (opaque.count(entry.first) ? 0 : 128); ++seed)
    {
        CAPTURE(entry.first, seed);
        string expected;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = maybe_pick_random_substring(getMiscString(entry.first, "", true));
            expected = replace_all(expected, "@your_weapon@", "your sword");
            expected = replace_all(expected, "@your_hands@", "your hands");
            expected = do_mon_name_replacements(expected);
            next = rng::peek_uint64();
        }
        for (lang_t mode : {lang_t::EN, lang_t::ZH})
        {
            rng::subgenerator isolated(seed, 0);
            Options.language = mode;
            auto speech = getPairedMiscString(entry.first);
            speech.pick_substrings();
            const string shown = render_decor_line(speech, "your sword", "your hands");
            CHECK(rng::peek_uint64() == next);
            if (mode == lang_t::EN) CHECK(shown == expected);
            else {
                CHECK(shown != expected);
                CHECK(shown.find_first_of("@[]{}ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
            }
        }
    }
    Options.language = lang_t::ZH;
    CHECK(render_decor_line({"Wash @your_hands@.", "清洗@your_hands@。", true}, "your sword", "your claws")
          == "Wash your claws.");
    CHECK(render_decor_line({"Clean @your_weapon@.", "清洗@your_weapon@。", true}, "your axe", "your hands")
          == "Clean your axe.");
    CHECK(render_decor_line({"An unreviewed line.", "未审消息。", false}, "", "") == "An unreviewed line.");
    CHECK(render_decor_line({"__NONE", "", false}, "", "") == "__NONE");
    CHECK(render_decor_line({"Unknown @marker@.", "未知@marker@。", true}, "", "") == "Unknown @marker@.");
    for (int seed = 0; seed < 32; ++seed)
    {
        string expected;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = do_mon_name_replacements("Name: @RANDGEN@");
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        CHECK(render_decor_line({"Name: @RANDGEN@", "名字：@RANDGEN@", true}, "", "") == "名字：" + expected.substr(6));
        CHECK(rng::peek_uint64() == next);
    }
}

TEST_CASE("Reviewed fountain Lua executes the canonical program once with exact result fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    lua_State *ls = clua.state();
    lua_stack_cleaner stack(ls);
    struct Globals {
        lua_State *ls;
        vector<pair<string, int>> values;
        Globals(lua_State *state) : ls(state) {
            for (const char *name : {"you", "crawl", "db14"}) {
                lua_getglobal(ls, name);
                values.emplace_back(name, luaL_ref(ls, LUA_REGISTRYINDEX));
            }
        }
        ~Globals() {
            for (const auto &value : values) {
                lua_rawgeti(ls, LUA_REGISTRYINDEX, value.second);
                lua_setglobal(ls, value.first.c_str());
                luaL_unref(ls, LUA_REGISTRYINDEX, value.second);
            }
        }
    } globals(ls);
    REQUIRE(clua.execstring(
        "db14={calls=0}; "
        "you={transform=function() db14.calls=db14.calls+1; return db14.form end, "
        "race=function() db14.calls=db14.calls+1; return db14.race end, "
        "god=function() db14.calls=db14.calls+1; return db14.god end, "
        "can_smell=function() db14.calls=db14.calls+1; return db14.smell end}; "
        "crawl={coinflip=function() db14.calls=db14.calls+1; return db14.flip end, "
        "one_chance_in=function(n) db14.calls=db14.calls+1; return db14.flip end}") == 0);
    const auto counter = [&]() {
        lua_getglobal(ls, "db14"); lua_getfield(ls, -1, "calls");
        const int value = lua_tointeger(ls, -1); lua_pop(ls, 2); return value;
    };
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    const char *states[] = {
        "db14.form='none';db14.race='Human';db14.god='No God';db14.smell=true;db14.flip=true",
        "db14.form='death';db14.race='Human';db14.god='Zin';db14.smell=false;db14.flip=true",
        "db14.form='none';db14.race='Mummy';db14.god='Cheibriados';db14.smell=false;db14.flip=false",
        "db14.form='storm';db14.race='Human';db14.god='Okawaru';db14.smell=true;db14.flip=true",
        "db14.form='wisp';db14.race='Human';db14.god='Trog';db14.smell=false;db14.flip=false",
        "db14.form='fungus';db14.race='Human';db14.god='Trog';db14.smell=true;db14.flip=false",
        "db14.form='none';db14.race='Human';db14.god='Trog';db14.smell=false;db14.flip=false",
        "db14.form='none';db14.race='Human';db14.god='No God';db14.smell=false;db14.flip=false"};
    int translated = 0, fallback = 0, trog_translated = 0;
    for (const char *state : states) {
        REQUIRE(clua.execstring(state) == 0);
        for (const char *key : {"default peaceful fountain_blue", "default fountain_blue",
            "default peaceful fountain_sparkling", "default fountain_sparkling", "default fountain_blood",
            "default peaceful dry_fountain", "default dry_fountain", "elyvilon peaceful fountain_blue",
            "gozag fountain_blood", "makhleb peaceful fountain_blood", "okawaru peaceful fountain_blue",
            "trog peaceful fountain_blue"})
        for (int seed = 0; seed < 64; ++seed) {
            CAPTURE(state, key, seed);
            string expected; uint64_t next; int calls;
            {
                rng::subgenerator isolated(seed, 0);
                REQUIRE(clua.execstring("db14.calls=0") == 0);
                expected = maybe_pick_random_substring(getMiscString(key, "", true));
                expected = replace_all(expected, "@your_weapon@", "your sword");
                expected = replace_all(expected, "@your_hands@", "your hands");
                expected = do_mon_name_replacements(expected);
                calls = counter(); next = rng::peek_uint64();
            }
            for (lang_t mode : {lang_t::EN, lang_t::ZH}) {
                rng::subgenerator isolated(seed, 0); Options.language = mode;
                REQUIRE(clua.execstring("db14.calls=0") == 0);
                auto speech = getPairedMiscString(key); speech.pick_substrings();
                const string shown = render_decor_line(speech, "your sword", "your hands");
                CHECK(counter() == calls); CHECK(rng::peek_uint64() == next);
                if (mode == lang_t::EN) CHECK(shown == expected);
                else if (shown == expected) ++fallback;
                else {
                    ++translated;
                    if (string(key) == "trog peaceful fountain_blue") ++trog_translated;
                    // Author identities intentionally retain their spelling.
                    CHECK(shown.find_first_of("@[]{}") == string::npos);
                }
            }
        }
    }
    CHECK(translated > 0); CHECK(fallback > 0); CHECK(trog_translated > 0);
    Options.language = lang_t::ZH;
    CHECK_FALSE(ui_text_arg("misc.lua.speech", "A changed unknown Lua result.").available);
    CHECK_FALSE(ui_text_arg("misc.lua.reviewed", "return 'An unreviewed program'").available);
}

TEST_CASE("Reviewed graffiti colours and fountain references share one canonical random stream", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    const auto entries = read_speech_fixture("dat/database/zh/graffiti.txt");
    CHECK(entries.size() == 58);
    int graffiti_translated = 0, graffiti_fallback = 0;
    map<string, int> translated_by_key;
    for (const auto &entry : entries)
    for (int seed = 0; seed < 128; ++seed) {
        CAPTURE(entry.first, seed);
        string expected; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = maybe_pick_random_substring(getMiscString(entry.first, "", true));
            expected = do_mon_name_replacements(expected);
            next = rng::peek_uint64();
        }
        for (lang_t mode : {lang_t::EN, lang_t::ZH}) {
            rng::subgenerator isolated(seed, 0); Options.language = mode;
            auto pair = getPairedMiscString(entry.first); pair.pick_substrings();
            const string shown = render_decor_line(pair, "your sword", "your hands");
            CHECK(rng::peek_uint64() == next);
            if (mode == lang_t::EN) CHECK(shown == expected);
            else if (shown == expected) {
                // Plain personal names, pure aliases and unresolved source
                // references may retain their canonical English.
                if (entry.first == "any_graffiti") ++graffiti_fallback;
            }
            else {
                CHECK(shown != expected);
                CHECK(shown.find_first_of("@[]{}") == string::npos);
                ++translated_by_key[entry.first];
                if (entry.first == "any_graffiti") ++graffiti_translated;
            }
        }
    }
    CHECK(graffiti_translated > 0);
    CHECK(graffiti_translated + graffiti_fallback == 128);
    // Do not require rare broken upstream branches to occur in random samples.
    // An unresolved reference must still preserve the complete English line.
    CHECK(render_decor_line({"Author: @graffiti_author_any@",
                             "作者：@graffiti_author_any@", true}, "", "")
          == "Author: @graffiti_author_any@");
    // Every prose-bearing entry must have an actual translated branch, rather
    // than passing all seeds through whole-English fallback unnoticed.
    const set<string> identity_aliases = {"_graffiti_author_any_",
        "_graffiti_author_any_changeable_", "_graffiti_author_any_changeable_non-randgen_",
        "_graffiti_author_any_non-orc_"};
    for (const auto &entry : entries)
        if (!identity_aliases.count(entry.first)) {
            CAPTURE(entry.first); CHECK(translated_by_key[entry.first] > 0);
        }
    Options.language = lang_t::ZH;
    for (const char *key : {"ancestor name", "bland name", "any orc name"})
    for (int seed = 0; seed < 32; ++seed) {
        string expected; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = getMiscString(key, "", true); next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        const auto pair = getPairedMiscString(key);
        CHECK(pair.complete); CHECK(pair.english == expected);
        CHECK(pair.text() == expected); CHECK(rng::peek_uint64() == next);
    }
    int fountain_graffiti = 0;
    for (const char *key : {"default dry_fountain", "default peaceful dry_fountain", "default fountain_blue"})
    for (int seed = 0; seed < 256; ++seed) {
        CAPTURE(key, seed);
        string expected; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = maybe_pick_random_substring(getMiscString(key, "", true));
            expected = do_mon_name_replacements(expected); next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0); Options.language = lang_t::ZH;
        auto pair = getPairedMiscString(key); pair.pick_substrings();
        const string shown = render_decor_line(pair, "your sword", "your hands");
        CHECK(rng::peek_uint64() == next);
        if (shown != expected) {
            CHECK(shown.find_first_of("@[]{}") == string::npos);
            if (shown.find("涂鸦") != string::npos) ++fountain_graffiti;
        }
    }
    CHECK(fountain_graffiti > 0);
}

TEST_CASE("Decoration random gods and skills share the untouched English replacement stream", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const vector<string> lines = {
        "@random_god@", "@random_god_good@", "@random_god_evil@", "@random_god_chaotic@",
        "@random_skill@", "@random_skill_magic@", "@random_skill_mundane@",
        "@random_god@/@random_god@/@random_god_good@/@random_skill_magic@/@random_skill_mundane@",
        "@RANDGEN@/@random_god@/@random_skill@"};
    for (const auto &line : lines)
    for (int seed = 0; seed < 128; ++seed) {
        CAPTURE(line, seed);
        string expected; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = do_mon_name_replacements(line); next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        textdb_speech pair(line, line, true);
        do_decor_name_replacements(pair);
        CHECK(pair.english == expected); CHECK(rng::peek_uint64() == next);
        CHECK(pair.complete);
        CHECK(pair.text() != expected);
        CHECK(pair.text().find('@') == string::npos);
        Options.language = lang_t::EN; CHECK(pair.text() == expected);
        Options.language = lang_t::ZH;
    }
    CHECK(render_decor_line({"@random_god@", "@random_god@", false}, "", "").find('@') == string::npos);
}

TEST_CASE("Reviewed artefact speech keeps canonical utterances anatomy and the original random stream", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-05-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    struct DatabaseScope
    {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    const auto entries = read_speech_fixture("dat/database/wpnnoise.txt");
    int quoted_keys = 0;
    for (const auto &entry : entries)
    {
        if (!starts_with(entry.first, "majin-bo ")
            && !starts_with(entry.first, "zonguldrok ")) continue;
        ++quoted_keys;
        CAPTURE(entry.first);
        for (int seed = 0; seed < 128; ++seed)
        {
            string expected;
            uint64_t next;
            {
                rng::subgenerator isolated(seed, 0);
                expected = "A voice whispers, \"" + getSpeakString(entry.first, true) + "\"";
                next = rng::peek_uint64();
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::EN;
                CHECK(quoted_artefact_speech(entry.first) == expected);
                CHECK(rng::peek_uint64() == next);
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::ZH;
                const string shown = quoted_artefact_speech(entry.first);
                CHECK(shown != expected);
                CHECK(shown.find_first_of("@[]{}ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
                CHECK(rng::peek_uint64() == next);
            }
        }
    }
    CHECK(quoted_keys == 16);
    for (int seed = 0; seed < 128; ++seed)
    {
        string expected;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = getSpeakString("fungus thoughts", true);
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        Options.language = lang_t::ZH;
        const auto pair = getPairedCastString("fungus thoughts");
        REQUIRE(pair.complete);
        CHECK(pair.english == expected);
        CHECK(pair.text() != expected);
        CHECK(rng::peek_uint64() == next);
    }
    int bracket_fallbacks = 0;
    for (const char *key : {"eel hand actions", "eel hand solo actions"})
    for (const char *head : {"head", "form"})
    for (const char *skin : {"skin", "scales", "feathers", "fur", "bandages",
                             "stone", "ectoplasm", "bones"})
    {
        CAPTURE(key, head, skin);
        for (int seed = 0; seed < 64; ++seed)
        {
            string expected;
            uint64_t next;
            {
                rng::subgenerator isolated(seed, 0);
                expected = replace_all(getSpeakString(key, true), "@head@", head);
                expected = replace_all(expected, "@skin@", skin);
                next = rng::peek_uint64();
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::EN;
                CHECK(display_eel_flavour(getPairedCastString(key), head, skin) == expected);
                CHECK(rng::peek_uint64() == next);
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::ZH;
                const string shown = display_eel_flavour(getPairedCastString(key), head, skin);
                CHECK(rng::peek_uint64() == next);
                CHECK(shown.find('@') == string::npos);
                if (expected.find('[') != string::npos)
                {
                    CHECK(shown == expected);
                    ++bracket_fallbacks;
                }
                else
                {
                    CHECK(shown != expected);
                    CHECK(shown.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
                }
            }
        }
    }
    CHECK(bracket_fallbacks > 0);
    Options.language = lang_t::ZH;
    textdb_speech missing("An eel nuzzles against your @head@.",
                         "一条鳗鱼亲昵地蹭着你的@head@。", true);
    CHECK(display_eel_flavour(missing, "unreviewed head", "skin")
          == "An eel nuzzles against your unreviewed head.");
    CHECK(quoted_artefact_speech("unreviewed future artefact speech")
          == "A voice whispers, \"\"");
}

TEST_CASE("Miscellaneous legacy review preserves recursive choices and executes canonical Lua only once", "[localization]")
{
    LocalizedTestDB approved(true);
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-04-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    const auto entries = read_speech_fixture("dat/database/miscname.txt");
    REQUIRE(entries.size() == 10);
    lua_State *ls = clua.state();
    lua_stack_cleaner stack(ls);
    struct LuaGlobals
    {
        lua_State *ls;
        vector<pair<string, int>> values;
        LuaGlobals(lua_State *state) : ls(state)
        {
            for (const char *name : {"you", "db04_calls", "db04_can_smell"})
            {
                lua_getglobal(ls, name);
                values.emplace_back(name, luaL_ref(ls, LUA_REGISTRYINDEX));
            }
        }
        ~LuaGlobals()
        {
            for (const auto &value : values)
            {
                lua_rawgeti(ls, LUA_REGISTRYINDEX, value.second);
                lua_setglobal(ls, value.first.c_str());
                luaL_unref(ls, LUA_REGISTRYINDEX, value.second);
            }
        }
    } globals(ls);
    REQUIRE(clua.execstring("db04_calls=0; db04_can_smell=true; you={can_smell=function() db04_calls=db04_calls+1; return db04_can_smell end}") == 0);
    const auto calls = [&]() {
        lua_getglobal(ls, "db04_calls");
        const int value = lua_tointeger(ls, -1);
        lua_pop(ls, 1);
        return value;
    };
    struct DatabaseScope
    {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    int lua_choices[2] = {0, 0};
    for (int smell = 0; smell < 2; ++smell)
    {
        lua_pushboolean(ls, smell);
        lua_setglobal(ls, "db04_can_smell");
        for (const auto &entry : entries)
        {
            CAPTURE(entry.first, smell);
            for (int seed = 0; seed < 256; ++seed)
            {
                string expected;
                uint64_t next;
                int expected_calls;
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::EN;
                    lua_pushinteger(ls, 0); lua_setglobal(ls, "db04_calls");
                    expected = getMiscString(entry.first, "", true);
                    REQUIRE(!expected.empty());
                    next = rng::peek_uint64();
                    expected_calls = calls();
                }
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::ZH;
                    lua_pushinteger(ls, 0); lua_setglobal(ls, "db04_calls");
                    const auto pair = getPairedMiscString(entry.first);
                    CAPTURE(pair.english, pair.translated);
                    REQUIRE(pair.complete);
                    CHECK(pair.english == expected);
                    CHECK(rng::peek_uint64() == next);
                    CHECK(calls() == expected_calls);
                    CHECK(calls() <= 1);
                    CHECK(pair.text() != expected);
                    CHECK(pair.text().find_first_of("@{}ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
                    if (expected_calls) ++lua_choices[smell];
                    Options.language = lang_t::EN;
                    CHECK(pair.text() == expected);
                }
            }
        }
    }
    CHECK(lua_choices[0] > 0);
    CHECK(lua_choices[1] > 0);
    Options.language = lang_t::ZH;
    CHECK(getPairedMiscString("unreviewed future misc key").text().empty());
    CHECK(getMiscString("SHT_int_loss", "", true).empty());
    auto prefix = getPairedMiscString("harlequin_trap_lines");
    const string original = prefix.english + " the attacks of the orc are laced with chaos!";
    CHECK(ui_render("t.log", "%s the attacks of %s are laced with chaos!", original,
        {{prefix.translated, prefix.complete}, ui_literal_arg("兽人")}) != original);
    CHECK(ui_render("t.log", "%s the attacks of %s are laced with chaos!", original,
        {{prefix.translated, prefix.complete}, {"unknown actor", false}}) == original);
    LocalizedTestDB english, legacy;
    for (const auto &entry : entries) english.put(entry.first, entry.second);
    english.put("future misc", "Changed upstream prose @welcome_spam@\n");
    legacy.put("future misc", "不允许恢复的旧译文\n");
    approved.erase("future misc");
    const auto stale = query_paired_speech(english.db, approved.db, "future misc", true);
    CHECK_FALSE(stale.complete);
    CHECK(stale.text() == stale.english);
}

TEST_CASE("Imp and demon taunts preserve every species draw and reject unpaired Chinese fragments", "[localization]")
{
    init_properties();
    init_monsters();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-03-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    const auto entries = read_speech_fixture("dat/database/insult.txt");
    REQUIRE(entries.size() == 33);
    struct DatabaseScope
    {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    monster imp;
    imp.type = imp.base_monster = MONS_CRIMSON_IMP;
    imp.hit_points = imp.max_hit_points = 10;
    imp.set_position(coord_def(10, 10));
    imp.foe = MHITYOU;
    const int voices[] = {S_DEMON_TAUNT, S_SHOUT, S_VERY_SOFT,
                          S_NORMAL_VOLUME, S_CAW, S_LAUGH};
    for (species_type race : {SP_HUMAN, SP_DEEP_ELF, SP_FELID,
                              SP_MOUNTAIN_DWARF, SP_TENGU, SP_MUMMY,
                              SP_VAMPIRE, SP_SPRIGGAN, SP_MINOTAUR})
    {
        you.species = race;
        for (const auto &entry : entries)
        {
            CAPTURE(race, entry.first);
            for (int seed = 0; seed < 12; ++seed)
            {
                const int voice = voices[seed % ARRAYSZ(voices)];
                string expected;
                uint64_t next;
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::EN;
                    const string selected = getSpeakString(entry.first, true);
                    REQUIRE(!selected.empty());
                    expected = do_mon_str_replacements(selected, imp, voice);
                    next = rng::peek_uint64();
                }
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::ZH;
                    auto pair = getPairedCastString(entry.first);
                    const string original = do_mon_str_replacements(pair.english, imp, voice, &pair);
                    CAPTURE(original, pair.translated);
                    REQUIRE(pair.complete);
                    CHECK(original == expected);
                    CHECK(rng::peek_uint64() == next);
                    CHECK(pair.text() != original);
                    CHECK(pair.text().find_first_of("@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
                    Options.language = lang_t::EN;
                    CHECK(pair.text() == expected);
                }
            }
        }
    }
    Options.language = lang_t::ZH;
    // Explicit English lookup stays canonical even with Chinese TextDB loaded.
    for (const auto &entry : entries)
    {
        const string legacy = getSpeakString(entry.first, true);
        CHECK(!legacy.empty());
        CHECK(std::all_of(legacy.begin(), legacy.end(), [](unsigned char c) { return c < 128; }));
    }
    for (const char *key : {"__DEMON_TAUNT", "imp"})
    {
        for (int seed = 0; seed < 64; ++seed)
        {
            string expected;
            uint64_t next;
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::EN;
                auto pair = getPairedShoutString(key);
                msg_channel_type channel = MSGCH_TALK;
                strip_channel_prefix(pair.english, channel);
                expected = do_mon_str_replacements(pair.english, imp, S_DEMON_TAUNT);
                next = rng::peek_uint64();
            }
            rng::subgenerator isolated(seed, 0);
            Options.language = lang_t::ZH;
            auto pair = getPairedShoutString(key);
            msg_channel_type en_channel = MSGCH_TALK, zh_channel = MSGCH_TALK;
            strip_channel_prefix(pair.english, en_channel);
            strip_channel_prefix(pair.translated, zh_channel);
            REQUIRE(en_channel == zh_channel);
            const string original = do_mon_str_replacements(pair.english, imp, S_DEMON_TAUNT, &pair);
            REQUIRE(pair.complete);
            CHECK(original == expected);
            CHECK(rng::peek_uint64() == next);
            const string shown = monster_shout_display(pair, imp, original);
            CHECK(shown != original);
            CHECK(shown.find_first_of("@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
            auto incomplete = pair;
            incomplete.complete = false;
            CHECK(monster_shout_display(incomplete, imp, original) == original);
        }
    }
}

TEST_CASE("Colour TextDB resolves every weighted namespace without redrawing or changing canonical gem colours", "[localization]")
{
    LocalizedTestDB approved(true);
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto entries = read_speech_fixture("dat/database/colourname.txt");
    REQUIRE(entries.size() == 9);
    LocalizedTestDB english, legacy;
    for (const auto &entry : entries) english.put(entry.first, entry.second);
    for (const auto &entry : entries)
    {
        CAPTURE(entry.first);
        for (int seed = 0; seed < 128; ++seed)
        {
            string canonical;
            uint64_t next;
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::EN;
                const auto pair = query_paired_speech(english.db, nullptr, entry.first, true);
                canonical = pair.text();
                next = rng::peek_uint64();
            }
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::ZH;
                const auto pair = query_paired_speech(english.db, approved.db, entry.first, true);
                REQUIRE(pair.complete);
                CHECK(pair.english == canonical);
                CHECK(pair.text() != canonical);
                CHECK(pair.text().find('@') == string::npos);
                CHECK(pair.text().find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
                CHECK(rng::peek_uint64() == next);
                Options.language = lang_t::EN;
                CHECK(pair.text() == canonical);
            }
        }
    }
    int colours = 0;
    for (string colour : split_string("\n\n", entries.at("misc_colour")))
    {
        trim_string(colour);
        Options.language = lang_t::ZH;
        CHECK(ui_text_arg("i.colour", colour).available);
        CHECK(string(ui_translate("i.colour", colour.c_str())) != colour);
        Options.language = lang_t::EN;
        CHECK(string(ui_translate("i.colour", colour.c_str())) == colour);
        ++colours;
    }
    CHECK(colours == 370);
    Options.language = lang_t::ZH;
    english.put("new colour", "Unreviewed upstream colour\n");
    legacy.put("new colour", "旧颜色不得复活\n");
    const auto missing = query_paired_speech(english.db, approved.db, "new colour", true);
    CHECK_FALSE(missing.complete);
    CHECK(missing.text() == "Unreviewed upstream colour");
}

TEST_CASE("Shout and sphinx riddle candidates share channels pronouns and the original random stream", "[localization]")
{
    LocalizedTestDB approved(true);
    init_properties();
    init_monsters();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    const auto entries = read_speech_fixture("dat/database/shout.txt");
    REQUIRE(entries.size() == 93);
    LocalizedTestDB english, legacy;
    for (const auto &entry : entries) english.put(entry.first, entry.second);
    for (const auto &entry : read_speech_fixture("dat/database/insult.txt"))
        english.put(entry.first, entry.second);
    monster caster;
    caster.hit_points = caster.max_hit_points = 30;
    caster.set_position(coord_def(10, 10));
    caster.foe = MHITYOU;
    int riddles = 0, new_shouts = 0;
    for (monster_type type : {MONS_JESSICA, MONS_TARANTELLA, MONS_CHAOS_SPAWN})
    {
        caster.type = caster.base_monster = type;
        for (const auto &entry : entries)
        {
            CAPTURE(type, entry.first);
            if (entry.first == "player sphinx riddle lines") continue;
            for (int seed = 0; seed < 12; ++seed)
            {
                string expected;
                uint64_t next;
                msg_channel_type expected_channel = MSGCH_TALK;
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::EN;
                    auto pair = query_paired_speech(english.db, nullptr, entry.first, true);
                    strip_channel_prefix(pair.english, expected_channel);
                    expected = do_mon_str_replacements(pair.english, caster, S_SILENT);
                    next = rng::peek_uint64();
                }
                rng::subgenerator isolated(seed, 0);
                Options.language = lang_t::ZH;
                auto pair = query_paired_speech(english.db, approved.db, entry.first, true);
                msg_channel_type channel = MSGCH_TALK, shown_channel = MSGCH_TALK;
                string original = pair.english;
                strip_channel_prefix(original, channel);
                strip_channel_prefix(pair.translated, shown_channel);
                pair.complete = pair.complete && channel == shown_channel;
                original = do_mon_str_replacements(original, caster, S_SILENT, &pair);
                CHECK(original == expected);
                CHECK(channel == expected_channel);
                CHECK(rng::peek_uint64() == next);
                const string shown = monster_shout_display(pair, caster, original);
                CAPTURE(pair.complete, pair.translated, channel, shown_channel);
                CHECK(shown.find('@') == string::npos);
                CHECK(shown.find("的的") == string::npos);
                if (!pair.complete) CHECK(shown == original);
                if (starts_with(entry.first, "sphinx riddle"))
                {
                    CHECK(shown.find("谜语") != string::npos);
                    CHECK(shown != original);
                    ++riddles;
                }
                if (starts_with(entry.first, "__caw") || starts_with(entry.first, "__laugh"))
                {
                    CHECK(shown != original);
                    CHECK(shown_channel == MSGCH_SOUND);
                    ++new_shouts;
                }
                Options.language = lang_t::EN;
                CHECK(monster_shout_display(pair, caster, original) == expected);
                CHECK(rng::peek_uint64() == next);
            }
        }
    }
    CHECK(riddles == 108);
    CHECK(new_shouts == 144);
    Options.language = lang_t::ZH;
    english.put("new shout", "SOUND:A new upstream shout.\n");
    legacy.put("new shout", "SOUND:旧喊声不得复活。\n");
    auto missing = query_paired_speech(english.db, approved.db, "new shout", true);
    CHECK_FALSE(missing.complete);
    CHECK(missing.text() == "SOUND:A new upstream shout.");
    // Non-prose alias chains may pass through without a redundant translation;
    // they must not hide a missing child or accept an untranslated literal.
    english.put("alias chain", "@__LAUGH unseen@@missing child@\n");
    english.put("missing child", "Unreviewed child.\n");
    const auto incomplete = query_paired_speech(english.db, approved.db, "alias chain", true);
    CHECK_FALSE(incomplete.complete);
    CHECK(incomplete.text() == incomplete.english);
    english.put("alias prose", "@__LAUGH unseen@ Unreviewed prose.\n");
    const auto prose = query_paired_speech(english.db, approved.db, "alias prose", true);
    CHECK_FALSE(prose.complete);
    CHECK(prose.text() == prose.english);
}

TEST_CASE("Miscast TextDB preserves English choices and falls back for incomplete anatomy or colours", "[localization]")
{
    LocalizedTestDB approved(true);
    init_properties();
    init_monsters();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    const auto read = [](const char *path) {
        std::ifstream input(path);
        REQUIRE(input.good());
        map<string, string> result;
        string key, body, line;
        bool active = false;
        const auto finish = [&]() {
            if (key.empty()) return;
            body.erase(0, body.find_first_not_of('\n'));
            result[lowercase_string(key)] = body;
        };
        while (std::getline(input, line))
        {
            trim_string_right(line);
            if (starts_with(line, "#")) continue;
            if (starts_with(line, "%%%%"))
            { finish(); key.clear(); body.clear(); active = true; }
            else if (active && key.empty()) { key = line; trim_string(key); }
            else if (active) body += line + "\n";
        }
        finish();
        return result;
    };
    const auto entries = read("dat/database/miscast.txt");
    REQUIRE(entries.size() == 33);
    LocalizedTestDB english, legacy;
    for (const auto &entry : entries) english.put(entry.first, entry.second);
    for (const auto &entry : read("dat/database/colourname.txt"))
        english.put(entry.first, entry.second);
    const auto original_render = [](actor &target, string msg, int damage) {
        bool plural;
        msg = replace_all(msg, "@hand@", target.hand_name(false, &plural));
        msg = replace_all(msg, "@hands@", target.hand_name(true));
        msg = replace_all(msg, "@hand_conj@", plural ? "" : "s");
        if (target.is_monster())
        {
            msg = do_mon_str_replacements(msg, *target.as_monster(), S_SILENT);
            if (!mons_has_body(*target.as_monster()))
            {
                msg = replace_all(msg, "'s body", "");
                msg = replace_all(msg, "'s skin", "");
            }
        }
        else msg = maybe_pick_random_substring(msg);
        return msg + attack_strength_punctuation(damage);
    };
    monster caster;
    caster.hit_points = caster.max_hit_points = 30;
    caster.set_position(coord_def(10, 10));
    caster.foe = MHITYOU;
    int translated = 0, fallback = 0;
    for (monster_type type : {MONS_JESSICA, MONS_CHAOS_SPAWN, MONS_ORB_OF_FIRE})
    {
        caster.type = caster.base_monster = type;
        for (const auto &entry : entries)
        {
            CAPTURE(type, entry.first);
            REQUIRE(approved.contains(entry.first));
            actor &target = ends_with(entry.first, "player")
                          ? static_cast<actor &>(you) : static_cast<actor &>(caster);
            for (int seed = 0; seed < 8; ++seed)
            {
                const int damage = seed * 5;
                string expected;
                uint64_t next;
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::EN;
                    const auto pair = query_paired_speech(english.db, nullptr, entry.first, true);
                    expected = original_render(target, pair.english, damage);
                    next = rng::peek_uint64();
                }
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::EN;
                    CHECK(render_miscast_message(target,
                        query_paired_speech(english.db, approved.db, entry.first, true), damage) == expected);
                    CHECK(rng::peek_uint64() == next);
                }
                {
                    rng::subgenerator isolated(seed, 0);
                    Options.language = lang_t::ZH;
                    const auto pair = query_paired_speech(english.db, approved.db, entry.first, true);
                    const string shown = render_miscast_message(target, pair, damage);
                    CHECK(rng::peek_uint64() == next);
                    CHECK(shown.find('@') == string::npos);
                    CHECK(shown.find('[') == string::npos);
                    CHECK(shown.find("的的") == string::npos);
                    if (shown == expected) ++fallback;
                    else
                    {
                        ++translated;
                        CHECK(shown.find('.') == string::npos);
                        CHECK(shown.find('!') == string::npos);
                    }
                    if (!pair.complete || (target.is_monster() && !mons_has_body(caster)))
                        CHECK(shown == expected);
                }
            }
        }
    }
    CHECK(translated > 100);
    CHECK(fallback > 0);
    Options.language = lang_t::ZH;
    caster.type = caster.base_monster = MONS_JESSICA;
    const string new_body = "An unreviewed force hits @The_monster@\n";
    english.put("new miscast", new_body);
    legacy.put("new miscast", "不得使用的旧译文\n");
    const auto missing = query_paired_speech(english.db, approved.db, "new miscast", true);
    CHECK_FALSE(missing.complete);
    CHECK(render_miscast_message(caster, missing, 3)
          == original_render(caster, missing.english, 3));
}

TEST_CASE("Every monster cast TextDB key shares canonical choices and renders complete typed messages", "[localization]")
{
    LocalizedTestDB approved(true);
    init_properties();
    init_monsters();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    std::ifstream input("dat/database/monspell.txt");
    REQUIRE(input.good());
    map<string,string> entries;
    string key, body, line;
    bool active = false;
    auto finish = [&]() {
        if (key.empty()) return;
        body.erase(0, body.find_first_not_of('\n'));
        entries[lowercase_string(key)] = body;
    };
    while (std::getline(input,line))
    {
        trim_string_right(line);
        if (starts_with(line,"#")) continue;
        if (starts_with(line,"%%%%")) { finish(); key.clear(); body.clear(); active=true; }
        else if (active && key.empty()) { key=line; trim_string(key); }
        else if (active) body+=line+"\n";
    }
    finish();
    REQUIRE(entries.size()==262);
    LocalizedTestDB english, legacy;
    for (const auto &entry : entries) english.put(entry.first,entry.second);
    english.put("orc name", "Grum\n\nBlork\n");
    monster caster;
    caster.hit_points=caster.max_hit_points=30;
    caster.set_position(coord_def(10,10));
    caster.foe=MHITYOU;
    caster.god=GOD_BEOGH;
    // Exercise generic templates with the reported casters, including random
    // body anatomy: display must not reroll any of the English substitutions.
    for (monster_type type : {MONS_JESSICA, MONS_ENTROPY_WEAVER,
                              MONS_PROTEAN_PROGENITOR, MONS_CHAOS_SPAWN})
    {
        caster.type=caster.base_monster=type;
        for (const auto &entry : entries)
        {
            CAPTURE(type,entry.first);
            if (entry.second.find("__NONE") != string::npos) continue;
            for (int seed=0;seed<6;++seed)
            {
                string expected;
                uint64_t expected_rng;
                const auto prepare = [&](textdb_speech &pair) {
                    pair.replace("@at@","at","向",true);
                    pair.replace("@target@","you","你",true);
                    pair.replace("@beam@","fire","火焰",true);
                };
                {
                    rng::subgenerator isolated(seed,0);
                    auto pair=query_paired_speech(english.db,nullptr,entry.first,true);
                    prepare(pair);
                    expected=do_mon_str_replacements(pair.english,caster);
                    expected_rng=rng::peek_uint64();
                }
                rng::subgenerator isolated(seed,0);
                auto pair=query_paired_speech(english.db,approved.db,entry.first,true);
                REQUIRE(pair.complete);
                prepare(pair);
                const string original=do_mon_str_replacements(pair.english,caster,-1,&pair);
                CHECK(original==expected);
                CHECK(rng::peek_uint64()==expected_rng);
                CAPTURE(original,pair.translated);
                REQUIRE(pair.complete);
                CHECK(pair.text().find('@')==string::npos);
                CHECK(pair.text().find('[')==string::npos);
                CHECK(pair.text()!=original);
                string en=original,zh=pair.text();
                msg_channel_type en_channel=MSGCH_TALK,zh_channel=MSGCH_TALK;
                strip_channel_prefix(en,en_channel);
                strip_channel_prefix(zh,zh_channel);
                CHECK(en_channel==zh_channel);
                Options.language=lang_t::EN;
                CHECK(pair.text()==original);
                Options.language=lang_t::ZH;
            }
        }
    }
    english.put("new cast","@The_monster@ utters an unreviewed incantation.\n");
    legacy.put("new cast","旧版译文不得复活。\n");
    approved.erase("new cast");
    auto stale=query_paired_speech(english.db,approved.db,"new cast",true);
    CHECK_FALSE(stale.complete);
    const string original=do_mon_str_replacements(stale.english,caster,-1,&stale);
    CHECK(stale.text()==original);
    auto missing=query_paired_speech(english.db,nullptr,"beam catchall cast",true);
    missing.replace("@at@","at","向",true);
    missing.replace("@target@","you","你",false);
    missing.replace("@beam@","fire","火焰",true);
    const string fallback=do_mon_str_replacements(missing.english,caster,-1,&missing);
    CHECK_FALSE(missing.complete);
    CHECK(missing.text()==fallback);
}

TEST_CASE("Orb spider charging shares the canonical random choice and falls back completely", "[localization]")
{
    LocalizedTestDB approved(true);
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    std::ifstream source("dat/database/monspeak.txt");
    REQUIRE(source.good());
    string body, line;
    bool found = false;
    while (std::getline(source, line))
    {
        if (line == "orb spider charge") { found = true; continue; }
        if (!found) continue;
        if (starts_with(line, "%%%%")) break;
        body += line + "\n";
    }
    body.erase(0, body.find_first_not_of('\n'));
    REQUIRE(approved.contains("orb spider charge"));
    LocalizedTestDB english, legacy;
    english.put("orb spider charge", body);
    monster spider;
    spider.type = spider.base_monster = MONS_ORB_SPIDER;
    spider.hit_points = spider.max_hit_points = 10;
    spider.set_position(coord_def(10, 10));
    std::set<string> variants;
    for (int seed = 0; seed < 32; ++seed)
    {
        string selected;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            selected = query_weighted_textdb_entry(english.db, nullptr, "orb spider charge");
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        auto pair = query_paired_speech(english.db, approved.db, "orb spider charge");
        REQUIRE(pair.complete);
        CHECK(pair.english == selected);
        const string original = replace_all(selected, "@The_monster@", "The orb spider");
        const string shown = monster_shout_display(pair, spider, original);
        CHECK(shown.find("球蛛") == 0);
        CHECK(shown.find('@') == string::npos);
        variants.insert(shown);
        auto missing = pair;
        missing.complete = false;
        CHECK(monster_shout_display(missing, spider, original) == original);
        Options.language = lang_t::EN;
        CHECK(monster_shout_display(pair, spider, original) == original);
        Options.language = lang_t::ZH;
        CHECK(rng::peek_uint64() == next);
    }
    CHECK(variants.size() == 3);
    CHECK(variants.count("球蛛开始将丝线编织成一个明亮发光的球体。") == 1);
    english.put("orb spider charge", "A new upstream charge message.\n");
    legacy.put("orb spider charge", "不得复活的旧译文。\n");
    approved.erase("orb spider charge");
    const auto stale = query_paired_speech(english.db, approved.db, "orb spider charge");
    CHECK_FALSE(stale.complete);
    CHECK(stale.text() == "A new upstream charge message.");
}

TEST_CASE("Orb spider cast replaces the raw actor marker and rejects stale legacy speech", "[localization]")
{
    LocalizedTestDB approved(true);
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    const string key = "orb of destruction orb spider cast";
    const string body = "@The_monster@ weaves a glowing orb of energy.\n";
    std::ifstream source("dat/database/monspell.txt");
    REQUIRE(source.good());
    const string raw((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());
    CHECK(raw.find(body) != string::npos);
    LocalizedTestDB english, legacy;
    english.put(key, body);
    legacy.put(key, "@The_monster@不应使用的旧译。\n");
    monster spider;
    spider.type = spider.base_monster = MONS_ORB_SPIDER;
    spider.hit_points = spider.max_hit_points = 10;
    spider.set_position(coord_def(10, 10));
    const string original = "The orb spider weaves a glowing orb of energy.";
    const auto pair = query_paired_speech(english.db, approved.db, key);
    REQUIRE(pair.complete);
    const auto after_selection = rng::peek_uint64();
    CHECK(monster_shout_display(pair, spider, original) == "球蛛编织出一个发光的能量球。");
    auto missing = pair;
    missing.complete = false;
    CHECK(monster_shout_display(missing, spider, original) == original);
    Options.language = lang_t::EN;
    CHECK(monster_shout_display(pair, spider, original) == original);
    Options.language = lang_t::ZH;
    CHECK(rng::peek_uint64() == after_selection);
    english.put(key, "New upstream cast prose.\n");
    approved.erase(key);
    const auto stale = query_paired_speech(english.db, approved.db, key);
    CHECK_FALSE(stale.complete);
    CHECK(stale.text() == "New upstream cast prose.");
}

TEST_CASE("TextDB fallback parents keep English aliases and recursive includes", "[localization]")
{
    LocalizedTestDB english, chinese;
    REQUIRE(english.db);
    REQUIRE(chinese.db);
    english.put("child", "English child\n");
    english.put("nested", "Nested [[child]]\n");
    english.put("parent", "English parent [[nested]]\n");
    english.put("alias", "<child>\n");
    chinese.put("child", "中文子条目\n");
    chinese.put("nested", "中文嵌套 [[child]]\n");
    const string fallback = query_textdb_entry(english.db, chinese.db, "PARENT", true, false);
    CHECK(fallback == query_textdb_entry(english.db, nullptr, "parent", true, false));
    CHECK(fallback == "English parent Nested English child\n");
    CHECK(query_textdb_entry(english.db, chinese.db, "alias", true, false) == "English child\n");
    chinese.put("parent", "中文父条目 [[nested]]\n");
    CHECK(query_textdb_entry(english.db, chinese.db, "parent", true, false)
          == "中文父条目 中文嵌套 中文子条目\n");
    CHECK(query_textdb_entry(english.db, chinese.db, "parent", true, false, true) == fallback);
    CHECK(query_textdb_entry(english.db, chinese.db, "missing", true, false).empty());
}

TEST_CASE("H TextDB reports the selected parent language through aliases", "[localization]")
{
    LocalizedTestDB english, chinese;
    english.put("parent", "English $1\n");
    english.put("alias", "<parent>\n");
    chinese.put("alias", "<parent>\n");
    bool translated = true;
    CHECK(query_textdb_entry(english.db, chinese.db, "alias", true, false,
                             false, &translated) == "English $1\n");
    CHECK_FALSE(translated);
    chinese.put("parent", "中文 $1\n");
    CHECK(query_textdb_entry(english.db, chinese.db, "alias", true, false,
                             false, &translated) == "中文 $1\n");
    CHECK(translated);
    CHECK(query_textdb_entry(english.db, chinese.db, "parent", true, false,
                             true, &translated) == "English $1\n");
    CHECK_FALSE(translated);
    CHECK(query_textdb_entry(english.db, chinese.db, "missing", true, false,
                             false, &translated).empty());
    CHECK_FALSE(translated);
}

TEST_CASE("Weighted encounter fallback bypasses old Chinese titles without an extra random draw", "[localization]")
{
    LocalizedTestDB english, chinese;
    REQUIRE(english.db);
    REQUIRE(chinese.db);
    english.put("title", "w:3\nOriginal title\n\nw:1\nOther title\n");
    chinese.put("title", "w:3\n旧中文称号\n\nw:1\n另一称号\n");
    CHECK(query_weighted_textdb_entry(english.db, chinese.db, "TITLE", "", 0) == "旧中文称号");
    CHECK(query_weighted_textdb_entry(english.db, chinese.db, "TITLE", "", 0, true) == "Original title");
    CHECK(query_weighted_textdb_entry(english.db, chinese.db, "title", " suffix", 3, true) == "Other title");
    CHECK(query_weighted_textdb_entry(english.db, chinese.db, "missing", "", 0, true).empty());
    string original;
    uint64_t next_random;
    {
        rng::subgenerator isolated(1234, 0);
        original = query_weighted_textdb_entry(english.db, nullptr, "title");
        next_random = rng::peek_uint64();
    }
    {
        rng::subgenerator isolated(1234, 0);
        CHECK(query_weighted_textdb_entry(english.db, chinese.db, "title", "", -1, true) == original);
        CHECK(rng::peek_uint64() == next_random);
    }
}

TEST_CASE("X paired TextDB speech shares recursive RNG and rejects missing or stale choices", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    LocalizedTestDB english, chinese;
    REQUIRE(english.db);
    REQUIRE(chinese.db);
    english.put("parent", "@child@ surrounds @item@.\n");
    chinese.put("parent", "@child@围绕着@item@。\n");
    english.put("child", "w:3\n[Smoke|Steam]\n\nw:1\n[Fire|Ice]\n");
    chinese.put("child", "w:3\n[烟|蒸汽]\n\nw:1\n[火|冰]\n");
    for (int seed = 0; seed < 32; ++seed)
    {
        string original;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            original = query_weighted_textdb_entry(english.db, nullptr, "parent");
            original = replace_all(original, "@child@", query_weighted_textdb_entry(english.db, nullptr, "child"));
            original = replace_all(original, "@item@", "your axe");
            original = maybe_pick_random_substring(original);
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        auto pair = query_paired_speech(english.db, chinese.db, "PARENT");
        pair.replace("@item@", "your axe", "你的斧", true);
        pair.pick_substrings();
        CHECK(pair.english == original);
        CHECK(pair.complete);
        CHECK(pair.text().find("围绕着你的斧") != string::npos);
        CHECK(rng::peek_uint64() == next);
    }
    english.put("missing-child", "@untranslated@ surrounds @item@.\n");
    chinese.put("missing-child", "@untranslated@围绕着@item@。\n");
    english.put("untranslated", "a strange mist\n");
    auto missing = query_paired_speech(english.db, chinese.db, "missing-child");
    missing.replace("@item@", "your axe", "你的斧", true);
    CHECK_FALSE(missing.complete);
    CHECK(missing.text() == "a strange mist surrounds your axe.");
    chinese.put("child", "w:4\n过时的单个选项\n");
    auto stale = query_paired_speech(english.db, chinese.db, "parent");
    stale.replace("@item@", "your axe", "你的斧", true);
    stale.pick_substrings();
    CHECK_FALSE(stale.complete);
    CHECK(stale.text() == stale.english);
}

TEST_CASE("Reviewed monster dialogue keeps canonical choices actors channels and Lua fallback", "[localization]")
{
    LocalizedTestDB approved(true);
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    lua_State *ls = clua.state(); lua_stack_cleaner stack(ls);
    lua_getglobal(ls, "you"); const int old_you=luaL_ref(ls,LUA_REGISTRYINDEX);
    lua_getglobal(ls, "db18"); const int old_counter=luaL_ref(ls,LUA_REGISTRYINDEX);
    lua_getglobal(ls, "spells"); const int old_spells=luaL_ref(ls,LUA_REGISTRYINDEX);
    struct Restore { lua_State *ls; int a,b,c; ~Restore() {
        lua_rawgeti(ls,LUA_REGISTRYINDEX,a); lua_setglobal(ls,"you");
        lua_rawgeti(ls,LUA_REGISTRYINDEX,b); lua_setglobal(ls,"db18");
        lua_rawgeti(ls,LUA_REGISTRYINDEX,c); lua_setglobal(ls,"spells");
        luaL_unref(ls,LUA_REGISTRYINDEX,a); luaL_unref(ls,LUA_REGISTRYINDEX,b);
        luaL_unref(ls,LUA_REGISTRYINDEX,c);
    }} restore{ls,old_you,old_counter,old_spells};
    REQUIRE(clua.execstring("db18={calls=0};you={race=function() db18.calls=db18.calls+1;return 'Human' end,"
        "mutation=function() db18.calls=db18.calls+1;return 0 end,"
        "transform=function() db18.calls=db18.calls+1;return 'none' end,"
        "genus=function() db18.calls=db18.calls+1;return 'human' end,"
        "skill=function() db18.calls=db18.calls+1;return 5 end,"
        "god=function() db18.calls=db18.calls+1;return 'Makhleb' end,"
        "can_smell=function() db18.calls=db18.calls+1;return true end};"
        "spells={memorised=function() db18.calls=db18.calls+1;return false end}") == 0);
    const auto calls=[&]() { lua_getglobal(ls,"db18");lua_getfield(ls,-1,"calls");
        const int n=lua_tointeger(ls,-1);lua_pop(ls,2);return n; };
    struct DatabaseScope { DatabaseScope(){databaseSystemInit();}
        ~DatabaseScope(){databaseSystemShutdown();clear_message_store();} } databases;
    const auto entries=read_speech_fixture("dat/database/zh/monspeak.txt");
    CHECK(entries.size()==731);
    monster priest;priest.type=priest.base_monster=MONS_ORC_PRIEST;
    priest.hit_points=priest.max_hit_points=30;priest.foe=MHITYOU;
    priest.god=GOD_BEOGH;priest.set_position(coord_def(10,10));
    int translated=0,fallback=0;
    for(const auto &entry:entries)
    for(int seed=0;seed<64;++seed) {
        CAPTURE(entry.first,seed);string expected;uint64_t next;int expected_calls;
        REQUIRE(clua.execstring("db18.calls=0")==0);
        {
            rng::subgenerator isolated(seed,0);
            expected=do_mon_str_replacements(getSpeakString(entry.first,true),priest,S_SHOUT);
            next=rng::peek_uint64();expected_calls=calls();
        }
        REQUIRE(clua.execstring("db18.calls=0")==0);
        rng::subgenerator isolated(seed,0);
        auto pair=getPairedCastString(entry.first);
        const string original=do_mon_str_replacements(pair.english,priest,S_SHOUT,&pair);
        CHECK(original==expected);CHECK(rng::peek_uint64()==next);CHECK(calls()==expected_calls);
        Options.language=lang_t::EN;CHECK(pair.text()==expected);Options.language=lang_t::ZH;
        if(pair.text()!=expected){++translated;CHECK(pair.text().find('@')==string::npos);
            CHECK(pair.text().find("{{")==string::npos);}
        else ++fallback;

        CHECK(starts_with(pair.text(),"VISUAL:")==starts_with(expected,"VISUAL:"));
        CHECK(starts_with(pair.text(),"SOUND:")==starts_with(expected,"SOUND:"));
    }
    CHECK(translated>0);CHECK(fallback>0);
    for (int evocations : {0,5}) {
        REQUIRE(clua.execstring(make_stringf("you.skill=function() db18.calls=db18.calls+1;return %d end",evocations).c_str())==0);
        int selected_lua=0;
        for (int seed=0;seed<64;++seed) {
            CAPTURE(evocations,seed);
            string expected;uint64_t next;int expected_calls;
            REQUIRE(clua.execstring("db18.calls=0")==0);
            {rng::subgenerator isolated(seed,0);
                expected=do_mon_str_replacements(getSpeakString("_ijyb_common_",true),priest,S_SHOUT);
                next=rng::peek_uint64();expected_calls=calls();}
            REQUIRE(clua.execstring("db18.calls=0")==0);
            rng::subgenerator isolated(seed,0);
            auto pair=getPairedCastString("_ijyb_common_");
            CHECK(do_mon_str_replacements(pair.english,priest,S_SHOUT,&pair)==expected);
            CHECK(rng::peek_uint64()==next);CHECK(calls()==expected_calls);
            if(expected_calls) {
                ++selected_lua;CHECK(pair.text()!=expected);
                CHECK(pair.text().find(evocations>=2?"你的魔杖":"实验对象")!=string::npos);
                CHECK(expected.find(evocations>=2?"Me learn from your wands next!":"You look like good test subject!")!=string::npos);
            }
        }
        CHECK(selected_lua>0);
    }
    for (int profile : {0,1,2}) {
        REQUIRE(clua.execstring(make_stringf(
            "you.race=function() db18.calls=db18.calls+1;return '%s' end;"
            "you.skill=function() db18.calls=db18.calls+1;return %d end;"
            "spells.memorised=function() db18.calls=db18.calls+1;return %s end",
            profile==2?"Felid":profile?"Armataur":"Human",profile?12:0,profile?"true":"false").c_str())==0);
        REQUIRE(clua.execstring(make_stringf(
            "you.god=function() db18.calls=db18.calls+1;return '%s' end;"
            "you.can_smell=function() db18.calls=db18.calls+1;return %s end",
            profile?"Makhleb":"Zin",profile?"true":"false").c_str())==0);
        int selected_lua=0;
        for (const char *key : {"sprozz","sprozz triumphant","_terence_rare_",
            "_saint roka rival_","_hostile_orc_beogh_believer_speech_common_","xak'krixis","nekomata","_asterion_rare_","friendly hound","friendly shoals hound"})
        for (int seed=0;seed<64;++seed) {
            CAPTURE(profile,key,seed);string expected;uint64_t next;int expected_calls;
            REQUIRE(clua.execstring("db18.calls=0")==0);
            {rng::subgenerator isolated(seed,0);
                expected=do_mon_str_replacements(getSpeakString(key,true),priest,S_SHOUT);
                next=rng::peek_uint64();expected_calls=calls();}
            REQUIRE(clua.execstring("db18.calls=0")==0);
            rng::subgenerator isolated(seed,0);auto pair=getPairedCastString(key);
            CHECK(do_mon_str_replacements(pair.english,priest,S_SHOUT,&pair)==expected);
            CHECK(rng::peek_uint64()==next);CHECK(calls()==expected_calls);
            if(expected_calls) {++selected_lua;
                if (string(key) == "sprozz" || string(key) == "nekomata" || string(key) == "_terence_rare_"
                    || string(key) == "sprozz triumphant" || string(key) == "xak'krixis"
                    || string(key) == "_asterion_rare_" || string(key) == "friendly hound"
                    || string(key) == "friendly shoals hound" || string(key) == "_saint roka rival_"
                    || string(key) == "_hostile_orc_beogh_believer_speech_common_") CHECK(pair.text()!=expected);
                else CHECK(pair.text()==expected);}
        }
        CHECK(selected_lua>0);
    }
    struct FragmentProfile { const char *race, *form, *expected; int beak; };
    for (const auto &profile : vector<FragmentProfile>{
        {"Mummy","none","绒布包裹的",0},{"Human","none","珠喙的",1},
        {"Human","death","保存完好的",0},{"Human","statue","大理石脚趾的",0},
        {"Human","dragon","蛋白石鳞片的",0},{"Felid","none","软垫脚掌的",0},
        {"Octopode","none","柔软触手的",0},{"Formicid","none","巨颚的",0},
        {"Poltergeist","none","精神抖擞的",0},{"Revenant","none","象牙骨骼的",0},
        {"Human","none","红润嘴唇的",0}}) {
        REQUIRE(clua.execstring(make_stringf(
            "you.race=function() db18.calls=db18.calls+1;return '%s' end;"
            "you.transform=function() db18.calls=db18.calls+1;return '%s' end;"
            "you.mutation=function() db18.calls=db18.calls+1;return %d end",
            profile.race,profile.form,profile.beak).c_str())==0);
        int rendered=0;
        for(int seed=0;seed<64;++seed) {
            string expected;uint64_t next;
            {rng::subgenerator isolated(seed,0);
             expected=getSpeakString("_suck_up_adj2_",true);next=rng::peek_uint64();}
            rng::subgenerator isolated(seed,0);auto pair=getPairedCastString("_suck_up_adj2_");
            CHECK(pair.english==expected);CHECK(rng::peek_uint64()==next);
            if(expected!="awesome" && expected!="greatest" && expected!="god-like") {
                ++rendered;CHECK(pair.complete);CHECK(pair.text()==profile.expected);
            }
        }
        CHECK(rendered>0);
        const string arms=string(profile.race)=="Felid"?"爪子":string(profile.race)=="Octopode"?"触手":string(profile.race)=="Formicid"?"肢体":"手臂";
        auto part=getPairedCastString("_player_arms_");CHECK(part.complete);CHECK(part.text()==arms);
        auto parent=getPairedCastString("azrael"); // Whole-parent English remains available.
        Options.language=lang_t::EN;CHECK(parent.text()==parent.english);Options.language=lang_t::ZH;
    }
    for(const auto &name : vector<pair<string,string>>{
        {"Ant","蚂蚁"},
        {"Armataur","犰狳人"},
        {"Barachi","蛙人"},
        {"Black Draconian","黑龙人"},
        {"Cat","猫"},
        {"Centaur","半人马"},
        {"Coglin","机甲哥布林"},
        {"Deep Dwarf","地底矮人"},
        {"Deep Elf","地底精灵"},
        {"Demigod","半神"},
        {"Demonspawn","魔人"},
        {"Djinni","灯灵"},
        {"Draconian","龙人"},
        {"Dwarf","矮人"},
        {"Elf","精灵"},
        {"Felid","猫族"},
        {"Formicid","蚁族"},
        {"Frog","青蛙"},
        {"Gargoyle","石像鬼"},
        {"Geist","灵体"},
        {"Ghoul","食尸鬼"},
        {"Gnoll","豺狼人"},
        {"Goblin","地精"},
        {"Green Draconian","绿龙人"},
        {"Grey Draconian","灰龙人"},
        {"Halfling","半身人"},
        {"High Elf","高等精灵"},
        {"Hill Orc","山地兽人"},
        {"Human","人类"},
        {"Kobold","狗头人"},
        {"Lava Orc","熔岩兽人"},
        {"Mayflytaur","蜉蝣人"},
        {"Merfolk","人鱼"},
        {"Meteoran","星陨人"},
        {"Minotaur","牛头人"},
        {"Mottled Draconian","斑纹龙人"},
        {"Mountain Dwarf","山地矮人"},
        {"Mummy","木乃伊"},
        {"Naga","娜迦"},
        {"Octopode","章鱼人"},
        {"Octopus","章鱼"},
        {"Oni","鬼人"},
        {"Pale Draconian","苍白龙人"},
        {"Poltergeist","骚灵"},
        {"Purple Draconian","紫龙人"},
        {"Red Draconian","红龙人"},
        {"Revenant","还魂者"},
        {"Sludge Elf","淤泥精灵"},
        {"Spriggan","斯普利根"},
        {"Tengu","天狗"},
        {"Troll","巨魔"},
        {"Vampire","吸血鬼"},
        {"Vine Stalker","藤蔓人"},
        {"White Draconian","白龙人"},
        {"Yak","牦牛"},
        {"Yellow Draconian","黄龙人"}}) {
        REQUIRE(clua.execstring(make_stringf(
            "you.race=function() db18.calls=db18.calls+1;return '%s' end;"
            "you.genus=function() db18.calls=db18.calls+1;return '%s' end",
            name.first.c_str(),lowercase_string(name.first).c_str()).c_str())==0);
        int noun_choices=0,race_choices=0;
        for(int seed=0;seed<64;++seed) {
            for(const char *key : {"_suck_up_noun_","_saint roka rival_"}) {
                string expected;uint64_t next;
                {rng::subgenerator isolated(seed,0);expected=getSpeakString(key,true);next=rng::peek_uint64();}
                rng::subgenerator isolated(seed,0);auto pair=getPairedCastString(key);
                CHECK(pair.english==expected);CHECK(rng::peek_uint64()==next);
                if(string(key)=="_suck_up_noun_" && starts_with(expected,"gentle")) {
                    ++noun_choices;CHECK(pair.complete);CHECK(pair.text()==name.second+"绅士");
                }
                if(string(key)=="_saint roka rival_" && expected.find("deceiver is a mere")!=string::npos) {
                    ++race_choices;CHECK(pair.complete);CHECK(pair.translated.find(name.second)!=string::npos);
                    do_mon_str_replacements(pair.english,priest,S_SHOUT,&pair);
                    CHECK(pair.text()!=pair.english);CHECK(pair.text().find('@')==string::npos);
                }
            }
        }
        CHECK(noun_choices>0);CHECK(race_choices>0);
    }
    REQUIRE(clua.execstring("you.race=function() return 'Future Species' end;you.genus=function() return 'future genus' end")==0);
    int unmapped=0;
    for(int seed=0;seed<64;++seed) {
        rng::subgenerator isolated(seed,0);auto pair=getPairedCastString("_suck_up_noun_");
        if(starts_with(pair.english,"gentle")) {++unmapped;CHECK_FALSE(pair.complete);CHECK(pair.text()==pair.english);}
    }
    CHECK(unmapped>0);
    auto target=getPairedCastString("_high_priest_medium_");
    const string original=do_mon_str_replacements(target.english,priest,S_SHOUT,&target);
    CHECK(target.text()!=original);
    LocalizedTestDB english,legacy;
    english.put("_high_priest_common_","Changed upstream prayer.\n");
    legacy.put("_high_priest_common_","不能复活的旧译。\n");
    approved.erase("_high_priest_common_");
    const auto stale=query_paired_speech(english.db,approved.db,"_high_priest_common_",true);
    CHECK_FALSE(stale.complete);CHECK(stale.text()==stale.english);
}

TEST_CASE("Monster dialogue prefix fallback preserves English selection and random state", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::EN);
    unwind_var<const char *> lang_name(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<int> width(crawl_view.msgsz.x,160);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    unwind_var<int> hp(you.hp,30);
    unwind_var<decltype(crawl_state.prev_cmd)> command(crawl_state.prev_cmd,CMD_LOOK_AROUND);
    struct DatabaseScope {DatabaseScope(){databaseSystemInit();}
        ~DatabaseScope(){databaseSystemShutdown();clear_message_store();}} databases;
    for(monster_type type:{MONS_ORC_PRIEST,MONS_ORC_WIZARD,MONS_CENTAUR,MONS_HOUND,
        MONS_AIZUL,MONS_AGNES,MONS_ASTERION,MONS_BORIS,MONS_CASSANDRA,MONS_CRAZY_YIUF,
        MONS_DONALD,MONS_DISSOLUTION,MONS_DOWAN,MONS_DUVESSA,MONS_EDMUND,
        MONS_ERICA,MONS_EROLCHA,MONS_EUSTACHIO,MONS_FRANCES,MONS_FREDERICK,
        MONS_FANNAR,MONS_GASTRONOK,MONS_GRINDER,MONS_GRUM,MONS_GRUNN,
        MONS_HAROLD,MONS_IGNACIO,MONS_IJYB,MONS_ILSUIW,MONS_JESSICA,
        MONS_JEREMIAH,MONS_JORGRUN,MONS_JORY,MONS_JOSEPH,MONS_JOSEPHINA,
        MONS_JOSEPHINE,MONS_KHUFU,MONS_KIRKE,MONS_LODUL,MONS_LOUISE,
        MONS_MAGGIE,MONS_MARA,MONS_MARGERY,MONS_MAURICE,MONS_MENKAURE,
        MONS_MENNAS,MONS_MURRAY,MONS_NATASHA,MONS_ROBIN,MONS_NERGALLE,
        MONS_NESSOS,MONS_NIKOLA,MONS_PIKEL,MONS_POLYPHEMUS,MONS_PRINCE_RIBBIT,
        MONS_PARGI,MONS_PARGHIT,MONS_ROXANNE,MONS_RUPERT,MONS_SAINT_ROKA,
        MONS_SIGMUND,MONS_SNORG,MONS_SOJOBO,MONS_SONJA,MONS_SPROZZ,
        MONS_TERENCE,MONS_URUG,MONS_VASHNIA,MONS_WIGLAF,MONS_XAKKRIXIS,
        MONS_XTAHUA,MONS_ZENATA})
    for(int seed=0;seed<64;++seed) {
        CAPTURE(type,seed);
        monster mon;mon.type=mon.base_monster=type;mon.hit_points=mon.max_hit_points=30;
        mon.foe=MHITYOU;mon.set_position(coord_def(10,10));
        mon.god=GOD_BEOGH;mon.behaviour=seed%2?BEH_FLEE:BEH_SEEK;
        bool expected;uint64_t next;
        {rng::subgenerator isolated(seed,0);Options.language=lang_t::EN;
            expected=mons_speaks(&mon);next=rng::peek_uint64();}
        rng::subgenerator isolated(seed,0);Options.language=lang_t::ZH;
        CHECK(mons_speaks(&mon)==expected);CHECK(rng::peek_uint64()==next);
        clear_message_store();
    }
}

TEST_CASE("Monster dialogue address markers and target identity preserve canonical English", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    monster mon;mon.type=mon.base_monster=MONS_ORC_PRIEST;
    mon.hit_points=mon.max_hit_points=30;mon.foe=MHITYOU;
    mon.set_position(coord_def(10,10));
    const vector<pair<string,string>> lines={
        {"@The_monster@ asks @foe,@ \"Who are you?\"","@The_monster@问@foe,@：“你是谁？”"},
        {"@The_monster@ cannot hear @objective@.","@The_monster@听不见@objective@。"},
        {"@The_monster@ says, \"One @foe_genus@ for @foe_name@.\"","@The_monster@说：“一份@foe_genus@，给@foe_name@。”"},
        {"@The_monster@ says, \"@Foe_genus@ for dinner?\"","@The_monster@说：“晚饭吃@Foe_genus@？”"},
        {"@The_monster@ is cut. @Subjective@ ignores it.","@The_monster@被割伤了。@Subjective@毫不在意。"}};
    for(species_type race:{SP_HUMAN,SP_DEEP_ELF,SP_MOUNTAIN_DWARF,SP_FELID})
    for(const auto &line:lines){
        you.species=race;
        string expected;uint64_t next;
        {rng::subgenerator isolated(19,0);expected=do_mon_str_replacements(line.first,mon,S_SHOUT);next=rng::peek_uint64();}
        rng::subgenerator isolated(19,0);textdb_speech pair(line.first,line.second,true);
        CHECK(do_mon_str_replacements(pair.english,mon,S_SHOUT,&pair)==expected);
        CHECK(rng::peek_uint64()==next);CHECK(pair.complete);
        CHECK(pair.text()!=expected);CHECK(pair.text().find('@')==string::npos);
        Options.language=lang_t::EN;CHECK(pair.text()==expected);Options.language=lang_t::ZH;
    }
}

TEST_CASE("Reviewed divine speech uses canonical weighted choices and preserves opaque Lua fallback", "[localization]")
{
    LocalizedTestDB approved(true);
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    lua_State *ls = clua.state();
    lua_stack_cleaner stack(ls);
    struct Globals {
        lua_State *ls; int you_ref, counter_ref;
        Globals(lua_State *state) : ls(state) {
            lua_getglobal(ls, "you"); you_ref = luaL_ref(ls, LUA_REGISTRYINDEX);
            lua_getglobal(ls, "db17"); counter_ref = luaL_ref(ls, LUA_REGISTRYINDEX);
        }
        ~Globals() {
            lua_rawgeti(ls, LUA_REGISTRYINDEX, you_ref); lua_setglobal(ls, "you");
            lua_rawgeti(ls, LUA_REGISTRYINDEX, counter_ref); lua_setglobal(ls, "db17");
            luaL_unref(ls, LUA_REGISTRYINDEX, you_ref);
            luaL_unref(ls, LUA_REGISTRYINDEX, counter_ref);
        }
    } globals(ls);
    REQUIRE(clua.execstring("db17={calls=0,rank=5}; you={piety_rank=function() "
        "db17.calls=db17.calls+1; return db17.rank end}") == 0);
    const auto calls = [&]() {
        lua_getglobal(ls, "db17"); lua_getfield(ls, -1, "calls");
        const int n = lua_tointeger(ls, -1); lua_pop(ls, 2); return n;
    };
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    const auto entries = read_speech_fixture("dat/database/godspeak.txt");
    CHECK(entries.size() == 193);
    map<string,int> translated;
    for (const auto &entry : entries)
    for (int seed = 0; seed < 64; ++seed) {
        CAPTURE(entry.first, seed);
        REQUIRE(clua.execstring(make_stringf("db17.calls=0;db17.rank=%d",seed%6).c_str()) == 0);
        string expected; uint64_t next; int expected_calls;
        {
            rng::subgenerator isolated(seed, 0);
            expected = getSpeakString(entry.first, true);
            next = rng::peek_uint64(); expected_calls = calls();
        }
        REQUIRE(clua.execstring("db17.calls=0") == 0);
        {
            rng::subgenerator isolated(seed, 0);
            auto pair = getPairedSpeakString(entry.first);
            CHECK(pair.english == expected); CHECK(rng::peek_uint64() == next);
            CHECK(calls() == expected_calls);
            Options.language = lang_t::EN; CHECK(pair.text() == expected);
            Options.language = lang_t::ZH;
            if (pair.text() != expected) {
                ++translated[entry.first];
                CHECK(pair.text().find('@') == string::npos);
                CHECK(pair.text().find("{{") == string::npos);
            }
            else CHECK(pair.text() == pair.english);
        }
        REQUIRE(clua.execstring("db17.calls=0") == 0);
        {
            rng::subgenerator isolated(seed, 0);
            const string shown = getSpeakString(entry.first);
            CHECK(rng::peek_uint64() == next); CHECK(calls() == expected_calls);
            if (expected_calls) CHECK(shown != expected);
        }
    }
    for (const char *key : {"beogh welcome", "dithmenos welcome", "kikubaaqudgha welcome",
        "zin penance", "makhleb executioner chatter", "ignis elemental wrath", "zot death"})
        CHECK(translated[key] > 0);
    for(int rank=0;rank<=6;++rank) {
        REQUIRE(clua.execstring(make_stringf("db17.rank=%d",rank).c_str())==0);
        const string wanted=rank>4?"泰迪熊":rank>2?"玩具":"玩物";
        auto pair=getPairedSpeakString("_xom_plaything_");
        CHECK(pair.complete);CHECK(pair.text()==wanted);
    }
    item_def blade;blade.base_type=OBJ_WEAPONS;blade.sub_type=WPN_SHORT_SWORD;
    blade.quantity=1;blade.flags|=ISFLAG_IDENTIFIED;
    for(god_type god:{GOD_NO_GOD,GOD_ZIN}) {
        unwind_var<god_type> religion(you.religion,god);
        REQUIRE(clua.execstring(make_stringf("you.god=function() db17.calls=db17.calls+1;return '%s' end",god==GOD_NO_GOD?"No God":"Zin").c_str())==0);
        int lua_choices=0;
        for(int seed=0;seed<128;++seed) {
            string expected;uint64_t next;int expected_calls;msg_channel_type before,after;
            REQUIRE(clua.execstring("db17.calls=0")==0);
            for(const char *noise : {"_speaking_low_tension_","_speaking_high_tension_"}) {
            REQUIRE(clua.execstring("db17.calls=0")==0);
            {rng::subgenerator isolated(seed,0);
             expected=original_item_noise(blade,getSpeakString(noise,true),before);
             next=rng::peek_uint64();expected_calls=calls();}
            REQUIRE(clua.execstring("db17.calls=0")==0);
            rng::subgenerator isolated(seed,0);
            auto pair=getPairedCastString(noise);
            const string shown=render_item_noise(blade,pair,after);
            CHECK(rng::peek_uint64()==next);CHECK(calls()==expected_calls);CHECK(after==before);
            if(expected_calls) {++lua_choices;CHECK(shown!=expected);
                if(string(noise)=="_speaking_low_tension_")
                    CHECK(shown.find(god==GOD_NO_GOD?"我的刀刃":"辛")!=string::npos);
                else if(god!=GOD_NO_GOD) CHECK(shown.find("辛")!=string::npos);
                CHECK(shown.find('@')==string::npos);}
            Options.language=lang_t::EN;CHECK(pair.text()==pair.english);Options.language=lang_t::ZH;
            }
        }
        CHECK(lua_choices>0);
    }
    CHECK(getSpeakString("Beogh welcome") == "比欧弗说：向世界展示何为兽人！");
    CHECK(getSpeakString("Dithmenos welcome") == "迪斯姆诺说：行于黑暗之地。");
    LocalizedTestDB english, legacy;
    english.put("beogh welcome", "Beogh has changed this speech upstream.\n");
    legacy.put("beogh welcome", "绝不能恢复的过时旧译。\n");
    approved.erase("beogh welcome");
    const auto stale = query_paired_speech(english.db, approved.db, "beogh welcome", true);
    CHECK_FALSE(stale.complete); CHECK(stale.text() == stale.english);
    english.put("zin penance", "Unknown prose @_important_subject_@\n");
    legacy.put("zin penance", "过时的父句。\n");
    const auto missing = query_paired_speech(english.db, approved.db, "zin penance", true);
    CHECK_FALSE(missing.complete); CHECK(missing.text() == missing.english);
}

TEST_CASE("Dream sheep fleeing speech pairs the same actor and preserves visual channel markers", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    struct DatabaseScope {
        DatabaseScope() { databaseSystemInit(); }
        ~DatabaseScope() { databaseSystemShutdown(); clear_message_store(); }
    } databases;
    monster sheep; sheep.type = sheep.base_monster = MONS_DREAM_SHEEP;
    sheep.hit_points = sheep.max_hit_points = 10;
    sheep.set_position(coord_def(10, 10)); sheep.foe = MHITYOU;
    set<string> choices;
    for (int seed = 0; seed < 256; ++seed) {
        string expected; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            expected = do_mon_str_replacements(getSpeakString("dream sheep flee", true), sheep, S_SHOUT);
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        auto pair = getPairedCastString("dream sheep flee");
        const string original = do_mon_str_replacements(pair.english, sheep, S_SHOUT, &pair);
        CHECK(original == expected); CHECK(rng::peek_uint64() == next);
        REQUIRE(pair.complete); CHECK(pair.text() != expected);
        CHECK(starts_with(pair.text(), "VISUAL:") == starts_with(expected, "VISUAL:"));
        CHECK(pair.text().find("梦羊") != string::npos);
        CHECK(pair.text().find('@') == string::npos); choices.insert(pair.text());
        Options.language = lang_t::EN; CHECK(pair.text() == expected);
        Options.language = lang_t::ZH;
    }
    CHECK(choices.size() == 5);
}

TEST_CASE("X live source speech bodies match the managed catalogue and stale roots stay English", "[localization]")
{
    LocalizedTestDB approved(true);
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    std::ifstream source("dat/database/godspeak.txt");
    REQUIRE(source.good());
    LocalizedTestDB english, legacy;
    REQUIRE(english.db);
    REQUIRE(legacy.db);
    string key, body, line;
    bool active = false;
    int checked = 0;
    const auto store = [&]()
    {
        if (key.empty())
            return;
        body.erase(0, body.find_first_not_of('\n'));
        english.put(lowercase_string(key), body);
        if (starts_with(lowercase_string(key), "xom "))
        {
            INFO(key);
            const bool translated = approved.contains(lowercase_string(key));
            const string trimmed = trimmed_string(body);
            const bool alias = trimmed.front() == '@' && trimmed.back() == '@'
                && std::count(trimmed.begin(), trimmed.end(), '@') == 2;
            CHECK((translated || alias));
            ++checked;
        }
    };
    while (std::getline(source, line))
    {
        if (!line.empty() && line[0] == '#')
            continue;
        if (starts_with(line, "%%%%"))
        {
            store();
            key.clear(); body.clear(); active = true;
        }
        else if (active && key.empty())
        {
            if (!trimmed_string(line).empty())
                key = trimmed_string(line);
        }
        else if (active)
        {
            trim_string_right(line);
            body += line + "\n";
        }
    }
    store();
    CHECK(checked == 120);
    for (int seed = 0; seed < 16; ++seed)
    {
        string original;
        uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            original = query_weighted_textdb_entry(english.db, nullptr, "xom feature lava");
            next = rng::peek_uint64();
        }
        rng::subgenerator isolated(seed, 0);
        auto pair = query_paired_speech(english.db, approved.db, "xom feature lava");
        CHECK(pair.complete);
        CHECK(pair.english == original);
        pair.replace("@The_feature@", "The lava", "熔岩", true);
        CHECK((pair.text() == "熔岩喷出了火星！" || pair.text() == "熔岩短暂地凝固了。"));
        CHECK(rng::peek_uint64() == next);
    }
    english.put("xom feature lava", "The lava now does something new.\n");
    legacy.put("xom feature lava", "不应复活的旧译。\n");
    approved.erase("xom feature lava");
    const auto stale = query_paired_speech(english.db, approved.db, "xom feature lava");
    CHECK_FALSE(stale.complete);
    CHECK(stale.text() == "The lava now does something new.");
}
#endif

TEST_CASE("Dynamic UI falls back as a group when a template or argument is unavailable", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    const char *context = "phase3.spl-cast";
    const char *english = "Your spells (%s)";
    const ui_translation_group complete({{context, english}, {"spell.action", "describe"}});
    REQUIRE(complete.available());
    CHECK(string(complete.get("spell.action", "describe")) == "查看说明");

    const ui_translation_group missing_template({
        {context, "Changed spells (%s)"}, {"spell.action", "describe"}
    });
    CHECK_FALSE(missing_template.available());
    CHECK(make_stringf(missing_template.get(context, "Changed spells (%s)"),
                      missing_template.get("spell.action", "describe")) == "Changed spells (describe)");

    const ui_translation_group missing_argument({{context, english}, {"spell.action", "new action"}});
    CHECK_FALSE(missing_argument.available());
    CHECK(make_stringf(missing_argument.get(context, english),
                      missing_argument.get("spell.action", "new action")) == "Your spells (new action)");

    ui_translation_group possessive({
        {"skill.enhanced", "Skills enhanced by %s are in <green>green</green>."}
    });
    possessive.require("phase3.skill-menu", "Ashenzari's power");
    CHECK_FALSE(possessive.available());
    CHECK(string(possessive.get("skill.enhanced", "Skills enhanced by %s are in <green>green</green>."))
          == "Skills enhanced by %s are in <green>green</green>.");

    Options.language = lang_t::EN;
    const ui_translation_group english_mode({{context, english}, {"spell.action", "describe"}});
    CHECK_FALSE(english_mode.available());
    CHECK(make_stringf(english_mode.get(context, english),
                      english_mode.get("spell.action", "describe")) == "Your spells (describe)");
    for (const auto &entry : zh_ui_catalogue)
        CHECK(string(ui_translate(entry.context, entry.english)) == entry.english);
}

#ifdef USE_TILE_LOCAL
#include "tilefont.h"
#include "tilereg-text.h"

namespace
{
// TextRegion's cell buffer is testable without a window or OpenGL context.
class GridTestFont : public FontWrapper
{
public:
    bool load_font(const char *, unsigned int) override { return true; }
    bool configure_font() override { return true; }
    bool resize(unsigned int) override { return true; }
    void render_textblock(unsigned int, unsigned int, char32_t *, uint8_t *,
                          unsigned int, unsigned int, bool) override {}
    void render_tooltip(unsigned int, unsigned int, const formatted_string &,
                        const coord_def &, const coord_def &) override {}
    void render_string(unsigned int, unsigned int, const formatted_string &) override {}
    void render_hover_string(unsigned int, unsigned int, const formatted_string &) override {}
    void store(FontBuffer &, float &, float &, const string &, const VColour &) override {}
    void store(FontBuffer &, float &, float &, const string &, const VColour &,
               const VColour &) override {}
    void store(FontBuffer &, float &, float &, const formatted_string &) override {}
    void store(FontBuffer &, float &, float &, char32_t, const VColour &) override {}
    void store(FontBuffer &, float &, float &, char32_t, const VColour &,
               const VColour &) override {}
    unsigned int char_width(bool) const override { return 1; }
    unsigned int char_height(bool) const override { return 1; }
    unsigned int max_width(int n, bool) const override { return n; }
    unsigned int max_height(int n, bool) const override { return n; }
    unsigned int string_width(const char *, bool) override { return 1; }
    unsigned int string_width(const formatted_string &, bool) override { return 1; }
    unsigned int string_height(const char *, bool) const override { return 1; }
    unsigned int string_height(const formatted_string &, bool) const override { return 1; }
    formatted_string split(const formatted_string &text, unsigned int,
                           unsigned int) override { return text; }
    const GenericTexture *font_tex() const override { return nullptr; }
};

class GridTestRegion : public TextRegion
{
public:
    explicit GridTestRegion(FontWrapper *font) : TextRegion(font) {}
    int handle_mouse(wm_mouse_event &) override { return 0; }
};
}

TEST_CASE("Tiles text cells preserve CJK columns and clear partial glyphs", "[localization]")
{
    GridTestFont font;
    GridTestRegion region(&font);
    region.resize(8, 2);
    unwind_var<int> x(TextRegion::print_x, 0);
    unwind_var<int> y(TextRegion::print_y, 0);
    unwind_var<int> cursor(TextRegion::cursor_flag, 0);
    unwind_var<int> colour(TextRegion::text_col, LIGHTGREY);

    SECTION("mixed text and cursor use display columns")
    {
        region.addstr("A中文B");
        CHECK(region.cbuf[0] == U'A');
        CHECK(region.cbuf[1] == U'中');
        CHECK(region.cbuf[2] == 0);
        CHECK(region.cbuf[3] == U'文');
        CHECK(region.cbuf[4] == 0);
        CHECK(region.cbuf[5] == U'B');
        CHECK(TextRegion::wherex() == 7);
    }
    SECTION("overwriting continuation clears the wide leading cell")
    {
        region.addstr("中文");
        TextRegion::cgotoxy(2, 1);
        region.addstr("x");
        CHECK(region.cbuf[0] == U' ');
        CHECK(region.cbuf[1] == U'x');
        CHECK(region.cbuf[2] == U'文');
    }
    SECTION("overwriting leading cell clears the old continuation")
    {
        region.addstr("中文");
        TextRegion::cgotoxy(1, 1);
        region.addstr("a");
        CHECK(region.cbuf[0] == U'a');
        CHECK(region.cbuf[1] == U' ');
        CHECK(region.cbuf[2] == U'文');
    }
    SECTION("wide glyph never straddles the right edge")
    {
        TextRegion::cgotoxy(8, 1);
        region.addstr("中");
        CHECK(region.cbuf[7] == U' ');
        CHECK(region.cbuf[8] == U' ');
    }
    SECTION("clear from continuation removes the entire wide glyph")
    {
        region.addstr("中文");
        TextRegion::cgotoxy(2, 1);
        region.clear_to_end_of_line();
        for (int i = 0; i < 8; ++i)
            CHECK(region.cbuf[i] == U' ');
    }
    SECTION("newlines retain CJK on the next row")
    {
        region.addstr("中\n文");
        CHECK(region.cbuf[0] == U'中');
        CHECK(region.cbuf[8] == U'文');
        CHECK(TextRegion::wherex() == 3);
        CHECK(TextRegion::wherey() == 2);
    }
    SECTION("long strings do not drop every 1024th character")
    {
        region.resize(1030, 1);
        region.addstr((string(1023, 'a') + "中文B").c_str());
        CHECK(region.cbuf[1023] == U'中');
        CHECK(region.cbuf[1025] == U'文');
        CHECK(region.cbuf[1027] == U'B');
        CHECK(TextRegion::wherex() == 1029);
    }
}
#endif

#ifdef USE_TILE_LOCAL
TEST_CASE("Each Tiles region accepts its own optional fallback font", "[localization]")
{
    unwind_var<string> crt(Options.tile_font_crt_fallback_file);
    unwind_var<string> msg(Options.tile_font_msg_fallback_file);
    unwind_var<string> stat(Options.tile_font_stat_fallback_file);
    unwind_var<string> tip(Options.tile_font_tip_fallback_file);
    unwind_var<string> lbl(Options.tile_font_lbl_fallback_file);
    Options.read_option_line("tile_font_crt_fallback_file = crt.ttf");
    CHECK(Options.tile_font_crt_fallback_file == "crt.ttf");
    Options.read_option_line("tile_font_msg_fallback_file = msg.ttf");
    CHECK(Options.tile_font_msg_fallback_file == "msg.ttf");
    Options.read_option_line("tile_font_stat_fallback_file = stat.ttf");
    CHECK(Options.tile_font_stat_fallback_file == "stat.ttf");
    Options.read_option_line("tile_font_tip_fallback_file = tip.ttf");
    CHECK(Options.tile_font_tip_fallback_file == "tip.ttf");
    Options.read_option_line("tile_font_lbl_fallback_file = lbl.ttf");
    CHECK(Options.tile_font_lbl_fallback_file == "lbl.ttf");
    Options.read_option_line("tile_font_msg_fallback_file =");
    CHECK(Options.tile_font_msg_fallback_file.empty());
}
#endif


TEST_CASE("Branch display names preserve parser and protocol identities", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::EN;
    const string protocol = branch_data_json();
    const auto random_state = rng::peek_uint64();
    int count = 0;
    for (branch_iterator it; it; ++it)
    {
        const string canonical = it->shortname;
        const string long_name = it->longname;
        CHECK(string(branch_display_name(it->id)) == canonical);
        CHECK(string(branch_display_name(it->id, true)) == long_name);
        Options.language = lang_t::ZH;
        CHECK(string(branch_display_name(it->id)) != canonical);
        CHECK(string(branch_display_name(it->id, true)) != long_name);
        CHECK(branch_by_shortname(canonical) == it->id);
        CHECK(branch_by_abbrevname(it->abbrevname) == it->id);
        CHECK(string(it->shortname) == canonical);
        CHECK(string(it->longname) == long_name);
        Options.language = lang_t::EN;
        ++count;
    }
    CHECK(count == NUM_BRANCHES);
    CHECK(branch_noise_desc(BRANCH_SWAMP) == "This branch is noisy: sounds don't travel as far here.");
    const string rune_text = branch_rune_desc(BRANCH_SNAKE, false);
    CHECK(rune_text == "This branch contains the serpentine rune of Zot.");
    Options.language = lang_t::ZH;
    CHECK(branch_noise_desc(BRANCH_SWAMP) == "此分支十分嘈杂，声音在这里传得较近。");
    CHECK(branch_rune_desc(BRANCH_SNAKE, false) != rune_text);
    CHECK(branch_data_json() == protocol);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("B-source beam templates require complete canonical components", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    Options.language = lang_t::ZH;
    const auto random_state = rng::peek_uint64();
    const string original = "The fireball reflects off an invisible shield around you!";
    string projectile;
    REQUIRE(try_ui_projectile_name("fireball", projectile));
    CHECK(ui_render("b.beam", "The %s reflects off an invisible shield around %s!", original,
        {{projectile, true}, ui_actor_arg(you)}) == projectile + "被你周围的隐形护盾反射了！");
    CHECK(ui_render("b.beam", "The %s reflects off an invisible shield around %s!", original,
        {{"new projectile", false}, ui_actor_arg(you)}) == original);
    CHECK(ui_render("b.explosion", "You hear %s!", "You hear an unknown sound!",
        {ui_text_arg("b.explosion.sound", "unknown sound")}) == "You hear an unknown sound!");
    CHECK(ui_render("b.explosion", "You hear %s!", "You hear an explosion!",
        {ui_text_arg("b.explosion.sound", "an explosion")}) == "你听到一声爆炸！");
    Options.language = lang_t::EN;
    CHECK(ui_render("b.beam", "The %s reflects off an invisible shield around %s!", original,
        {{projectile, true}, ui_actor_arg(you)}) == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("G wall jump validation preserves canonical errors while opting into display", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto random_state = rng::peek_uint64();
    const coord_def target = you.pos() + coord_def(2, 2);
    string original, display;
    CHECK_FALSE(wu_jian_can_wall_jump(target, original));
    CHECK(original == "Please select an adjacent position to wall jump against.");
    string canonical;
    CHECK_FALSE(wu_jian_can_wall_jump(target, canonical, &display));
    CHECK(canonical == original);
    CHECK(display == "请选择相邻位置作为蹬墙跳的借力点。");
    Options.language = lang_t::EN;
    CHECK_FALSE(wu_jian_can_wall_jump(target, canonical, &display));
    CHECK(canonical == original);
    CHECK(display == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("G recitation display uses the same deterministic passages and preserves English", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    for (int type = 0; type < 4; ++type)
        for (int seed = 0; seed < 27 * 81; ++seed)
            for (int step : {-1, 0, 1, 2})
            {
                Options.language = lang_t::EN;
                const string original = zin_recite_text(seed, type, step);
                CHECK(zin_recite_text(seed, type, step, true) == original);
                Options.language = lang_t::ZH;
                const string translated = zin_recite_text(seed, type, step, true);
                CHECK(translated != original);
                CHECK(translated.find('@') == string::npos);
                CHECK(zin_recite_text(seed, type, step) == original);
            }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("G god conduct prose opts into display and preserves canonical metadata", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    init_mut_index();
    const auto random_state = rng::peek_uint64();
    for (int i = GOD_ZIN; i < NUM_GODS; ++i)
    {
        const god_type god = static_cast<god_type>(i);
        if (is_unavailable_god(god) || god == GOD_ECUMENICAL)
            continue;
        Options.language = lang_t::EN;
        const string likes = get_god_likes(god);
        const string dislikes = get_god_dislikes(god);
        CHECK(get_god_likes(god, true) == likes);
        CHECK(get_god_dislikes(god, true) == dislikes);
        Options.language = lang_t::ZH;
        CHECK(get_god_likes(god) == likes);
        CHECK(get_god_dislikes(god) == dislikes);
        if (!likes.empty())
            CHECK(get_god_likes(god, true) != likes);
        if (!dislikes.empty())
            CHECK(get_god_dislikes(god, true) != dislikes);
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("G monster event templates are registered at the actual message boundary", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    init_monsters();
    monster_info mon(MONS_ORC);
    string name;
    REQUIRE(mon.try_display_name(name));
    CHECK(ui_render("log.monster", "%s is recalled by Beogh.", "the orc is recalled by Beogh.",
        {ui_literal_arg(name)}) == name + "被比欧弗召回了。");
    CHECK(ui_render("log.monster", "%s's chaotic flesh bubbles and boils.", "original English",
        {{"", false}}) == "original English");
}

TEST_CASE("F form metadata display preserves English and consumes no RNG", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    init_properties();
    init_monsters();
    const auto random_state = rng::peek_uint64();
    for (int i = 0; i < NUM_TRANSFORMS; ++i)
    {
        const Form *form = get_form(static_cast<transformation>(i));
        Options.language = lang_t::EN;
        const auto good = form->get_fakemuts();
        const auto bad = form->get_bad_fakemuts();
        Options.language = lang_t::ZH;
        CHECK(form->get_fakemuts() == good);
        CHECK(form->get_bad_fakemuts() == bad);
        for (const auto &traits : {good, bad})
            for (const auto &trait : traits)
                if (!trait.second.empty())
                    CHECK(ui_text_arg("f.form.trait", trait.second).available);
        if (!form->short_name.empty())
            CHECK(ui_text_arg("f.form.name", form->short_name).available);
    }
    for (transformation which : {transformation::spider, transformation::statue,
                                transformation::dragon, transformation::rime_yak,
                                transformation::aqua, transformation::medusa})
    {
        const Form *form = get_form(which);
        Options.language = lang_t::EN;
        const string original = form->get_description();
        CHECK(form->display_description() == original);
        Options.language = lang_t::ZH;
        CHECK(form->display_description() != original);
        CHECK(form->get_description() == original);
        Options.language = lang_t::EN;
        CHECK(form->display_description() == original);
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("F complete warning templates fall back when an action or victim is missing", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto random_state = rng::peek_uint64();
    const string original = "Really attack near your ally rat?";
    const ui_arg attack = ui_text_arg("f.attack.action", "attack");
    const ui_arg victim = ui_literal_arg("盟友老鼠");
    CHECK(ui_render("f.warning", "Really %s near %s?%s", original,
        {attack, victim, ui_literal_arg("")}) == "确定要攻击（在盟友老鼠附近）吗？");
    CHECK(ui_render("f.warning", "Really %s near %s?%s", original,
        {attack, {"", false}, ui_literal_arg("")}) == original);
    CHECK(ui_render("f.warning", "Really %s near %s?%s", original,
        {ui_text_arg("f.attack.action", "unknown custom action"), victim,
         ui_literal_arg("")}) == original);
    Options.language = lang_t::EN;
    CHECK(ui_render("f.warning", "Really %s near %s?%s", original,
        {attack, victim, ui_literal_arg("")}) == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Bane display data does not change canonical names and descriptions", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    init_mut_index();
    for (int i = 0; i < NUM_BANES; ++i)
    {
        const bane_type bane = static_cast<bane_type>(i);
        Options.language = lang_t::EN;
        const string name = bane_name(bane);
        const string key = bane_name(bane, true);
        const string desc = bane_desc(bane);
        CHECK(bane_display_name(bane) == name);
        CHECK(bane_display_desc(bane) == desc);
        Options.language = lang_t::ZH;
        CHECK(bane_display_name(bane) != name);
        CHECK(bane_display_desc(bane) != desc);
        CHECK(bane_name(bane) == name);
        CHECK(bane_name(bane, true) == key);
        CHECK(bane_desc(bane) == desc);
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("N character display preserves canonical creation choices and RNG", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    const auto random_state = rng::peek_uint64();
    for (auto choices : {std::make_pair(SP_HUMAN, JOB_FIGHTER),
                         std::make_pair(SP_RANDOM, JOB_RANDOM),
                         std::make_pair(SP_VIABLE, JOB_VIABLE),
                         std::make_pair(SP_HUMAN, JOB_RANDOM),
                         std::make_pair(SP_RANDOM, JOB_FIGHTER)})
    {
        newgame_def ng;
        ng.species = choices.first;
        ng.job = choices.second;
        const string canonical = newgame_char_description(ng);
        Options.language = lang_t::EN;
        CHECK(newgame_char_display_description(ng) == canonical);
        Options.language = lang_t::ZH;
        CHECK(newgame_char_display_description(ng) != canonical);
        CHECK(newgame_char_display_arg(ng).available);
        CHECK(newgame_char_description(ng) == canonical);
        CHECK(ng.species == choices.first);
        CHECK(ng.job == choices.second);
        Options.language = lang_t::EN;
        CHECK(newgame_char_display_description(ng) == canonical);
    }
    Options.language = lang_t::ZH;
    CHECK(ui_render("n.character", "%s %s", "Human UnknownJob",
        {ui_text_arg("display.creation", "Human"), {"", false}}) == "Human UnknownJob");
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("N typed notes preserve canonical records and whole-row English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    init_properties();
    init_monsters();
    init_mut_index();
    init_spell_descs();
    const auto random_state = rng::peek_uint64();
    const vector<Note> notes = {
        Note(NOTE_XOM_REVIVAL), Note(NOTE_MP_CHANGE, 3, 9),
        Note(NOTE_MAXHP_CHANGE, 42), Note(NOTE_MAXMP_CHANGE, 12),
        Note(NOTE_GET_GOD, GOD_ZIN), Note(NOTE_LOSE_GOD, GOD_ZIN),
        Note(NOTE_PENANCE, GOD_ZIN), Note(NOTE_MOLLIFY_GOD, GOD_ZIN),
        Note(NOTE_GOD_GIFT, GOD_ZIN), Note(NOTE_DONATE_MONEY, 100),
        Note(NOTE_GAIN_SKILL, SK_FIGHTING, 5), Note(NOTE_LOSE_SKILL, SK_FIGHTING, 3),
        Note(NOTE_LEARN_SPELL, SPELL_MAGIC_DART), Note(NOTE_OFFERED_SPELL, SPELL_MAGIC_DART),
        Note(NOTE_GET_MUTATION, MUT_TOUGH_SKIN, 1),
        Note(NOTE_LOSE_MUTATION, MUT_TOUGH_SKIN, 1),
        Note(NOTE_PERM_MUTATION, MUT_TOUGH_SKIN, 1),
        Note(NOTE_GEM_LOST, GEM_DUNGEON), Note(NOTE_DREAMSHARD),
        Note(NOTE_GAIN_LIFE, 2), Note(NOTE_LOSE_LIFE, 1),
        Note(NOTE_ZOT_TOUCHED, 100, 90), Note(NOTE_FLED_CHALLENGE),
        Note(NOTE_INFERNAL_MARK, 0, 0, "Mark of Carnage"),
        Note(NOTE_TESSERACT_ACTIVATED)
    };
    for (const Note& note : notes)
    {
        const string original = note.describe(false, false, true);
        Options.language = lang_t::EN;
        CHECK(note.display_describe(false, false, true) == original);
        Options.language = lang_t::ZH;
        INFO(original);
        CHECK(note.display_describe(false, false, true) != original);
        CHECK(note.describe(false, false, true) == original);
        CHECK(note.display_describe(true, true, false) == note.describe(true, true, false));
        Options.language = lang_t::EN;
        CHECK(note.display_describe(false, false, true) == original);
    }
    Options.language = lang_t::ZH;
    for (NOTE_TYPES type : {NOTE_SEEN_MONSTER, NOTE_ID_ITEM, NOTE_GOD_GIFT,
                            NOTE_GET_MUTATION, NOTE_XOM_EFFECT, NOTE_PARALYSIS,
                            NOTE_XP_LEVEL_CHANGE, NOTE_RECRUITED_APOSTLE})
    {
        Note note(type, type == NOTE_GET_MUTATION ? MUT_TOUGH_SKIN : 1, 1,
                  "unavailable canonical snapshot", "unavailable description");
        CHECK(note.display_describe() == note.describe());
    }
    for (NOTE_TYPES type : {NOTE_USER_NOTE, NOTE_MESSAGE, NOTE_DEATH})
    {
        Note note(type, 0, 0, "player/script text 中文 English");
        CHECK(note.display_describe() == note.describe());
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("N note display does not change serialization", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    Note note(NOTE_GAIN_LIFE, 2);
    vector<unsigned char> before;
    auto canonical = writer(&before);
    note.save(canonical);
    const string original = note.describe();
    Options.language = lang_t::ZH;
    CHECK(note.display_describe() != original);
    vector<unsigned char> after;
    auto displayed = writer(&after);
    note.save(displayed);
    CHECK(after == before);
    auto input = reader(after, TAG_MINOR_VERSION);
    Note loaded;
    loaded.load(input);
    CHECK(loaded.describe() == original);
    CHECK(loaded.display_describe() == note.display_describe());
    Options.language = lang_t::EN;
    CHECK(loaded.display_describe() == original);
}

TEST_CASE("O monster list uses typed names without English articles", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    init_monsters();
    const auto random_state = rng::peek_uint64();
    const vector<monster_info> single = {monster_info(MONS_ORC)};
    const vector<monster_info> grouped = {monster_info(MONS_ORC), monster_info(MONS_ORC)};
    for (const auto& list : {single, grouped})
    {
        for (bool past : {false, true})
        {
            Options.language = lang_t::EN;
            const string original = monster_list_description(list, past);
            CHECK(original.find(past ? "You could see " : "You can see ") == 0);
            Options.language = lang_t::ZH;
            const string translated = monster_list_description(list, past);
            CHECK(translated != original);
            CHECK(translated.find("a ") == string::npos);
            CHECK(translated.find("an ") == string::npos);
            CHECK(translated.find("the ") == string::npos);
            Options.language = lang_t::EN;
            CHECK(monster_list_description(list, past) == original);
        }
    }
    Options.language = lang_t::ZH;
    CHECK(monster_list_description({}) == "视野内没有怪物！");
    CHECK(monster_list_description({}, true) == "当时视野内没有怪物！");
    CHECK(ui_render("o.monsters", "You can see %s.", "You can see a custom monster.",
        {{"", false}}) == "You can see a custom monster.");
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("O form attack display covers canonical form names without changing them", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> original_form(you.form);
    init_properties();
    init_mut_index();
    const auto random_state = rng::peek_uint64();
    for (species_type race : {SP_HUMAN, SP_OCTOPODE, SP_FELID})
    {
        you.species = race;
        for (int i = 0; i < NUM_TRANSFORMS; ++i)
        {
            you.form = static_cast<transformation>(i);
            const Form* form = get_form();
            Options.language = lang_t::EN;
            const string canonical = form->get_uc_attack_name("Claws");
            CHECK_FALSE(form->uc_attack_display_arg("Claws").available);
            Options.language = lang_t::ZH;
            const ui_arg name = form->uc_attack_display_arg("Claws");
            INFO(canonical);
            CHECK(name.available);
            CHECK_FALSE(name.text.empty());
            CHECK(form->get_uc_attack_name("Claws") == canonical);
            Options.language = lang_t::EN;
            CHECK(form->get_uc_attack_name("Claws") == canonical);
        }
    }
    Options.language = lang_t::ZH;
    CHECK_FALSE(get_form(transformation::none)->uc_attack_display_arg("Unknown custom attack").available);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("O damage and property rows fall back as complete expressions", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto random_state = rng::peek_uint64();
    const string original = "Your corrosive artefact corrodes you!";
    const ui_arg source = ui_text_arg("o.corrode.source", "Your corrosive artefact");
    REQUIRE(source.available);
    CHECK(ui_render("o.corrode", "%s corrodes you!", original, {source})
        == "你那具有腐蚀性的神器使你受到腐蚀！");
    CHECK(ui_render("o.corrode", "%s corrodes you!", original, {{"", false}}) == original);
    CHECK(ui_render("o.log", "%s scurries out from under your cloak.", "a rat scurries out from under your cloak.",
        {{"", false}}) == "a rat scurries out from under your cloak.");
    const ui_translation_group properties({{"o.passive", "harm (+%d%% outgoing, +%d%% incoming)"}});
    REQUIRE(properties.available());
    const string harm = make_stringf(properties.get("o.passive", "harm (+%d%% outgoing, +%d%% incoming)"), 30, 20);
    CHECK(harm == "伤害增幅（造成+30%，承受+20%）");
    CHECK(ui_render("o.passive", "infuse magic (%d %s)", "infuse magic (3 HP)",
        {ui_literal_arg("3"), {"", false}}) == "infuse magic (3 HP)");
    Options.language = lang_t::EN;
    CHECK(ui_render("o.corrode", "%s corrodes you!", original, {source}) == original);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Monster resistance table uses the same complete labels as the player", "[localization]")
{
    init_properties();
    init_monsters();
    init_mut_index();
    init_duration_index();
    init_spell_descs();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    monster_info info(MONS_FIRE_DRAGON);
    describe_info description;
    bool stats = false;
    get_monster_db_desc(info, description, stats);
    REQUIRE(stats);
    const string shown = formatted_string::parse_string(description.body.str()).tostring();
    for (const char *label : {"火焰抗性", "寒冷抗性", "毒素抗性", "负能量抗性", "电击抗性"})
        CHECK(shown.find(label) != string::npos);
    bool resistance_row = false;
    for (const string &row : split_string("\n", shown))
        if (row.find("火焰抗性") != string::npos && row.find("电击抗性") != string::npos)
        {
            resistance_row = true;
            INFO(row);
            CHECK(strwidth(row) <= 80);
        }
    CHECK(resistance_row);
    Options.language = lang_t::EN;
    describe_info original;
    get_monster_db_desc(info, original, stats);
    const string en = original.body.str();
    for (const char *label : {"rF", "rC", "rPois", "rNeg", "rElec"})
        CHECK(en.find(label) != string::npos);
    CHECK(en.find("负能量抗性") == string::npos);
}

TEST_CASE("O complete overview uses localized rows and retains English mode", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<god_type> god(you.religion, GOD_NO_GOD);
    init_properties();
    init_monsters();
    init_mut_index();
    init_duration_index();
    unwind_var<int> cloak_slots(you.equipment.num_slots[SLOT_CLOAK], 1);
    const auto random_state = rng::peek_uint64();
    Options.language = lang_t::EN;
    const string english = dump_overview_screen();
    CHECK(english.find("AC:") != string::npos);
    CHECK(english.find("rFire") != string::npos);
    Options.language = lang_t::ZH;
    const string chinese = dump_overview_screen();
    CHECK(chinese.find("护甲值") != string::npos);
    CHECK(chinese.find("火焰抗性") != string::npos);
    CHECK(chinese.find("寒冷抗性") != string::npos);
    CHECK(chinese.find("毒素抗性") != string::npos);
    CHECK(chinese.find("电击抗性") != string::npos);
    CHECK(chinese.find("负能量抗性") != string::npos);
    CHECK(chinese.find("格挡值") != string::npos);
    CHECK(chinese.find("rFire") == string::npos);
    CHECK(chinese.find("(no ") == string::npos);
    CHECK(chinese.find("unavailable)") == string::npos);
    CHECK(chinese.find("披风") != string::npos);
    CHECK(chinese.find("no passive effects") == string::npos);
    Options.language = lang_t::EN;
    const string restored = dump_overview_screen();
    CHECK(restored.find("AC:") != string::npos);
    CHECK(restored.find("rFire") != string::npos);
    CHECK(restored.find("护甲值") == string::npos);
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("P anatomy display preserves canonical names across forms", "[localization]")
{
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> race(you.species);
    unwind_var<transformation> form(you.form);
    init_properties();
    init_mut_index();
    const auto state = rng::peek_uint64();
    for (species_type species : {SP_HUMAN, SP_OCTOPODE, SP_FELID, SP_COGLIN, SP_POLTERGEIST})
    {
        you.species = species;
        for (int index = 0; index < NUM_TRANSFORMS; ++index)
        {
            you.form = static_cast<transformation>(index);
            const string hand = you.hand_name(true);
            const string arm = you.arm_name(true);
            const string foot = you.foot_name(true);
            INFO(hand << ", " << arm << ", " << foot);
            Options.language = lang_t::EN;
            CHECK_FALSE(p_hand_arg().available);
            CHECK_FALSE(p_arm_arg().available);
            CHECK_FALSE(p_foot_arg().available);
            Options.language = lang_t::ZH;
            CHECK(p_hand_arg().available);
            CHECK(p_arm_arg().available);
            CHECK(p_foot_arg().available);
            CHECK(you.hand_name(true) == hand);
            CHECK(you.arm_name(true) == arm);
            CHECK(you.foot_name(true) == foot);
        }
    }
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("P optional clauses and item lists preserve whole-message fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    init_properties();
    const auto state = rng::peek_uint64();
    item_def potion;
    potion.base_type = OBJ_POTIONS;
    potion.sub_type = POT_CURING;
    potion.quantity = 1;
    potion.flags |= ISFLAG_IDENTIFIED;
    const string name = potion.name(DESC_PLAIN);
    const vector<const item_def*> items = {&potion};
    const ui_arg list = p_item_list_arg(items);
    REQUIRE(list.available);
    CHECK(list.text.find("治疗药水") != string::npos);
    CHECK(potion.name(DESC_PLAIN) == name);
    CHECK(p_clause_arg("").available);
    CHECK(p_clause_arg("").text.empty());
    CHECK_FALSE(p_clause_list_arg(vector<string>{"injured", "unknown custom clause"}).available);
    const string original = "The unknown equipment cannot attune itself to your injured body.";
    CHECK(ui_render("p.log.48", "Equipment %s cannot attune to your %s body.", original,
        {{"unknown custom equipment", false}, p_clause_arg("injured")}) == original);
    CHECK(ui_render("p.log.48", "Equipment %s cannot attune to your %s body.", original,
        {list, {"unknown custom state", false}}) == original);
    const string en = "You feel more clever all of a sudden.";
    CHECK(ui_render("p.log.124", "You feel %sclever all of a sudden.", en,
        {p_clause_arg("more ")}) == "你突然感觉更加聪明了。");
    CHECK(string(stat_desc(STAT_STR, SD_NAME)) == "strength");
    const string canonical_duration = duration_end_message(DUR_AGILITY, false);
    CHECK(canonical_duration == "You feel a little less agile now.");
    CHECK(string(duration_end_message(DUR_AGILITY)) != canonical_duration);
    CHECK(p_duration_arg(canonical_duration).available);
    CHECK(ui_render("p.duration", "Careful! %s", "Careful! unknown script warning",
        {p_duration_arg("unknown script warning")}) == "Careful! unknown script warning");
    CHECK(make_stringf(ui_translate("p.ui", "Your base attributes are Str %d, Int %d, Dex %d."), 9, 12, 15)
        == "你的基础属性为力量 9、智力 12、敏捷 15。");
    Options.language = lang_t::EN;
    CHECK_FALSE(p_item_list_arg(items).available);
    CHECK(ui_render("p.log.124", "You feel %sclever all of a sudden.", en,
        {p_clause_arg("more ")}) == en);
    CHECK(potion.name(DESC_PLAIN) == name);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Q empty and primary actions switch language without changing save data", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<vector<player_equip_entry>> equipment(you.equipment.items, {});
    init_properties();
    init_mut_index();
    const auto state = rng::peek_uint64();
    quiver::action empty;
    bool translated = true;
    CHECK(empty.quiver_description(false, &translated).tostring() == "Nothing quivered");
    CHECK_FALSE(translated);
    Options.language = lang_t::ZH;
    CHECK(empty.quiver_description(false, &translated).tostring() == "未选择备选动作");
    CHECK(translated);
    CHECK(empty.quiver_description(true).tostring() == "空");
    for (species_type race : {SP_HUMAN, SP_OCTOPODE, SP_FELID})
    {
        you.species = race;
        for (int i = 0; i < NUM_TRANSFORMS; ++i)
        {
            you.form = static_cast<transformation>(i);
            const auto action = quiver::get_primary_action();
            Options.language = lang_t::EN;
            const string en = action->quiver_description(false, &translated).tostring();
            CHECK_FALSE(translated);
            const string verb = action->quiver_verb();
            CrawlHashTable before;
            action->save(before);
            const string type = before["type"].get_string();
            Options.language = lang_t::ZH;
            const string zh = action->quiver_description(false, &translated).tostring();
            INFO(en << ", " << verb);
            CHECK(translated);
            CHECK(zh != en);
            CHECK(action->quiver_verb() == verb);
            CrawlHashTable after;
            action->save(after);
            CHECK(after["type"].get_string() == type);
            Options.language = lang_t::EN;
            CHECK(action->quiver_description().tostring() == en);
        }
    }
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Q inventory action preserves slot names and serialization", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    init_properties();
    const int slot = ENDOFPACK - 1;
    unwind_var<item_def> inventory(you.inv[slot]);
    item_def &item = you.inv[slot];
    item.clear();
    item.base_type = OBJ_POTIONS;
    item.sub_type = POT_CURING;
    item.quantity = 2;
    item.link = slot;
    item.slot = 'Z';
    item.flags = ISFLAG_IDENTIFIED;
    const auto action = quiver::slot_to_action(slot);
    REQUIRE(action);
    REQUIRE(action->is_valid());
    const auto state = rng::peek_uint64();
    const string name = item.name(DESC_PLAIN, true);
    const string en = action->quiver_description().tostring();
    CHECK(en == "Drink: " + name);
    CrawlHashTable before;
    action->save(before);
    Options.language = lang_t::ZH;
    bool translated = false;
    const string zh = action->quiver_description(false, &translated).tostring();
    CHECK(translated);
    CHECK(zh.find("饮用：") == 0);
    CHECK(zh.find("治疗药水") != string::npos);
    CHECK(action->quiver_verb() == "Drink");
    CHECK(item.name(DESC_PLAIN, true) == name);
    CrawlHashTable after;
    action->save(after);
    CHECK(after["type"].get_string() == before["type"].get_string());
    CHECK(after["param"].get_int() == before["param"].get_int());
    const string original = "You cannot attack while caught in a custom trap.";
    CHECK(ui_render("q.log", "You cannot attack while %s.", original,
        {ui_text_arg("q.clause", "caught in a custom trap")}) == original);
    Options.language = lang_t::EN;
    CHECK(action->quiver_description().tostring() == en);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Q spell rows retain canonical spell data and numeric suffixes", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<species_type> race(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<CrawlHashTable> props(you.props);
    unwind_var<spell_type> known(you.spells[0]);
    init_properties();
    init_mut_index();
    init_spell_descs();
    you.props[GRAVE_CLAW_CHARGES_KEY] = 2;
    for (spell_type spell : {SPELL_MAGIC_DART, SPELL_GRAVE_CLAW, SPELL_MAXWELLS_COUPLING})
    {
        you.spells[0] = spell;
        const auto action = quiver::spell_to_action(spell);
        REQUIRE(action->is_valid());
        const auto state = rng::peek_uint64();
        Options.language = lang_t::EN;
        const string en = action->quiver_description().tostring();
        CHECK(en.find("Cast: ") == 0);
        CHECK(en.find(spell == SPELL_MAXWELLS_COUPLING ? "Capacitive Coupling" : spell_title(spell)) != string::npos);
        Options.language = lang_t::ZH;
        bool translated = false;
        const string zh = action->quiver_description(false, &translated).tostring();
        INFO(en << ", " << zh);
        CHECK(translated);
        CHECK(zh.find("施法：") == 0);
        if (spell == SPELL_GRAVE_CLAW)
        {
            const string charges = make_stringf(" (2/%d)", GRAVE_CLAW_MAX_CHARGES);
            CHECK(zh.find(charges) != string::npos);
            CHECK(you.props[GRAVE_CLAW_CHARGES_KEY].get_int() == 2);
        }
        if (spell == SPELL_MAXWELLS_COUPLING)
            CHECK(zh.find("电容耦合") != string::npos);
        CrawlHashTable saved;
        action->save(saved);
        CHECK(saved["type"].get_string() == "spell_action");
        CHECK(saved["param"].get_int() == spell);
        CHECK(you.spells[0] == spell);
        Options.language = lang_t::EN;
        CHECK(action->quiver_description().tostring() == en);
        CHECK(rng::peek_uint64() == state);
    }
}

TEST_CASE("R every god power switches language while retaining canonical data", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    const auto state = rng::peek_uint64();
    for (const auto &powers : get_all_god_powers())
        for (const auto &power : powers)
        {
            const string gain = power.gain, loss = power.loss, general = power.general;
            const auto ability = power.abil;
            for (bool gaining : {false, true})
            {
                const char *str = gaining ? power.gain : power.loss;
                const char *wrapper = gaining ? "You can now %s." : "You can no longer %s.";
                const string expected = !*str ? "" : isupper(str[0])
                    ? string(str) : make_stringf(wrapper, str);
                Options.language = lang_t::EN;
                CHECK(power.display_text(gaining, wrapper) == expected);
                Options.language = lang_t::ZH;
                const string translated = power.display_text(gaining, wrapper);
                INFO(str);
                if (*str)
                    CHECK(translated != expected);
                else
                    CHECK(translated.empty());
                CHECK(string(power.gain) == gain);
                CHECK(string(power.loss) == loss);
                CHECK(string(power.general) == general);
                CHECK(power.abil == ability);
                Options.language = lang_t::EN;
                CHECK(power.display_text(gaining, wrapper) == expected);
            }
        }
    god_power unknown(1, "invoke an unknown upstream power");
    Options.language = lang_t::ZH;
    CHECK(unknown.display_text(true, "You can now %s.") == "You can now invoke an unknown upstream power.");
    god_power complete(1, "New upstream full sentence.");
    CHECK(complete.display_text(true, "You can now %s.") == "New upstream full sentence.");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("R lists reflection and ancestor memories fall back as whole messages", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    init_properties();
    const auto state = rng::peek_uint64();
    const string offer = "Vehumet offers you knowledge of Magic Dart. These spells will remain available as long as you worship Vehumet.";
    const ui_arg god = ui_text_arg("display.god", god_name(GOD_VEHUMET));
    const ui_arg spell = ui_text_arg("display.spell", spell_title(SPELL_MAGIC_DART));
    const ui_arg duration = ui_text_arg("r.clause", " These spells will remain available as long as you worship Vehumet.");
    CHECK(ui_render("r.log", "%s offers you knowledge of %s.%s", offer,
        {god, spell, duration}) == "维胡梅特向你传授魔法飞弹的知识。只要你继续信奉维胡梅特，这些法术就会一直可用。");
    CHECK(ui_render("r.log", "%s offers you knowledge of %s.%s", offer,
        {god, {"unknown spell", false}, duration}) == offer);
    CHECK(ui_render("r.log", "%s offers you knowledge of %s.%s", offer,
        {god, spell, {"unknown duration", false}}) == offer);
    const string reflected = "You block the stone with your kite shield... and reflect it back!";
    const ui_arg stone = ui_text_arg("log.projectile.name", "stone");
    const ui_arg shield = ui_text_arg("display.item", "kite shield");
    CHECK(ui_render("r.reflection", "%s blocks the %s with their %s... and reflects it back!",
        reflected, {ui_actor_arg(you), stone, shield}) == "你挡下了石子，使用的是鸢盾……并将其反射回来！");
    CHECK(ui_render("r.reflection", "%s blocks the %s with their %s... and reflects it back!",
        reflected, {ui_actor_arg(you), stone, {"unknown shield", false}}) == reflected);
    const ui_arg axe = ui_text_arg("display.item", "broad axe");
    const ui_arg brand = ui_text_arg("r.brand", "flame");
    CHECK(ui_render("r.memory.item", "brand %s on %s", "broad axe of flame", {brand, axe}) == "火焰阔斧");
    CHECK(ui_render("r.memory.item", "brand %s on %s", "broad axe of unknown", {{"unknown", false}, axe}) == "broad axe of unknown");
    Options.language = lang_t::EN;
    CHECK(ui_render("r.log", "%s offers you knowledge of %s.%s", offer, {god, spell, duration}) == offer);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("R god gold unlock wrappers and spell warning groups preserve English", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    for (const auto &power : get_all_god_powers()[GOD_GOZAG])
    {
        if (!*power.gain) continue;
        const string english = isupper(power.gain[0]) ? string(power.gain)
            : make_stringf("You have enough gold to %s.", power.gain);
        CHECK(power.display_text(true, "You have enough gold to %s.") != english);
        Options.language = lang_t::EN;
        CHECK(power.display_text(true, "You have enough gold to %s.") == english);
        Options.language = lang_t::ZH;
    }
    const string warning = god_spell_warn_string(SPELL_MAGIC_DART, GOD_TROG);
    CHECK(warning == "This will place you under penance!");
    const string prompt = "Memorise Magic Dart, consuming 1 spell level and leaving 3? This will place you under penance!";
    CHECK(ui_render("r.memorise", "Memorise %s, consuming %s spell levels and leaving %s?%s", prompt,
        {ui_text_arg("display.spell", spell_title(SPELL_MAGIC_DART)), ui_literal_arg("1"),
         ui_literal_arg("3"), ui_text_arg("r.spell.warning", warning)})
        == "记忆魔法飞弹，消耗1级法术容量，剩余3级？这会使你受到赎罪惩罚！");
    CHECK(ui_render("r.memorise", "Memorise %s, consuming %s spell levels and leaving %s?%s", prompt,
        {ui_text_arg("display.spell", spell_title(SPELL_MAGIC_DART)), ui_literal_arg("1"),
         ui_literal_arg("3"), {"unknown warning", false}}) == prompt);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("R ancestor memory snapshots preserve known names outside combat visibility", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    monster ancestor;
    ancestor.type = MONS_ANCESTOR;
    ancestor.mname = "测试先祖";
    ancestor.attitude = ATT_FRIENDLY;
    ancestor.hit_points = ancestor.max_hit_points = 1;
    const auto state = rng::peek_uint64();
    const string english = ancestor.name(DESC_YOUR, true);
    string shown;
    REQUIRE(monster_info(&ancestor, MILEV_NAME).try_display_name(shown));
    CHECK(shown.find(ancestor.mname) != string::npos);
    CHECK(shown != ui_text_arg("log.actor", "something").text);
    const string original = english + " remembers more of his old skill.";
    CHECK(ui_render("r.log", "%s remembers more of their old skill.", original,
        {{shown, true}}).find(ancestor.mname) != string::npos);
    Options.language = lang_t::EN;
    CHECK(ancestor.name(DESC_YOUR, true) == english);
    CHECK(ui_render("r.log", "%s remembers more of their old skill.", original,
        {{shown, true}}) == original);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("S spell names schools and lists preserve canonical APIs and RNG", "[localization]")
{
    init_spell_descs();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    const auto state = rng::peek_uint64();
    for (spell_type spell : {SPELL_MAGIC_DART, SPELL_FIREBALL, SPELL_GRAVE_CLAW})
    {
        const string title = spell_title(spell);
        const string schools = spell_schools_string(spell);
        CHECK_FALSE(s_spell_schools_arg(spell).available);
        Options.language = lang_t::ZH;
        const ui_arg shown = s_spell_schools_arg(spell);
        CHECK(shown.available);
        CHECK(shown.text != schools);
        CHECK(string(spell_title(spell)) == title);
        CHECK(spell_schools_string(spell) == schools);
        Options.language = lang_t::EN;
    }
    Options.language = lang_t::ZH;
    const ui_arg list = s_spell_list_arg({SPELL_MAGIC_DART, SPELL_FIREBALL});
    REQUIRE(list.available);
    CHECK(list.text == "魔法飞弹、火球术");
    const string original = "You add the spells Magic Dart and Fireball to your library.";
    CHECK(ui_render("s.log", "You add the spells %s to your library.", original,
        {list}) == "你将法术魔法飞弹、火球术加入了法术库。");
    CHECK(ui_render("s.log", "You add the spells %s to your library.", original,
        {{"new upstream spell", false}}) == original);
    Options.language = lang_t::EN;
    CHECK(ui_render("s.log", "You add the spells %s to your library.", original, {list}) == original);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Shop discoveries count actual shops and floor descriptions use typed shop names", "[localization]")
{
    init_properties(); init_monsters(); init_show_table();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<bool> level(you.on_current_level, false);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type, GAME_TYPE_NORMAL);
    unwind_var<int> width(crawl_view.msgsz.x, 180);
    unwind_var<int> columns(crawl_view.termsz.x, 180);
    unwind_var<int> explore_stop(Options.explore_stop, ES_SHOP);
    unwind_var<int> autopickup(Options.autopickup_on, 0);
    const coord_def cell(10,10), old_pos = you.pos();
    struct cleanup {coord_def p; ~cleanup(){you.set_position(p); clear_message_store();}} reset{old_pos};
    you.set_position(cell);
    unwind_var<dungeon_feature_type> floor(env.grid(cell), DNGN_ENTER_SHOP);
    unwind_var<map_cell> known(env.map_knowledge(cell));
    unwind_var<decltype(env.shop)> shops(env.shop);
    env.map_knowledge(cell) = map_cell();
    env.map_knowledge(cell).set_feature(DNGN_ENTER_SHOP);
    shop_struct shop;
    shop.pos = cell; shop.type = SHOP_ARMOUR_ANTIQUE;
    shop.shop_name = "Iloema Urghi"; shop.shop_suffix_name = "Shop";
    env.shop[cell] = shop;
    const string original = shop_name(shop);
    const auto state = rng::peek_uint64();
    for (const auto lang : {lang_t::EN, lang_t::ZH})
    {
        Options.language = lang;
        clear_message_store();
        explore_discoveries discoveries;
        discoveries.found_feature(cell, DNGN_ENTER_SHOP);
        CHECK(discoveries.stop_explore());
        const string discovery = get_last_messages(10,true);
        if (lang == lang_t::ZH)
        {
            CHECK(discovery.find("发现Iloema Urghi的古董防具商店。") != string::npos);
            CHECK(discovery.find("40") == string::npos);
            string shown;
            REQUIRE(try_feature_display_at(cell, true, shown));
            CHECK(shown == "Iloema Urghi的古董防具商店");
        }
        else CHECK(discovery.find("Found " + original + ".") != string::npos);
        clear_message_store(); describe_floor();
        const string entrance = get_last_messages(10,true);
        if (lang == lang_t::ZH)
        {
            CHECK(entrance.find("Iloema Urghi的古董防具商店") != string::npos);
            CHECK(entrance.find("There is") == string::npos);
        }
        else CHECK(entrance.find("There is an entrance to " + original + " here.") != string::npos);
        CHECK(shop_name(env.shop[cell]) == original);
    }
    Options.language = lang_t::ZH;
    env.shop[cell].shop_type_name = "Unreviewed upstream shop";
    string shown = "untouched";
    CHECK_FALSE(try_feature_display_at(cell, true, shown));
    CHECK(shown == "untouched");
    clear_message_store(); describe_floor();
    CHECK(get_last_messages(10,true).find("There is an entrance to ") != string::npos);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("S shop names preserve generated owners and opaque custom descriptions", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    shop_struct shop;
    shop.type = SHOP_WEAPON;
    shop.keeper_name[0] = 1; shop.keeper_name[1] = 2; shop.keeper_name[2] = 3;
    const auto state = rng::peek_uint64();
    ui_arg display;
    const string english = shop_name(shop, &display);
    CHECK_FALSE(display.available);
    Options.language = lang_t::ZH;
    CHECK(shop_name(shop, &display) == english);
    CHECK(display.available);
    CHECK(display.text.find("武器") != string::npos);
    CHECK(shop_name(shop) == english);
    shop.shop_type_name = "Unreviewed upstream shop description";
    const string unknown = shop_name(shop, &display);
    CHECK_FALSE(display.available);
    const string original = "Welcome to " + unknown + "! What would you like to do?";
    CHECK(ui_render("s.shop", "Welcome to %s! What would you like to do?", original, {display}) == original);
    Options.language = lang_t::EN;
    CHECK(shop_name(shop) == unknown);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("S status display retains canonical numeric labels colors and charge animation", "[localization]")
{
    init_duration_index();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<CrawlHashTable> props(you.props);
    unwind_var<int> shield(you.duration[DUR_DIVINE_SHIELD], 3);
    unwind_var<int> lash(you.attribute[ATTR_SERPENTS_LASH], 2);
    unwind_var<int> fugue(you.duration[DUR_FUGUE], 100);
    unwind_var<int> channel(you.attribute[ATTR_CHANNELLED_SPELL], SPELL_MAXWELLS_COUPLING);
    you.props[FUGUE_KEY] = 3;
    const auto state = rng::peek_uint64();
    for (int status : {int(DUR_DIVINE_SHIELD), int(STATUS_SERPENTS_LASH), int(DUR_FUGUE)})
    {
        status_info canonical, shown, restored;
        REQUIRE(fill_status_info(status, canonical));
        REQUIRE(fill_status_display_info(status, shown));
        CHECK(shown.light_text != canonical.light_text);
        CHECK(shown.light_colour == canonical.light_colour);
        REQUIRE(fill_status_info(status, restored));
        CHECK(restored.light_text == canonical.light_text);
        Options.language = lang_t::EN;
        REQUIRE(fill_status_display_info(status, restored));
        CHECK(restored.light_text == canonical.light_text);
        CHECK(restored.long_text == canonical.long_text);
        Options.language = lang_t::ZH;
    }
    vector<string> canonical_frames, shown_frames;
    for (int i = 0; i < 4; ++i)
    {
        status_info inf;
        REQUIRE(fill_status_info(STATUS_CHANNELLING_SPELL, inf));
        canonical_frames.push_back(inf.light_text);
    }
    for (int i = 0; i < 4; ++i)
    {
        status_info inf;
        REQUIRE(fill_status_display_info(STATUS_CHANNELLING_SPELL, inf));
        shown_frames.push_back(inf.light_text);
        CHECK(inf.light_text.find("充能") == 0);
        CHECK(inf.light_text.back() == canonical_frames[i].back());
    }
    CHECK(you.props[FUGUE_KEY].get_int() == 3);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("S dynamic spell rows require every typed component", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const string original = "A prism of explosive energy appears from nowhere!";
    CHECK(ui_render("s.log", "A prism of %s energy appears from nowhere!", original,
        {s_clause_arg("explosive")}) == "一个爆炸能量棱镜凭空出现！");
    CHECK(ui_render("s.log", "A prism of %s energy appears from nowhere!", original,
        {s_clause_arg("unreviewed energy")}) == original);
    const string forest = "A tree reaches out but misses an orc!";
    CHECK(ui_render("s.forest.hit", "A tree reaches out but misses %s%s", forest,
        {ui_text_arg("display.monster", "orc"), ui_attack_punctuation("!")}) == "一棵树伸出枝条，却没打中兽人！");
    CHECK(ui_render("s.forest.hit", "A tree reaches out but misses %s%s", forest,
        {{"unknown actor", false}, ui_literal_arg("!")}) == forest);
    const string jinx = "A giggling sprite leaps out and smacks an orc with its own hand.";
    CHECK(ui_render("s.jinx", "A giggling sprite leaps out and smacks %s with their own %s.", jinx,
        {ui_text_arg("display.monster", "orc"), s_anatomy_arg("hand")}) == "咯咯笑的小精灵跳了出来，用兽人自己的手打它。");
    CHECK(ui_render("s.jinx", "A giggling sprite leaps out and smacks %s with their own %s.", jinx,
        {ui_text_arg("display.monster", "orc"), s_anatomy_arg("unknown anatomy")}) == jinx);
    Options.language = lang_t::EN;
    CHECK(ui_render("s.log", "A prism of %s energy appears from nowhere!", original,
        {s_clause_arg("explosive")}) == original);
    CHECK(rng::peek_uint64() == state);
}


TEST_CASE("T target validation preserves canonical failures and clears cached display", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    targeter_smite target(&you, 5);
    CHECK_FALSE(target.fail("Out of range."));
    CHECK(target.why_not == "Out of range.");
    CHECK(target.display_why_not() == "超出范围。");
    const string original = "There is a stone wall there.";
    const string translated = ui_render("t.target", "There is %s there.", "",
        {s_feature_arg(DNGN_STONE_WALL)});
    REQUIRE_FALSE(translated.empty());
    target.fail(original, translated);
    CHECK(target.why_not == original);
    CHECK(target.display_why_not() == translated);
    Options.language = lang_t::EN;
    CHECK(target.display_why_not() == original);
    Options.language = lang_t::ZH;
    target.fail("An unreviewed upstream target restriction.");
    CHECK(target.display_why_not() == target.why_not);
    target.fail(original, translated);
    target.why_not = "Another upstream validator wrote this.";
    CHECK(target.display_why_not() == target.why_not);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("T form messages retain English methods and do not repeat random selection", "[localization]")
{
    init_properties();
    init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> current(you.form, transformation::none);
    const auto state = rng::peek_uint64();
    for (int i = 0; i < NUM_TRANSFORMS; ++i)
    {
        you.form = static_cast<transformation>(i);
        const Form *form = get_form();
        const string entering = form->transform_message();
        const string ending = form->get_untransform_message();
        INFO(i << ": " << entering << " / " << ending);
        Options.language = lang_t::EN;
        CHECK(form->display_form_message(entering) == entering);
        CHECK(form->display_form_message(ending, true) == ending);
        Options.language = lang_t::ZH;
        // Appendage and hydra are old save-compatibility stubs with no entry
        // description. Preserve the original incomplete diagnostic fallback.
        if (!entering.empty() && entering != "You turn into ")
            CHECK(form->display_form_message(entering) != entering);
        if (!ending.empty()) CHECK(form->display_form_message(ending, true) != ending);
        CHECK(form->transform_message() == entering);
        CHECK(form->get_untransform_message() == ending);
        const string unknown = "A new upstream transformation message.";
        // Dynamic form adapters deliberately use their original typed state;
        // static forms exercise unknown whole-message fallback here.
        if (you.form == transformation::flux)
            CHECK(form->display_form_message(unknown) == unknown);
    }
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("T selected trap lines and projectiles use reviewed canonical keys", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const string source = "A round of applause drowns out your thoughts as";
    const ui_arg line = t_harlequin_arg(source);
    REQUIRE(line.available);
    CHECK(line.text == "一阵掌声淹没了你的思绪，");
    CHECK(t_harlequin_arg(line.text).available);
    CHECK_FALSE(t_harlequin_arg("An opaque Lua callback result").available);
    CHECK_FALSE(t_projectile_arg("an unknown rendered projectile phrase").available);
    CHECK(t_clause_arg("").available);
    CHECK(t_clause_arg("").text.empty());
    Options.language = lang_t::EN;
    CHECK_FALSE(t_harlequin_arg(source).available);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("T discovery and combat rows fall back as complete messages", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const string original = "Found a scroll and a potion.";
    CHECK(ui_render("t.discovery", "Found %s.", original, {{"卷轴、药水", true}})
        == "发现卷轴、药水。");
    CHECK(ui_render("t.discovery", "Found %s.", original, {{"unreviewed item", false}}) == original);
    const string shot = "You shoot an arrow into the darkness.";
    CHECK(ui_render("t.log", "You %s %s%s.", shot,
        {t_clause_arg("shoot"), {"unknown projectile", false}, t_clause_arg(" into the darkness")}) == shot);
    CHECK(ui_render("t.tiles.row", "%s Skill %s Aptitude %s", "Fighting Skill 5.0 Aptitude +1",
        {ui_text_arg("display.skill", "Fighting"), ui_literal_arg("5.0"), ui_literal_arg("+1")})
        == "战斗 技能 5.0 天资 +1");
    Options.language = lang_t::EN;
    CHECK(ui_render("t.discovery", "Found %s.", original, {{"卷轴、药水", true}}) == original);
    CHECK(rng::peek_uint64() == state);
}


TEST_CASE("V map titles preserve canonical level descriptions and whole-row fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<int> dungeon_depth(brdepth[BRANCH_DUNGEON], 15);
    unwind_var<int> temple_depth(brdepth[BRANCH_TEMPLE], 1);
    const auto state = rng::peek_uint64();
    for (level_id id : {level_id(BRANCH_DUNGEON, 3), level_id(BRANCH_TEMPLE, 1)})
    {
        Options.language = lang_t::EN;
        const string canonical = id.describe(true, true);
        const string original = uppercase_first(canonical);
        CHECK(viewmap_display_level_name(id) == original);
        Options.language = lang_t::ZH;
        CHECK(viewmap_display_level_name(id) != original);
        CHECK(id.describe(true, true) == canonical);
        Options.language = lang_t::EN;
        CHECK(viewmap_display_level_name(id) == original);
    }
    Options.language = lang_t::ZH;
    const string unknown = "Level 3 of a future upstream branch";
    CHECK(ui_render("v.map", "Level %s of %s", unknown,
        {ui_literal_arg("3"), {"future branch", false}}) == unknown);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("V layer menu retains hotkeys and colour formatting in both variants", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const string local = "Select layers to display:\n<%s>(m)onsters</%s>|<%s>(p)layer</%s>|<%s>(i)tems</%s>|<%s>(c)louds</%s>";
    const string console = local + "|<%s>monster (w)eapons</%s>|<%s>monster (h)ealth</%s>";
    const string shown = make_stringf(ui_translate("v.layers", local.c_str()),
        "lightgrey", "lightgrey", "darkgrey", "darkgrey", "lightgrey", "lightgrey", "darkgrey", "darkgrey");
    CHECK(shown.find("怪物(m)") != string::npos);
    CHECK(shown.find("玩家(p)") != string::npos);
    CHECK(shown.find("物品(i)") != string::npos);
    CHECK(shown.find("云雾(c)") != string::npos);
    CHECK(shown.find("<darkgrey>") != string::npos);
    CHECK(string(ui_translate("v.layers", console.c_str())).find("怪物武器(w)") != string::npos);
    CHECK(string(ui_translate("v.layers", console.c_str())).find("怪物生命(h)") != string::npos);
    Options.language = lang_t::EN;
    CHECK(string(ui_translate("v.layers", local.c_str())) == local);
    CHECK(string(ui_translate("v.layers", console.c_str())) == console);
}

TEST_CASE("V saved version history remains opaque canonical data", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    unwind_var<CrawlHashTable> props(you.props);
    unwind_var<bool> saving(crawl_state.need_save, true);
    you.props["version_history"].new_vector(SV_STR);
    you.props["version_history"].get_vector().push_back(CrawlStoreValue("Game started: historical-version"));
    you.props["version_history"].get_vector().push_back(CrawlStoreValue("opaque custom history"));
    const string canonical = Version::history();
    CHECK(Version::history_size() == 2);
    Options.language = lang_t::ZH;
    CHECK(Version::history() == canonical);
    CHECK(canonical == "Game started: historical-version\nopaque custom history");
    Options.language = lang_t::EN;
    CHECK(Version::history() == canonical);
}


#ifdef WIZARD
TEST_CASE("W wizard choices translate copies and preserve canonical English and outputs", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::EN);
    const auto state = rng::peek_uint64();
    for (int i = 0; i < NUM_TRANSFORMS; ++i)
    {
        const string name = transform_name(static_cast<transformation>(i));
        CHECK(wizard_display_choice("Which form (ESC to exit)?", name) == name);
    }
    vector<WizardEntry> choices = {{'a', "armour", OBJ_ARMOUR}, {'g', "gold", OBJ_GOLD}};
    Options.language = lang_t::ZH;
    CHECK(wizard_display_choice("Which item class (ESC to exit)?", "armour") == "防具");
    CHECK(wizard_display_choice("Which form (ESC to exit)?", "flux") != "flux");
    CHECK(wizard_display_choice("Make which kind of trap?", "net") == "捕网陷阱");
    CHECK(wizard_display_choice("Make which kind of shop?", "armour") == "防具店");
    CHECK(wizard_display_choice("New upstream menu title", "armour") == "armour");
    CHECK(wizard_display_choice("Which item class (ESC to exit)?", "unknown upstream class") == "unknown upstream class");
    const WizardEntry shown = wizard_display_entry("Which item class (ESC to exit)?", choices[0]);
    CHECK(shown.text == "防具");
    CHECK(shown.output == choices[0].output);
    CHECK(shown.hotkeys_count() == choices[0].hotkeys_count());
    CHECK(choices[0].text == "armour");
    CHECK(choices[0].output == OBJ_ARMOUR);
    CHECK(choices[0].hotkeys_count() == 1);
    CHECK(choices[1].text == "gold");
    CHECK(choices[1].output == OBJ_GOLD);
    Options.language = lang_t::EN;
    CHECK(wizard_display_choice("Make which kind of trap?", "net") == "net");
    CHECK(rng::peek_uint64() == state);
}
#endif

TEST_CASE("Stationary corpse pickup uses display names and preserves corpse metadata", "[localization]")
{
    init_properties(); init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<bool> level(you.on_current_level, false);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type, GAME_TYPE_NORMAL);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    const coord_def cell(10,10), old_pos = you.pos();
    struct cleanup { coord_def p; ~cleanup(){ you.set_position(p); clear_message_store(); } } reset{old_pos};
    you.set_position(cell);
    unwind_var<dungeon_feature_type> floor(env.grid(cell), DNGN_FLOOR);
    unwind_var<int> link(env.igrid(cell), 0);
    unwind_var<item_def> saved_item(env.item[0]);
    unwind_var<decltype(you.last_pickup)> last(you.last_pickup);
    item_def &item = env.item[0];
    item = item_def(); item.base_type = OBJ_CORPSES; item.quantity = 1;
    item.mon_type = MONS_RAT; item.pos = cell; item.link = NON_ITEM;
    const auto state = rng::peek_uint64();
    for (const auto subtype : {CORPSE_BODY, CORPSE_SKELETON})
    {
        item.sub_type = subtype;
        const string canonical = item.name(DESC_A);
        Options.language = lang_t::ZH; clear_message_store(); pickup();
        const string log = get_last_messages(10,true);
        CHECK(log.find(subtype == CORPSE_BODY ? "老鼠的尸体" : "老鼠的骸骨") != string::npos);
        CHECK(log.find("corpse") == string::npos);
        CHECK(log.find("skeleton") == string::npos);
        CHECK(item.name(DESC_A) == canonical);
        CHECK(item.quantity == 1);
        CHECK(static_cast<monster_type>(item.mon_type) == MONS_RAT);
        Options.language = lang_t::EN; clear_message_store(); pickup();
        CHECK(get_last_messages(10,true).find(canonical) != string::npos);
    }
    item.props[CORPSE_NAME_KEY] = "Unreviewed scripted creature";
    item.props[CORPSE_NAME_TYPE_KEY] = static_cast<int64_t>(MF_NAME_REPLACE);
    const string custom = item.name(DESC_A);
    Options.language = lang_t::ZH; clear_message_store(); pickup();
    CHECK(get_last_messages(10,true).find(custom) != string::npos);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Target prompt rows require both a canonical prefix and complete body", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const string body = "a rat (sleeping)";
    const ui_arg shown{"老鼠（沉睡）",true};
    for (const auto &example : vector<pair<string,string>>{
             {"Aim","瞄准"},{"Here","此处"},{"Look","查看"},{"Heal","治疗"},
             {"Apport","拉取"},{"Smite","神罚"}})
    {
        CHECK(target_prompt_display(example.first.c_str(), body, shown, true)
            == example.second + "：<lightgrey>老鼠（沉睡）</lightgrey>");
        CHECK(target_prompt_display(example.first.c_str(), body, {"",false}, true)
            == example.first + ": <lightgrey>" + body + "</lightgrey>");
        Options.language = lang_t::EN;
        CHECK(target_prompt_display(example.first.c_str(), body, shown, true)
            == example.first + ": <lightgrey>" + body + "</lightgrey>");
        Options.language = lang_t::ZH;
    }
    CHECK(target_prompt_display("Custom upstream prefix", body, shown, true)
        == "Custom upstream prefix: <lightgrey>" + body + "</lightgrey>");
    CHECK(target_prompt_display("Here", "a rat corpse", {"老鼠的尸体",true}, false)
        == "此处：老鼠的尸体");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Target callback alternatives retain canonical percentages in one invocation", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    int calls = 0;
    auto callback = [&]() {
        ++calls;
        return target_description_format("s.target", "chance to affect: %d%%", 42);
    };
    CHECK(callback() == "生效概率：42%");
    {
        target_description_capture capture;
        const string canonical = callback();
        CHECK(canonical == "chance to affect: 42%");
        const ui_arg shown = capture.argument(canonical);
        CHECK(shown.available);
        CHECK(shown.text == "生效概率：42%");
        CHECK_FALSE(capture.argument("unknown upstream descriptor").available);
        const string missing = target_description_render("s.target", "will become %s", "will become unknown", {{"",false}});
        CHECK(missing == "will become unknown");
        CHECK_FALSE(capture.argument(missing).available);
        monster_info rat(MONS_RAT);
        const auto descriptions = desc_wl_success_chance(rat, 50, nullptr);
        REQUIRE_FALSE(descriptions.empty());
        for (const auto &desc : descriptions)
        {
            CHECK(desc.find("chance to affect:") != string::npos);
            CHECK(capture.argument(desc).available);
            CHECK(capture.argument(desc).text.find("生效概率：") != string::npos);
        }
    }
    CHECK(calls == 2);
    CHECK(callback() == "生效概率：42%");
    Options.language = lang_t::EN;
    CHECK(callback() == "chance to affect: 42%");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Hydra display head counts use Chinese numerals while canonical names stay English", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    monster_info hydra(MONS_HYDRA);
    for (const auto &entry : vector<pair<int,string>>{
             {1,"单"},{2,"双"},{7,"七"},{10,"十"},{11,"十一"},{20,"二十"},{27,"二十七"},
             {100,"一百"},{101,"一百零一"},{110,"一百一十"},{1001,"一千零一"},
             {10000,"一万"},{10010,"一万零一十"},{100000001,"一亿零一"}})
    {
        hydra.num_heads = entry.first;
        const string canonical = hydra.full_name(DESC_A);
        string shown;
        REQUIRE(hydra.try_display_name(shown));
        CHECK(shown == entry.second + "头海德拉");
        CHECK(hydra.full_name(DESC_A) == canonical);
        Options.language = lang_t::EN;
        shown = "untouched";
        CHECK_FALSE(hydra.try_display_name(shown));
        CHECK(shown == "untouched");
        CHECK(hydra.full_name(DESC_A) == canonical);
        Options.language = lang_t::ZH;
    }
    hydra.num_heads = 0;
    string shown = "untouched";
    CHECK_FALSE(hydra.try_display_name(shown));
    CHECK(shown == "untouched");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Special beam hit verbs use the real display path and complete English fallback", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<int> health(you.hp, 20);
    monster rat;
    rat.type = rat.base_monster = MONS_RAT;
    rat.hit_points = rat.max_hit_points = 10;
    rat.set_position(coord_def(10,10));
    bolt beam;
    beam.flavour = beam.real_flavour = BEAM_FIRE;
    beam.name = "foxfire";
    beam.hit_verb = "burns";
    const char *source = "The %s %s %s%s%s%s";
    const auto state = rng::peek_uint64();
    string shown;
    REQUIRE(try_beam_hit_display(source, beam, rat, false, ".", shown));
    CHECK(shown == "狐火灼烧老鼠。");
    beam.name = "volley of spikes";
    beam.hit_verb = "skewers";
    REQUIRE(try_beam_hit_display(source, beam, you, false, "!", shown));
    CHECK(shown == "尖刺齐射刺穿你！");
    for (const char *verb : {"burns", "skewers", "skewer", "envelops", "slices through",
             "engulf", "batters", "passes through", "grips", "grasp", "weakly hits",
             "blasts", "feebly blasts", "drowns", "pelts", "hits", "engulfs", "pierces through"})
    {
        CAPTURE(verb);
        beam.hit_verb = verb;
        REQUIRE(try_beam_hit_display(source, beam, rat, false, "!!!", shown));
        CHECK(ends_with(shown, "！！！"));
        CHECK(shown.find(verb) == string::npos);
        REQUIRE(try_beam_hit_display(source, beam, rat, true, ".", shown));
        CHECK(shown.find("，却未造成伤害") != string::npos);
        CHECK(ends_with(shown, "。"));
    }
    auto unavailable = [&]() {
        shown = "untouched";
        CHECK_FALSE(try_beam_hit_display(source, beam, rat, false, ".", shown));
        CHECK(shown == "untouched");
    };
    beam.hit_verb = "new upstream verb"; unavailable();
    beam.hit_verb = "skewers"; beam.name = "new upstream projectile"; unavailable();
    beam.name = "volley of spikes"; beam.real_flavour = BEAM_CHAOS;
    beam.flavour = BEAM_NONE; unavailable(); // Unknown chaos component retains English.
    beam.real_flavour = BEAM_FIRE;
    shown = "untouched";
    CHECK_FALSE(try_beam_hit_display(source, beam, rat, false, "?", shown));
    CHECK(shown == "untouched");
    Options.language = lang_t::EN; unavailable();
    CHECK(beam.name == "volley of spikes");
    CHECK(beam.hit_verb == "skewers");
    CHECK(rat.hit_points == 10);
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Actual player corrosion translates fixed sources without changing corrosion or RNG", "[localization]")
{
    init_properties(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<bool> level(you.on_current_level, false);
    unwind_var<decltype(crawl_state.type)> mode(crawl_state.type, GAME_TYPE_NORMAL);
    unwind_var<int> width(crawl_view.msgsz.x, 180);
    unwind_var<decltype(you.props)> properties(you.props);
    unwind_var<int> duration(you.duration[DUR_CORROSION]);
    unwind_var<decltype(you.redraw_armour_class)> redraw(you.redraw_armour_class);
    unwind_var<decltype(you.wield_change)> wield(you.wield_change);
    struct cleanup { ~cleanup() { clear_message_store(); } } reset;
    const vector<pair<string,string>> sources = {
        {"the entropic weave", "熵之编织"}, {"the acid", "酸液"},
        {"the rust", "锈蚀"}, {"a flying bit", "飞散的碎片"},
        {"Your corrupting presence", "你的腐化气息"}, {"wild magic", "狂野魔法"},
        {"The trap's stomach acid", "陷阱的胃酸"}};
    for (const auto &source : sources)
        for (int seed = 0; seed < 8; ++seed)
        {
            CAPTURE(source.first, seed);
            int amount = 0, turns = 0; uint64_t next = 0; bool result = false;
            for (const auto lang : {lang_t::EN, lang_t::ZH})
            {
                rng::subgenerator isolated(seed, 0);
                Options.language = lang;
                you.props[CORROSION_KEY] = 0; you.duration[DUR_CORROSION] = 0;
                clear_message_store();
                const bool applied = you.corrode(nullptr, source.first.c_str());
                const string log = get_last_messages(10, true);
                if (lang == lang_t::EN)
                {
                    amount = you.props[CORROSION_KEY].get_int();
                    turns = you.duration[DUR_CORROSION]; result = applied;
                    next = rng::peek_uint64();
                    CHECK(log.find("corrodes you!") != string::npos);
                }
                else
                {
                    CHECK(log.find(source.second + "使你受到腐蚀！") != string::npos);
                    CHECK(log.find("corrodes you") == string::npos);
                    CHECK(you.props[CORROSION_KEY].get_int() == amount);
                    CHECK(you.duration[DUR_CORROSION] == turns);
                    CHECK(applied == result);
                    CHECK(rng::peek_uint64() == next);
                }
            }
        }
    Options.language = lang_t::ZH;
    you.props[CORROSION_KEY] = 0; you.duration[DUR_CORROSION] = 0;
    clear_message_store(); you.corrode(nullptr, "a new upstream source");
    CHECK(get_last_messages(10,true).find("A new upstream source corrodes you!") != string::npos);
}

TEST_CASE("Equipment property tags localize typed values without changing canonical inscriptions", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    item_def item;
    item.base_type = OBJ_ARMOUR; item.sub_type = ARM_CLOAK; item.quantity = 1;
    REQUIRE(make_item_randart(item, true));
    item.flags |= ISFLAG_IDENTIFIED;
    for (int p = 0; p < ARTP_NUM_PROPERTIES; ++p)
        artefact_set_property(item, static_cast<artefact_prop_type>(p), 0);
    const vector<pair<artefact_prop_type, int>> values = {
        {ARTP_STEALTH, 2}, {ARTP_SLAYING, 4}, {ARTP_STRENGTH, -3},
        {ARTP_NEGATIVE_ENERGY, 1}, {ARTP_SEE_INVISIBLE, 1}, {ARTP_FLY, 1}};
    for (const auto &entry : values)
        artefact_set_property(item, entry.first, entry.second);
    item.inscription = "!w Stlth+ user text";
    const string canonical = artefact_inscription(item);
    const auto state = rng::peek_uint64();
    string shown;
    REQUIRE(try_artefact_display_inscription(item, shown));
    for (const string expected : {"潜行++", "杀戮+4", "力量-3", "负能抗性+", "看破隐形", "飞行"})
        CHECK(shown.find(expected) != string::npos);
    CHECK(artefact_inscription(item) == canonical);
    CHECK(item.inscription == "!w Stlth+ user text");
    vector<string> lines;
    desc_randart_props(item, lines);
    string details;
    for (const auto &line : lines) details += line;
    CHECK(details.find("潜行++：") != string::npos);
    CHECK(details.find("杀戮+4：") != string::npos);
    CHECK(details.find("Stlth") == string::npos);
    CHECK(details.find("Slay") == string::npos);
    artefact_set_property(item, ARTP_STEALTH, -5);
    REQUIRE(try_artefact_display_inscription(item, shown));
    CHECK(shown.find("潜行-----") != string::npos);
    artefact_set_property(item, ARTP_STEALTH, 5);
    REQUIRE(try_artefact_display_inscription(item, shown));
    CHECK(shown.find("潜行+5") != string::npos);
    item.flags &= ~ISFLAG_IDENTIFIED;
    REQUIRE(try_artefact_display_inscription(item, shown));
    CHECK(shown.empty());
    Options.language = lang_t::EN;
    shown = "untouched";
    CHECK_FALSE(try_artefact_display_inscription(item, shown));
    CHECK(shown == "untouched");
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Travel branch prompts and barbs confirmations retain English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    CHECK(string(branch_display_name(BRANCH_DUNGEON)) == "地下城");
    CHECK(string(branch_display_name(BRANCH_LAIR)) == "野兽巢穴");
    const string original = "What level of the Dungeon? (default D:2, ? - help) ";
    CHECK(ui_render("t.log", "What level of %s? (default %s, ? - help) ", original,
        {ui_text_arg("b.branch.name", branches[BRANCH_DUNGEON].longname),
         ui_literal_arg("D:2")}) == "前往地下城的哪一层？（默认 D:2，? - 帮助） ");
    CHECK(ui_render("t.log", "What level of %s? (default %s, ? - help) ", original,
        {ui_text_arg("b.branch.name", "unknown upstream branch"),
         ui_literal_arg("D:2")}) == original);
    const char *barbs = "The barbs in your skin will harm you if you move. Continue?";
    CHECK(string(ui_translate("m.move.confirm", barbs))
        == "移动会使你皮肤中的倒刺伤害你。继续吗？");
    const char *future = "New upstream movement warning. Continue?";
    CHECK(string(ui_translate("m.move.confirm", future)) == future);
    Options.language = lang_t::EN;
    CHECK(string(branch_display_name(BRANCH_DUNGEON)) == branches[BRANCH_DUNGEON].shortname);
    CHECK(ui_render("t.log", "What level of %s? (default %s, ? - help) ", original,
        {ui_text_arg("b.branch.name", branches[BRANCH_DUNGEON].longname),
         ui_literal_arg("D:2")}) == original);
    CHECK(string(ui_translate("m.move.confirm", barbs)) == barbs);
}

TEST_CASE("Tiles hover labels preserve command placeholders and English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const string cast = "[L-Click] Cast (%)\n[R-Click] Describe (%)";
    const string use = "[L-Click] Use (%)\n[R-Click] Describe";
    CHECK(string(ui_translate("t.tiles.tooltip", cast.c_str()))
        == "[鼠标左键] 施放 (%)\n[鼠标右键] 查看说明 (%)");
    CHECK(string(ui_translate("t.tiles.tooltip", use.c_str()))
        == "[鼠标左键] 使用 (%)\n[鼠标右键] 查看说明");
    const string future = "New upstream restriction\n[R-Click] Describe (%)";
    CHECK(string(ui_translate("t.tiles.tooltip", future.c_str())) == future);
    CHECK(ui_render("t.tiles.row", "%s MP    %s    (%s)",
        "1 MP    Summon Small Mammal    (10%)",
        {ui_literal_arg("1"), ui_text_arg("display.spell", "Summon Small Mammal"),
         ui_literal_arg("10%")}) == "1 魔力    召唤小型哺乳动物    (10%)");
    CHECK(string(ui_translate("l.lookup.name", "breath++ status")) == "吐息++");
    CHECK(string(ui_translate("l.lookup.name", "destr+++ status")) == "毁灭增长+++");
    CHECK(string(ui_translate("display.ability", "Clear terrain to floor")) == "清除地形为地面");
    Options.language = lang_t::EN;
    CHECK(string(ui_translate("t.tiles.tooltip", cast.c_str())) == cast);
    CHECK(string(ui_translate("t.tiles.tooltip", use.c_str())) == use);
    CHECK(string(ui_translate("l.lookup.name", "breath++ status")) == "breath++ status");
}

TEST_CASE("W help variants retain all literal command bindings", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    auto bindings = [](const string &text) {
        vector<string> result;
        for (size_t p = 0; (p = text.find("<w>", p)) != string::npos; )
        {
            const size_t end = text.find("</w>", p);
            REQUIRE(end != string::npos);
            result.push_back(text.substr(p, end + 4 - p));
            p = end + 4;
        }
        sort(result.begin(), result.end());
        return result;
    };
    int variants = 0;
    for (const auto &row : zh_ui_catalogue)
    {
        if (string(row.context) != "w.help") continue;
        ++variants;
        const string en = row.english;
        const string zh = ui_translate("w.help", row.english);
        CHECK(zh != en);
        CHECK(bindings(en) == bindings(zh));
        CHECK(count(en.begin(), en.end(), '%') == count(zh.begin(), zh.end(), '%'));
        Options.language = lang_t::EN;
        CHECK(string(ui_translate("w.help", row.english)) == en);
        Options.language = lang_t::ZH;
    }
    CHECK(variants == 6);
}

TEST_CASE("W dynamic wizard messages require complete typed components", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const string original = "Increased Fighting to skill level 3.0.";
    CHECK(ui_render("w.log", "%s %s to skill level %s.", original,
        {ui_text_arg("w.clause", "Increased"), ui_text_arg("display.skill", "Fighting"), ui_literal_arg("3.0")})
        == "提高战斗至3.0级。");
    CHECK(ui_render("w.log", "%s %s to skill level %s.", original,
        {ui_text_arg("w.clause", "Increased"), {"unknown skill", false}, ui_literal_arg("3.0")}) == original);
    const string feature = "Building '<w>new upstream feature</w>'.";
    CHECK(ui_render("w.log", "Building '<w>%s</w>'.\n[<w>.</w>] place feature and continue, [<w>ret</w>] place and exit, [<w>esc</w>] exit.", feature,
        {{"unreviewed feature", false}}) == feature);
    Options.language = lang_t::EN;
    CHECK(ui_render("w.log", "%s %s to skill level %s.", original,
        {{"提高", true}, {"战斗", true}, ui_literal_arg("3.0")}) == original);
    CHECK(rng::peek_uint64() == state);
}
TEST_CASE("Monster speech deity and alternative target markers preserve English and fallback", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<int> width(crawl_view.msgsz.x, 160);
    unwind_var<god_type> religion(you.religion, GOD_ZIN);
    monster mon; mon.type = mon.base_monster = MONS_ORC_PRIEST;
    mon.hit_points = mon.max_hit_points = 30; mon.foe = MHITYOU;
    mon.set_position(coord_def(10,10));
    const auto check = [&](const string &source, const string &display,
                           const string &wanted, bool available) {
        string expected; uint64_t next;
        { rng::subgenerator isolated(381,0);
          expected = do_mon_str_replacements(source, mon, S_SHOUT);
          next = rng::peek_uint64(); }
        rng::subgenerator isolated(381,0);
        textdb_speech pair(source, display, true);
        CHECK(do_mon_str_replacements(pair.english, mon, S_SHOUT, &pair) == expected);
        CHECK(rng::peek_uint64() == next);
        if (available) {
            CHECK(pair.complete); CHECK(pair.text() != expected);
            CHECK(pair.text().find(wanted) != string::npos);
            CHECK(pair.text().find('@') == string::npos);
        } else CHECK(pair.text() == expected);
        Options.language = lang_t::EN; CHECK(pair.text() == expected);
        Options.language = lang_t::ZH;
    };
    for (god_type god : {GOD_ZIN, GOD_NO_GOD, GOD_NAMELESS}) {
        you.religion = god;
        const string wanted = god == GOD_ZIN ? "辛" : god == GOD_NO_GOD ? "你" : "你的神";
        check("@foe_god@ and @Foe_god@.", "@foe_god@与@Foe_god@。", wanted, true);
    }
    check("@The_monster@ looks @at_foe/around@ nervously.",
          "@The_monster@紧张地看着@at_foe/around@。", "周围", true);
    // Use a real monster foe, so the alternative must not select 'around'.
    monster &target = env.mons[0];
    monster fresh_target;
    unwind_var<monster> saved_target(target, fresh_target);
    target.type = target.base_monster = MONS_ORC;
    target.hit_points = target.max_hit_points = 30;
    target.god = GOD_ZIN;
    target.set_position(coord_def(11,10));
    mon.foe = 0;
    check("@The_monster@ looks @at_foe/around@ nervously.",
          "@The_monster@紧张地看着@at_foe/around@。", "兽人", true);
    check("@foe_god@ and @Foe_god@.", "@foe_god@与@Foe_god@。", "辛", true);
    check("@random_god@ @random_god_chaotic@ @random_god_evil@ @random_god_good@ @random_god_good@.",
          "@random_god@、@random_god_chaotic@、@random_god_evil@、@random_god_good@、@random_god_good@。", "、", true);
    mon.foe = MHITNOT;
    check("@foe_god@ watches @at_foe/around@.",
          "@foe_god@看着@at_foe/around@。", "", false);
}
#ifdef USE_SQLITE_DBM
TEST_CASE("Gizmo names localize all reviewed naming candidates without changing saves or RNG", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto source=read_speech_fixture("dat/database/gizmo.txt");
    const auto target=read_speech_fixture("dat/database/zh/gizmo.txt");
    REQUIRE(source.size()==3);REQUIRE(target.size()==3);
    item_def prototype;prototype.base_type=OBJ_GIZMOS;prototype.sub_type=0;
    prototype.quantity=1;prototype.flags|=ISFLAG_IDENTIFIED;
    CrawlHashTable holder;holder["items"].new_vector(SV_ITEM);
    auto &gizmos=holder["items"].get_vector();gizmos.push_back(prototype);
    fill_gizmo_properties(gizmos);prototype=gizmos[0].get_item();
    REQUIRE(is_random_artefact(prototype));
    size_t checked=0;
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);
        const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_gizmo_display_name(canonical,shown));CHECK(shown==wanted);
        CHECK(rng::peek_uint64()==next);
        item_def item=prototype;item.props[ARTEFACT_NAME_KEY]=canonical;
        const string original=item.name(DESC_PLAIN);string display;
        REQUIRE(try_item_display_name(item,display));CHECK(display.find(wanted)!=string::npos);
        CHECK(item.props[ARTEFACT_NAME_KEY].get_string()==canonical);
        CHECK(item.name(DESC_PLAIN)==original);
        Options.language=lang_t::EN;CHECK_FALSE(try_gizmo_display_name(canonical,shown));
        CHECK_FALSE(try_item_display_name(item,display));CHECK(item.name(DESC_PLAIN)==original);
        Options.language=lang_t::ZH;
    };
    for(const char *key:{"gizmo_adjective","gizmo_noun","gizmo_modifier"}) {
        const auto left=split_string("\n\n",source.at(key));
        const auto right=split_string("\n\n",target.at(key));
        REQUIRE(left.size()==right.size());
        for(size_t i=0;i<left.size();++i) {
            const string word=replace_all(left[i],"@gizmo_modifier@","aero");
            const string translated=replace_all(right[i],"@gizmo_modifier@","气");
            if(string(key)=="gizmo_adjective") check(word+" capacitor",translated+"电容器");
            else if(string(key)=="gizmo_noun") check("automatic "+word,"自动"+translated);
            else {
                check("automatic "+word+"capacitor","自动"+translated+"电容器");
                check(word+"capacitor Mk.7",translated+"电容器 Mk.7");
                check("AB-20 "+word+"capacitor","AB-20 "+translated+"电容器");
            }
            ++checked;
        }
    }
    CHECK(checked==539);
    size_t expanded=0;
    const auto prefixes=split_string("\n\n",source.at("gizmo_modifier"));
    const auto translations=split_string("\n\n",target.at("gizmo_modifier"));
    const uint64_t before_expansion=rng::peek_uint64();
    for(const char *key:{"gizmo_adjective","gizmo_noun"}) {
        const auto words=split_string("\n\n",source.at(key));
        const auto displays=split_string("\n\n",target.at(key));
        map<string,string> literals;
        for(size_t i=0;i<words.size();++i)
            if(words[i].find('@')==string::npos) literals.emplace(words[i],displays[i]);
        for(size_t i=0;i<words.size();++i) {
            if(words[i].find('@')==string::npos) continue;
            for(size_t j=0;j<prefixes.size();++j) {
                const string word=replace_all(words[i],"@gizmo_modifier@",prefixes[j]);
                const auto literal=literals.find(word);
                const string translated=literal==literals.end()
                    ?replace_all(displays[i],"@gizmo_modifier@",translations[j]):literal->second;
                const bool adjective=string(key)=="gizmo_adjective";
                const string name=adjective?word+" capacitor":"automatic "+word;
                const string wanted=adjective?translated+"电容器":"自动"+translated;
                CAPTURE(name,wanted);string shown;
                CHECK(try_gizmo_display_name(name,shown));CHECK(shown==wanted);
                ++expanded;
            }
        }
    }
    CHECK(expanded==7878);CHECK(rng::peek_uint64()==before_expansion);

    check("microaerodynamic turbocycler","微气动态涡轮循环器");
    check("aerotransformer Ver.12","气变压器 Ver.12");
    check("aerocapacitor A-1000","气电容器 A-1000");
    for(const string name:{"aerocapacitor","automatic unknown_part","unknown capacitor",
        "A-xx aerocapacitor","aerocapacitor Mk.123","aerocapacitor Ver.x",
        "automatic capacitor extra","custom_saved_name"}) {
        string shown="sentinel";CHECK_FALSE(try_gizmo_display_name(name,shown));CHECK(shown=="sentinel");
        item_def item=prototype;item.props[ARTEFACT_NAME_KEY]=name;
        CHECK_FALSE(try_item_display_name(item,shown));CHECK(item.props[ARTEFACT_NAME_KEY].get_string()==name);
    }
    // The creation path explicitly requests canonical data in either language.
    for(const char *key:{"gizmo_adjective","gizmo_noun","gizmo_modifier"})
    for(int seed=0;seed<64;++seed) {
        string original;uint64_t next;
        {rng::subgenerator isolated(seed,0);Options.language=lang_t::EN;
         original=getMiscString(key,"",true);next=rng::peek_uint64();}
        rng::subgenerator isolated(seed,0);Options.language=lang_t::ZH;
        CHECK(getMiscString(key,"",true)==original);CHECK(rng::peek_uint64()==next);
    }
}
#endif


TEST_CASE("Randart appearances use reviewed TextDB without renaming saved items", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    auto source=read_speech_fixture("dat/database/randname.txt");
    auto target=read_speech_fixture("dat/database/zh/randname.txt");
    const auto books=read_speech_fixture("dat/database/randbook.txt");
    const auto translated_books=read_speech_fixture("dat/database/zh/randbook.txt");
    source.insert(books.begin(),books.end());target.insert(translated_books.begin(),translated_books.end());
    std::function<map<string,string>(string)> expand=[&](string key) {
        map<string,string> result;
        const auto en=split_string("\n\n",source.at(key));
        const auto zh=split_string("\n\n",target.at(key));
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];
            if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            vector<pair<string,string>> choices{{a,b}};
            while(choices.front().first.find('@')!=string::npos) {
                const string pattern=choices.front().first;
                const size_t start=pattern.find('@'),end=pattern.find('@',start+1);
                const string marker=pattern.substr(start,end-start+1);
                const auto children=expand(pattern.substr(start+1,end-start-1));
                vector<pair<string,string>> next;
                for(const auto &parent:choices) for(const auto &child:children)
                    next.emplace_back(replace_all(parent.first,marker,child.first),replace_all(parent.second,marker,child.second));
                choices=std::move(next);
            }
            for(const auto &word:choices) result.emplace(word);
        }
        return result;
    };
    const uint64_t before=rng::peek_uint64();size_t checked=0;
    for(const char *key:{"general appearance","weapon appearance","robe appearance","body armour appearance",
                        "armour appearance","ring appearance","amulet appearance","book appearance"}) {
        const auto words=expand(key);
        for(const auto &word:words) {
            CAPTURE(key,word.first);string shown;
            REQUIRE(try_randart_appearance(key,word.first,shown));CHECK(shown==word.second);++checked;
        }
    }
    CHECK(checked>1300);CHECK(rng::peek_uint64()==before);
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags&=~ISFLAG_IDENTIFIED;
    sword.props[ARTEFACT_NAME_KEY]="short sword of Secret";
    sword.props[ARTEFACT_APPEAR_KEY]="ichor-stained short sword";
    const string original=sword.name(DESC_PLAIN);const uint64_t next=rng::peek_uint64();string shown;
    REQUIRE(try_item_display_name(sword,shown,false,false));CHECK(shown=="沾有灵液的短剑");
    CHECK(sword.name(DESC_PLAIN)==original);CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()=="short sword of Secret");
    CHECK(sword.props[ARTEFACT_APPEAR_KEY].get_string()=="ichor-stained short sword");CHECK(rng::peek_uint64()==next);
    shown="untouched";CHECK_FALSE(try_randart_appearance("weapon appearance","future adjective",shown));CHECK(shown=="untouched");
    CHECK_FALSE(try_randart_appearance("book_noun","Book",shown));
    sword.props[ARTEFACT_APPEAR_KEY]="future adjective short sword";
    CHECK_FALSE(try_item_display_name(sword,shown,false,false));
    Options.language=lang_t::EN;CHECK_FALSE(try_randart_appearance("weapon appearance","ichor-stained",shown));
    for(const char *key:{"weapon appearance","body armour appearance","ring appearance","amulet appearance","book appearance"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string english=getRandNameString(key);const uint64_t expected=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==english);CHECK(rng::peek_uint64()==expected);
    }
}


TEST_CASE("Randart titles localize complete reviewed forms and preserve saved English", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto source=read_speech_fixture("dat/database/rand_all.txt");
    const auto target=read_speech_fixture("dat/database/zh/rand_all.txt");
    REQUIRE(target.size()==18);
    map<string,vector<pair<string,string>>> words;size_t candidates=0,checked=0;
    for(const auto &entry:target) {
        const auto en=split_string("\n\n",source.at(entry.first));const auto zh=split_string("\n\n",entry.second);
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            words[entry.first].emplace_back(a,b);++candidates;
        }
    }
    CHECK(candidates==584);
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_randart_title(canonical,shown));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);++checked;
    };
    for(const char *key:{"_jewellery_special_name_","_suspicion_no_conflict_name_","_time_name_","_virtue_name_","_vice_name_","_destiny_name_"})
        for(const auto &word:words.at(key)) check(word.first,word.second);
    for(const auto &word:words.at("_jewellery_animal_"))check("the "+word.first,word.second);
    for(const auto &fruit:words.at("_fruit_name_")) {
        check("the "+fruit.first,fruit.second);
        for(const auto &adj:words.at("_adjective_"))check("the "+adj.first+" "+fruit.first,adj.second+fruit.second);
    }
    for(const auto &verb:words.at("_verbing_"))check("the "+verb.first+" Gods",verb.second+"的诸神");
    for(const auto &adj:words.at("_adjective_"))check("the "+adj.first+" God",adj.second+"神");
    for(const auto &number:words.at("_number_or_qualifier_"))
        for(const auto &thing:words.at("_thing_names_"))
            check(number.first+" "+thing.first,number.first=="the" && thing.first=="Heavens"
                ? "诸天" : number.first=="the" && thing.first=="People"
                ? "人民" : number.first=="the" && thing.first=="Stars"
                ? "星辰" : number.first=="the" && thing.first=="Planets"
                ? "行星" : number.second+thing.second);
    for(const auto &verb:words.at("_verbing_"))check("the Three "+verb.first+" Stars","三"+verb.second+"的星辰");
    CHECK(checked>11000);
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags|=ISFLAG_IDENTIFIED;
    for(const auto &entry:vector<pair<string,string>>{{"Eternal Night","永夜"},{"the Three Gods","三神"},{"the Singing Gods","歌唱的诸神"},{"the Weak God","虚弱的神"},{"the Blue Banana","蓝色的香蕉"}}) {
        const string saved="short sword of "+entry.first;sword.props[ARTEFACT_NAME_KEY]=saved;
        const string canonical=sword.name(DESC_PLAIN);string shown;const uint64_t next=rng::peek_uint64();
        REQUIRE(try_item_display_name(sword,shown,false,false));CHECK(shown.find("短剑「"+entry.second+"」")!=string::npos);
        CHECK(sword.name(DESC_PLAIN)==canonical);CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()==saved);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_randart_title(entry.first,shown));CHECK_FALSE(try_item_display_name(sword,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"Future Mystery","the Three Unknowns","the Future God","the Blue Unknown Fruit","the Singing Gods extra"}) {
        string shown="untouched";CHECK_FALSE(try_randart_title(unknown,shown));CHECK(shown=="untouched");
    }
    for(const char *key:{"weapon","armour","jewellery","Trog weapon","Xom jewellery"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string english=getRandNameString(key);const uint64_t next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==english);CHECK(rng::peek_uint64()==next);
    }
}


TEST_CASE("Cross dictionary randart titles keep reviewed words gods and canonical identity", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    map<string,string> source,target;
    for(const char *file:{"rand_all.txt","rand_wpn.txt"}) {
        const auto en=read_speech_fixture((string("dat/database/")+file).c_str());
        const auto zh=read_speech_fixture((string("dat/database/zh/")+file).c_str());
        source.insert(en.begin(),en.end());target.insert(zh.begin(),zh.end());
    }
    REQUIRE(target.size()==63);
    std::function<map<string,string>(string)> expand=[&](string key) {
        map<string,string> result;
        const auto en=split_string("\n\n",source.at(key));const auto zh=split_string("\n\n",target.at(key));
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            vector<pair<string,string>> choices{{a,b}};
            while(choices.front().first.find('@')!=string::npos) {
                const string pattern=choices.front().first;const size_t start=pattern.find('@'),end=pattern.find('@',start+1);
                const string marker=pattern.substr(start,end-start+1);const auto children=expand(pattern.substr(start+1,end-start-1));
                vector<pair<string,string>> next;
                for(const auto &parent:choices) for(const auto &child:children)
                    next.emplace_back(replace_all(parent.first,marker,child.first),replace_all(parent.second,marker,child.second));
                choices=std::move(next);
            }
            for(const auto &word:choices)result.emplace(word);
        }
        return result;
    };
    size_t checked=0;
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);string shown;const uint64_t next=rng::peek_uint64();
        REQUIRE(try_randart_title(canonical,shown));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);++checked;
    };
    map<string,string> complete;
    for(const char *key:{"_suspicion_name_","_sky_or_light_","_virtue_or_vice_","_power_or_anger_",
         "_battle_or_war_","death_or_doom","_strategy_or_justice_","_evil_stuff_","_catastrophe_name_",
         "_chaos_name_","_balance_or_order_","_economics_or_politics_","_place_name_","_discomfort_weapon_",
         "_philosophy_name_","_science_name_","_magic_name_","_substance_name_"})
        for(const auto &word:expand(key)) {
            const auto inserted=complete.emplace(word);CHECK((inserted.second || inserted.first->second==word.second));
            check(word.first,word.second);
        }
    const auto combination=[&](const string &canonical,const string &composed) {
        // Expected whole-phrase precedence comes from the source/PO fixtures,
        // including Quick Death and Shrieking Terror, rather than word splitting.
        const auto found=complete.find(canonical);
        check(canonical,found==complete.end() ? composed : found->second);
    };
    for(const auto &word:expand("divine_esteem")) {
        check("Zin's "+word.first,"辛之"+word.second);
        check("Dithmenos' "+word.first,"迪斯姆诺之"+word.second);
    }
    for(const auto &word:expand("_xom_esteem_"))check("Xom's "+word.first,"佐姆之"+word.second);
    for(const auto &word:expand("_diviner_name_"))check("the Singing "+word.first,"歌唱的"+word.second);
    for(const auto &adj:expand("_adjective_"))
        for(const char *key:{"_strategy_or_justice_","_power_or_anger_","death_or_doom"})
            for(const auto &noun:expand(key))combination(adj.first+" "+noun.first,adj.second+noun.second);
    for(const auto &verb:expand("_verbing_")) {
        for(const auto &noun:expand("death_or_doom"))combination(verb.first+" "+noun.first,verb.second+"的"+noun.second);
        for(const auto &noun:expand("_battle_or_war_"))combination("the "+verb.first+" "+noun.first,verb.second+"的"+noun.second);
        check("Zin, "+verb.first,"辛之"+verb.second);
    }
    check("Quick Death","速死");check("the People","人民");check("Shrieking Terror","尖啸恐怖");
    CHECK(checked>18000);
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags|=ISFLAG_IDENTIFIED;
    for(const auto &entry:vector<pair<string,string>>{{"the Black Skies","漆黑天穹"},{"Zin's Approval","辛之赞许"},
        {"Xom's Attic","佐姆之阁楼"},{"Guile","狡诈"},{"the Ghost","鬼魂"},{"Bloodstone","血石"}}) {
        const string saved="short sword of "+entry.first;sword.props[ARTEFACT_NAME_KEY]=saved;
        const string original=sword.name(DESC_PLAIN);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_item_display_name(sword,shown,false,false));CHECK(shown.find("短剑「"+entry.second+"」")!=string::npos);
        CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()==saved);CHECK(sword.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_randart_title(entry.first,shown));CHECK_FALSE(try_item_display_name(sword,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"Unknown's Approval","Zin's Future Favour","Xom's Future Joke","the Future Skies","Future Guile","Singing Unknown"}) {
        string shown="untouched";CHECK_FALSE(try_randart_title(unknown,shown));CHECK(shown=="untouched");
    }
}


TEST_CASE("Remaining weapon and common armour titles use typed names without changing saves", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<string> player_name(you.your_name,"Ceneedgh");
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    map<string,string> source,target;
    for(const char *file:{"rand_all.txt","rand_wpn.txt","rand_arm.txt"}) {
        const auto en=read_speech_fixture((string("dat/database/")+file).c_str());
        const auto zh=read_speech_fixture((string("dat/database/zh/")+file).c_str());
        source.insert(en.begin(),en.end());target.insert(zh.begin(),zh.end());
    }
    REQUIRE(target.size()==82);
    map<string,map<string,string>> cached;
    std::function<map<string,string>(string)> expand=[&](string key) {
        if(key=="player_species")return map<string,string>{{"Human","人类"},{"Elf","精灵"}};
        if(cached.count(key))return cached.at(key);
        map<string,string> result;
        const auto en=split_string("\n\n",source.at(key));const auto zh=split_string("\n\n",target.at(key));
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            vector<pair<string,string>> choices{{a,b}};
            while(choices.front().first.find('@')!=string::npos) {
                const string pattern=choices.front().first;const size_t start=pattern.find('@'),end=pattern.find('@',start+1);
                const string marker=pattern.substr(start,end-start+1);const auto children=expand(pattern.substr(start+1,end-start-1));
                vector<pair<string,string>> next;
                for(const auto &parent:choices) for(const auto &child:children)
                    next.emplace_back(replace_all(parent.first,marker,child.first),replace_all(parent.second,marker,child.second));
                choices=std::move(next);
            }
            for(const auto &word:choices)result.emplace(word);
        }
        cached.emplace(key,result);return result;
    };
    map<string,string> complete;size_t checked=0;
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_randart_title(canonical,shown));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);++checked;
    };
    for(const char *key:{"other_weapon_name","_real_weapon_name_","_joke_weapon_name_","_flower_name_",
        "_discomfort_and_inaction_","_good_or_divine_","_armour_name_shielding_","_armour_property_name_",
        "_wacky_armour_name_","_instrument_name_","_evil_being_","_weather_name_","_game_name_",
        "_politics_name_armour_","_other_armour_name_","_plain_armour_name_"})
        for(const auto &word:expand(key)) {complete.emplace(word);check(word.first,word.second);}
    for(const char *key:{"_people_name_","_weapon_animal_","_plant_name_","_item_name_",
                         "_critter_name_","_profession_name_","_celestial_bodies_"})
        for(const auto &word:expand(key)) {
            const string name="the "+word.first;complete.emplace(name,word.second);check(name,word.second);
        }
    const auto combination=[&](const string &canonical,const string &composed) {
        const auto found=complete.find(canonical);check(canonical,found==complete.end()?composed:found->second);
    };
    for(const auto &verb:expand("_verbing_"))
        for(const char *key:{"_people_name_","_weapon_animal_"})
            for(const auto &noun:expand(key))combination("the "+verb.first+" "+noun.first,verb.second+"的"+noun.second);
    for(const auto &adj:expand("_adjective_")) {
        for(const auto &noun:expand("_good_or_divine_"))combination(adj.first+" "+noun.first,adj.second+noun.second);
        for(const char *key:{"_critter_name_","_profession_name_"})
            for(const auto &noun:expand(key))combination("the "+adj.first+" "+noun.first,adj.second+noun.second);
    }
    for(const auto &monster:expand("short_monster_name_")) {
        for(const auto &ending:expand("_short_killer_name_"))check(monster.first+ending.first,monster.second+ending.second);
        for(const auto &ending:expand("_long_killer_name_"))check(monster.first+ending.first,monster.second+ending.second);
    }
    for(const auto &monster:expand("_monster_name_"))
        for(const auto &ending:expand("killer_name"))check(monster.first+ending.first,monster.second+ending.second);
    for(const auto &ending:expand("killer_name")) {
        const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_randart_title("Ceneedgh"+ending.first,shown,"Ceneedgh"));CHECK(shown=="Ceneedgh"+ending.second);
        CHECK_FALSE(try_randart_title("Future Name"+ending.first,shown));CHECK(rng::peek_uint64()==next);
    }
    CHECK(checked>20000);
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags|=ISFLAG_IDENTIFIED;
    for(const auto &word:vector<pair<string,string>>{{"Dragonbane","龙之灾厄"},{"Dragon's Bane","龙之灾厄"},
         {"Human's Death","人类之死"},{"Killer Bee's Bane","杀人蜂之灾厄"},{"Ceneedgh's Regret","Ceneedgh之悔"},
         {"the Queen of Wands","权杖王后"},{"the Hot Ocelot","热辣豹猫"},{"Death Rattle","死亡喉鸣"}}) {
        const string saved="short sword \""+word.first+"\"";sword.props[ARTEFACT_NAME_KEY]=saved;
        const string original=sword.name(DESC_PLAIN);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_item_display_name(sword,shown,false,false));CHECK(shown.find("短剑「"+word.second+"」")!=string::npos);
        CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()==saved);CHECK(sword.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(sword,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"Futuremonsterslayer","Future Species's Bane","Unknown Player's Death","the Future Bird","the Queen of Unknown"}) {
        string shown="untouched";CHECK_FALSE(try_randart_title(unknown,shown));CHECK(shown=="untouched");
    }
}


TEST_CASE("Armour animals and reviewed book themes localize without regenerating titles", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<string> player_name(you.your_name,"Ceneedgh");
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto animals=read_speech_fixture("dat/database/rand_arm.txt");
    const auto translated_animals=read_speech_fixture("dat/database/zh/rand_arm.txt");
    REQUIRE(animals.size()==19);REQUIRE(translated_animals.size()==19);
    const auto source=read_speech_fixture("dat/database/randbook.txt");
    const auto target=read_speech_fixture("dat/database/zh/randbook.txt");
    REQUIRE(target.size()==72);
    map<string,map<string,string>> cached;
    std::function<map<string,string>(string)> expand=[&](string key) {
        lowercase(key);if(cached.count(key))return cached.at(key);
        map<string,string> result;const auto en=split_string("\n\n",source.at(key));const auto zh=split_string("\n\n",target.at(key));
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            vector<pair<string,string>> choices{{a,b}};
            while(choices.front().first.find('@')!=string::npos) {
                const string pattern=choices.front().first;const size_t start=pattern.find('@'),end=pattern.find('@',start+1);
                const string marker=pattern.substr(start,end-start+1);const auto children=expand(pattern.substr(start+1,end-start-1));
                vector<pair<string,string>> next;
                for(const auto &parent:choices)for(const auto &child:children)
                    next.emplace_back(replace_all(parent.first,marker,child.first),replace_all(parent.second,marker,child.second));
                choices=std::move(next);
            }
            for(const auto &word:choices)result.emplace(word);
        }
        cached.emplace(key,result);return result;
    };
    const auto en=split_string("\n\n",animals.at("_armour_animal_"));
    const auto zh=split_string("\n\n",translated_animals.at("_armour_animal_"));
    REQUIRE(en.size()==129);REQUIRE(zh.size()==en.size());
    for(size_t i=0;i<en.size();++i) {
        const string canonical="the "+en[i];CAPTURE(canonical);string shown;const uint64_t next=rng::peek_uint64();
        REQUIRE(try_randart_title(canonical,shown));CHECK(shown==zh[i]);CHECK(rng::peek_uint64()==next);
        REQUIRE(try_randart_title("the Blue "+en[i],shown));CHECK(shown=="蓝色的"+zh[i]);
    }
    map<string,string> subjects;
    for(const char *key:{"air","conjuration","earth","fire","hexes","ice","necromancy","alchemy","summoning","translocation","forgecraft"})
        for(const auto &word:expand(key)) {
            const auto inserted=subjects.emplace(word);CHECK((inserted.second || inserted.first->second==word.second));
        }
    size_t checked=0;
    const auto check=[&](const string &canonical,const string &wanted,const string &who="") {
        CAPTURE(canonical,wanted);string shown;const uint64_t next=rng::peek_uint64();
        REQUIRE(try_randbook_title(canonical,shown,who));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);++checked;
    };
    for(const auto &word:subjects) {
        check("Book of "+word.first,word.second+"之书");
        check("Annotations on "+word.first,word.second+"之批注");
        check("Sif Muna's Book of "+word.first,"席夫·穆纳的"+word.second+"之书");
        check("Ceneedgh's Book of "+word.first,"Ceneedgh的"+word.second+"之书","Ceneedgh");
        check("Book of "+word.first+" and Ice",word.second+"与寒冰之书");
    }
    for(const auto &noun:expand("book_noun"))check(noun.first+" of Fire","火焰之"+noun.second);
    for(const auto &noun:expand("_book_noun_on_"))check(noun.first+" on Fire","火焰之"+noun.second);
    CHECK(checked>4000);
    item_def book;book.base_type=OBJ_BOOKS;book.quantity=1;
    init_book_theme_randart(book,{SPELL_MAGIC_DART});book.flags|=ISFLAG_IDENTIFIED;
    REQUIRE(is_random_artefact(book));
    for(const auto &word:vector<pair<string,string>>{{"Book of Fire","火焰之书"},
        {"Book of Fire and Ice","火焰与寒冰之书"},{"Sif Muna's Book of Fire","席夫·穆纳的火焰之书"},
        {"Ceneedgh's Book of Fire","Ceneedgh的火焰之书"},{"Annotations on Earth","大地之批注"}}) {
        book.props[ARTEFACT_NAME_KEY]=word.first;const string original=book.name(DESC_PLAIN);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_item_display_name(book,shown,false,false));CHECK(shown==word.second);
        CHECK(book.props[ARTEFACT_NAME_KEY].get_string()==word.first);CHECK(book.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(book,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"Book of Future Theme","Unknown Author's Book of Fire","Book of the Future Traveller","Book of Fire and Future Theme","Book of Fire extra"}) {
        string shown="untouched";CHECK_FALSE(try_randbook_title(unknown,shown));CHECK(shown=="untouched");
        book.props[ARTEFACT_NAME_KEY]=unknown;CHECK_FALSE(try_item_display_name(book,shown,false,false));
    }
    Options.language=lang_t::EN;string shown;CHECK_FALSE(try_randbook_title("Book of Fire",shown));Options.language=lang_t::ZH;
    for(const char *key:{"_armour_animal_","air","necromancy","forgecraft","book_name"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string original=getRandNameString(key);const uint64_t next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==original);CHECK(rng::peek_uint64()==next);
    }
}


TEST_CASE("Reviewed book modifiers and authors preserve complete English fallback", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto source=read_speech_fixture("dat/database/randbook.txt");
    const auto target=read_speech_fixture("dat/database/zh/randbook.txt");
    REQUIRE(source.size()==72);REQUIRE(target.size()==source.size());
    map<string,map<string,string>> cached;
    std::function<map<string,string>(string)> expand=[&](string key) {
        lowercase(key);if(cached.count(key))return cached.at(key);
        map<string,string> result;const auto en=split_string("\n\n",source.at(key));const auto zh=split_string("\n\n",target.at(key));
        REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")) {const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            vector<pair<string,string>> choices{{a,b}};
            while(choices.front().first.find('@')!=string::npos) {
                const string pattern=choices.front().first;const size_t start=pattern.find('@'),end=pattern.find('@',start+1);
                const string marker=pattern.substr(start,end-start+1);const auto children=expand(pattern.substr(start+1,end-start-1));
                vector<pair<string,string>> next;
                for(const auto &parent:choices)for(const auto &child:children)
                    next.emplace_back(replace_all(parent.first,marker,child.first),replace_all(parent.second,marker,child.second));
                choices=std::move(next);
            }
            for(const auto &word:choices)result.emplace(word);
        }
        cached.emplace(key,result);return result;
    };
    size_t modifiers=0,authors=0;
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);string shown;const uint64_t next=rng::peek_uint64();
        REQUIRE(try_randbook_title(canonical,shown));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);
    };
    for(const char *key:{"air adj","conjuration adj","earth adj","fire adj","hexes adj","ice adj",
                         "necromancy adj","alchemy adj","summoning adj","translocation adj","forgecraft adj"})
        for(const auto &word:expand(key)) {
            check("Book of "+word.first+" Ice",word.second+"寒冰之书");
            check("Annotations on the "+word.first+" Traveller",word.second+"旅人之批注");
            check("Sif Muna's Book of the "+word.first+" Traveller","席夫·穆纳的"+word.second+"旅人之书");
            ++modifiers;
        }
    CHECK(modifiers==516);
    for(const auto &entry:source)
        if(ends_with(entry.first," owner") || entry.first=="_the_powerful_caster_")
            for(const auto &word:expand(entry.first)) {
                if(word.first=="__NONE")continue;
                check(apostrophise(word.first)+" Book of Fire",word.second+"的火焰之书");
                check(apostrophise(word.first)+" Book of the Fiery Traveller",word.second+"的烈火的旅人之书");
                ++authors;
            }
    CHECK(authors>70);
    item_def book;book.base_type=OBJ_BOOKS;book.quantity=1;init_book_theme_randart(book,{SPELL_MAGIC_DART});book.flags|=ISFLAG_IDENTIFIED;
    for(const auto &word:vector<pair<string,string>>{{"Book of the Fiery Traveller","烈火的旅人之书"},
        {"Lehudib's Book of Fire","雷胡迪布的火焰之书"},{"Ozocubu's Book of Ice","奥佐库布的寒冰之书"},
        {"Golubria's Book of the Blinking Traveller","格鲁布里亚的闪烁的旅人之书"}}) {
        book.props[ARTEFACT_NAME_KEY]=word.first;const string original=book.name(DESC_PLAIN);const uint64_t next=rng::peek_uint64();string shown;
        REQUIRE(try_item_display_name(book,shown,false,false));CHECK(shown==word.second);
        CHECK(book.props[ARTEFACT_NAME_KEY].get_string()==word.first);CHECK(book.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(book,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"Book of the Future Traveller","Book of Fiery the Traveller","__NONE's Book of Fire",
                             "Unknown Author's Book of the Fiery Traveller","Elementary Future Magic"}) {
        string shown="untouched";CHECK_FALSE(try_randbook_title(unknown,shown));CHECK(shown=="untouched");
        book.props[ARTEFACT_NAME_KEY]=unknown;CHECK_FALSE(try_item_display_name(book,shown,false,false));
    }
    for(const char *key:{"fire adj","forgecraft adj","hexes owner","highlevel conjuration owner","starting level book"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string original=getRandNameString(key);const uint64_t next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==original);CHECK(rng::peek_uint64()==next);
    }
}

TEST_CASE("Graffiti random personal names pair each occurrence without extra RNG", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    for (int seed = 0; seed < 128; ++seed)
    {
        string names; uint64_t next;
        {
            rng::subgenerator isolated(seed, 0);
            names = do_mon_name_replacements("@RANDGEN@/@RANDGEN@/@RANDGEN@");
            next = rng::peek_uint64();
        }
        for (lang_t mode : {lang_t::EN, lang_t::ZH}) {
            rng::subgenerator isolated(seed, 0); Options.language = mode;
            textdb_speech pair("Names: @RANDGEN@/@RANDGEN@/@RANDGEN@",
                               "署名：@RANDGEN@/@RANDGEN@/@RANDGEN@", true);
            do_decor_name_replacements(pair);
            CHECK(pair.complete);
            CHECK(pair.english == "Names: " + names);
            CHECK(pair.text() == (mode == lang_t::ZH ? "署名：" : "Names: ") + names);
            CHECK(rng::peek_uint64() == next);
        }
        for (const string &display : {string("署名：@RANDGEN@"),
                                      string("署名：@RANDGEN@/@RANDGEN@/@RANDGEN@"),
                                      string("署名：@unknown@/@RANDGEN@/@RANDGEN@")}) {
            string expected; uint64_t tail;
            {
                rng::subgenerator isolated(seed, 0);
                expected = do_mon_name_replacements("Names: @RANDGEN@/@RANDGEN@");
                tail = rng::peek_uint64();
            }
            rng::subgenerator isolated(seed, 0); Options.language = lang_t::ZH;
            textdb_speech pair("Names: @RANDGEN@/@RANDGEN@", display, true);
            do_decor_name_replacements(pair);
            CHECK(pair.text() == expected);
            CHECK(rng::peek_uint64() == tail);
        }
        rng::subgenerator isolated(seed, 0);
        textdb_speech missing("Names: @RANDGEN@", "署名：@RANDGEN@", false);
        do_decor_name_replacements(missing);
        CHECK_FALSE(missing.complete);
        CHECK(missing.text() == missing.english);
    }
}


TEST_CASE("Ashenzari title programs localize reviewed candidates without changing identities", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    map<string,string> source,target;
    for(const char *file:{"randname.txt","rand_all.txt","rand_wpn.txt","rand_arm.txt"}) {
        auto en=read_speech_fixture((string("dat/database/")+file).c_str());source.insert(en.begin(),en.end());
        auto zh=read_speech_fixture((string("dat/database/zh/")+file).c_str());target.insert(zh.begin(),zh.end());
    }
    REQUIRE(read_speech_fixture("dat/database/zh/randname.txt").size()==33);
    std::function<pair<string,string>(string,string,int)> example;
    example=[&](string a,string b,int depth) -> pair<string,string> {
        REQUIRE(depth<10);
        while(a.find('@')!=string::npos) {
            const size_t start=a.find('@'),end=a.find('@',start+1);REQUIRE(end!=string::npos);
            const string marker=a.substr(start,end-start+1);string key=a.substr(start+1,end-start-1);lowercase(key);
            string x,y;
            if(key=="god_name" || key=="god_name_possessive") {
                x=key=="god_name" ? "Ashenzari" : "Ashenzari's";y="艾申扎利";
            } else {
                REQUIRE(source.count(key));REQUIRE(target.count(key));
                x=split_string("\n\n",source.at(key)).front();y=split_string("\n\n",target.at(key)).front();
                if(starts_with(x,"w:")){const size_t n=x.find('\n')+1;x.erase(0,n);y.erase(0,n);}
                auto child=example(x,y,depth+1);x=child.first;y=child.second;
            }
            a=replace_all(a,marker,x);b=replace_all(b,marker,y);
        }
        return {a,b};
    };
    const auto check=[&](string canonical,const string &wanted) {
        if(starts_with(canonical,"of "))canonical.erase(0,3);
        else if(canonical.size()>=2 && canonical.front()=='"' && canonical.back()=='"')canonical=canonical.substr(1,canonical.size()-2);
        CAPTURE(canonical,wanted);string shown="unchanged";const auto next=rng::peek_uint64();
        REQUIRE(try_randart_title(canonical,shown));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);
    };
    size_t candidates=0;
    for(const char *key:{"ashenzari","_ashenzari_object_","_ashenzari_adjective_","_ashenzari_bound_adjective_",
                         "_ashenzari_celestial_adjective_","_ashenzari_knowledge_adjective_","_ashenzari_epithet_","_ashenzari_heavens_"}) {
        auto en=split_string("\n\n",source.at(key)),zh=split_string("\n\n",target.at(key));REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")){const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            const auto word=example(a,b,0);const string type=key;
            if(type=="ashenzari")check(word.first,word.second);
            else if(type=="_ashenzari_object_")check("the Bound "+word.first,"受缚的"+word.second);
            else if(type=="_ashenzari_epithet_")check("the "+word.first,word.second);
            else if(type=="_ashenzari_heavens_")check("the Bound "+word.first,"受缚的"+word.second);
            else check("the "+word.first+" Adversity",word.second+"逆境");
            ++candidates;
        }
    }
    CHECK(candidates==124);
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags|=ISFLAG_IDENTIFIED;
    for(const string &suffix:{string("of the Bound Adversity"),string("\"At What Cost Knowledge?\"")}) {
        const string saved="short sword "+suffix;sword.props[ARTEFACT_NAME_KEY]=saved;
        const string original=sword.name(DESC_PLAIN);const auto next=rng::peek_uint64();string shown;
        REQUIRE(try_item_display_name(sword,shown,false,false));CHECK(shown.find("短剑")!=string::npos);
        CHECK(shown.find(suffix=="of the Bound Adversity" ? "受缚的逆境" : "求知代价几何？")!=string::npos);
        CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()==saved);CHECK(sword.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(sword,shown,false,false));Options.language=lang_t::ZH;
    }
    for(const char *unknown:{"the Future Adversity","Future's Adversity","Share My Unknown", "the Bound Unknown"}) {
        string shown="untouched";CHECK_FALSE(try_randart_title(unknown,shown));CHECK(shown=="untouched");
    }
    Options.language=lang_t::EN;string shown="untouched";
    CHECK_FALSE(try_randart_title("the Bound Adversity",shown));CHECK(shown=="untouched");Options.language=lang_t::ZH;
    for(const char *key:{"ashenzari","_ashenzari_object_","_ashenzari_epithet_"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string english=getRandNameString(key);const auto next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==english);CHECK(rng::peek_uint64()==next);
    }
}


TEST_CASE("Remaining title templates preserve typed branches gods players and independent Xom verbs", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto names=read_speech_fixture("dat/database/randname.txt");
    const auto translated=read_speech_fixture("dat/database/zh/randname.txt");
    REQUIRE(names.size()==33);REQUIRE(translated.size()==names.size());
    auto en=read_speech_fixture("dat/database/rand_wpn.txt"),zh=read_speech_fixture("dat/database/zh/rand_wpn.txt");
    auto common=read_speech_fixture("dat/database/rand_all.txt"),common_zh=read_speech_fixture("dat/database/zh/rand_all.txt");
    en.insert(common.begin(),common.end());zh.insert(common_zh.begin(),common_zh.end());
    const auto words=[&](const string &key) {
        vector<pair<string,string>> result;auto a=split_string("\n\n",en.at(key)),b=split_string("\n\n",zh.at(key));REQUIRE(a.size()==b.size());
        for(size_t i=0;i<a.size();++i){if(starts_with(a[i],"w:")){size_t n=a[i].find('\n')+1;a[i].erase(0,n);b[i].erase(0,n);}result.emplace_back(a[i],b[i]);}
        return result;
    };
    const string player="Ceneedgh";
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);string shown="untouched";const auto next=rng::peek_uint64();
        REQUIRE(try_randart_title(canonical,shown,player));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);
    };
    size_t branches_checked=0;
    for(int i=0;i<NUM_BRANCHES;++i) {
        const string key=branches[i].longname;const auto label=ui_text_arg("b.branch.name",key);
        if(!key.empty() && label.available){check(key,label.text);++branches_checked;}
    }
    CHECK(branches_checked>20);
    for(const char *god:{"Trog","Okawaru","Xom","Ashenzari"}) {
        const auto label=ui_text_arg("display.god",god);REQUIRE(label.available);check(god,label.text);
    }
    const auto player_word=[&](const pair<string,string> &word) {
        string wanted="Ceneedgh之"+word.second;
        for(const char *key:{"killer_name","_long_killer_name_"})
            for(const auto &ending:words(key))
                if(ending.first=="'s "+word.first) wanted="Ceneedgh"+ending.second;
        check("Ceneedgh's "+word.first,wanted);
    };
    for(const auto &word:words("_valour_name_"))player_word(word);
    for(const auto &word:words("death_or_doom"))player_word(word);
    for(const auto &word:words("_anger_quality_"))check("Trog's "+word.first,"特洛戈之"+word.second);
    const auto verbs=words("_verbing_");REQUIRE(verbs.size()>10);
    for(size_t i=0;i<12;++i) {
        const auto &a=verbs[i],&b=verbs[(i+1)%verbs.size()];
        check("Xom, "+a.first+" and "+b.first,"佐姆之"+a.second+"与"+b.second);
        check("Xom, "+a.first+" and "+a.first,"佐姆之"+a.second+"与"+a.second);
        check(a.first+" Xom",a.second+"的佐姆");
        check("the "+a.first+" Apple",a.second+"的苹果");
    }
    for(const char *unknown:{"Unknown's Courage","Ceneedgh's Future Courage","Xom, Ageing and Unknown",
                             "the Unknown Branch","Unknown Xom","Ageing\nApple"}) {
        string shown="untouched";CHECK_FALSE(try_randart_title(unknown,shown,player));CHECK(shown=="untouched");
    }
    string shown="untouched";CHECK_FALSE(try_randart_title("Ceneedgh's Courage",shown,"Another"));
    for(const char *key:{"weapon","talisman","armour","jewellery","trog weapon","okawaru weapon","okawaru robe",
                         "okawaru body armour","okawaru armour","xom weapon","xom robe","xom body armour","xom armour","xom jewellery"})
    for(int seed=0;seed<32;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string original=getRandNameString(key);const auto next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==original);CHECK(rng::peek_uint64()==next);
    }
    item_def sword;sword.base_type=OBJ_WEAPONS;sword.sub_type=WPN_SHORT_SWORD;sword.quantity=1;
    REQUIRE(make_item_randart(sword,true));sword.flags|=ISFLAG_IDENTIFIED;
    for(const string &suffix:{string("of the Dungeon"),string("of Ceneedgh's Courage"),string("\"Xom, Ageing and Baking\"")}) {
        const string saved="short sword "+suffix;sword.props[ARTEFACT_NAME_KEY]=saved;
        const string original=sword.name(DESC_PLAIN);const auto next=rng::peek_uint64();string display;
        unwind_var<string> player_name(you.your_name,player);
        REQUIRE(try_item_display_name(sword,display,false,false));CHECK(display.find("短剑")!=string::npos);
        CHECK(display.find("Courage")==string::npos);CHECK(display.find("Ageing")==string::npos);CHECK(display.find("Dungeon")==string::npos);
        CHECK(sword.props[ARTEFACT_NAME_KEY].get_string()==saved);CHECK(sword.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(sword,display,false,false));Options.language=lang_t::ZH;
    }
}


TEST_CASE("Fixed level book programs preserve authors articles levels and English identities", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_spell_descs();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    const auto source=read_speech_fixture("dat/database/randbook.txt");
    const auto target=read_speech_fixture("dat/database/zh/randbook.txt");
    std::function<pair<string,string>(string,string,int)> example;
    example=[&](string a,string b,int depth) -> pair<string,string> {
        REQUIRE(depth<10);
        while(a.find('@')!=string::npos) {
            const size_t start=a.find('@'),end=a.find('@',start+1);REQUIRE(end!=string::npos);
            const string marker=a.substr(start,end-start+1);string key=a.substr(start+1,end-start-1);lowercase(key);
            string x,y;
            if(key=="level"){x="Three";y="三";}
            else if(key=="player_name"){x=y="Ceneedgh";}
            else {
                REQUIRE(source.count(key));REQUIRE(target.count(key));
                x=split_string("\n\n",source.at(key)).front();y=split_string("\n\n",target.at(key)).front();
                if(starts_with(x,"w:")){const size_t n=x.find('\n')+1;x.erase(0,n);y.erase(0,n);}
                auto child=example(x,y,depth+1);x=child.first;y=child.second;
            }
            a.replace(start,marker.size(),x);const size_t shown=b.find(marker);REQUIRE(shown!=string::npos);b.replace(shown,marker.size(),y);
        }
        return {a,b};
    };
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);string shown="untouched";const auto next=rng::peek_uint64();
        REQUIRE(try_randbook_title(canonical,shown,"Ceneedgh"));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);
    };
    size_t candidates=0;
    for(const char *key:{"starting level book","easy level book","moderate level book","_difficult level book owner_","difficult level book"}) {
        const auto en=split_string("\n\n",source.at(key)),zh=split_string("\n\n",target.at(key));REQUIRE(en.size()==zh.size());
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")){const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            auto word=example(a,b,0);word.first=uppercase_first(word.first);check(word.first,word.second);
            string owned=word.first;
            for(const char *article:{"The ","A ","An "})if(starts_with(owned,article)){owned.erase(0,string(article).size());break;}
            check("Sif Muna's "+owned,"席夫·穆纳的"+word.second);++candidates;
        }
    }
    CHECK(candidates>70);
    const char *english[]={"One","Two","Three","Four","Five","Six","Seven","Eight","Nine"};
    const char *chinese[]={"一","二","三","四","五","六","七","八","九"};
    for(int i=0;i<9;++i) {
        check(string("Magic, Part ")+english[i],string("魔法，第")+chinese[i]+"篇");
        check(string("Boris's Magic, Volume ")+english[i],string("鲍里斯的魔法，第")+chinese[i]+"卷");
    }
    for(const char *key:{"_book_magic_","_the_book_magic_"}) {
        const auto en=split_string("\n\n",source.at(key)),zh=split_string("\n\n",target.at(key));
        for(size_t i=0;i<en.size();++i) {
            string a=en[i],b=zh[i];if(starts_with(a,"w:")){const size_t n=a.find('\n')+1;a.erase(0,n);b.erase(0,n);}
            const auto word=example(a,b,0);
            if(string(key)=="_book_magic_")check("Elementary "+word.first,"初等"+word.second);
            else check("An Introduction to "+word.first,word.second+"介绍");
        }
    }
    check("Ceneedgh's Elementary Magic","Ceneedgh的初等魔法");
    check("A Book from Sif Muna to Ceneedgh","席夫·穆纳赠予Ceneedgh的书");
    for(const char *unknown:{"Elementary Future Magic","Unknown Author's Elementary Magic","Magic, Part Ten",
                             "Magic, Volume 4","A Book from Sif Muna to Another","Magic\n101"}) {
        string shown="untouched";CHECK_FALSE(try_randbook_title(unknown,shown,"Ceneedgh"));CHECK(shown=="untouched");
    }
    string shown="untouched";CHECK_FALSE(try_randbook_title("Ceneedgh's Elementary Magic",shown,"Another"));
    item_def book;book.base_type=OBJ_BOOKS;book.quantity=1;REQUIRE(make_book_level_randart(book,1));book.flags|=ISFLAG_IDENTIFIED;
    for(const string &title:{string("Elementary Magic"),string("Sif Muna's Introduction to Magic"),string("Boris's Magic, Volume Nine")}) {
        book.props[ARTEFACT_NAME_KEY]=title;const string original=book.name(DESC_PLAIN);const auto next=rng::peek_uint64();string display;
        REQUIRE(try_item_display_name(book,display,false,false));CHECK(display!=original);CHECK(display.find("Magic")==string::npos);
        CHECK(book.props[ARTEFACT_NAME_KEY].get_string()==title);CHECK(book.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(book,display,false,false));Options.language=lang_t::ZH;
    }
    for(const char *key:{"starting level book","easy level book","moderate level book","difficult level book"})
    for(int seed=0;seed<32;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string canonical=getRandNameString(key);const auto next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==canonical);CHECK(rng::peek_uint64()==next);
    }
    Options.language=lang_t::EN;CHECK_FALSE(try_randbook_title("Elementary Magic",shown,"Ceneedgh"));
}


TEST_CASE("Random and Xom book subjects reuse reviewed programs and canonical player names", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_spell_descs();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<string> player_name(you.your_name,"Ceneedgh");
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    map<string,string> source,target;
    for(const char *file:{"randbook.txt","randname.txt","rand_all.txt","rand_arm.txt","rand_wpn.txt"}) {
        const auto a=read_speech_fixture((string("dat/database/")+file).c_str());source.insert(a.begin(),a.end());
        const auto b=read_speech_fixture((string("dat/database/zh/")+file).c_str());target.insert(b.begin(),b.end());
    }
    std::function<pair<string,string>(string,string,int)> example;
    example=[&](string a,string b,int depth) -> pair<string,string> {
        REQUIRE(depth<16);
        while(a.find('@')!=string::npos) {
            const size_t start=a.find('@'),end=a.find('@',start+1);REQUIRE(end!=string::npos);
            const string marker=a.substr(start,end-start+1);string key=a.substr(start+1,end-start-1);lowercase(key);
            string x,y;
            if(key=="player_death"){x="Ceneedgh@killer_name@";y=x;}
            else {
                REQUIRE(source.count(key));REQUIRE(target.count(key));
                x=split_string("\n\n",source.at(key)).front();y=split_string("\n\n",target.at(key)).front();
                if(starts_with(x,"w:")){const size_t n=x.find('\n')+1;x.erase(0,n);y.erase(0,n);}
            }
            auto child=example(x,y,depth+1);
            a.replace(start,marker.size(),child.first);const size_t pos=b.find(marker);REQUIRE(pos!=string::npos);b.replace(pos,marker.size(),child.second);
        }
        return {a,b};
    };
    const auto check=[&](const string &canonical,const string &wanted) {
        CAPTURE(canonical,wanted);string shown="untouched";const auto next=rng::peek_uint64();
        REQUIRE(try_randbook_title(canonical,shown,"Ceneedgh"));CHECK(shown==wanted);CHECK(rng::peek_uint64()==next);
    };
    size_t roots=0;
    for(const char *key:{"random_book_title","xom_book_title"}) {
        const auto a=split_string("\n\n",source.at(key)),b=split_string("\n\n",target.at(key));REQUIRE(a.size()==b.size());
        for(size_t i=0;i<a.size();++i) {
            string en=a[i],zh=b[i];if(starts_with(en,"w:")){const size_t n=en.find('\n')+1;en.erase(0,n);zh.erase(0,n);}
            const auto value=example(en,zh,0);
            if(string(key)=="random_book_title") {
                check("Book of "+value.first,value.second+"之书");
                check("Sif Muna's Book of "+value.first,"席夫·穆纳的"+value.second+"之书");
            }
            check("Xom's Book of "+value.first,"佐姆的"+value.second+"之书");++roots;
        }
    }
    CHECK(roots==19);
    const auto endings=split_string("\n\n",source.at("killer_name"));
    const auto translated_endings=split_string("\n\n",target.at("killer_name"));
    for(size_t i=0;i<endings.size();++i)
        check("Xom's Book of Ceneedgh"+endings[i],"佐姆的Ceneedgh"+translated_endings[i]+"之书");
    for(const char *key:{"_jewellery_animal_","_critter_name_","_plant_name_","_weapon_animal_","_armour_animal_"}) {
        const auto a=split_string("\n\n",source.at(key)),b=split_string("\n\n",target.at(key));REQUIRE(a.size()==b.size());
        for(size_t i:{size_t(0),a.size()/2,a.size()-1}) {
            string en=a[i],zh=b[i];if(starts_with(en,"w:")){const size_t n=en.find('\n')+1;en.erase(0,n);zh.erase(0,n);}
            const auto value=example(en,zh,0);check("Book of the "+value.first,value.second+"之书");
        }
    }
    for(const char *unknown:{"Book of the Future Traveller","Xom's Book of the Future Traveller",
                             "Unknown Author's Book of the Dolphin","Xom's Book of Another's Death",
                             "Book of Ceneedgh's Death","Sif Muna's Book of Ceneedgh's Death",
                             "Annotations on the Dolphin"}) {
        string shown="untouched";CHECK_FALSE(try_randbook_title(unknown,shown,"Ceneedgh"));CHECK(shown=="untouched");
    }
    item_def book;book.base_type=OBJ_BOOKS;book.quantity=1;init_book_theme_randart(book,{SPELL_MAGIC_DART});book.flags|=ISFLAG_IDENTIFIED;
    for(const string &title:{string("Book of the Dolphin"),string("Xom's Book of Ceneedgh's Death")}) {
        book.props[ARTEFACT_NAME_KEY]=title;const string original=book.name(DESC_PLAIN);const auto next=rng::peek_uint64();string display;
        REQUIRE(try_item_display_name(book,display,false,false));CHECK(display.find("Dolphin")==string::npos);CHECK(display.find("Death")==string::npos);
        CHECK(book.props[ARTEFACT_NAME_KEY].get_string()==title);CHECK(book.name(DESC_PLAIN)==original);CHECK(rng::peek_uint64()==next);
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(book,display,false,false));Options.language=lang_t::ZH;
    }
    for(const char *key:{"random_book_title","Xom_book_title","_book_plant_animal_"})
    for(int seed=0;seed<64;++seed) {
        Options.language=lang_t::EN;rng::seed(seed);const string original=getRandNameString(key);const auto next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);CHECK(getRandNameString(key)==original);CHECK(rng::peek_uint64()==next);
    }
    Options.language=lang_t::EN;string shown="untouched";CHECK_FALSE(try_randbook_title("Book of the Dolphin",shown,"Ceneedgh"));
}


TEST_CASE("Deep trolls use the unified Chinese names and preserve canonical English", "[localization]")
{
    init_monsters();unwind_var<lang_t> language(Options.language,lang_t::ZH);
    for(const auto &entry:vector<pair<monster_type,string>>{{MONS_DEEP_TROLL,"地底巨魔"},
        {MONS_DEEP_TROLL_EARTH_MAGE,"地底巨魔土法师"},{MONS_DEEP_TROLL_SHAMAN,"地底巨魔萨满"}}) {
        monster_info info(entry.first);const string original=info.full_name(DESC_PLAIN);const auto next=rng::peek_uint64();string shown;
        REQUIRE(info.try_display_name(shown));CHECK(shown==entry.second);CHECK(info.full_name(DESC_PLAIN)==original);
        Options.language=lang_t::EN;CHECK_FALSE(info.try_display_name(shown));CHECK(info.full_name(DESC_PLAIN)==original);
        Options.language=lang_t::ZH;CHECK(rng::peek_uint64()==next);
    }
}

TEST_CASE("Wizard overview localizes god names while retaining piety values and English mode", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_duration_index();
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    unwind_var<transformation> form(you.form,transformation::none);
    unwind_var<uint8_t> piety(you.raw_piety,123);
    unwind_var<int> pool(you.duration[DUR_PIETY_POOL],7);
    unwind_var<god_type> religion(you.religion,GOD_SIF_MUNA);
    unwind_var<bool> wizard(you.wizard,true);
    const auto next=rng::peek_uint64();
    for(god_type god:{GOD_SIF_MUNA,GOD_TROG,GOD_GOZAG,GOD_ASHENZARI}) {
        you.religion=god;const string canonical=god_name(god);
        Options.language=lang_t::ZH;const auto name=ui_text_arg("display.god",canonical);REQUIRE(name.available);
        const string chinese=dump_overview_screen();CHECK(chinese.find(name.text)!=string::npos);CHECK(chinese.find(canonical)==string::npos);
#ifdef WIZARD
        CHECK(chinese.find("123 (7)")!=string::npos);
#endif
        Options.language=lang_t::EN;const string english=dump_overview_screen();CHECK(english.find(canonical)!=string::npos);
#ifdef WIZARD
        CHECK(english.find(canonical+" 123 (7)")!=string::npos);
#endif
        CHECK(you.raw_piety==123);CHECK(you.duration[DUR_PIETY_POOL]==7);CHECK(god_name(god)==canonical);
    }
    CHECK(rng::peek_uint64()==next);
}


TEST_CASE("Reported unique corpses and custom ring shops preserve their canonical identities", "[localization]")
{
    init_properties();init_monsters();unwind_var<lang_t> language(Options.language,lang_t::ZH);
    item_def corpse;corpse.base_type=OBJ_CORPSES;corpse.sub_type=CORPSE_BODY;corpse.quantity=1;
    corpse.mon_type=MONS_HUMAN;corpse.orig_monnum=MONS_CASSANDRA;corpse.freshness=FRESHEST_CORPSE;
    corpse.props[CORPSE_NAME_KEY]="Cassandra";corpse.props[CORPSE_NAME_TYPE_KEY].get_int64()=0;
    const auto next=rng::peek_uint64();const string original=corpse.name(DESC_PLAIN);string shown;
    REQUIRE(try_item_display_name(corpse,shown,false,false));CHECK(shown.find("卡珊德拉")!=string::npos);
    CHECK(shown.find("Cassandra")==string::npos);CHECK(corpse.name(DESC_PLAIN)==original);
    CHECK(corpse.props[CORPSE_NAME_KEY].get_string()=="Cassandra");
    corpse.orig_monnum=MONS_HUMAN;REQUIRE(try_item_display_name(corpse,shown,false,false));CHECK(shown.find("Cassandra")!=string::npos);
    corpse.orig_monnum=MONS_CASSANDRA;corpse.props[CORPSE_NAME_KEY]="Custom Name";
    REQUIRE(try_item_display_name(corpse,shown,false,false));CHECK(shown.find("Custom Name")!=string::npos);
    shop_struct shop;shop.type=SHOP_JEWELLERY;shop.shop_name="Oxtytz";shop.shop_type_name="One-of-a-Kind";shop.shop_suffix_name="Rings";
    ui_arg display;const string shop_en=shop_name(shop,&display);CHECK(shop_en=="Oxtytz's One-of-a-Kind Rings");
    REQUIRE(display.available);CHECK(display.text.find("独一无二")!=string::npos);CHECK(display.text.find("戒指店")!=string::npos);
    CHECK(shop_name(shop)==shop_en);shop.shop_suffix_name="Unknown Goods";shop_name(shop,&display);CHECK_FALSE(display.available);
    Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(corpse,shown,false,false));CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported combat and travel parameters translate or preserve complete English", "[localization]")
{
    init_properties();init_monsters();unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<int> health(you.hp,30);const auto next=rng::peek_uint64();
    const auto knock=ui_render("a.actor","%s is knocked back%s.","You are knocked back by the lance of force.",
        {ui_actor_arg(you),ui_text_arg("a.knockback.cause","lance of force")});
    CHECK(knock.find("力场长枪")!=string::npos);CHECK(knock.find("knocked")==string::npos);
    const auto prompt=ui_render("p.prompt","Really %s into that cloud of %s?","Really rampage into that cloud of flame?",
        {ui_text_arg("p.move","rampage"),ui_text_arg("c.cloud.name","flame")});CHECK(prompt.find("横冲直撞")!=string::npos);
    const auto branch=ui_text_arg("b.branch.name",branches[BRANCH_SWAMP].longname);REQUIRE(branch.available);
    CHECK(ui_render("t.log","A malevolent force fills %s and a teleportation trap spontaneously manifests!","English",{branch}).find("沼泽")!=string::npos);
    for(const auto &word:vector<pair<string,string>>{{"marshlight","沼泽幽光"},{"splinters","木屑"},{"flames","火焰"}}) {
        const auto value=ui_text_arg(word.first=="marshlight" ? "display.monster" : "log.projectile.name",word.first);
        REQUIRE(value.available);CHECK(value.text==word.second);
    }
    CHECK(string(ui_translate("s.forest","The forest abruptly stops moving."))=="森林突然停止了活动。");
    item_def scales;scales.base_type=OBJ_ARMOUR;scales.sub_type=ARM_ICE_DRAGON_HIDE;scales.quantity=1;scales.plus=7;
    const auto flags=scales.flags;const string original=scales.name(DESC_THE);
    const auto label=ui_text_arg("display.item",scales.name(DESC_DBNAME,true,false,false));REQUIRE(label.available);
    const auto log=ui_render("m.log.165","%s %s intact enough to wear.",original,{label,ui_literal_arg("")});
    CHECK(log.find("intact")==string::npos);CHECK(log.find("+7")==string::npos);CHECK(scales.flags==flags);
    bolt beam;beam.name="wavering orb of destruction";beam.hit_verb="hits";beam.flavour=beam.real_flavour=BEAM_DESTRUCTION;
    string hit;REQUIRE(try_beam_hit_display("The %s %s %s%s%s%s",beam,you,false,"!!",hit));
    CHECK(hit.find("摇曳的毁灭之球")!=string::npos);CHECK(hit.find("！！")!=string::npos);
    beam.name="new unknown orb";CHECK_FALSE(try_beam_hit_display("The %s %s %s%s%s%s",beam,you,false,"!!",hit));
    Options.language=lang_t::EN;CHECK(ui_render("p.prompt","Really %s into that cloud of %s?","Original",{ui_literal_arg(""),ui_literal_arg("")})=="Original");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported Margery eye glow uses the selected reviewed TextDB colour and actor", "[localization]")
{
    init_monsters();unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    struct Scope {Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}} databases;
    monster margery;margery.type=margery.base_monster=MONS_MARGERY;margery.hit_points=margery.max_hit_points=10;margery.set_position(coord_def(10,10));
    size_t eyes=0;
    for(int seed=0;seed<64;++seed) {
        rng::subgenerator isolated(seed,0);auto pair=getPairedCastString("_Margery_common_");
        if(pair.english.find("eyes start to glow")==string::npos)continue;
        CAPTURE(seed,pair.translated,pair.complete);
        do_mon_str_replacements(pair.english,margery,S_SHOUT,&pair);
        CHECK(pair.complete);CHECK(pair.text().find("玛格丽")!=string::npos);CHECK(pair.text().find("双眼")!=string::npos);
        CHECK(pair.text().find("eyes")==string::npos);++eyes;
    }
    CHECK(eyes>0);
}

TEST_CASE("Morgue sections isolate canonical English and localize live and dead records", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index(); init_spell_descs();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<god_type> religion(you.religion, GOD_NO_GOD);
    unwind_var<transformation> form(you.form, transformation::tree);
    unwind_var<decltype(you.experience_level)> xl(you.experience_level, 1);
    unwind_var<decltype(you.max_level)> max_xl(you.max_level, 1);
    unwind_var<decltype(you.spell_no)> spell_no(you.spell_no, 0);
    unwind_var<decltype(you.spells)> spells(you.spells);
    unwind_var<decltype(you.spell_library)> library(you.spell_library);
    unwind_var<decltype(you.spell_letter_table)> letters(you.spell_letter_table);
    unwind_var<bool> all_notes(Options.note_all_skill_levels, true);
    unwind_var<decltype(you.inv)> inventory(you.inv);
    unwind_var<decltype(you.action_count)> counts(you.action_count);
    unwind_var<vector<Note>> notes(note_list, {});
    unwind_var<vector<string>> order(Options.dump_order,
        {"header", "stats", "location", "inventory", "spells", "transform", "notes", "action_counts"});
    you.spell_library.reset();
    for (auto &spell : you.spells) spell = SPELL_NO_SPELL;
    for (auto &item : you.inv) item.clear();
    item_def &sword = you.inv[0];
    sword.base_type = OBJ_WEAPONS; sword.sub_type = WPN_LONG_SWORD;
    sword.quantity = 1; sword.plus = 1; sword.link = 0;
    sword.flags = ISFLAG_IDENTIFIED;
    sword.orig_place = level_id(BRANCH_DUNGEON, 2);
    Note note(NOTE_GAIN_SKILL, SK_FIGHTING, 3);
    note.place = level_id(BRANCH_DUNGEON, 2); note.turn = 23;
    note_list.push_back(note);
    you.action_count.clear();
    count_action(CACT_MELEE, WPN_LONG_SWORD);
    count_action(CACT_ATTACK, ATTACK_LUNGE);
    count_action(CACT_FORM, static_cast<int>(transformation::slaughter));
    const auto random = rng::peek_uint64();
    const auto flags = sword.flags;
    const string original_item = sword.name(DESC_INVENTORY_EQUIP);
    const string canonical = character_dump_text();
    CHECK(canonical.find("AC:") != string::npos);
    CHECK(canonical.find("rFire") != string::npos);
    CHECK(canonical.find("Reached skill level 3 in Fighting") != string::npos);
    CHECK(canonical.find("long sword") != string::npos);
    CHECK(canonical.find("护甲值") == string::npos);
    CHECK(Options.language == lang_t::ZH);
    CHECK(string(Options.lang_name) == "zh");
    const string shown = character_dump_text(true);
    CHECK(shown.find("护甲值") != string::npos);
    CHECK(shown.find("长剑") != string::npos);
    CHECK(shown.find("long sword") == string::npos);
    CHECK(shown.find("突刺") != string::npos);
    CHECK(shown.find("屠戮") != string::npos);
    CHECK(shown.find(note.display_describe(false, false, true)) != string::npos);
    CHECK(shown.find(get_form()->display_description()) != string::npos);
    scorefile_entry dead;
    const string obituary = character_dump_text(true, false, &dead);
    CHECK(obituary.find("你当时尚未掌握任何法术。") != string::npos);
    CHECK(obituary.find("你当时的法术库为空。") != string::npos);
    CHECK(obituary.find(get_form()->display_description(true)) != string::npos);
    CHECK(shown.find("地下城：2") != string::npos);
    CHECK(character_dump_text() == canonical);
    you.spells[0] = SPELL_MAGIC_DART;
    you.spell_no = 1;
    you.spell_letter_table[0] = 0;
    you.spell_library.set(SPELL_MAGIC_DART);
    const string spell_dump = character_dump_text(true);
    CHECK(spell_dump.find("魔法飞弹") != string::npos);
    CHECK(spell_dump.find("Magic Dart") == string::npos);
    you.spells[0] = SPELL_NO_SPELL;
    you.spell_no = 0;
    you.spell_library.reset();
    Options.language = lang_t::EN;
    CHECK(character_dump_text(true) == canonical);
    CHECK(sword.flags == flags);
    CHECK(sword.name(DESC_INVENTORY_EQUIP) == original_item);
    CHECK(you.action_count.at({CACT_MELEE, caction_compound(WPN_LONG_SWORD)})[0] == 1);
    CHECK(rng::peek_uint64() == random);
}

TEST_CASE("Morgue bilingual files keep original paths and execute Lua once per section", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang_name(Options.lang_name, "zh");
    unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/morgue-01-runtime");
    unwind_var<int> gold(you.attribute[ATTR_GOLD_FOUND], 123);
    unwind_var<vector<string>> order(Options.dump_order,
        {"header", "gold", "morgue01section", "morgue01section"});
    lua_State *ls = clua.state();
    lua_stack_cleaner stack(ls);
    struct restore_globals
    {
        lua_State *ls;
        vector<pair<string,int>> saved;
        restore_globals(lua_State *state) : ls(state)
        {
            for (const char *name : {"morgue01", "morgue01section"})
            {
                lua_getglobal(ls, name);
                saved.emplace_back(name, luaL_ref(ls, LUA_REGISTRYINDEX));
            }
        }
        ~restore_globals()
        {
            for (const auto &value : saved)
            {
                lua_rawgeti(ls, LUA_REGISTRYINDEX, value.second);
                lua_setglobal(ls, value.first.c_str());
                luaL_unref(ls, LUA_REGISTRYINDEX, value.second);
            }
        }
    } restore(ls);
    REQUIRE(clua.execstring("morgue01={calls=0}; function morgue01section() "
        "morgue01.calls=morgue01.calls+1; return 'custom Lua text\\n' end") == 0);
    const auto random = rng::peek_uint64();
    REQUIRE(dump_char("native-bilingual", true));
    REQUIRE(clua.execstring("assert(morgue01.calls==2)") == 0);
    const auto read = [&](const string &name) {
        std::ifstream file(morgue_directory() + name);
        return string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    };
    const string english = read("native-bilingual.txt");
    const string chinese = read("native-bilingual.zh.txt");
    CHECK(english.find("You have collected 123 gold pieces.") != string::npos);
    CHECK(english.find("你共收集") == string::npos);
    CHECK(chinese.find("你共收集了 123 枚金币。") != string::npos);
    CHECK(count_occurrences(english, "custom Lua text") == 2);
    CHECK(count_occurrences(chinese, "custom Lua text") == 2);
    CHECK(std::ifstream(morgue_directory() + "native-bilingual.lst").good());
    CHECK(character_dump_text() == english);
    REQUIRE(dump_char("native-explicit-english", true, false, nullptr, false));
    CHECK(read("native-explicit-english.txt") == english);
    CHECK_FALSE(std::ifstream(morgue_directory() + "native-explicit-english.zh.txt").good());
    Options.language = lang_t::EN;
    REQUIRE(dump_char("native-language-english", true));
    CHECK(read("native-language-english.txt") == english);
    CHECK_FALSE(std::ifstream(morgue_directory() + "native-language-english.zh.txt").good());
    CHECK(string(Options.lang_name) == "zh");
    CHECK(rng::peek_uint64() == random);
}

TEST_CASE("Morgue reviewed conduct and place fields retain missing-input English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    for (int c = 1; c < NUM_CONDUCTS; ++c)
    {
        const string original = conduct_description(static_cast<conduct_type>(c));
        CHECK(ui_text_arg("morgue.conduct", original).available);
    }
    CHECK(string(ui_translate("morgue.conduct", "Future conduct")) == "Future conduct");
    const string original = "Future:7";
    CHECK(ui_render("morgue.place", "%s:%s", original,
        {ui_text_arg("b.branch.name", "Future"), ui_literal_arg("7")}) == original);
    CHECK(ui_render("morgue.place", "%s:%s", "D:7",
        {ui_text_arg("b.branch.name", branches[BRANCH_DUNGEON].shortname),
         ui_literal_arg("7")}) == "地下城：7");
}

namespace
{
struct search_annotation_scope
{
    lua_State *ls = clua.state();
    vector<pair<string,int>> saved;
    search_annotation_scope()
    {
        for (const char *name : {STASH_LUA_SEARCH_ANNOTATE, "search_zh_calls"})
        {
            lua_getglobal(ls,name);
            saved.emplace_back(name,luaL_ref(ls,LUA_REGISTRYINDEX));
        }
    }
    ~search_annotation_scope()
    {
        for (const auto &value : saved)
        {
            lua_rawgeti(ls,LUA_REGISTRYINDEX,value.second);
            lua_setglobal(ls,value.first.c_str());
            luaL_unref(ls,LUA_REGISTRYINDEX,value.second);
        }
    }
};
}

TEST_CASE("Chinese stash aliases preserve canonical annotations and unknown item boundaries", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index(); init_spell_descs();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    const auto random=rng::peek_uint64();
    const string english="{artefact} {Long Blades} {rF++ rC-} {Slay+3} {in_shop} {custom-tag}";
    const string aliases=stash_prefix_aliases(english);
    for (const char *word : {"{神器}","{长刃}","{火抗++}","{冰抗-}","{杀戮+3}","{商店内}"})
        CHECK(aliases.find(word)!=string::npos);
    CHECK(aliases.find("custom-tag")==string::npos);
    CHECK(stash_prefix_aliases("{not a weapon}").empty());
    for (const char *ego : {"RegenMP","Rampage","Ponderous","Spirit","Wildshape","Wiz","MP+9"})
        CHECK_FALSE(stash_prefix_aliases("{"+string(ego)+"}").empty());
    CHECK(stash_prefix_aliases("unknown-menu-token",true).empty());
    CHECK(stash_prefix_aliases("identified melee weapon",true).find("{已鉴定}")!=string::npos);
    item_def sword; sword.base_type=OBJ_WEAPONS; sword.sub_type=WPN_LONG_SWORD;
    sword.quantity=1; sword.flags=ISFLAG_IDENTIFIED;
    const string name=sword.name(DESC_PLAIN);
    const auto flags=sword.flags;
    const string bilingual=stash_item_aliases(sword,"{weapon} {Long Blades}");
    CHECK(bilingual.find("长剑")!=string::npos);
    CHECK(bilingual.find("长刃")!=string::npos);
    item_def potion; potion.base_type=OBJ_POTIONS; potion.sub_type=POT_CURING;
    potion.quantity=1; potion.subtype_rnd=0;
    const string unknown=stash_item_aliases(potion,"{potion}");
    CHECK(unknown.find("治疗")==string::npos);
    potion.flags=ISFLAG_IDENTIFIED;
    CHECK(stash_item_aliases(potion,"{potion}").find("治疗药水")!=string::npos);
    item_def book; book.base_type=OBJ_BOOKS; book.sub_type=BOOK_CONJURATIONS;
    book.quantity=1;
    CHECK(book.is_identified()); // Spellbooks have no unidentified appearance.
    CHECK(stash_item_aliases(book,"{book}").find("魔法飞弹")!=string::npos);
    book.flags=ISFLAG_IDENTIFIED;
    CHECK(stash_item_aliases(book,"{book}").find("魔法飞弹")!=string::npos);
    for (lang_t lang : {lang_t::EN,lang_t::FR})
    {
        Options.language=lang;
        CHECK(stash_prefix_aliases(english).empty());
        CHECK(stash_item_aliases(sword,"{weapon}").empty());
    }
    CHECK(sword.flags==flags);
    CHECK(sword.name(DESC_PLAIN)==name);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Chinese stash matching evaluates bilingual boolean expressions once and scopes UTF8 regex", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> regex(Options.regex_search,false);
    const auto random=rng::peek_uint64();
    cluaopen_globals(clua);
    REQUIRE(clua.execstring("assert(type(pmatch)=='function'); assert(pmatch('防具','{防具}',false))")==0);
    const string english="{body armour} a robe";
    const string aliases="{防具} {躯干防具} 长袍";
    for (const auto &entry : vector<pair<string,bool>>{
        {"防具 && !!躯干防具",false},{"防具 && !!火抗",true},
        {"body armour && 长袍",true},{"!!躯干防具",false},
        {"<< 长袍 || 武器 >> && !!火抗",true}})
    {
        lua_text_pattern pattern(stash_search_pattern(entry.first));
        REQUIRE(pattern.valid());
        const bool matched = pattern.matches(stash_search_haystack(english,aliases,pattern));
        CAPTURE(entry.first, clua.error);
        CHECK(matched==entry.second);
    }
    plaintext_pattern ascii("robe",true), chinese("长袍",true);
    CHECK(stash_search_haystack(english,aliases,ascii)==english);
    CHECK(chinese.matches(stash_search_haystack(english,aliases,chinese)));
    CHECK(stash_search_pattern("=药水")=="=药水");
#ifdef REGEX_PCRE
    text_pattern unicode(stash_search_pattern("^[药水]{2}$",true),true);
    REQUIRE(unicode.valid());
    CHECK(unicode.matches("药水")); CHECK(unicode.matches("水药"));
    CHECK_FALSE(unicode.matches("药X")); CHECK_FALSE(unicode.matches("药水水"));
    lua_text_pattern both(stash_search_pattern("/[药水]{2} && !!不存在"));
    REQUIRE(both.valid()); CHECK(both.matches("药水"));
    Options.regex_search=true;
    CHECK(stash_search_pattern("药水 && !!长剑").find("(*UTF8)")!=string::npos);
#endif
    Options.language=lang_t::EN;
    CHECK(stash_search_haystack(english,aliases,chinese)==english);
    CHECK_FALSE(chinese.matches(stash_search_haystack(english,aliases,chinese)));
    CHECK(stash_search_pattern("^[药水]{2}$",true)=="^[药水]{2}$");
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Chinese stash ground shop and feature searches retain canonical result identities", "[localization]")
{
    init_show_table(); init_feat_desc_cache();
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    unwind_var<god_type> god(you.religion,GOD_NO_GOD);
    search_annotation_scope restore;
    REQUIRE(clua.execstring("search_zh_calls=0; function ch_stash_search_annotate_item(it) "
        "search_zh_calls=search_zh_calls+1; return '{weapon} {Long Blades}' end")==0);
    const coord_def cell(12,12);
    unwind_var<dungeon_feature_type> floor(env.grid(cell),DNGN_FLOOR);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<unsigned int> vault_id(env.level_map_ids(cell),INVALID_MAP_INDEX);
    unwind_var<int> link(env.igrid(cell),0);
    unwind_var<item_def> original(env.item[0]);
    env.item[0].clear();
    item_def &item=env.item[0]; item.base_type=OBJ_WEAPONS;
    item.sub_type=WPN_LONG_SWORD; item.quantity=1; item.flags=ISFLAG_IDENTIFIED;
    item.pos=cell; item.link=NON_ITEM;
    Stash stash(cell);
    const auto random=rng::peek_uint64();
    REQUIRE(clua.execstring("search_zh_calls=0")==0);
    plaintext_pattern chinese("长剑",true), english("long sword",true);
    auto results=stash.matches_search("{D:3}",chinese);
    REQUIRE(results.size()==1);
    CHECK(results[0].match==Stash::stash_item_name(item));
    REQUIRE(clua.execstring("assert(search_zh_calls==1)")==0);
    plaintext_pattern place("地下城:3",true);
    CHECK(stash.matches_search("{D:3}",place).size()==1);
    shop_struct shop; shop.type=SHOP_WEAPON; shop.pos=cell;
    shop.shop_name="Tester"; shop.shop_suffix_name="Shop"; shop.greed=100;
    shop.stock.push_back(item);
    ShopInfo info(shop);
    auto stock=info.matches_search("{D:3}",chinese);
    REQUIRE(stock.size()==1); CHECK(stock[0].match_type==MATCH_ITEM);
    plaintext_pattern shops("商店",true);
    auto found=info.matches_search("{D:3}",shops);
    REQUIRE(found.size()==1); CHECK(found[0].match_type==MATCH_SHOP);
    env.igrid(cell)=NON_ITEM; env.grid(cell)=DNGN_STONE_STAIRS_DOWN_I;
    env.map_knowledge(cell)=map_cell();
    env.map_knowledge(cell).set_feature(DNGN_STONE_STAIRS_DOWN_I);
    Stash stairs(cell); string feature;
    REQUIRE(try_feature_display_name(DNGN_STONE_STAIRS_DOWN_I,TRAP_UNASSIGNED,feature));
    plaintext_pattern terrain(feature,true);
    CHECK(stairs.matches_search("{D:3}",terrain).size()==1);
    Options.language=lang_t::EN;
    CHECK(stash.matches_search("{D:3}",chinese).empty());
    CHECK(stash.matches_search("{D:3}",english).size()==1);
    CHECK(info.matches_search("{D:3}",chinese).empty());
    CHECK(stairs.matches_search("{D:3}",terrain).empty());
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Chinese stash description appends aliases while shared Lua annotations stay English", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    search_annotation_scope restore;
    REQUIRE(clua.execstring("search_zh_calls=0; function ch_stash_search_annotate_item(it) "
        "search_zh_calls=search_zh_calls+1; return '{weapon} {Long Blades} {custom-tag}' end")==0);
    item_def sword; sword.base_type=OBJ_WEAPONS; sword.sub_type=WPN_LONG_SWORD;
    sword.quantity=1; sword.flags=ISFLAG_IDENTIFIED;
    const string raw="{weapon} {Long Blades} {custom-tag}";
    const auto random=rng::peek_uint64();
    const string shown=get_item_description(sword,IDM_DEFAULT,true);
    CHECK(shown.find(raw)!=string::npos);
    CHECK(shown.find("{武器}")!=string::npos);
    CHECK(shown.find("{长刃}")!=string::npos);
    CHECK(shown.find(item_prefix(sword,false))!=string::npos);
    REQUIRE(clua.execstring("assert(search_zh_calls==1)")==0);
    CHECK(userdef_annotate_item(STASH_LUA_SEARCH_ANNOTATE,&sword)==raw);
    const string canonical=get_item_description(sword,IDM_DEFAULT);
    CHECK(canonical.find(raw)!=string::npos); CHECK(canonical.find("{武器}")==string::npos);
    CHECK(Options.language==lang_t::ZH);
    Options.language=lang_t::EN;
    CHECK(get_item_description(sword,IDM_DEFAULT,true)==canonical);
    lua_State *ls=clua.state();
    REQUIRE(luaL_loadfile(ls,"dat/clua/stash.lua")==0);
    REQUIRE(lua_pcall(ls,0,0,0)==0);
    REQUIRE(clua.execstring("local it={dropped=true,ego_type_terse='',class=function() return 'weapon' end}; "
        "local a=ch_stash_search_annotate_item(it); assert(a:find('{dropped}',1,true)); "
        "assert(a:find('{melee weapon}',1,true)); assert(not a:find('武器',1,true))")==0);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Help document display keeps Chinese UTF8 lines separate for the Tiles atlas", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    for (const char *name : {"quickstart.txt", "macros_guide.txt",
                             "tiles_help.txt", "crawl_manual.txt", "aptitudes.txt"})
    {
        FILE *file=fopen_u(("../docs/"+string(name)).c_str(),"r");
        REQUIRE(file!=nullptr);
        formatted_string original;
        char buffer[200];
        while (fgets(buffer,sizeof buffer,file)) original+=string(buffer);
        fclose(file);
        trim_string_right(original.ops.back().text);
        const string english=original.tostring();
        const string translated=ui_translate("help.document",english.c_str());
        REQUIRE(translated!=english);
        const auto shown=help_document_display(original);
        CHECK(shown.tostring()==translated);
        size_t bytes=0;
        bool line_operations=true, valid_utf8=true;
        set<char32_t> glyphs;
        for (const auto &op : shown.ops)
        {
            if (op.type!=FSOP_TEXT) continue;
            bytes+=op.text.size();
            const size_t newline=op.text.find('\n');
            line_operations = line_operations &&
                (newline==string::npos || newline==op.text.size()-1);
            const char *it=op.text.c_str();
            while (*it)
            {
                char32_t code;
                const int count=utf8towc(&code,it);
                if (count<=0 || code==0xfffd) { valid_utf8=false; break; }
                glyphs.insert(code); it+=count;
            }
        }
        CHECK(bytes==translated.size());
        CHECK(line_operations);
        CHECK(valid_utf8);
        CHECK(shown.ops.size()>static_cast<size_t>(count(translated.begin(),translated.end(),'\n')));
        if (string(name)=="crawl_manual.txt")
        {
            // Reproduces why one whole-document operation corrupts the atlas.
            CHECK(glyphs.size()>1023);
            CHECK(help_document_hotkeys(shown.tostring())==help_document_hotkeys(translated));
        }
        Options.language=lang_t::EN;
        CHECK(help_document_display(original)==original);
        Options.language=lang_t::FR;
        CHECK(help_document_display(original)==original);
        Options.language=lang_t::ZH;
        formatted_string changed=original; changed+="\nUpstream changed.";
        CHECK(help_document_display(changed)==changed);
    }
    formatted_string unknown("Untranslated help text.");
    CHECK(help_document_display(unknown)==unknown);
}

TEST_CASE("Reported M library schools translate only the display label", "[localization]")
{
    init_spell_descs();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    for (int i=0;i<NUM_SPELLS;++i)
    {
        const spell_type spell=static_cast<spell_type>(i);
        if (!is_valid_spell(spell)) continue;
        const string canonical=spell_schools_string(spell);
        const string shown=spell_library_school_label(spell);
        for (const auto school : spschools_type::range())
            if (spell_typematch(spell,school))
            {
                const ui_arg label=ui_text_arg("c.dump.school",spelltype_short_name(school));
                REQUIRE(label.available);
                CHECK(shown.find(label.text)!=string::npos);
            }
        CHECK(spell_schools_string(spell)==canonical);
        // The library reserves 32 columns for the title and 26 for schools.
        CAPTURE(i,shown);
        CHECK(strwidth(shown)<=26);
        Options.language=lang_t::EN;
        CHECK(spell_library_school_label(spell)==canonical);
        Options.language=lang_t::FR;
        CHECK(spell_library_school_label(spell)==canonical);
        Options.language=lang_t::ZH;
    }
    CHECK(spell_library_school_label(SPELL_FIREBALL).find("火系")!=string::npos);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Reported search result locations translate branch display and preserve identities", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    for (branch_type branch : {BRANCH_DUNGEON,BRANCH_TEMPLE,BRANCH_LAIR,
         BRANCH_ABYSS,BRANCH_PANDEMONIUM,BRANCH_ZIGGURAT,BRANCH_SEWER,BRANCH_TROVE})
    {
        unwind_var<int> depth(brdepth[branch],branches[branch].numlevels);
        for (int floor : {0,1})
        {
            const level_id level(branch,floor);
            const string canonical=level.describe();
            const ui_arg place=stash_level_arg(level);
            REQUIRE(place.available);
            CHECK(place.text.find(ui_text_arg("b.branch.name",branches[branch].shortname).text)==0);
            if (brdepth[branch]==1) CHECK(place.text.find(':')==string::npos);
            else CHECK(ends_with(place.text,floor ? ":1" : ":$"));
            const string original="["+canonical+"] a long sword";
            const string shown=ui_render("s.search.row","Result %s%s%s",original,
                {{"["+place.text+"] ",place.available},{"长剑",true},ui_literal_arg("")});
            CHECK(shown.find(place.text)!=string::npos);
            CHECK(level.describe()==canonical);
            Options.language=lang_t::EN;
            CHECK_FALSE(stash_level_arg(level).available);
            CHECK(ui_render("s.search.row","Result %s%s%s",original,
                {place,{"长剑",true},ui_literal_arg("")})==original);
            Options.language=lang_t::ZH;
        }
    }
    level_id invalid; invalid.branch=NUM_BRANCHES;
    CHECK_FALSE(stash_level_arg(invalid).available);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Reported constriction and related damage logs use typed Chinese punctuation", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    const string english="The vines constrict you.";
    for (int damage : {0,HIT_WEAK,HIT_MED,HIT_STRONG,1024})
    {
        const string canonical=attack_strength_punctuation(damage);
        const ui_arg mark=ui_attack_punctuation(canonical);
        REQUIRE(mark.available);
        const string constrict=ui_render("a.actor","%s constricts %s%s%s",english,
            {{"藤蔓",true},{"你",true},ui_literal_arg(""),mark});
        CHECK(constrict.find("藤蔓")!=string::npos);
        CHECK(ends_with(constrict,mark.text));
        CHECK(constrict.find('.')==string::npos);
        const string hidden=ui_render("a.actor","%s is constricted%s%s",english,
            {{"你",true},ui_literal_arg(""),mark});
        CHECK(ends_with(hidden,mark.text));
        for (const auto &entry : vector<pair<string,string>>{
            {"a.attack","You are electrocuted%s"},
            {"s.log","You shatter%s"}})
        {
            const string original=entry.second.substr(0,entry.second.size()-2)+canonical;
            const string shown=ui_render(entry.first.c_str(),entry.second.c_str(),original,{mark});
            CHECK(shown!=original); CHECK(ends_with(shown,mark.text));
            Options.language=lang_t::EN;
            CHECK(ui_render(entry.first.c_str(),entry.second.c_str(),original,{mark})==original);
            Options.language=lang_t::ZH;
        }
        CHECK(ui_render("a.actor","%s constricts %s%s%s",english,
            {{"unreviewed attacker",false},{"你",true},ui_literal_arg(""),mark})==english);
        Options.language=lang_t::EN;
        CHECK(ui_render("a.actor","%s constricts %s%s%s",english,
            {{"藤蔓",true},{"你",true},ui_literal_arg(""),mark})==english);
        Options.language=lang_t::ZH;
    }
    const auto no_damage=ui_text_arg("a.actor.state",", but do no damage.");
    REQUIRE(no_damage.available);
    CHECK(ends_with(no_damage.text,"。"));
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Reported description page footers translate every active page and retain English controls", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    for (bool status : {false,true})
        for (bool quote : {false,true})
            for (int page : {0,1,2})
            {
                if ((page==1 && !status) || (page==2 && !quote)) continue;
                string english=page==0 ? "[<w>!</w>]: <w>Description</w>" : "[<w>!</w>]: Description";
                if (status) english+=page==1 ? "|<w>Statuses</w>" : "|Statuses";
                if (quote) english+=page==2 ? "|<w>Quote</w>" : "|Quote";
                const string shown=description_page_footer(page,status,quote);
                CHECK(shown.find("说明")!=string::npos);
                CHECK(shown.find("Description")==string::npos);
                CHECK(shown.find("[<w>!</w>]")==0);
                const char *active=page==0 ? "<w>说明</w>" : page==1 ? "<w>状态</w>" : "<w>引文</w>";
                CHECK(shown.find(active)!=string::npos);
                CHECK((shown.find("状态")!=string::npos)==status);
                CHECK((shown.find("引文")!=string::npos)==quote);
                for (lang_t lang : {lang_t::EN,lang_t::FR})
                {
                    Options.language=lang;
                    CHECK(description_page_footer(page,status,quote)==english);
                }
                Options.language=lang_t::ZH;
            }
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Reported lookup feature titles translate independently of canonical feature descriptions", "[localization]")
{
    init_show_table(); init_feat_desc_cache();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    for (auto feat : {DNGN_ROCK_WALL,DNGN_STONE_STAIRS_DOWN_I,DNGN_CLOSED_DOOR,DNGN_ENTER_SHOP})
    {
        const string key=feature_description(feat,NUM_TRAPS,"",DESC_A,NUM_BRANCHES);
        const string canonical=feature_description(feat,NUM_TRAPS,"",DESC_PLAIN,NUM_BRANCHES);
        const ui_arg name=ui_text_arg("d.feature.name",canonical);
        CAPTURE(canonical);
        REQUIRE(name.available);
        CHECK(feature_type_display_title(feat)==name.text+"。");
        CHECK(feature_description(feat,NUM_TRAPS,"",DESC_A,NUM_BRANCHES)==key);
        Options.language=lang_t::EN;
        const string english=feature_type_display_title(feat);
        CHECK(english==uppercase_first(key)+".");
        Options.language=lang_t::FR;
        CHECK(feature_type_display_title(feat)==english);
        Options.language=lang_t::ZH;
    }
    Options.language=lang_t::EN;
    const string fallback=feature_type_display_title(NUM_FEATURES);
    Options.language=lang_t::ZH;
    CHECK(feature_type_display_title(NUM_FEATURES)==fallback);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Reported lookup bane labels share complete display names and preserve database keys", "[localization]")
{
    init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto random=rng::peek_uint64();
    for (int i=0;i<NUM_BANES;++i)
    {
        const bane_type bane=static_cast<bane_type>(i);
        const string key=bane_name(bane,true)+" bane";
        const string canonical=bane_name(bane);
        string shown;
        REQUIRE(try_lookup_help_display_name(key,shown));
        CHECK(shown==bane_display_name(bane));
        CHECK(ends_with(shown,"灾厄"));
        CHECK(shown.find("厄咒")==string::npos);
        CHECK(bane_name(bane)==canonical);
        CHECK(bane_name(bane,true)+" bane"==key);
        Options.language=lang_t::EN;
        CHECK_FALSE(try_lookup_help_display_name(key,shown));
        CHECK(bane_display_name(bane)==canonical);
        Options.language=lang_t::ZH;
    }
    string unknown;
    CHECK_FALSE(try_lookup_help_display_name("future upstream bane",unknown));
    CHECK(rng::peek_uint64()==random);
}

// Overview stores English metadata separately from its displayed labels.
extern map<branch_type,set<level_id>> stair_level;
extern map<level_pos,shop_type> shops_present;
extern map<level_pos,god_type> altars_present;
extern map<level_pos,branch_type> portals_present;
extern map<level_pos,string> portal_notes;
extern map<level_id,string> level_annotations;
extern map<level_id,string> level_exclusions;
extern map<level_id,string> level_uniques;
extern set<pair<string,level_id>> auto_unique_annotations;

TEST_CASE("Morgue overview localizes structured locations guardians and uniques while retaining English exports", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char*> lang_name(Options.lang_name,"zh");
    unwind_var<species_type> species(you.species,SP_HUMAN);
    unwind_var<god_type> god(you.religion,GOD_NO_GOD);
    unwind_var<branch_type> root(root_branch,BRANCH_DUNGEON);
    unwind_var<branch_type> branch(you.where_are_you,BRANCH_DUNGEON);
    unwind_var<int> depth(you.depth,15);
    unwind_var<bool> wizard(you.wizard,false);
    unwind_var<decltype(you.save)> save(you.save,nullptr);
    unwind_var<decltype(you.props)> props(you.props);
    unwind_var<decltype(brdepth)> depths(brdepth);
    unwind_var<bool> known(you.zot_orb_monster_known,true);
    unwind_var<monster_type> guardian(you.zot_orb_monster,MONS_ORB_OF_WINTER);
    unwind_var<decltype(stair_level)> stairs(stair_level,{});
    unwind_var<decltype(shops_present)> shops(shops_present,{});
    unwind_var<decltype(altars_present)> altars(altars_present,{});
    unwind_var<decltype(portals_present)> portals(portals_present,{});
    unwind_var<decltype(portal_notes)> portal_text(portal_notes,{});
    unwind_var<decltype(level_annotations)> annotations(level_annotations,{});
    unwind_var<decltype(level_exclusions)> exclusions(level_exclusions,{});
    unwind_var<decltype(level_uniques)> uniques(level_uniques,{});
    unwind_var<decltype(auto_unique_annotations)> auto_uniques(auto_unique_annotations,{});
    unwind_var<vector<string>> order(Options.dump_order,{"overview"});
    for (int i=0;i<NUM_BRANCHES;++i) brdepth[i]=branches[i].numlevels;
    you.props["visited_levels"].new_table();
    const level_id location(BRANCH_DUNGEON,9);
    you.set_level_visited(location);
    stair_level[BRANCH_LAIR].insert(location);
    const level_pos at(location,coord_def(12,12));
    shops_present[at]=SHOP_WEAPON;
    altars_present[at]=GOD_ZIN;
    altars_present[level_pos(location,coord_def(13,12))]=GOD_SHINING_ONE;
    portals_present[at]=BRANCH_ABYSS;
    portal_notes[at]="player-authored portal price";
    level_annotations[location]="my custom note / 4 transporters";
    auto_unique_annotations.insert({"Kirke",location});
    level_uniques[location]="Kirke";
    const auto random=rng::peek_uint64();
    const string canonical=character_dump_text();
    CHECK(canonical.find("D:9")!=string::npos);
    CHECK(canonical.find("Kirke")!=string::npos);
    CHECK(canonical.find("The Realm of Zot is guarded by orbs of winter.")!=string::npos);
    const string shown=character_dump_text(true);
    CHECK(shown.find("地下城:9")!=string::npos);
    CHECK(shown.find("喀耳刻")!=string::npos);
    CHECK(shown.find("Kirke")==string::npos);
    CHECK(shown.find("佐特领域由寒冬之球守卫。")!=string::npos);
    CHECK(shown.find("D:9")==string::npos);
    CHECK(shown.find(ui_text_arg("display.god",god_name(GOD_SHINING_ONE)).text)!=string::npos);
    CHECK(shown.find("press")==string::npos);
    CHECK(shown.find("my custom note / 4 transporters")!=string::npos);
    CHECK(shown.find("player-authored portal price")!=string::npos);
    CHECK(level_uniques[location]=="Kirke");
    CHECK(level_annotations[location]=="my custom note / 4 transporters");
    CHECK(character_dump_text()==canonical);
    auto_unique_annotations.insert({"Future custom unique",location});
    level_uniques[location]="Kirke, Future custom unique";
    const string fallback=character_dump_text(true);
    CHECK(fallback.find("Kirke, Future custom unique")!=string::npos);
    CHECK(fallback.find("喀耳刻")==string::npos);
    auto_unique_annotations.erase({"Future custom unique",location});
    level_uniques[location]="Kirke";
    unwind_var<string> directory(Options.morgue_dir,"../../../.term-audit/morgue-02-runtime");
    REQUIRE(dump_char("native-overview",true));
    Options.language=lang_t::EN;
    CHECK(character_dump_text(true)==canonical);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Morgue vault locations keep canonical map identifiers and explicit English mode", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> wizard(you.wizard,true);
    unwind_var<decltype(you.save)> save(you.save,nullptr);
    unwind_var<decltype(you.props)> props(you.props);
    unwind_var<decltype(you.vault_list)> vaults(you.vault_list);
    unwind_var<decltype(brdepth)> depths(brdepth);
    unwind_var<vector<string>> order(Options.dump_order,{"vaults"});
    for (int i=0;i<NUM_BRANCHES;++i) brdepth[i]=branches[i].numlevels;
    you.props["visited_levels"].new_table();
    const level_id location(BRANCH_DUNGEON,1);
    you.set_level_visited(location);
    you.vault_list.clear();
    const string identifier="test_map_identifier_must_stay_English";
    you.vault_list[location]={identifier};
    const auto random=rng::peek_uint64();
    const string canonical=dump_vault_maps();
    CHECK(canonical.find("D:1")!=string::npos);
    CHECK(canonical.find("地下城")==string::npos);
    const string shown=dump_vault_maps(true);
    CHECK(shown.find("地下城:1")!=string::npos);
    CHECK(shown.find(identifier)!=string::npos);
    CHECK(you.vault_list[location][0]==identifier);
    CHECK(dump_vault_maps()==canonical);
    CHECK(character_dump_text().find("地下城")==string::npos);
    CHECK(character_dump_text(true).find("地下城:1")!=string::npos);
    Options.language=lang_t::EN;
    CHECK(dump_vault_maps(true)==canonical);
    CHECK(rng::peek_uint64()==random);
}

TEST_CASE("Morgue death records translate exact components and preserve complete fallback", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<int> paralysis(you.duration[DUR_PARALYSIS], 0);
    unwind_var<int> petrified(you.duration[DUR_PETRIFIED], 0);
    unwind_var<int> sleep(you.duration[DUR_SLEEP], 0);
    unwind_var<int> vexed(you.duration[DUR_VEXED], 0);
    const string common = "v=0.34.1:sc=42:name=Tester:race=Human:cls=Fighter:char=HuFi:xl=5:title=Fighter:br=D:lvl=2:hp=-4:mhp=10:mmhp=10:dam=8:start=20261023080000S:end=20261023090000S:dur=60:";
    const auto next = rng::peek_uint64();
    for (const string fields : {"ktyp=mon:killer=an orc:kaux=a short sword",
                                "ktyp=mon:killer=an orc:kaux=a +0 short sword",
                                "ktyp=beam:killer=Jessica:kaux=puff of flame",
                                "ktyp=constriction:killer=a ball python",
                                "ktyp=pois:killer=an adder",
                                "ktyp=cloud:kaux=flame"})
    {
        scorefile_entry score; REQUIRE(score.parse(common + fields + ":turn=10\n"));
        const string raw = score.raw_string();
        for (auto verbosity : {scorefile_entry::DDV_NORMAL, scorefile_entry::DDV_VERBOSE,
                               scorefile_entry::DDV_TERSE, scorefile_entry::DDV_ONELINE})
        {
            const string original = score.death_description(verbosity);
            const string shown = score.display_death_description(verbosity);
            CAPTURE(fields, original, shown);
            CHECK(shown != original);
            CHECK(shown.find("killed") == string::npos);
            Options.language = lang_t::EN;
            CHECK(score.display_death_description(verbosity) == original);
            Options.language = lang_t::ZH;
        }
        Note note(NOTE_DEATH, -4, 10, score.death_description(scorefile_entry::DDV_NORMAL),
            "morgue-score-normal-v1\n" + raw);
        CHECK(note.display_describe(false, false) == score.display_death_description(scorefile_entry::DDV_NORMAL));
        CHECK(note.describe(false, false) == score.death_description(scorefile_entry::DDV_NORMAL));
        vector<unsigned char> saved;
        auto output = writer(&saved); note.save(output);
        auto input = reader(saved, TAG_MINOR_VERSION); Note restored; restored.load(input);
        CHECK(restored.describe(false, false) == note.describe(false, false));
        CHECK(restored.display_describe(false, false) == note.display_describe(false, false));
        vector<unsigned char> after; auto again = writer(&after); note.save(again);
        CHECK(saved == after);
        Note old_note(NOTE_DEATH, -4, 10, note.name);
        CHECK(old_note.display_describe(false, false) == old_note.describe(false, false));
        note.name = "Historical text does not match the metadata";
        CHECK(note.display_describe(false, false) == note.describe(false, false));
        CHECK(score.raw_string() == raw);
        unwind_var<vector<string>> order(Options.dump_order, {"hiscore", "notes"});
        unwind_var<vector<Note>> notes(note_list, {restored});
        unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/morgue-03-runtime");
        const string filename = fields.find("ktyp=mon") == 0 ? "native-death-melee"
            : fields.find("ktyp=beam") == 0 ? "native-death-beam"
            : fields.find("ktyp=pois") == 0 ? "native-death-poison"
            : fields.find("ktyp=cloud") == 0 ? "native-death-cloud" : "native-death-constriction";
        REQUIRE(dump_char(filename, true, false, &score));
        auto read_file = [&](const string &suffix) {
            std::ifstream file(morgue_directory() + filename + suffix);
            return string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        };
        CHECK(read_file(".txt").find(score.death_description(scorefile_entry::DDV_NORMAL)) != string::npos);
        CHECK(read_file(".zh.txt").find(score.display_death_description(scorefile_entry::DDV_NORMAL)) != string::npos);
    }
    for (const string fields : {"ktyp=mon:killer=an unknown upstream creature",
                                "ktyp=mon:killer=an orc:kaux=a +99 unknown weapon",
                                "ktyp=beam:killer=Jessica:kaux=unknown future beam",
                                "ktyp=mon:killer=an orc:kpath=summoned by somebody"})
    {
        scorefile_entry score; REQUIRE(score.parse(common + fields + ":turn=10\n"));
        CHECK(score.display_death_description(scorefile_entry::DDV_VERBOSE)
              == score.death_description(scorefile_entry::DDV_VERBOSE));
    }
    scorefile_entry disabled; REQUIRE(disabled.parse(common + "ktyp=mon:killer=an orc:turn=10\n"));
    you.duration[DUR_PETRIFIED] = 10;
    CHECK(disabled.display_death_description(scorefile_entry::DDV_VERBOSE)
          == disabled.death_description(scorefile_entry::DDV_VERBOSE));
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Morgue high scores prefer Chinese files with linewise UTF8 and English fallback", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/morgue-03-runtime");
    const string base = "native-score-browser";
    const string path = morgue_directory() + base;
    const string english = "English morgue <literal>\n\nLast line";
    string chinese = "中文记录 <literal>\n\n";
    // More than the Tiles atlas can retain at once, spread over visible lines.
    for (unsigned c = 0x4e00; c < 0x4e00 + 1600; ++c)
    {
        char bytes[5] = {char(0xe0 | (c >> 12)), char(0x80 | ((c >> 6) & 63)),
                         char(0x80 | (c & 63)), 0, 0};
        chinese += bytes;
        if ((c - 0x4e00) % 80 == 79) chinese += "\n";
    }
    chinese += "末尾无换行";
    { std::ofstream out(path + ".txt", std::ios::binary); out << english; }
    { std::ofstream out(path + ".zh.txt", std::ios::binary); out << chinese; }
    formatted_string shown;
    REQUIRE(read_morgue_display(base, shown)); CHECK(shown.tostring() == chinese);
    const auto lines = scroller_linewise_text(shown);
    CHECK(lines.tostring() == chinese);
    for (const auto &op : lines.ops)
        if (op.type == FSOP_TEXT)
        {
            CHECK((op.text.find('\n') == string::npos || op.text.find('\n') == op.text.size() - 1));
            CHECK(op.text.size() <= 250);
        }
    Options.language = lang_t::EN;
    REQUIRE(read_morgue_display(base, shown)); CHECK(shown.tostring() == english);
    Options.language = lang_t::ZH;
    REQUIRE(std::remove((path + ".zh.txt").c_str()) == 0);
    REQUIRE(read_morgue_display(base, shown)); CHECK(shown.tostring() == english);
    REQUIRE(std::remove((path + ".txt").c_str()) == 0);
    CHECK_FALSE(read_morgue_display(base, shown));
}

TEST_CASE("Morgue starting and level notes keep English snapshots and translate typed records", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name, "zh");
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<job_type> job(you.char_class, JOB_FIGHTER);
    unwind_var<decltype(you.save)> save(you.save, nullptr);
    unwind_var<vector<Note>> notes(note_list, {});
    unwind_var<vector<string>> order(Options.dump_order, {"notes"});
    unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/morgue-04-runtime");
    const auto next = rng::peek_uint64();
    const auto start = make_starting_note("Ceneedgh", SP_MINOTAUR, JOB_FIGHTER);
    CHECK(start.describe(false, false) == "Ceneedgh the Minotaur Fighter began the quest for the Orb.");
    CHECK(start.display_describe(false, false).find("牛头人战士") != string::npos);
    CHECK(start.display_describe(false, false).find("佐特宝珠") != string::npos);
    CHECK(start.display_describe(false, false).find("Ceneedgh") != string::npos);
    // Character state changes must never rewrite the historical starting race/job.
    CHECK(you.species == SP_HUMAN);
    vector<Note> fixtures = {start, make_xp_level_note(1, 19, 19, 0, 0),
        make_xp_level_note(2, 27, 27, 1, 1), make_xp_level_note(3, -1, 33, 0, 2),
        make_version_note("0.34.0:local", "0.34.1:zh"),
        Note(NOTE_MESSAGE, 0, 0, "You started the game in wizard mode."),
        Note(NOTE_MESSAGE, 0, 0, "Entered wizard mode."),
        Note(NOTE_MESSAGE, 0, 0, "Entered explore mode."),
        Note(NOTE_MESSAGE, 0, 0, "Voluntarily entered the Abyss."),
        Note(NOTE_MESSAGE, 0, 0, "Took an exit into the Abyss."),
        Note(NOTE_MESSAGE, 0, 0, "Escaped the Abyss"),
        Note(NOTE_MESSAGE, 0, 0, "Escaped Pandemonium"),
        Note(NOTE_MESSAGE, 0, 0, "Escaped into the Abyss"),
        Note(NOTE_MESSAGE, 0, 0, "Glimpsed the power of Zot.")};
    for (const auto &note : fixtures)
    {
        const string english = note.describe(false, false);
        const string chinese = note.display_describe(false, false);
        CAPTURE(english, chinese);
        CHECK(chinese != english);
        vector<unsigned char> bytes; auto output = writer(&bytes); note.save(output);
        auto input = reader(bytes, TAG_MINOR_VERSION); Note restored; restored.load(input);
        CHECK(restored.describe(false, false) == english);
        CHECK(restored.display_describe(false, false) == chinese);
        vector<unsigned char> after; auto second = writer(&after); note.save(second);
        CHECK(bytes == after);
        Options.language = lang_t::EN;
        CHECK(note.display_describe(false, false) == english);
        Options.language = lang_t::ZH;
    }
    CHECK(fixtures[1].name == "HP: 19/19 MP: 0/0");
    CHECK(fixtures[1].display_describe(false, false).find("生命值：19/19，魔力值：0/0") != string::npos);
    CHECK(fixtures[2].display_describe(false, false).find("经验等级达到2") != string::npos);
    CHECK(fixtures[4].display_describe(false, false).find("0.34.0:local") != string::npos);
    vector<Note> fallback = {Note(NOTE_XP_LEVEL_CHANGE, 2, 0, "HP: 27/27 MP: 1/1"),
        Note(NOTE_MESSAGE, 0, 0, start.name), Note(NOTE_MESSAGE, 0, 0, "custom script message")};
    auto invalid = start; invalid.first = NUM_SPECIES; fallback.push_back(invalid);
    invalid = start; invalid.second = NUM_JOBS; fallback.push_back(invalid);
    invalid = start; invalid.name += " modified"; fallback.push_back(invalid);
    invalid = fixtures[1]; invalid.name = "HP: 999/999 MP: 0/0"; fallback.push_back(invalid);
    invalid = fixtures[1]; invalid.desc = "morgue-xp-v1\nhp=19:mhp=19:mp=0"; fallback.push_back(invalid);
    invalid = fixtures[1]; invalid.desc = "morgue-xp-v1\nhp=19x:mhp=19:mp=0:mmp=0"; fallback.push_back(invalid);
    invalid = fixtures[4]; invalid.name += " extra"; fallback.push_back(invalid);
    fallback.emplace_back(NOTE_USER_NOTE, 0, 0, "Entered wizard mode.");
    for (const auto &note : fallback)
        CHECK(note.display_describe(false, false) == note.describe(false, false));
    note_list = fixtures;
    for (size_t i = 0; i < note_list.size(); ++i)
    { note_list[i].turn = int(i); note_list[i].place = level_id(BRANCH_DUNGEON, 1); }
    const string english = character_dump_text();
    const string chinese = character_dump_text(true);
    CHECK(english.find("began the quest for the Orb.") != string::npos);
    CHECK(english.find("Reached XP level 2. HP: 27/27 MP: 1/1") != string::npos);
    CHECK(chinese.find("Reached XP level") == string::npos);
    CHECK(chinese.find("began the quest") == string::npos);
    CHECK(chinese.find("生命值：27/27，魔力值：1/1") != string::npos);
    CHECK(character_dump_text() == english);
    REQUIRE(dump_char("native-start-levels", true));
    formatted_string displayed;
    REQUIRE(read_morgue_display("native-start-levels", displayed));
    CHECK(displayed.tostring() == chinese);
    Options.language = lang_t::EN;
    REQUIRE(read_morgue_display("native-start-levels", displayed));
    CHECK(displayed.tostring() == english);
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Morgue elemental deaths retain sources effects and whole English fallback", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name, "zh");
    unwind_var<int> paralysis(you.duration[DUR_PARALYSIS], 0);
    unwind_var<int> stone(you.duration[DUR_PETRIFIED], 0);
    unwind_var<int> sleep(you.duration[DUR_SLEEP], 0);
    unwind_var<int> vexed(you.duration[DUR_VEXED], 0);
    unwind_var<decltype(you.save)> save(you.save, nullptr);
    unwind_var<vector<string>> order(Options.dump_order, {"hiscore", "notes"});
    unwind_var<vector<Note>> notes(note_list, {});
    unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/morgue-05-runtime");
    const string common = "v=0.34.1:sc=42:name=Plog:race=Human:cls=Fighter:char=HuFi:xl=2:title=Fighter:br=D:lvl=1:hp=-1:mhp=21:mmhp=21:dam=4:start=20261023080000S:end=20261023090000S:dur=60:";
    const auto next = rng::peek_uint64();
    for (const string fields : {"ktyp=freezing:killer=an endoplasm", "ktyp=freezing",
        "ktyp=burning:killer=Jessica:kaux=sticky flame", "ktyp=burning",
        "ktyp=draining:killer=a wraith:kaux=negative energy", "ktyp=draining:kaux=negative energy",
        "ktyp=rotting:killer=a necrophage:kaux=poison", "ktyp=rotting",
        "ktyp=blinking:killer=an orc:kaux=magic dart", "ktyp=acid:killer=an acid blob",
        "ktyp=collision:killer=an ogre:kaux=a freezing wraith"})
    {
        scorefile_entry score; REQUIRE(score.parse(common + fields + ":turn=316\n"));
        const string raw = score.raw_string();
        for (auto verbosity : {scorefile_entry::DDV_NORMAL, scorefile_entry::DDV_VERBOSE,
                               scorefile_entry::DDV_TERSE, scorefile_entry::DDV_ONELINE})
        {
            const string original = score.death_description(verbosity);
            const string shown = score.display_death_description(verbosity);
            CAPTURE(fields, original, shown);
            CHECK(shown != original);
            Options.language = lang_t::EN;
            CHECK(score.display_death_description(verbosity) == original);
            Options.language = lang_t::ZH;
        }
        CHECK(score.raw_string() == raw);
        Note death(NOTE_DEATH, -1, 21, score.death_description(scorefile_entry::DDV_NORMAL),
            "morgue-score-normal-v1\n" + raw);
        CHECK(death.display_describe(false, false) == score.display_death_description(scorefile_entry::DDV_NORMAL));
        if (fields == "ktyp=freezing:killer=an endoplasm")
        {
            const string shown = score.display_death_description(scorefile_entry::DDV_VERBOSE);
            CHECK(shown.find("原生胶体") != string::npos); CHECK(shown.find("冻死") != string::npos);
            Note hp(NOTE_HP_CHANGE, 1, 21, score.death_description(scorefile_entry::DDV_TERSE),
                "morgue-damage-v1\n" + score.death_cause_snapshot());
            note_list = {hp, death};
            for (auto &note : note_list) { note.turn = 316; note.place = level_id(BRANCH_DUNGEON, 1); }
            REQUIRE(dump_char("native-frozen", true, false, &score));
            formatted_string file; REQUIRE(read_morgue_display("native-frozen", file));
            CHECK(file.tostring().find("Frozen") == string::npos);
            CHECK(file.tostring().find("frozen") == string::npos);
            CHECK(file.tostring().find("生命值：1/21［冻伤（4 点伤害）］") != string::npos);
        }
        if (fields == "ktyp=burning:killer=Jessica:kaux=sticky flame")
        {
            const string shown = score.display_death_description(scorefile_entry::DDV_VERBOSE);
            CHECK(shown.find("杰西卡") != string::npos); CHECK(shown.find("黏附火焰") != string::npos);
            CHECK(shown.find("4 点伤害") != string::npos);
        }
    }
    for (const string fields : {"ktyp=freezing:killer=an unknown future monster",
        "ktyp=burning:killer=Jessica:kaux=unknown flame effect",
        "ktyp=draining:killer=a wraith:kaux=unknown draining effect",
        "ktyp=rotting:killer=a necrophage:kaux=unknown source",
        "ktyp=collision:killer=unknown pusher:kaux=a freezing wraith",
        "ktyp=freezing:killer=an endoplasm:kpath=summoned by unknown"})
    {
        scorefile_entry score; REQUIRE(score.parse(common + fields + ":turn=316\n"));
        CHECK(score.display_death_description(scorefile_entry::DDV_VERBOSE)
              == score.death_description(scorefile_entry::DDV_VERBOSE));
    }
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Morgue nonlethal damage records use injury words and canonical compact snapshots", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next = rng::peek_uint64();
    for (const char *kind : {"freezing", "burning", "draining", "rotting", "acid", "curare", "barbs",
                            "mirror_damage", "falling_down_stairs", "falling_through_gate"})
    {
        scorefile_entry score;
        REQUIRE(score.parse(string("ktyp=") + kind + ":dam=2:turn=1"));
        const string original = score.death_description(scorefile_entry::DDV_TERSE);
        const string translated = score.display_damage_description();
        CAPTURE(kind, original, translated);
        CHECK(translated != original); CHECK(translated.find("死") == string::npos);
        Note hp(NOTE_HP_CHANGE, 1, 21, original, "morgue-damage-v1\n" + score.death_cause_snapshot());
        const string shown = hp.display_describe(false, false);
        CHECK(shown.find("生命值：1/21") == 0);
        CHECK(shown.find(translated) != string::npos);
        vector<unsigned char> bytes; auto output = writer(&bytes); hp.save(output);
        auto input = reader(bytes, TAG_MINOR_VERSION); Note restored; restored.load(input);
        CHECK(restored.display_describe(false, false) == shown);
        CHECK(restored.describe(false, false) == hp.describe(false, false));
        Options.language = lang_t::EN;
        CHECK(hp.display_describe(false, false) == hp.describe(false, false));
        Options.language = lang_t::ZH;
        auto broken = hp; broken.name += " unknown clause";
        CHECK(broken.display_describe(false, false) == broken.describe(false, false));
    }
    // Exercise the live damage-only constructor: no character/skill initialization.
    scorefile_entry minimal(2, MID_NOBODY, KILLED_BY_FREEZING, nullptr, true);
    const string snapshot = minimal.death_cause_snapshot();
    CHECK(snapshot.find("name=") == string::npos);
    scorefile_entry replay; REQUIRE(replay.parse(snapshot));
    CHECK(replay.death_description(scorefile_entry::DDV_TERSE)
          == minimal.death_description(scorefile_entry::DDV_TERSE));
    Note unseen(NOTE_HP_CHANGE, 1, 21, "something (2)", "morgue-damage-unseen-v1\n2");
    CHECK(unseen.display_describe(false, false).find("某种来源（2 点伤害）") != string::npos);
    unseen.desc += "x";
    CHECK(unseen.display_describe(false, false) == unseen.describe(false, false));
    Note old(NOTE_HP_CHANGE, 1, 21, "frozen (2)");
    CHECK(old.display_describe(false, false) == old.describe(false, false));
    scorefile_entry extra; REQUIRE(extra.parse("ktyp=mon:killer=an orc:kaux=unknown weapon:dam=4:turn=1"));
    Note unknown(NOTE_HP_CHANGE, 1, 21, extra.death_description(scorefile_entry::DDV_TERSE),
        "morgue-damage-v1\n" + extra.death_cause_snapshot());
    CHECK(unknown.display_describe(false, false) == unknown.describe(false, false));
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Target compact hit chances cover melee missiles blindness and auxiliary attacks", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language);
    unwind_var<species_type> species(you.species, SP_MINOTAUR);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<int> blind(you.duration[DUR_BLIND], 0);
    unwind_var<decltype(you.mutation)> mutations(you.mutation);
    you.mutation[MUT_HORNS] = 2;
    item_def bow; bow.base_type = OBJ_WEAPONS; bow.sub_type = WPN_SHORTBOW;
    bow.quantity = 1; bow.flags = ISFLAG_IDENTIFIED;
    item_def stone; stone.base_type = OBJ_MISSILES; stone.sub_type = MI_STONE;
    stone.quantity = 1; stone.flags = ISFLAG_IDENTIFIED;
    monster_info target(MONS_GOBLIN); target.pos = you.pos() + coord_def(1, 0);
    const auto next = rng::peek_uint64();
    for (const item_def *weapon : vector<const item_def *>{nullptr, &bow, &stone})
        for (bool blinded : {false, true})
            for (bool verbose : {false, true})
            {
                you.duration[DUR_BLIND] = blinded ? 10 : 0;
                ostringstream english, canonical; string display; uint64_t after;
                { rng::subgenerator sample(123456, 987654); Options.language = lang_t::EN;
                  describe_to_hit(target, english, weapon, verbose, nullptr, 1); after = rng::peek_uint64(); }
                { rng::subgenerator sample(123456, 987654); Options.language = lang_t::ZH;
                  describe_to_hit(target, canonical, weapon, verbose, nullptr, 1, &display);
                  CHECK(rng::peek_uint64() == after); }
                CAPTURE(verbose, blinded, english.str(), display);
                CHECK(canonical.str() == english.str());
                CHECK_FALSE(display.empty()); CHECK(display.find("命中率") != string::npos);
                if (!verbose && blinded) CHECK(display.find("此距离") != string::npos);
                if (!weapon) CHECK(display.find("头槌") != string::npos);
                const string row = target_prompt_display("Aim", "a goblin (" + canonical.str() + ")",
                    {"地精（" + display + "）", !display.empty()}, true);
                CHECK(row.find("瞄准") == 0); CHECK(row.find("to hit") == string::npos);
            }
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Target complete monster rows cover every listed status and dynamic equipment attributes", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index(); init_spell_descs();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<god_type> god(you.religion, GOD_NO_GOD);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<int> confuse(you.duration[DUR_CONFUSING_TOUCH], 0);
    unwind_var<int> jinx(you.duration[DUR_JINXBITE], 0);
    const auto next = rng::peek_uint64();
    for (const auto &flag : monster_info_flag_names)
    {
        monster_info mi(MONS_GOBLIN); mi.mb.set(flag.flag);
        mi.pos = you.pos() + coord_def(1, 0);
        ui_arg shown; const string canonical = monster_target_description(mi, nullptr, shown);
        CAPTURE(flag.long_singular, canonical, shown.text);
        CHECK(shown.available);
        CHECK(target_prompt_display("Aim", canonical, shown, true).find("瞄准") == 0);
        string equipment;
        CHECK(try_monster_equipment_display(mi, equipment, DESC_FULL, DESC_A, true));
        Options.language = lang_t::EN;
        ui_arg en; CHECK(monster_target_description(mi, nullptr, en) == canonical);
        CHECK_FALSE(en.available);
        Options.language = lang_t::ZH;
    }
    for (auto type : {MONS_GOBLIN, MONS_IRON_GOLEM, MONS_NAMELESS_REVENANT})
        for (int damage = MDAM_OKAY; damage <= MDAM_ALMOST_DEAD; ++damage)
        {
            monster_info mi(type); mi.dam = static_cast<mon_dam_level_type>(damage);
            mi.pos = you.pos() + coord_def(1, 0);
            ui_arg shown; const string canonical = monster_target_description(mi, nullptr, shown);
            CAPTURE(type, damage, canonical, shown.text);
            CHECK(shown.available);
            CHECK(target_prompt_display("Aim", canonical, shown, true).find("Aim:") == string::npos);
        }
    monster_info unknown(MONS_GOBLIN); unknown.mname = "unreviewed scripted creature";
    unknown.mb.set(MB_NAME_REPLACE); ui_arg shown;
    const string canonical = monster_target_description(unknown, nullptr, shown);
    CHECK_FALSE(shown.available);
    CHECK(target_prompt_display("Aim", canonical, shown, true) == "Aim: <lightgrey>" + canonical + "</lightgrey>");
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Target actual callbacks compose compact attacks spell chances and absent descriptors once", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index(); init_spell_descs(); init_zap_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<god_type> god(you.religion, GOD_NO_GOD);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<int> blind(you.duration[DUR_BLIND], 0);
    unwind_var<int> confuse(you.duration[DUR_CONFUSING_TOUCH], 0);
    unwind_var<int> jinx(you.duration[DUR_JINXBITE], 0);
    monster_info mi(MONS_GOBLIN); mi.pos = you.pos() + coord_def(1, 0);
    mi.dam = MDAM_MODERATELY_DAMAGED; mi.mb.set(MB_POISONED);
    mi.inv[MSLOT_WEAPON].reset(new item_def);
    auto &sword = *mi.inv[MSLOT_WEAPON]; sword.base_type = OBJ_WEAPONS;
    sword.sub_type = WPN_SHORT_SWORD; sword.quantity = 1; sword.flags = ISFLAG_IDENTIFIED;
    targeting_behaviour behavior;
    int calls = 0;
    behavior.get_desc_func = [&](const monster_info &target) {
        ++calls; ostringstream stream; string display;
        describe_to_hit(target, stream, nullptr, false, nullptr, 1, &display);
        return vector<string>{target_description_text(stream.str(), {display, !display.empty()})};
    };
    const auto next = rng::peek_uint64();
    ui_arg shown; string canonical;
    { rng::subgenerator sample(123456, 987654); canonical = monster_target_description(mi, &behavior, shown); }
    CHECK(calls == 1); REQUIRE(shown.available);
    CHECK(shown.text.find("命中率") != string::npos); CHECK(shown.text.find("短剑") != string::npos);
    CHECK(shown.text.find("中毒") != string::npos);
    CHECK(target_prompt_display("Aim", canonical, shown, true).find("Aim:") == string::npos);
    for (auto spell : {SPELL_STONE_ARROW, SPELL_MOMENTUM_STRIKE, SPELL_ELECTRIC_CHARGE,
                       SPELL_SLOW, SPELL_STICKY_FLAME})
    {
        targeter_beam beam(&you, 7, ZAP_STONE_ARROW, 40, 0, 0);
        behavior.get_desc_func = targeter_addl_desc(spell, 40, get_spell_flags(spell), &beam);
        REQUIRE(bool(behavior.get_desc_func));
        { rng::subgenerator sample(123456, 987654); canonical = monster_target_description(mi, &behavior, shown); }
        CAPTURE(spell, canonical, shown.text);
        CHECK(shown.available);
        CHECK(target_prompt_display("Aim", canonical, shown, true).find("瞄准") == 0);
    }
    behavior.get_desc_func = [](const monster_info &) { return vector<string>{""}; };
    canonical = monster_target_description(mi, &behavior, shown);
    CHECK(shown.available);
    behavior.get_desc_func = [](const monster_info &) { return vector<string>{"new upstream probability text"}; };
    canonical = monster_target_description(mi, &behavior, shown);
    CHECK_FALSE(shown.available);
    CHECK(target_prompt_display("Aim", canonical, shown, true) == "Aim: <lightgrey>" + canonical + "</lightgrey>");
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Reported four gaps translate feature and identified item note snapshots", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    init_show_table(); init_feat_desc_cache();
    unwind_var<bool> on_level(you.on_current_level, false);
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<decltype(brdepth)> depths(brdepth);
    for (int i = 0; i < NUM_BRANCHES; ++i) brdepth[i] = branches[i].numlevels;
    const coord_def cell(12,12);
    unwind_var<dungeon_feature_type> terrain(env.grid(cell), DNGN_FOUNTAIN_BLUE);
    unwind_var<unsigned int> vault_id(env.level_map_ids(cell), INVALID_MAP_INDEX);
    unwind_var<map_cell> memory(env.map_knowledge(cell));
    unwind_var<decltype(env.shop)> shops(env.shop);
    const auto next = rng::peek_uint64();
    vector<Note> notes;
    for (auto feat : {DNGN_FOUNTAIN_BLUE, DNGN_ENTER_LAIR, DNGN_STONE_STAIRS_DOWN_I})
    {
        env.grid(cell) = feat; env.map_knowledge(cell).set_feature(feat);
        const Note note = make_feature_note(cell);
        CAPTURE(feat, note.name, note.display_describe(false, false));
        CHECK(note.name == feature_description_at(cell, false, DESC_A) + ".");
        CHECK(note.display_describe(false, false).find("发现") == 0);
        CHECK(note.display_describe(false, false) != note.describe(false, false));
        notes.push_back(note);
    }
    shop_struct shop; shop.pos = cell; shop.type = SHOP_ARMOUR_ANTIQUE;
    shop.shop_name = "Iloema Urghi"; shop.keeper_name.init(0);
    env.shop[cell] = shop; env.grid(cell) = DNGN_ENTER_SHOP;
    env.map_knowledge(cell).set_feature(DNGN_ENTER_SHOP);
    const Note found_shop = make_feature_note(cell);
    CHECK(found_shop.display_describe(false, false).find("古董防具") != string::npos);
    notes.push_back(found_shop);
    item_def item; item.base_type = OBJ_JEWELLERY; item.sub_type = RING_PROTECTION;
    item.rnd = 1; item.quantity = 1; item.plus = 3; item.flags = ISFLAG_IDENTIFIED;
    item.pos = ITEM_IN_INVENTORY; item.link = 0; item.slot = 'a';
    item.orig_place = level_id(BRANCH_DUNGEON, 3);
    const Note identified = make_identified_item_note(item, origin_desc(item));
    REQUIRE_FALSE(identified.screen.empty());
    CAPTURE(identified.name, identified.desc, identified.display_describe(false, false));
    CHECK(identified.display_describe(false, false).find("鉴定") != string::npos);
    CHECK(identified.display_describe(false, false).find("戒指") != string::npos);
    CHECK(identified.display_describe(false, false).find("地下城") != string::npos);
    CHECK(identified.display_describe(false, false).find("Identified") == string::npos);
    notes.push_back(identified);
    item_def randart = item;
    { rng::subgenerator isolated(1223, 4556); REQUIRE(make_item_randart(randart, true)); }
    const Note random_identified = make_identified_item_note(randart, origin_desc(randart));
    REQUIRE_FALSE(random_identified.screen.empty());
    CHECK(random_identified.display_describe(false, false).find("Identified") == string::npos);
    CHECK(random_identified.display_describe(false, false).find("戒指") != string::npos);
    notes.push_back(random_identified);
    const auto prior = get_unique_item_status(UNRAND_SINGING_SWORD);
    item_def unrand; unrand.quantity = 1;
    { rng::subgenerator isolated(1223, 4556); REQUIRE(make_item_unrandart(unrand, UNRAND_SINGING_SWORD)); }
    set_unique_item_status(unrand, prior);
    unrand.flags |= ISFLAG_IDENTIFIED; unrand.orig_place = level_id(BRANCH_DUNGEON, 3);
    const Note unique_identified = make_identified_item_note(unrand, origin_desc(unrand));
    REQUIRE_FALSE(unique_identified.screen.empty());
    CHECK(unique_identified.display_describe(false, false).find("Identified") == string::npos);
    CHECK(unique_identified.display_describe(false, false).find("歌唱之剑") != string::npos);
    notes.push_back(unique_identified);
    item.plus = 9;
    CHECK(identified.display_describe(false, false).find("+3") != string::npos);
    for (const auto &note : notes)
    {
        vector<unsigned char> saved; auto output = writer(&saved); note.save(output);
        auto input = reader(saved, TAG_MINOR_VERSION); Note restored; restored.load(input);
        CHECK(restored.describe(false, false) == note.describe(false, false));
        CHECK(restored.display_describe(false, false) == note.display_describe(false, false));
        Options.language = lang_t::EN;
        CHECK(restored.display_describe(false, false) == note.describe(false, false));
        Options.language = lang_t::ZH;
    }
    auto broken = identified; broken.screen.pop_back();
    CHECK(broken.display_describe(false, false) == broken.describe(false, false));
    broken = identified; broken.name += " custom";
    CHECK(broken.display_describe(false, false) == broken.describe(false, false));
    broken = found_shop; broken.name += " custom";
    CHECK(broken.display_describe(false, false) == broken.describe(false, false));
    unwind_var<vector<Note>> list(note_list, notes);
    unwind_var<vector<string>> order(Options.dump_order, {"notes"});
    unwind_var<string> directory(Options.morgue_dir, "../../../.term-audit/reported-four-gaps-runtime");
    unwind_var<decltype(you.save)> save(you.save, nullptr);
    REQUIRE(dump_char("native-feature-identification", true));
    formatted_string displayed; REQUIRE(read_morgue_display("native-feature-identification", displayed));
    CHECK(displayed.tostring().find("Found ") == string::npos);
    CHECK(displayed.tostring().find("Identified ") == string::npos);
    CHECK(displayed.tostring().find("古董防具") != string::npos);
    Note old_item(NOTE_ID_ITEM, 0, 0, identified.name, identified.desc);
    CHECK(old_item.display_describe(false, false) == old_item.describe(false, false));
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Reported four gaps fountain weapon and anatomy parameters retain original English", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name, "zh");
    unwind_var<string> savedir(Options.save_dir, "../../../.term-audit/database-07-runtime/");
    unwind_var<string> datadir(SysEnv.crawl_dir, ".");
    unwind_var<bool> io(crawl_state.io_inited, false);
    struct DatabaseScope { DatabaseScope(){databaseSystemInit();} ~DatabaseScope(){databaseSystemShutdown();} } databases;
    const ui_arg weapon = ui_text_arg("display.item", "eveningstar"); REQUIRE(weapon.available);
    const string named = ui_render("d.decor", "your %s", "", {weapon});
    const auto speech = getPairedMiscString("okawaru peaceful fountain_blue");
    const auto next = rng::peek_uint64();
    REQUIRE(speech.complete); CHECK(speech.english.find("@your_weapon@") != string::npos);
    const string english = replace_all(speech.english, "@your_weapon@", "your eveningstar");
    const string chinese = render_decor_line(speech, "your eveningstar", "your hands",
        {named, true}, {"你的双手", true});
    CHECK(chinese.find("昏星锤") != string::npos); CHECK(chinese.find("practiced discipline") == string::npos);
    CHECK(render_decor_line(speech, "your eveningstar", "your hands") == english);
    Options.language = lang_t::EN;
    CHECK(render_decor_line(speech, "your eveningstar", "your hands", {named, true}, {"你的双手", true}) == english);
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Reported four gaps corrosion uses matching typed actors without changing effects", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language);
    unwind_var<bool> io(crawl_state.io_inited, false);
    unwind_var<bool> arena(crawl_state.arena_suspended, true);
    unwind_var<int> width(crawl_view.msgsz.x, 180);
    unwind_var<decltype(you.props)> props(you.props);
    unwind_var<vector<player_equip_entry>> equipment(you.equipment.items, {});
    unwind_var<int> corrosion(you.duration[DUR_CORROSION]);
    unwind_var<decltype(you.redraw_armour_class)> redraw(you.redraw_armour_class);
    unwind_var<decltype(you.wield_change)> wield(you.wield_change);
    struct Cleanup { ~Cleanup(){clear_message_store();} } cleanup;
    for (auto type : {MONS_RUST_DEVIL, MONS_ACID_DRAGON, MONS_ENTROPY_WEAVER})
    {
        monster attacker; attacker.type = attacker.base_monster = type;
        attacker.hit_points = attacker.max_hit_points = 20; attacker.set_position(coord_def(10,10));
        const string canonical = uppercase_first(attacker.name(DESC_THE));
        uint64_t after = 0; int amount = 0, duration = 0;
        for (auto mode : {lang_t::EN, lang_t::ZH})
        {
            rng::subgenerator sample(12345, 6789); Options.language = mode;
            you.props[CORROSION_KEY] = 0; you.duration[DUR_CORROSION] = 0; clear_message_store();
            you.corrode(&attacker, canonical.c_str());
            const string log = get_last_messages(10, true);
            if (mode == lang_t::EN)
            { CHECK(log.find("corrodes you!") != string::npos); after = rng::peek_uint64();
              amount = you.props[CORROSION_KEY].get_int(); duration = you.duration[DUR_CORROSION]; }
            else
            { CHECK(log.find("corrodes you") == string::npos); CHECK(log.find("腐蚀") != string::npos);
              CHECK(you.props[CORROSION_KEY].get_int() == amount); CHECK(you.duration[DUR_CORROSION] == duration);
              CHECK(rng::peek_uint64() == after);
              CHECK_FALSE(ui_corrode_source_arg(&attacker, "new upstream source").available); }
        }
    }
}

TEST_CASE("Reported four gaps replacement ring rows preserve letters hotkeys and equipped status", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<species_type> species(you.species, SP_HUMAN);
    unwind_var<transformation> form(you.form, transformation::none);
    unwind_var<decltype(you.inv)> inventory(you.inv);
    unwind_var<vector<player_equip_entry>> equipment(you.equipment.items, {});
    for (int i = 0; i < 2; ++i)
    {
        auto &ring = you.inv[i]; ring.clear(); ring.base_type = OBJ_JEWELLERY;
        ring.sub_type = i ? RING_PROTECTION : RING_PROTECTION_FROM_FIRE;
        ring.quantity = 1; ring.flags = ISFLAG_IDENTIFIED; ring.plus = i ? 3 : 0;
        ring.pos = ITEM_IN_INVENTORY; ring.link = i; ring.slot = 'a' + i;
        you.equipment.items.emplace_back(i, SLOT_RING, false, false, false);
    }
    const auto next = rng::peek_uint64();
    for (int i = 0; i < 2; ++i)
    {
        const auto &ring = you.inv[i]; const char key = i ? '>' : '<';
        const string original = ring.name(DESC_INVENTORY);
        const string row = equipment_swap_choice_display(ring, key);
        CAPTURE(row);
        CHECK(row.find("或") != string::npos); CHECK(row.find("戒指") != string::npos);
        CHECK(row.find(string(1, ring.slot) + " - ") != string::npos);
        CHECK(row.find("已装备") != string::npos);
        CHECK(ring.name(DESC_INVENTORY) == original);
        Options.language = lang_t::EN;
        CHECK(equipment_swap_choice_display(ring, key)
              == string("<w>") + (key == '<' ? "<<" : ">") + "</w> or " + original);
        Options.language = lang_t::ZH;
    }
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Portal timer maps and disappearance display use reviewed templates", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), lua_close);
    REQUIRE(state);
    lua_State *ls = state.get(); luaL_openlibs(ls); cluaopen_crawl(ls);
    const auto next = rng::peek_uint64();
    REQUIRE(luaL_loadfile(ls, "../../tools/localization/portal_ui_checks.lua") == 0);
    const int result = lua_pcall(ls, 0, 0, 0);
    INFO((result ? lua_tostring(ls, -1) : "Portal map checks passed"));
    REQUIRE(result == 0);
    Options.language = lang_t::EN;
    REQUIRE(luaL_dostring(ls,
        "assert(crawl.ui_translate('lua.ui', 'The walls and floor vibrate strangely for a moment.') == 'The walls and floor vibrate strangely for a moment.'); "
        "for _, name in ipairs({'volcano','sewer','ossuary','icecave'}) do "
        "local message = 'There is an entrance to a ' .. name; "
        "assert(util.display_message(nil,message)==message) end; "
        "assert(util.display_message(nil,'You feel an oppressive heat about you.')=='You feel an oppressive heat about you.')") == 0);
    CHECK(rng::peek_uint64() == next);
}

TEST_CASE("Unidentified cosmetic equipment covers every visible equipment adjective", "[localization]")
{
    init_properties();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next = rng::peek_uint64();
    for (int subtype : {ARM_BOOTS, ARM_GLOVES, ARM_ROBE, ARM_CLOAK,
                        ARM_SCARF, ARM_HAT, ARM_LEATHER_ARMOUR, ARM_CHAIN_MAIL})
    for (auto cosmetic : {ISFLAG_RUNED, ISFLAG_GLOWING, ISFLAG_EMBROIDERED_SHINY})
    {
        item_def item; item.base_type=OBJ_ARMOUR; item.sub_type=subtype;
        item.quantity=1; item.plus=9; item.flags=cosmetic;
        const string original=item.name(DESC_A, false, false, false);
        const auto flags=item.flags;
        string shown;
        CAPTURE(subtype, cosmetic, original);
        REQUIRE(try_item_display_name(item,shown,false,false));
        CHECK(shown!=original); CHECK(shown.find("+9")==string::npos);
        CHECK(item.flags==flags); CHECK(item.name(DESC_A,false,false,false)==original);
        Options.language=lang_t::EN;
        CHECK_FALSE(try_item_display_name(item,shown,false,false));
        Options.language=lang_t::ZH;
    }
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported UI seven eleven monster tags preserve proper names and English", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next=rng::peek_uint64();
    for (auto type : {MONS_DONALD, MONS_JESSICA, MONS_ORC})
    {
        monster_info info(type);
        const string original=info.proper_name(DESC_PLAIN);
        CAPTURE(type, original);
        CHECK(info.display_name_tag()!=original);
        if (type==MONS_DONALD) CHECK(info.display_name_tag()=="唐纳德");
        CHECK(info.proper_name(DESC_PLAIN)==original);
        Options.language=lang_t::EN;
        CHECK(info.display_name_tag()==original);
        Options.language=lang_t::ZH;
    }
    monster_info named(MONS_ORC); named.mname="Pet Foo";
    CHECK(named.display_name_tag()=="Pet Foo");
    monster_info ghost(MONS_PLAYER_GHOST); ghost.mname="Cassandra";
    CHECK(ghost.display_name_tag()=="Cassandra");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported UI seven eleven annotation branches and play time retain hotkeys and values", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next=rng::peek_uint64();
    for (int i=0;i<NUM_BRANCHES;++i)
    {
        const auto br=static_cast<branch_type>(i);
        if (!branches[br].shortname) continue;
        const string original=make_stringf("(%c) %-14s ",branches[br].travel_shortcut,branches[br].shortname);
        const string shown=annotation_branch_choice(br);
        CAPTURE(br,shown);
        CHECK(shown.substr(0,4)==original.substr(0,4));
        const auto name=ui_text_arg("b.branch.name",branches[br].shortname);
        if (name.available) CHECK(shown.find(name.text)!=string::npos);
        Options.language=lang_t::EN;
        CHECK(annotation_branch_choice(br)==original);
        Options.language=lang_t::ZH;
    }
    CHECK(m_play_time_display("00:12:34",123)=="游戏时长：00:12:34（123回合）。");
    Options.language=lang_t::EN;
    CHECK(m_play_time_display("00:12:34",123)=="Play time: 00:12:34 (123 turns).");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported UI seven eleven rune locks use all reviewed visible rune names", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next=rng::peek_uint64();
    for (int rune=0;rune<NUM_RUNE_TYPES;++rune)
    {
        const string key=rune_type_name(rune);
        const string original="You insert the "+key+" rune into the lock.";
        const auto name=ui_text_arg("s.rune",key);
        CAPTURE(rune,key);
        REQUIRE(name.available);
        const string shown=ui_render("s.log","You insert the %s rune into the lock.",original,{name});
        CHECK(shown!=original); CHECK(shown.find(name.text)!=string::npos);
        const auto item=ui_text_arg("display.item",key+" rune of Zot");
        REQUIRE(item.available); CHECK(item.text==name.text+"佐特符文");
        Options.language=lang_t::EN;
        CHECK(ui_render("s.log","You insert the %s rune into the lock.",original,{name})==original);
        Options.language=lang_t::ZH;
    }
    CHECK(ui_render("s.log","You insert the %s rune into the lock.","unknown",{ui_text_arg("s.rune","upstream new rune")})=="unknown");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported UI seven eleven special shop components keep canonical owners and fallback", "[localization]")
{
    init_properties(); init_monsters();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto next=rng::peek_uint64();
    shop_struct shop; shop.type=SHOP_WEAPON; shop.pos=coord_def(12,12);
    shop.keeper_name.init(0); shop.shop_name="Lemm";
    shop.shop_type_name="Firing"; shop.shop_suffix_name="Range";
    ui_arg shown;
    CHECK(shop_name(shop,&shown)=="Lemm's Firing Range");
    REQUIRE(shown.available); CHECK(shown.text=="Lemm的射击场");
    const vector<pair<string,string>> components = {
        {"suffix","Accessories"},
        {"suffix","Aerie"},
        {"suffix","Armaments"},
        {"suffix","Armour"},
        {"suffix","Armours"},
        {"suffix","Armoury"},
        {"suffix","Artefacts"},
        {"suffix","Bar"},
        {"suffix","Books"},
        {"suffix","Boutique"},
        {"suffix","Catalogue"},
        {"suffix","Centre"},
        {"suffix","Chaos Theory"},
        {"suffix","Clinic"},
        {"suffix","Collection"},
        {"suffix","Corner"},
        {"suffix","Deal"},
        {"suffix","Deals"},
        {"suffix","Depot"},
        {"suffix","Elixirs"},
        {"suffix","Emporium"},
        {"suffix","Escapes"},
        {"suffix","Fire"},
        {"suffix","Forbidden Knowledge"},
        {"suffix","Fripperies"},
        {"suffix","Galleria"},
        {"suffix","Gallery"},
        {"suffix","Garage"},
        {"suffix","Goods"},
        {"suffix","Haberdashery"},
        {"suffix","Hardware"},
        {"suffix","Hoard"},
        {"suffix","Hole"},
        {"suffix","Hut"},
        {"suffix","Inventory"},
        {"suffix","Junkyard"},
        {"suffix","Kiosk"},
        {"suffix","Loot"},
        {"suffix","Magic"},
        {"suffix","Manuscripts"},
        {"suffix","Market"},
        {"suffix","Merchandise"},
        {"suffix","Millinery"},
        {"suffix","Mysteries"},
        {"suffix","Niceties"},
        {"suffix","Offerings"},
        {"suffix","Outfitters"},
        {"suffix","Paraphernalia"},
        {"suffix","Potions"},
        {"suffix","Products"},
        {"suffix","Quintessences"},
        {"suffix","Rack"},
        {"suffix","Range"},
        {"suffix","Readings"},
        {"suffix","Reliquary"},
        {"suffix","Resellers"},
        {"suffix","Reserve"},
        {"suffix","Resort"},
        {"suffix","Rings"},
        {"suffix","Rocks"},
        {"suffix","Room"},
        {"suffix","Runway"},
        {"suffix","Sales"},
        {"suffix","Scales"},
        {"suffix","Scrolls"},
        {"suffix","Secrets"},
        {"suffix","Smithy"},
        {"suffix","Spoils"},
        {"suffix","Station"},
        {"suffix","Store"},
        {"suffix","Sundries"},
        {"suffix","Supermarket"},
        {"suffix","Supplies"},
        {"suffix","Talismans"},
        {"suffix","Tomes"},
        {"suffix","Training"},
        {"suffix","Trifles"},
        {"suffix","Utilities"},
        {"suffix","Venue"},
        {"suffix","Wands"},
        {"suffix","Warehouse"},
        {"suffix","Wares"},
        {"suffix","Weapons"},
        {"suffix","Wisdom"},
        {"suffix","Xenodochy"},
        {"suffix","Yield"},
        {"suffix","Zone"},
        {"type","Acquisitions"},
        {"type","Advanced"},
        {"type","All or Nothing"},
        {"type","Alliterative"},
        {"type","Antique"},
        {"type","Assorted"},
        {"type","Avant-Garde"},
        {"type","Basic"},
        {"type","Bespoke"},
        {"type","Blasphemy"},
        {"type","Blood"},
        {"type","Bottled"},
        {"type","Bounteous"},
        {"type","CLEARANCE SALE!"},
        {"type","Changing"},
        {"type","Comfort"},
        {"type","Commerce"},
        {"type","Delphic"},
        {"type","Demolitions"},
        {"type","Demonic Rune"},
        {"type","Desiderata"},
        {"type","Do-It-Yourself"},
        {"type","Dragon-Slaying"},
        {"type","Easily-Learnt"},
        {"type","Eclectic"},
        {"type","Elemental"},
        {"type","Elven"},
        {"type","Ever-Affordable"},
        {"type","Evil"},
        {"type","Exquisite"},
        {"type","Fabulous"},
        {"type","Fathomable"},
        {"type","Fine"},
        {"type","Fine Metal"},
        {"type","Fire"},
        {"type","Firing"},
        {"type","Gadget"},
        {"type","General"},
        {"type","Gilded"},
        {"type","Globular"},
        {"type","Goods"},
        {"type","Hallowed"},
        {"type","Hat"},
        {"type","Haute Couture"},
        {"type","Humble"},
        {"type","Humungous"},
        {"type","Hydra"},
        {"type","Incredible"},
        {"type","Infernal"},
        {"type","Information"},
        {"type","Island"},
        {"type","Jewellery"},
        {"type","Jolly"},
        {"type","Killer"},
        {"type","Last-Chance"},
        {"type","Legendary"},
        {"type","Lightly-Used"},
        {"type","Luxury"},
        {"type","Magical"},
        {"type","Magnificent"},
        {"type","Mail-Order"},
        {"type","Mass"},
        {"type","Mayhem"},
        {"type","Merfolk"},
        {"type","Merry"},
        {"type","Miraculous"},
        {"type","Miscellaneous"},
        {"type","Must-Have"},
        {"type","Neck"},
        {"type","Needful"},
        {"type","Offbeat"},
        {"type","One-of-a-Kind"},
        {"type","Orb"},
        {"type","Overcharged"},
        {"type","Party"},
        {"type","Pet"},
        {"type","Platinum"},
        {"type","Practical"},
        {"type","Premium"},
        {"type","Profane"},
        {"type","Purification"},
        {"type","Quality"},
        {"type","Quotidian"},
        {"type","Rarities"},
        {"type","Ready-to-Wear"},
        {"type","Retail"},
        {"type","Round"},
        {"type","Sacrilege"},
        {"type","Self-Improvement"},
        {"type","Self-Published"},
        {"type","Serpentskin"},
        {"type","Specialty"},
        {"type","Specific"},
        {"type","Sphere"},
        {"type","Spherical"},
        {"type","Staff"},
        {"type","Store"},
        {"type","Superb"},
        {"type","Superior"},
        {"type","Superlative"},
        {"type","Tempting"},
        {"type","Timeless"},
        {"type","Tiny"},
        {"type","Tremendous"},
        {"type","Trivially-Accessible"},
        {"type","Unparalleled"},
        {"type","Vending"},
        {"type","Visitors'"},
        {"type","Vocational"},
        {"type","Wardrobe"},
        {"type","Weapon"},
        {"type","Whimsical"},
        {"type","Wicked"},
        {"type","Wondrous"},
        {"type","Xenial"},
        {"type","Yearning"},
        {"type","Zaibatsu"},
    };
    for (const auto &value:components)
    {
        shop.shop_type_name=value.first=="type" ? value.second : "Firing";
        shop.shop_suffix_name=value.first=="suffix" ? value.second : "Range";
        const string original=shop_name(shop);
        CAPTURE(value.first,value.second,original);
        CHECK(shop_name(shop,&shown)==original); REQUIRE(shown.available);
        CHECK(shown.text.find("Lemm")!=string::npos);
        Options.language=lang_t::EN;
        CHECK(shop_name(shop,&shown)==original); CHECK_FALSE(shown.available);
        Options.language=lang_t::ZH;
    }
    shop.shop_type_name="upstream special shop";
    const string original=shop_name(shop);
    CHECK(shop_name(shop,&shown)==original); CHECK_FALSE(shown.available);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported twelve twenty three breath short names translate real draconian speech", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> directory(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> datadir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    unwind_var<int> width(crawl_view.msgsz.x,200);
    struct Scope { Scope(){databaseSystemInit();} ~Scope(){databaseSystemShutdown();clear_message_store();} } databases;
    for (auto flavour : {BEAM_FIRE,BEAM_COLD,BEAM_ELECTRICITY,BEAM_NEG})
    {
        bolt beam; beam.flavour=beam.real_flavour=flavour;
        beam.name=flavour==BEAM_ELECTRICITY ? "bolt of lightning" : "bolt of dispelling energy";
        beam.pierce=flavour==BEAM_ELECTRICITY;
        const string beam_name=beam.get_short_name();
        string display; REQUIRE(try_ui_projectile_name(beam_name,display));
    }
    struct Example { const char *key; monster_type type; monster_type colour; const char *name; };
    for (const auto &value : {
        Example{"Quicksilver Bolt natural cast",MONS_DRACONIAN_SHIFTER,MONS_PURPLE_DRACONIAN,"dispelling energy"},
        Example{"Lightning Bolt natural cast",MONS_DRACONIAN_KNIGHT,MONS_BLACK_DRACONIAN,"lightning"},
        Example{"Fire Breath cast",MONS_RED_DRACONIAN,MONS_RED_DRACONIAN,"fire"},
        Example{"Cold Breath cast",MONS_WHITE_DRACONIAN,MONS_WHITE_DRACONIAN,"cold"}})
    {
        monster caster; caster.type=value.type; caster.base_monster=value.colour;
        caster.hit_points=caster.max_hit_points=20; caster.set_position(coord_def(10,10));
        auto speech=getPairedCastString(value.key);
        CAPTURE(value.key,speech.english,speech.translated);
        REQUIRE(speech.complete);
        const auto next=rng::peek_uint64();
        string shown; REQUIRE(try_ui_projectile_name(value.name,shown));
        speech.replace("@beam@",value.name,shown,true);
        speech.replace("@at@","at",ui_text_arg("mon.cast.preposition","at").text,true);
        speech.replace("@target@","you",ui_text_arg("mon.cast.target","you").text,true);
        clear_message_store();
        mons_speaks_msg(&caster,speech.english,MSGCH_MONSTER_SPELL,true,"",&speech);
        const string log=get_last_messages(10,true);
        CHECK(log.find(shown)!=string::npos); CHECK(log.find("breathes")==string::npos);
        CHECK(rng::peek_uint64()==next);
    }
}

TEST_CASE("Reported twelve twenty three chaos hits consume selected flavour and preserve fallback", "[localization]")
{
    init_monsters();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<int> hp(you.hp,20);
    monster rat; rat.type=rat.base_monster=MONS_RAT;
    rat.hit_points=rat.max_hit_points=10; rat.set_position(coord_def(10,10));
    bolt beam; beam.real_flavour=BEAM_CHAOS; beam.name="surge of chaos";beam.hit_verb="hits";
    const auto next=rng::peek_uint64();
    for (auto flavour : {BEAM_FIRE,BEAM_COLD,BEAM_ELECTRICITY,BEAM_POISON,BEAM_NEG,BEAM_HOLY,BEAM_ACID,BEAM_FOUL_FLAME})
    for (bool no_damage : {false,true})
    for (actor *victim : {static_cast<actor*>(&you),static_cast<actor*>(&rat)})
    {
        beam.flavour=flavour; string shown;
        REQUIRE(try_beam_hit_display("The %s %s %s%s%s%s",beam,*victim,no_damage,".",shown));
        CHECK(shown.find("混沌涌流")!=string::npos);
        CHECK(shown.find("伴随")!=string::npos); CHECK(ends_with(shown,"。"));
        CHECK((shown.find("，却未造成伤害")!=string::npos)==no_damage);
        CHECK(beam.flavour==flavour); CHECK(beam.real_flavour==BEAM_CHAOS);
    }
    beam.flavour=BEAM_NONE; string missing="untouched";
    CHECK_FALSE(try_beam_hit_display("The %s %s %s%s%s%s",beam,you,false,".",missing));
    CHECK(missing=="untouched");
    Options.language=lang_t::EN;beam.flavour=BEAM_NEG;
    CHECK_FALSE(try_beam_hit_display("The %s %s %s%s%s%s",beam,you,false,".",missing));
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported twelve twenty three klown verbs and spectral weapons translate without rerolling", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    monster clown;clown.type=clown.base_monster=MONS_KILLER_KLOWN;
    clown.hit_points=clown.max_hit_points=50;clown.set_position(coord_def(10,10));
    item_def weapon;weapon.base_type=OBJ_WEAPONS;weapon.sub_type=WPN_CLUB;
    weapon.quantity=1;weapon.plus=12;weapon.brand=SPWPN_SPECTRAL;weapon.flags=ISFLAG_IDENTIFIED;
    const auto next=rng::peek_uint64();
    for (const char *verb : {"hit","poke","prod","flog","pound","slap","tickle","defenestrate","sucker-punch","elbow","pinch","strangle-hug","squeeze","tease","eye-gouge","karate-kick","headlock","wrestle","trip-wire","kneecap"})
    {
        string shown; CAPTURE(verb);
        REQUIRE(try_ui_melee_hit("%s %s%s %s%s%s%s%s",clown,you,verb,"",&weapon,false,false,false,"",".",shown));
        CHECK(shown.find("杀戮小丑")!=string::npos);CHECK(shown.find("+12")!=string::npos);
        CHECK(shown.find("spectral")==string::npos);CHECK(ends_with(shown,"。"));
        const string original="The Killer Klown "+clown.conj_verb(verb)+" you but does no damage.";
        CHECK(ui_render("log.melee","%s %s %s but %s no damage.",original,
            {ui_actor_arg(clown),ui_text_arg("log.attack.verb",verb),ui_actor_arg(you),ui_literal_arg("")})!=original);
    }
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported twelve twenty three callbacks knockback banes and special touches use typed metadata", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<int> width(crawl_view.msgsz.x,180);
    unwind_var<species_type> species(you.species,SP_HUMAN);
    unwind_var<transformation> form(you.form,transformation::none);
    unwind_var<branch_type> branch(you.where_are_you,BRANCH_DUNGEON);
    unwind_var<int> potion(you.duration[DUR_NO_POTIONS],0);
    struct Cleanup { ~Cleanup(){clear_message_store();} } cleanup;
    const auto next=rng::peek_uint64();
    for (auto duration : {DUR_NO_POTIONS,DUR_BLIND,DUR_NO_SCROLLS,DUR_CANINE_FAMILIAR_DEAD})
    {
        Options.language=lang_t::EN;clear_message_store();duration_end_effect(duration);
        const string original=get_last_messages(10,true); REQUIRE_FALSE(original.empty());
        Options.language=lang_t::ZH;clear_message_store();duration_end_effect(duration);
        const string shown=get_last_messages(10,true);CHECK(shown!=original);
        CHECK(shown.find("Your ")==string::npos);CHECK(shown.find("You can ")==string::npos);
    }
    for (const char *source : {"torrent of water","lance of force","gust of wind","sudden gust","hand of Xom","blow"})
    {
        const auto cause=ui_knockback_cause_arg(source); REQUIRE(cause.available);
        const string original=string("You are knocked back by the ")+source+".";
        CHECK(ui_render("a.actor","%s is knocked back%s.",original,{ui_actor_arg(you),cause})!=original);
    }
    CHECK(ui_knockback_cause_arg("").available);
    CHECK_FALSE(ui_knockback_cause_arg("new upstream projectile").available);
    for (int i=0;i<NUM_BANES;++i)
    {
        const auto bane=static_cast<bane_type>(i); const string original="The "+bane_name(bane)+" upon you is lifted.";
        const auto arg=bane_display_arg(bane); REQUIRE(arg.available);CHECK(arg.text==bane_display_name(bane));
        CHECK(ui_render("m.log.283","The %s upon you is lifted.",original,{arg})!=original);
        Options.language=lang_t::EN;CHECK_FALSE(bane_display_arg(bane).available);Options.language=lang_t::ZH;
    }
    for (int i=1;i<NUM_SPECIAL_WEAPONS;++i)
    {
        const auto brand=static_cast<brand_type>(i); const string adj=brand_type_adj(brand);
        if (adj=="obsolete"||adj=="num_special"||adj=="debug") continue;
        monster_info lord(MONS_PANDEMONIUM_LORD);lord.mname="Deeh";
        lord.props[SPECIAL_WEAPON_KEY]=i;
        string shown; CAPTURE(i,adj);
        REQUIRE(try_monster_equipment_display(lord,shown,DESC_FULL,DESC_A,true));
        CHECK(shown.find("之触")!=string::npos);CHECK(shown.find("touch")==string::npos);
        CHECK(ghost_brand_name(brand,lord.type)==adj+" touch");
    }
    CHECK(string(ui_translate("t.fire","Non-targeted "))=="非指定目标：");
    Options.language=lang_t::EN;
    CHECK(string(ui_translate("t.fire","Non-targeted "))=="Non-targeted ");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported twelve twenty three Pandemonium map warnings keep canonical notes and milestones", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    std::unique_ptr<lua_State,decltype(&lua_close)> state(luaL_newstate(),lua_close);REQUIRE(state);
    lua_State *ls=state.get();luaL_openlibs(ls);cluaopen_crawl(ls);
    const auto next=rng::peek_uint64();
    REQUIRE(luaL_loadfile(ls,"../../tools/localization/pan_ui_checks.lua")==0);
    int result=lua_pcall(ls,0,0,0); INFO((result ? lua_tostring(ls,-1) : "Pan checks passed"));REQUIRE(result==0);
    Options.language=lang_t::EN;
    REQUIRE(luaL_dostring(ls,"pan_display_checks(false)")==0);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Pan encounter summary translates random lords groups and equipment from one canonical selection", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> directory(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> datadir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    unwind_var<int> width(crawl_view.msgsz.x,1200);
    struct Scope{Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();clear_message_store();}}database;
    vector<monster> mons(14);
    vector<monster*> list;
    const vector<monster_type> types={MONS_RAKSHASA,MONS_BALRUG,MONS_ORANGE_DEMON,MONS_ORANGE_DEMON,
        MONS_DEMONSPAWN_BLOOD_SAINT,MONS_DEMONSPAWN_BLOOD_SAINT,MONS_DEMONSPAWN_BLOOD_SAINT,
        MONS_PANDEMONIUM_LORD,MONS_ICE_FIEND,MONS_BLIZZARD_DEMON,MONS_BLIZZARD_DEMON,
        MONS_EXECUTIONER,MONS_BRIMSTONE_FIEND,MONS_DEMONSPAWN_CORRUPTER};
    for(size_t i=0;i<mons.size();++i)
    {
        auto &mon=mons[i];mon.type=mon.base_monster=types[i];
        mon.hit_points=mon.max_hit_points=50;mon.flags|=MF_WAS_IN_VIEW;
        mon.set_position(coord_def(10+int(i)%5,10+int(i)/5));
        if(mons_is_ghost_demon(mon.type)) mon.ghost.reset(new ghost_demon);
        if(mon.type==MONS_PANDEMONIUM_LORD)mon.mname="Zadaohuo";
        list.push_back(&mon);
    }
    unwind_var<item_def> first(env.item[0]),second(env.item[1]),third(env.item[2]);
    for(int i=0;i<3;++i)
    {
        auto &item=env.item[i];item.clear();item.base_type=OBJ_WEAPONS;item.quantity=1;item.rnd=1;
        item.flags=ISFLAG_IDENTIFIED;item.pos=ITEM_IN_MONSTER_INVENTORY;item.link=NON_ITEM+1;
    }
    env.item[0].sub_type=WPN_DEMON_WHIP;env.item[0].plus=2;env.item[0].brand=SPWPN_ELECTROCUTION;mons[1].inv[MSLOT_WEAPON]=0;
    env.item[1].sub_type=WPN_DAGGER;env.item[1].brand=SPWPN_VENOM;mons[4].inv[MSLOT_WEAPON]=1;
    env.item[2].sub_type=WPN_DEMON_BLADE;env.item[2].plus=2;env.item[2].brand=SPWPN_ANTIMAGIC;mons[13].inv[MSLOT_WEAPON]=2;
    auto announce=[&](const vector<monster*> &input){
        vector<monster*> empty,visible=input;clear_message_store();notice_new_monsters(empty,visible,SC_NONE);
        return get_last_messages(40,true);
    };
    string condensed,english,chinese;uint64_t next=0;
    Options.language=lang_t::EN;
    {rng::subgenerator sample(9876,5432);condensed=describe_monsters_condensed(list);next=rng::peek_uint64();}
    {rng::subgenerator sample(9876,5432);english=announce(list);next=rng::peek_uint64();}
    CHECK(replace_all(replace_all(english,"\n","")," ","").find(replace_all("You encounter "+condensed+"."," ",""))!=string::npos);
    CHECK(english.find("You encounter a rakshasa")!=string::npos);
    Options.language=lang_t::ZH;
    {rng::subgenerator sample(9876,5432);chinese=announce(list);CHECK(rng::peek_uint64()==next);}
    CAPTURE(english,chinese);
    CHECK(chinese.find("你遇到了")!=string::npos);CHECK(chinese.find("Zadaohuo")!=string::npos);
    CHECK(chinese.find("You encounter")==string::npos);CHECK(chinese.find(" is wielding ")==string::npos);
    CHECK(chinese.find("恶魔长鞭")!=string::npos);CHECK(chinese.find("匕首")!=string::npos);
    CHECK(chinese.find("恶魔之刃")!=string::npos);
    for(auto type:{MONS_TERENCE,MONS_SIGMUND,MONS_DONALD})
    {
        monster unique;unique.type=unique.base_monster=type;unique.hit_points=unique.max_hit_points=30;
        unique.flags|=MF_WAS_IN_VIEW;unique.set_position(coord_def(10,10));
        vector<monster*> one{&unique};
        Options.language=lang_t::EN;
        {rng::subgenerator sample(1234,5678);condensed=describe_monsters_condensed(one);next=rng::peek_uint64();}
        {rng::subgenerator sample(1234,5678);english=announce(one);next=rng::peek_uint64();}
        Options.language=lang_t::ZH;
        {rng::subgenerator sample(1234,5678);chinese=announce(one);CHECK(rng::peek_uint64()==next);}
        CAPTURE(type,english,chinese);
        CHECK(english.find(condensed)!=string::npos);CHECK(chinese.find("你遇到了")!=string::npos);
        CHECK(chinese.find("You encounter")==string::npos);
    }
    mons[7].flags|=MF_NAME_REPLACE;mons[7].mname="Unreviewed scripted creature";
    CHECK(announce(list).find("You encounter ")!=string::npos);
    CHECK(env.item[0].name(DESC_A).find("electrocution")!=string::npos);
}

TEST_CASE("Pan random lord holy and priced rune maps translate real declarations", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    std::unique_ptr<lua_State,decltype(&lua_close)> state(luaL_newstate(),lua_close);REQUIRE(state);
    lua_State *ls=state.get();luaL_openlibs(ls);cluaopen_crawl(ls);
    const auto next=rng::peek_uint64();
    REQUIRE(luaL_loadfile(ls,"../../tools/localization/pan_random_ui_checks.lua")==0);
    const int result=lua_pcall(ls,0,0,0);INFO((result?lua_tostring(ls,-1):"Random Pan checks passed"));REQUIRE(result==0);
    Options.language=lang_t::EN;REQUIRE(luaL_dostring(ls,"pan_random_checks(false)")==0);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Pan custom statue feature names translate actual vault renames and retain raw keys", "[localization]")
{
    init_show_table();init_feat_desc_cache();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> level(you.on_current_level,false);
    const coord_def cell(12,12);
    unwind_var<dungeon_feature_type> terrain(env.grid(cell),DNGN_METAL_STATUE);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<unsigned int> index(env.level_map_ids(cell),env.level_vaults.size());
    struct Cleanup{size_t n;~Cleanup(){env.level_vaults.resize(n);}}cleanup{env.level_vaults.size()};
    env.level_vaults.emplace_back(new vault_placement);
    auto &renames=env.level_vaults.back()->map.feat_renames;
    env.map_knowledge(cell).set_feature(DNGN_METAL_STATUE);
    const auto next=rng::peek_uint64();
    for(const char *key:{"dimensional conduit","weirdly glowing ball","arcane conduit","soul conduit","alchemical conduit","fiery conduit","icy conduit","earthen conduit","storm conduit","misfortune conduit"})
    {
        renames[DNGN_METAL_STATUE]=key;
        CHECK(raw_feature_description(cell)==key);
        const string original=feature_description_at(cell,false,DESC_A);
        string shown;CAPTURE(key,original);
        REQUIRE(try_feature_display_at(cell,false,shown));CHECK(shown!=key);
        CHECK(feature_description_at(cell,false,DESC_A)==original);
        Options.language=lang_t::EN;CHECK_FALSE(try_feature_display_at(cell,false,shown));
        CHECK(feature_description_at(cell,false,DESC_A)==original);Options.language=lang_t::ZH;
    }
    renames[DNGN_METAL_STATUE]="Unreviewed scripted feature";
    string shown="untouched";CHECK_FALSE(try_feature_display_at(cell,false,shown));CHECK(shown=="untouched");
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Wording HUD names preserve canonical duration metadata and English fallback", "[localization]")
{
    init_duration_index();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<int> will(you.duration[DUR_LOWERED_WL],100);
    unwind_var<int> scroll(you.duration[DUR_NO_SCROLLS],100);
    const auto next=rng::peek_uint64();
    for(int status:vector<int>{DUR_LOWERED_WL,STATUS_NO_SCROLL})
    {
        status_info original,shown,after;
        REQUIRE(fill_status_info(status,original));
        REQUIRE(fill_status_display_info(status,shown));
        CHECK(shown.light_text==(status==DUR_LOWERED_WL?"意志减半":"禁读"));
        status_info web_display;
        REQUIRE(fill_status_display_info(status,web_display,&original));
        CHECK(web_display.light_text==shown.light_text);
        CHECK(web_display.short_text==shown.short_text);
        CHECK(web_display.long_text==shown.long_text);
        CHECK(shown.light_colour==original.light_colour);
        REQUIRE(fill_status_info(status,after));
        CHECK(after.light_text==original.light_text);CHECK(after.short_text==original.short_text);
        CHECK(after.long_text==original.long_text);
        Options.language=lang_t::EN;
        REQUIRE(fill_status_display_info(status,shown));
        CHECK(shown.light_text==original.light_text);CHECK(shown.long_text==original.long_text);
        REQUIRE(fill_status_display_info(status,web_display,&original));
        CHECK(web_display.light_text==original.light_text);
        CHECK(web_display.short_text==original.short_text);
        CHECK(web_display.long_text==original.long_text);
        Options.language=lang_t::ZH;
    }
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported fourteen item appearances preserve unknown properties and English", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope{Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}}db;
    const auto next=rng::peek_uint64();
    for(const string adjective:{"glimmering","translucent","jewelled"})
    for(bool staff:{true,false})
    {
        item_def item;item.base_type=staff?OBJ_STAVES:OBJ_TALISMANS;
        item.sub_type=staff?static_cast<int>(STAFF_FIRE):static_cast<int>(TALISMAN_SPHINX);item.quantity=1;
        item.flags=ISFLAG_RANDART;item.props[ARTEFACT_NAME_KEY]="Hidden name";
        item.props[ARTEFACT_APPEAR_KEY]=adjective+(staff?" ":" riddle talisman");
        const string original=item.name(DESC_PLAIN);string shown;
        CAPTURE(staff,adjective,original);
        REQUIRE(try_item_display_name(item,shown,false,false));CHECK(shown!=original);
        CHECK(shown.find(staff?"法杖":"谜语护符")!=string::npos);
        CHECK(shown.find("Hidden")==string::npos);CHECK(shown.find("火焰")==string::npos);
        CHECK(item.name(DESC_PLAIN)==original);CHECK_FALSE(item.is_identified());
        Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(item,shown,false,false));Options.language=lang_t::ZH;
        item.props[ARTEFACT_APPEAR_KEY]=staff?"unknown adjective ":"unknown adjective riddle talisman";
        CHECK_FALSE(try_item_display_name(item,shown,false,false));
    }
    for(int type:{ARM_ICE_DRAGON_ARMOUR,ARM_STORM_DRAGON_ARMOUR})
    for(auto cosmetic:{ISFLAG_GLOWING,ISFLAG_RUNED,ISFLAG_EMBROIDERED_SHINY})
    {
        item_def item;item.base_type=OBJ_ARMOUR;item.sub_type=type;item.quantity=1;item.flags=cosmetic;item.plus=9;
        const string original=item.name(DESC_PLAIN);string shown;
        REQUIRE(try_item_display_name(item,shown,false,false));CHECK(shown!=original);
        CHECK(shown.find("+9")==string::npos);CHECK(item.name(DESC_PLAIN)==original);
    }
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported fourteen paired memories and hell effects use one canonical selection", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");
    unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope{Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}}db;
    for(int seed=0;seed<128;++seed)
    {
        rng::seed(seed);const string en=getSpeakString("nobody_recollection undead",true);const auto next=rng::peek_uint64();
        rng::seed(seed);const auto speech=getPairedSpeakString("nobody_recollection undead");
        CHECK(speech.english==en);REQUIRE(speech.complete);CHECK(rng::peek_uint64()==next);
        const string original=make_stringf("\"We remember... %s...\"",en.c_str());
        const string shown=ui_render("m.nobody.recollection","\"We remember... %s...\"",original,{{speech.translated,speech.complete}});
        CHECK(shown.find("我们记得")!=string::npos);
        CHECK(ui_render("m.nobody.recollection","\"We remember... %s...\"",original,{{"",false}})==original);
        for(const char *key:{"hell_effect_quiet","hell_effect_noisy"})
        {
            Options.language=lang_t::EN;rng::seed(seed);const auto a=getPairedMiscString(key);const auto after=rng::peek_uint64();
            Options.language=lang_t::ZH;rng::seed(seed);const auto b=getPairedMiscString(key);
            CAPTURE(key,b.english);REQUIRE(b.complete);CHECK(b.english==a.english);CHECK(b.text()!=b.english);CHECK(rng::peek_uint64()==after);
        }
    }
    CHECK(string(ui_translate("p.clause","ooze"))=="黏液");
    CHECK(string(ui_translate("p.duration.message","The barbed spikes snap loose."))=="倒刺脱落了。");
    init_properties();init_monsters();init_mut_index();
    unwind_var<bool> arena(crawl_state.arena_suspended,true);
    monster nobody;nobody.type=nobody.base_monster=MONS_NAMELESS_REVENANT;
    nobody.hit_points=nobody.max_hit_points=50;nobody.set_position(coord_def(10,10));
    const string original="You attempt to attack Nobody, but flinch away in fear!";
    CHECK(ui_render("combat.fear.actor","You attempt to attack %s, but flinch away in fear!",original,{m_actor_arg(nobody)})=="你试图攻击无名氏，却因恐惧而退缩！");
    Options.language=lang_t::EN;
    CHECK(ui_render("combat.fear.actor","You attempt to attack %s, but flinch away in fear!",original,{m_actor_arg(nobody)})==original);
}

TEST_CASE("Reported fourteen portal Lua keeps canonical notes and unknown names", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    std::unique_ptr<lua_State,decltype(&lua_close)> state(luaL_newstate(),lua_close);REQUIRE(state);
    lua_State *ls=state.get();luaL_openlibs(ls);cluaopen_crawl(ls);
    const auto next=rng::peek_uint64();
    REQUIRE(luaL_loadfile(ls,"../../tools/localization/reported_portal_checks.lua")==0);
    const int result=lua_pcall(ls,0,0,0);INFO((result?lua_tostring(ls,-1):"Portal checks passed"));REQUIRE(result==0);
    Options.language=lang_t::EN;REQUIRE(luaL_dostring(ls,"reported_portal_checks(false)")==0);CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Reported fourteen look handles Lua markers without evaluating them twice", "[localization]")
{
    init_show_table();init_feat_desc_cache();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<bool> level(you.on_current_level,false);
    const coord_def cell(12,12);
    unwind_var<int> floor_item(env.igrid(cell),NON_ITEM);
    unwind_var<dungeon_feature_type> terrain(env.grid(cell),DNGN_METAL_STATUE);
    unwind_var<map_cell> knowledge(env.map_knowledge(cell));
    unwind_var<unsigned int> index(env.level_map_ids(cell),env.level_vaults.size());
    struct Cleanup{size_t n;~Cleanup(){env.level_vaults.resize(n);}}cleanup{env.level_vaults.size()};
    env.level_vaults.emplace_back(new vault_placement);
    auto &renames=env.level_vaults.back()->map.feat_renames;
    env.map_knowledge(cell)=map_cell();env.map_knowledge(cell).set_feature(DNGN_METAL_STATUE);
    struct Marker:map_lua_marker{mutable int calls=0;string property(const string&)const override{++calls;return "";}};
    auto *marker=new Marker;marker->pos=cell;env.markers.add(marker);
    struct Remove{map_marker*p;~Remove(){env.markers.remove(p);}}remove{marker};
    for(const char *key:{"weirdly glowing ball","metal staircase leading up","one-way staircase leading down"})
    {
        renames[DNGN_METAL_STATUE]=key;
        marker->calls=0;Options.language=lang_t::EN;const string original=get_square_desc(cell);const int count=marker->calls;
        Options.language=lang_t::ZH;const string shown=get_square_desc(cell);
        CHECK(shown.find(key)==string::npos);CHECK(shown!=original);CHECK(marker->calls-count==count);
    }
    renames[DNGN_METAL_STATUE]="unknown authored terrain";
    CHECK(get_square_desc(cell).find("unknown authored terrain")!=string::npos);
}

TEST_CASE("Collectible morgue notes translate all new and legacy rune gem and orb records", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto next=rng::peek_uint64();
    for(auto base:{OBJ_RUNES,OBJ_GEMS,OBJ_ORBS})
    for(int sub=0;sub<(base==OBJ_RUNES?NUM_RUNE_TYPES:base==OBJ_GEMS?NUM_GEM_TYPES:1);++sub)
    {
        item_def item;item.base_type=base;item.sub_type=sub;item.quantity=1;item.rnd=1;
        item.flags=ISFLAG_IDENTIFIED;item.pos=ITEM_IN_INVENTORY;item.link=0;item.slot='a';
        const int time=base==OBJ_GEMS?121:0;
        const string canonical=item.name(DESC_A);
        vector<Note> notes{Note(NOTE_GET_ITEM,time,0,canonical,origin_desc(item)),make_obtained_item_note(item,time)};
        REQUIRE_FALSE(notes.back().screen.empty());
        for(const auto &note:notes)
        {
            const string original=note.describe(false,false);const string shown=note.display_describe(false,false);
            CAPTURE(base,sub,original,shown);CHECK(shown!=original);CHECK(shown.find("Got ")==string::npos);
            if(time)CHECK(shown.find("13")!=string::npos);
            vector<unsigned char> bytes;auto out=writer(&bytes);note.save(out);
            auto input=reader(bytes,TAG_MINOR_VERSION);Note restored;restored.load(input);
            CHECK(restored.describe(false,false)==original);CHECK(restored.display_describe(false,false)==shown);
            Options.language=lang_t::EN;CHECK(restored.display_describe(false,false)==original);Options.language=lang_t::ZH;
            vector<unsigned char> after;auto again=writer(&after);note.save(again);CHECK(bytes==after);
        }
        auto broken=notes.back();broken.screen.pop_back();CHECK(broken.display_describe(false,false)==broken.describe(false,false));
        CHECK(item.name(DESC_A)==canonical);
    }
    Note unknown(NOTE_GET_ITEM,0,0,"an unknown collectible");CHECK(unknown.display_describe(false,false)==unknown.describe(false,false));
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Collectible morgue notes retain random Pan lord names and translate their typed role", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    for(const string name:{"Zadaohuo","Xov the Unnamed"})
    for(auto type:{NOTE_SEEN_MONSTER,NOTE_DEFEAT_MONSTER})
    {
        Note note(type,MONS_PANDEMONIUM_LORD,0,name,type==NOTE_DEFEAT_MONSTER?"killed":"");
        const string original=note.describe(false,false),shown=note.display_describe(false,false);
        CAPTURE(original,shown);CHECK(shown!=original);CHECK(shown.find(name)!=string::npos);
        CHECK(shown.find("魔窟领主")!=string::npos);CHECK(shown.find("pandemonium lord")==string::npos);
        Options.language=lang_t::EN;CHECK(note.display_describe(false,false)==original);Options.language=lang_t::ZH;
    }
    Note unknown(NOTE_SEEN_MONSTER,MONS_ORC,0,"Unreviewed custom species");
    CHECK(unknown.display_describe(false,false)==unknown.describe(false,false));
}

TEST_CASE("Collectible morgue notes translate portal identities and legacy records without parsing English", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    for(const string name:{"Lehudib's Moon Base","Rutra's Hidden Sanctum","The Name-Rending Infernalists' Reservoir"})
    {
        const string original="Entered "+name;
        Note typed=make_location_note(original,"portal",name),legacy(NOTE_MESSAGE,0,0,original);
        const string shown=typed.display_describe(false,false);CHECK(shown!=original);CHECK(shown==legacy.display_describe(false,false));
        vector<unsigned char> bytes;auto out=writer(&bytes);typed.save(out);auto input=reader(bytes,TAG_MINOR_VERSION);Note restored;restored.load(input);
        CHECK(restored.display_describe(false,false)==shown);CHECK(restored.describe(false,false)==original);
        Options.language=lang_t::EN;CHECK(restored.display_describe(false,false)==original);Options.language=lang_t::ZH;
        typed.name+=" modified";CHECK(typed.display_describe(false,false)==typed.describe(false,false));
    }
    for(const string name:{"Mnoleg","Lom Lobon","Cerebov","Gloorx Vloq"})
    {
        const string original="Entered the realm of "+name+".";
        auto typed=make_location_note(original,"realm",name);Note legacy(NOTE_MESSAGE,0,0,original);
        CHECK(typed.display_describe(false,false)!=original);CHECK(typed.display_describe(false,false)==legacy.display_describe(false,false));
    }
    for(const string name:{"An unreviewed portal","Lehudib's Moon Base extra"})
    {
        const string original="Entered "+name;const auto typed=make_location_note(original,"portal",name);
        CHECK(typed.display_describe(false,false)==original);Note legacy(NOTE_MESSAGE,0,0,original);CHECK(legacy.display_describe(false,false)==original);
    }
    CHECK(make_location_note("Edited note","portal","Lehudib's Moon Base").desc.empty());
}

TEST_CASE("Collectible morgue Lua note API stores canonical location metadata", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<vector<Note>> list(note_list,{});
    struct Active{bool old=notes_are_active();Active(){activate_notes(true);}~Active(){activate_notes(old);}} active;
    std::unique_ptr<lua_State,decltype(&lua_close)> state(luaL_newstate(),lua_close);REQUIRE(state);
    lua_State *ls=state.get();luaL_openlibs(ls);cluaopen_crawl(ls);
    REQUIRE(luaL_dostring(ls,"crawl.take_note(\"Entered Lehudib's Moon Base\", 'portal', \"Lehudib's Moon Base\"); crawl.take_note('Entered the realm of Mnoleg.', 'realm', 'Mnoleg'); crawl.take_note('Unreviewed user note')")==0);
    REQUIRE(note_list.size()==3);CHECK(starts_with(note_list[0].desc,"morgue-location-v1\n"));
    CHECK(note_list[0].display_describe(false,false)=="进入了雷胡迪布的月球基地");
    CHECK(note_list[1].display_describe(false,false).find("Mnoleg")==string::npos);
    CHECK(note_list[2].display_describe(false,false)=="Unreviewed user note");
}

TEST_CASE("Collectible morgue welcome goal messages keep every canonical random choice", "[localization]")
{
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char *> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> data_dir(SysEnv.crawl_dir,".");unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope{Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}}db;
    bool found=false;
    for(const char *key:{"welcome_spam","welcome_spam dungeon descent","welcome_spam Halloween"})
    for(int seed=0;seed<256;++seed)
    {
        Options.language=lang_t::EN;rng::seed(seed);const auto original=getPairedMiscString(key);const auto next=rng::peek_uint64();
        Options.language=lang_t::ZH;rng::seed(seed);const auto shown=getPairedMiscString(key);
        CAPTURE(key,seed,shown.english);REQUIRE(shown.complete);CHECK(shown.english==original.english);CHECK(shown.text()!=shown.english);CHECK(rng::peek_uint64()==next);
        found|=shown.english=="If you return without finding the Orb of Zot, so far below, it's all over!";
    }
    CHECK(found);CHECK(string(ui_translate("d.feature.name","garden patch"))=="花圃");
}

TEST_CASE("Collectible morgue file exports keep canonical English paths and bilingual notes", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char*> lang(Options.lang_name,"zh");
    unwind_var<bool> level(you.on_current_level,false);
    unwind_var<decltype(you.save)> save(you.save,nullptr);
    unwind_var<vector<string>> order(Options.dump_order,{"notes"});
    unwind_var<string> directory(Options.morgue_dir,"../../../.term-audit/morgue-collectibles-runtime");
    vector<Note> notes;
    for(auto base:{OBJ_RUNES,OBJ_GEMS,OBJ_ORBS})
    {
        item_def item;item.base_type=base;item.sub_type=0;item.quantity=1;item.rnd=1;
        item.flags=ISFLAG_IDENTIFIED;item.pos=ITEM_IN_INVENTORY;item.link=0;item.slot='a';
        notes.push_back(make_obtained_item_note(item,base==OBJ_GEMS?121:0));
    }
    notes.push_back(Note(NOTE_SEEN_MONSTER,MONS_PANDEMONIUM_LORD,0,"Zadaohuo"));
    notes.push_back(make_location_note("Entered Lehudib's Moon Base","portal","Lehudib's Moon Base"));
    notes.push_back(Note(NOTE_MESSAGE,0,0,"Entered Rutra's Hidden Sanctum"));
    unwind_var<vector<Note>> list(note_list,notes);
    const auto next=rng::peek_uint64();
    bool localized_written = false;
    REQUIRE(dump_char("native-collectibles",true,false,nullptr,true,&localized_written));
    CHECK(localized_written);
    const auto read=[&](const string &name){std::ifstream f(morgue_directory()+name);return string(std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>());};
    const string english=read("native-collectibles.txt"),chinese=read("native-collectibles.zh.txt");
    CHECK(english.find("Got ")!=string::npos);CHECK(english.find("Entered Lehudib's Moon Base")!=string::npos);
    CHECK(chinese.find("Got ")==string::npos);CHECK(chinese.find("the pandemonium lord")==string::npos);
    CHECK(chinese.find("佐特宝珠")!=string::npos);CHECK(chinese.find("符文")!=string::npos);CHECK(chinese.find("宝石")!=string::npos);
    CHECK(chinese.find("雷胡迪布的月球基地")!=string::npos);CHECK(chinese.find("鲁特拉的隐秘圣所")!=string::npos);
    REQUIRE(dump_char("native-collectibles-en",true,false,nullptr,false,&localized_written));
    CHECK_FALSE(localized_written);
    CHECK(read("native-collectibles-en.txt")==english);
    // Block only the optional Chinese path; canonical output must still succeed.
    string blocked = morgue_directory() + "native-collectibles-blocked.zh.txt";
    REQUIRE(check_mkdir("test Chinese dump failure", &blocked, true));
    localized_written = true;
    REQUIRE(dump_char("native-collectibles-blocked",true,false,nullptr,true,&localized_written));
    CHECK_FALSE(localized_written);
    CHECK(read("native-collectibles-blocked.txt")==english);
    localized_written = true;
    Options.language=lang_t::EN;
    REQUIRE(dump_char("native-collectibles-language-en",true,false,nullptr,true,&localized_written));
    CHECK_FALSE(localized_written);
    CHECK(read("native-collectibles-language-en.txt")==english);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Morgue status books place Pan lord roles inside the actor and preserve old sources", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    vector<Note> notes{Note(NOTE_SEEN_MONSTER,MONS_PANDEMONIUM_LORD,0,"Ylod Wia"),
        Note(NOTE_SEEN_MONSTER,MONS_PANDEMONIUM_LORD,0,"Fubb")};
    unwind_var<vector<Note>> list(note_list,notes);
    CHECK(notes[0].display_describe(false,false)=="遇见魔窟领主Ylod Wia");
    Note kill(NOTE_DEFEAT_MONSTER,MONS_PANDEMONIUM_LORD,0,"Ylod Wia","killed");
    CHECK(kill.display_describe(false,false)=="杀死魔窟领主Ylod Wia");
    Note paralysis(NOTE_PARALYSIS,45,0,"Fubb");
    CHECK(paralysis.display_describe(false,false)=="被魔窟领主Fubb麻痹，持续4.5回合");
    Note vex(NOTE_VEXED,4,0,"Fubb");CHECK(vex.display_describe(false,false)!=vex.describe(false,false));
    Note mutation(NOTE_GET_MUTATION,MUT_FANGS,3,"Fubb");CHECK(mutation.display_describe(false,false)!=mutation.describe(false,false));
    for(const auto &note:vector<Note>{notes[0],kill,paralysis,vex,mutation})
    {
        const string original=note.describe(false,false),shown=note.display_describe(false,false);
        vector<unsigned char> bytes;auto out=writer(&bytes);note.save(out);auto input=reader(bytes,TAG_MINOR_VERSION);Note restored;restored.load(input);
        CHECK(restored.display_describe(false,false)==shown);CHECK(restored.describe(false,false)==original);
        Options.language=lang_t::EN;CHECK(restored.display_describe(false,false)==original);Options.language=lang_t::ZH;
    }
    note_list.clear();CHECK(paralysis.display_describe(false,false)==paralysis.describe(false,false));
    paralysis.desc="morgue-pan-source-v1\nFubb";
    CHECK(paralysis.display_describe(false,false)=="被魔窟领主Fubb麻痹，持续4.5回合");
    paralysis.name="another source";CHECK(paralysis.display_describe(false,false)==paralysis.describe(false,false));
}

TEST_CASE("Morgue status books cover every mutation level and species growth source", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<species_type> race(you.species,SP_HUMAN);
    const auto next=rng::peek_uint64();size_t checked=0;
    for(int id=0;id<NUM_MUTATIONS;++id)
    {
        const auto mut=static_cast<mutation_type>(id);if(!mutation_name(mut))continue;
        for(int level=1;level<=mutation_max_levels(mut);++level)
        for(auto type:{NOTE_GET_MUTATION,NOTE_LOSE_MUTATION,NOTE_PERM_MUTATION})
        {
            Note note(type,id,type==NOTE_LOSE_MUTATION?level-1:level,"Vine Stalker growth");
            const string original=note.describe(false,false),shown=note.display_describe(false,false);
            CAPTURE(id,level,type,original,shown);CHECK(shown!=original);CHECK(shown.find("Vine Stalker")==string::npos);
            CHECK(shown.find("Gained mutation")==string::npos);CHECK(shown.find("藤蔓人的成长")!=string::npos);
            Options.language=lang_t::EN;CHECK(note.display_describe(false,false)==original);Options.language=lang_t::ZH;++checked;
        }
    }
    CHECK(checked>500);
    for(auto species:species::get_all_species())
    {
        Note note(NOTE_GET_MUTATION,MUT_FANGS,3,species::name(species)+" growth");
        CAPTURE(species,note.describe(false,false));CHECK(note.display_describe(false,false)!=note.describe(false,false));
    }
    for(const char *reason:{"demonic ancestry","potion of mutation","hidden defects","hidden potential","hidden defects expressed",
        "hidden potential expressed","wizard power","mutagenic glow","Jiyva's grace","Makhleb's blessing","Xom's mischief",
        "Xom's grace","Xom's lifesaving","Ru sacrifice","Equipping an artefact","the wrath of Zin","the darkness of Dithmenos"})
    {
        Note note(NOTE_GET_MUTATION,MUT_FANGS,3,reason);CAPTURE(reason);CHECK(note.display_describe(false,false)!=note.describe(false,false));
    }
    Note unknown(NOTE_GET_MUTATION,MUT_FANGS,3,"unreviewed upstream reason");CHECK(unknown.display_describe(false,false)==unknown.describe(false,false));
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Morgue status books preserve typed random book authors and canonical titles", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<const char*> lang(Options.lang_name,"zh");
    unwind_var<string> savedir(Options.save_dir,"../../../.term-audit/database-07-runtime/");
    unwind_var<string> datadir(SysEnv.crawl_dir,".");unwind_var<bool> io(crawl_state.io_inited,false);
    struct Scope{Scope(){databaseSystemInit();}~Scope(){databaseSystemShutdown();}}db;
    item_def book;book.base_type=OBJ_BOOKS;book.quantity=1;init_book_theme_randart(book,{SPELL_MAGIC_DART});
    book.flags|=ISFLAG_IDENTIFIED;book.props[BOOK_TITLED_KEY]=true;
    book.props[ARTEFACT_NAME_KEY]="Jusinoud's Precepts of Stones";
    const string original=book.name(DESC_PLAIN);const auto next=rng::peek_uint64();string shown;
    REQUIRE(try_item_display_name(book,shown,false,false));CHECK(shown=="Jusinoud的石块之训诫");
    CHECK(book.name(DESC_PLAIN)==original);CHECK(book.props[ARTEFACT_NAME_KEY].get_string()=="Jusinoud's Precepts of Stones");
    CHECK_FALSE(try_randbook_title("Jusinoud's Precepts of Stones",shown));
    REQUIRE(try_randbook_title("Jusinoud's Precepts of Stones",shown,"",true));
    CHECK_FALSE(try_randbook_title("Jusinoud's Precepts of Unknown Future School",shown,"",true));
    REQUIRE(try_randbook_title("Jusinoud's Elementary Magic",shown,"",true));CHECK(shown=="Jusinoud的初等魔法");
    Options.language=lang_t::EN;CHECK_FALSE(try_item_display_name(book,shown,false,false));CHECK(book.name(DESC_PLAIN)==original);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Morgue status books export corrected role source and mutation records in separate files", "[localization]")
{
    init_properties();init_monsters();init_mut_index();init_duration_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);unwind_var<const char*> lang(Options.lang_name,"zh");
    unwind_var<bool> level(you.on_current_level,false);unwind_var<decltype(you.save)> save(you.save,nullptr);
    unwind_var<vector<string>> order(Options.dump_order,{"notes"});
    unwind_var<string> directory(Options.morgue_dir,"../../../.term-audit/morgue-status-books-runtime");
    unwind_var<vector<Note>> list(note_list,{Note(NOTE_SEEN_MONSTER,MONS_PANDEMONIUM_LORD,0,"Fubb"),
        Note(NOTE_PARALYSIS,45,0,"Fubb"),Note(NOTE_GET_MUTATION,MUT_FANGS,3,"Vine Stalker growth"),
        Note(NOTE_DEFEAT_MONSTER,MONS_PANDEMONIUM_LORD,0,"Ylod Wia","killed")});
    const auto next=rng::peek_uint64();REQUIRE(dump_char("native-status-books",true));
    const auto read=[&](const string &name){std::ifstream f(morgue_directory()+name);return string(std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>());};
    const string en=read("native-status-books.txt"),zh=read("native-status-books.zh.txt");
    CHECK(en.find("Paralysed by Fubb for 4.5 turns")!=string::npos);CHECK(en.find("Gained mutation:")!=string::npos);
    CHECK(zh.find("遇见魔窟领主Fubb")!=string::npos);CHECK(zh.find("杀死魔窟领主Ylod Wia")!=string::npos);
    CHECK(zh.find("被魔窟领主Fubb麻痹")!=string::npos);CHECK(zh.find("藤蔓人的成长")!=string::npos);
    CHECK(zh.find("Paralysed by")==string::npos);CHECK(zh.find("Gained mutation:")==string::npos);
    REQUIRE(dump_char("native-status-books-explicit-en",true,false,nullptr,false));CHECK(read("native-status-books-explicit-en.txt")==en);
    Options.language=lang_t::EN;REQUIRE(dump_char("native-status-books-language-en",true));CHECK(read("native-status-books-language-en.txt")==en);
    CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Species monster list covers friendly neutral and summoned visible rows", "[localization]")
{
    init_monsters();unwind_var<lang_t> language(Options.language,lang_t::ZH);
    const auto next=rng::peek_uint64();
    for(auto attitude:{ATT_HOSTILE,ATT_FRIENDLY,ATT_NEUTRAL})
    for(auto type:{MONS_BALL_PYTHON,MONS_IRON_DRAGON,MONS_REAPER})
    for(auto flag:{MB_CHARMED,MB_SUMMONED,MB_ALLY_TARGET,MB_BREATH_WEAPON})
    for(int count:{1,2})
    for(bool past:{false,true})
    {
        monster_info mi(type);mi.attitude=attitude;mi.mb.set(flag);
        vector<monster_info> monsters(count,mi);
        Options.language=lang_t::EN;const string original=monster_list_description(monsters,past);
        Options.language=lang_t::ZH;const string shown=monster_list_description(monsters,past);
        CAPTURE(attitude,type,flag,count,past,original,shown);CHECK(shown!=original);
        CHECK(shown.find("You can see")==string::npos);CHECK(shown.find("You could see")==string::npos);
        CHECK(shown.find("friendly")==string::npos);CHECK(shown.find("summoned")==string::npos);
        Options.language=lang_t::EN;CHECK(monster_list_description(monsters,past)==original);Options.language=lang_t::ZH;
    }
    monster_info unknown(MONS_ORC);unknown.mb.set(MB_NAME_REPLACE);unknown.mname="Unreviewed creature";unknown.attitude=ATT_FRIENDLY;
    Options.language=lang_t::EN;const string original=monster_list_description({unknown});Options.language=lang_t::ZH;
    CHECK(monster_list_description({unknown})==original);CHECK(rng::peek_uint64()==next);
}

TEST_CASE("Species monster list all player species names resolve creation and welcome labels", "[localization]")
{
    init_properties();init_monsters();init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    for(auto race:species::get_all_species())
    {
        const ui_arg label=ui_text_arg("display.creation",species::name(race));CAPTURE(race,species::name(race));REQUIRE(label.available);
        const string welcome=character_welcome_display(false,"Koorghiu",race,JOB_SUMMONER);CHECK_FALSE(welcome.empty());
        CHECK(welcome.find(species::name(race))==string::npos);
    }
    CHECK(string(ui_translate("display.creation","White Draconian"))=="白龙人");
    Options.language=lang_t::EN;CHECK(string(ui_translate("display.creation","White Draconian"))=="White Draconian");
}


TEST_CASE("Feedback five appendage weapon descriptions preserve canonical English", "[localization]")
{
    init_properties(); init_mut_index(); init_duration_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<species_type> race(you.species, SP_COGLIN);
    unwind_var<god_type> god(you.religion, GOD_NO_GOD);
    unwind_var<transformation> form(you.form, transformation::none);
    item_def sword;
    sword.base_type=OBJ_WEAPONS; sword.sub_type=WPN_SHORT_SWORD;
    sword.quantity=1; sword.flags=ISFLAG_IDENTIFIED;
    const auto state=rng::peek_uint64();
    for (species_type species : {SP_COGLIN, SP_POLTERGEIST, SP_HUMAN, SP_OCTOPODE})
    {
        you.species=species;
        const string original=get_item_description(sword, IDM_MONSTER);
        CHECK(original.find("It is a one-")!=string::npos);
        const string shown=get_item_description(sword, IDM_MONSTER, true);
        CHECK(shown.find("It is a one-")==string::npos);
        CHECK(shown.find(ui_translate("a.bodypart", species::hand_name(species).c_str()))!=string::npos);
        CHECK(get_item_description(sword, IDM_MONSTER)==original);
        Options.language=lang_t::EN;
        CHECK(get_item_description(sword, IDM_MONSTER, true)==original);
        Options.language=lang_t::ZH;
    }
    CHECK(string(ui_translate("display.item.state","quivered"))=="已备");
    Options.language=lang_t::EN;
    CHECK(string(ui_translate("display.item.state","quivered"))=="quivered");
    CHECK(rng::peek_uint64()==state);
}

TEST_CASE("Feedback five identification remap messages use display names and English fallback", "[localization]")
{
    init_properties(); init_mut_index();
    unwind_var<lang_t> language(Options.language,lang_t::ZH);
    unwind_var<decltype(you.inv)> inventory(you.inv);
    for (auto &item : you.inv) item.clear();
    unwind_var<char> potion_key(Options.potion_shortcuts[POT_CANCELLATION],'C');
    unwind_var<char> scroll_key(Options.scroll_shortcuts[SCR_IDENTIFY],'i');
    const auto state=rng::peek_uint64();
    for (bool scroll : {false,true})
    {
        item_def &item=you.inv[MAX_GEAR];
        item.clear(); item.base_type=scroll ? OBJ_SCROLLS : OBJ_POTIONS;
        item.sub_type=scroll ? SCR_IDENTIFY : POT_CANCELLATION;
        item.quantity=scroll ? 5 : 1; item.flags=ISFLAG_IDENTIFIED;
        item.pos=ITEM_IN_INVENTORY; item.link=MAX_GEAR;
        for (auto lang : {lang_t::ZH,lang_t::EN})
        {
            Options.language=lang; item.slot=scroll ? 'e' : 'd';
            clear_message_store();
            CHECK(auto_assign_item_slot(item)==nullptr);
            CHECK(item.slot==(scroll ? 'i' : 'C'));
            const string original=item.name(DESC_INVENTORY);
            const string message=get_last_messages(10,true);
            CHECK(message.find(scroll ? "e -> i - " : "d -> C - ")!=string::npos);
            if (lang==lang_t::ZH)
            {
                CHECK(message.find(scroll ? "鉴定卷轴" : "消除药水")!=string::npos);
                CHECK(message.find(scroll ? "scrolls of identify" : "potion of cancellation")==string::npos);
            }
            else
                CHECK(message.find(original)!=string::npos);
        }
    }
    CHECK(rng::peek_uint64()==state);
}

TEST_CASE("Feedback six god messages preserve canonical translation keys", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    for (auto god : {GOD_SHINING_ONE, GOD_ZIN, GOD_WU_JIAN})
    {
        for (const char *event : {" welcomes you!", " welcomes you back!"})
        {
            Options.language = lang_t::ZH;
            clear_message_store();
            simple_god_message(event, false, god);
            const string chinese = get_last_messages(10, true);
            CHECK(chinese.find("欢迎") != string::npos);
            CHECK(chinese.find("welcomes") == string::npos);
            if (god == GOD_SHINING_ONE)
                CHECK(chinese.find("光明神") != string::npos);
            Options.language = lang_t::EN;
            clear_message_store();
            simple_god_message(event, false, god);
            CHECK(get_last_messages(10, true).find(god_speaker(god) + event) != string::npos);
        }
    }
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Feedback six enchant armour identification and fire prompts", "[localization]")
{
    init_properties(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<decltype(you.inv)> inventory(you.inv);
    for (auto &item : you.inv) item.clear();
    item_def &item = you.inv[MAX_GEAR];
    item.base_type = OBJ_SCROLLS; item.sub_type = SCR_ENCHANT_ARMOUR;
    item.quantity = 1; item.flags = ISFLAG_IDENTIFIED;
    item.pos = ITEM_IN_INVENTORY; item.link = MAX_GEAR; item.slot = 'a';
    string display;
    REQUIRE(try_item_display_name(item, display, true));
    CHECK(display == "a - 防具附魔卷轴");
    CHECK(item.name(DESC_INVENTORY) == "a - a scroll of enchant armour");
    for (const char *prompt : {"<lightgray>Fire/throw/use which item?</lightgray>",
                              "<lightgray>Fire/use which item?</lightgray>"})
    {
        Options.language = lang_t::ZH;
        CHECK(string(ui_translate("t.prompt", prompt)).find("哪件物品") != string::npos);
        Options.language = lang_t::EN;
        CHECK(string(ui_translate("t.prompt", prompt)) == prompt);
    }
}

TEST_CASE("Feedback seven unarmed menu and monster will preserve English fallback", "[localization]")
{
    init_properties(); init_monsters(); init_mut_index();
    init_duration_index(); init_spell_descs();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    unwind_var<transformation> form(you.form, transformation::none);
    const auto random_state = rng::peek_uint64();
    for (const char *name : {"Unarmed", "Claws", "Teeth and claws", "Tentacles"})
    {
        Options.language = lang_t::ZH;
        const ui_arg shown = get_form()->uc_attack_display_arg(name);
        CHECK(shown.available);
        CHECK(shown.text.find(name) == string::npos);
        Options.language = lang_t::EN;
        CHECK_FALSE(get_form()->uc_attack_display_arg(name).available);
    }
    monster_info rat(MONS_RAT);
    for (auto lang : {lang_t::ZH, lang_t::EN})
    {
        Options.language = lang;
        describe_info description; bool stats = false;
        get_monster_db_desc(rat, description, stats);
        REQUIRE(stats);
        const string body = formatted_string::parse_string(description.body.str()).tostring();
        bool found = false;
        for (const string &row : split_string("\n", body))
            if (row.find(lang == lang_t::ZH ? "意志" : "Will") != string::npos)
            {
                found = true;
                CHECK(row.find(lang == lang_t::ZH ? "无" : "none") != string::npos);
                if (lang == lang_t::ZH) CHECK(row.find("none") == string::npos);
            }
        CHECK(found);
    }
    CHECK(rng::peek_uint64() == random_state);
}

TEST_CASE("Ice movement confirmations translate every effect or fall back as a whole", "[localization]")
{
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    const vector<vector<string>> lists = {{"icy armour"}, {"frozen ramparts"},
        {"icy armour", "frozen ramparts"}, {"icy armour", "unknown custom effect"}};
    for (const auto &effects : lists)
    {
        const string original = "Your " + comma_separated_line(effects.begin(), effects.end())
            + " will break if you move. Continue?";
        Options.language = lang_t::ZH;
        const string shown = m_ice_move_prompt(effects, original);
        if (effects.back() == "unknown custom effect") CHECK(shown == original);
        else
        {
            CHECK(shown.find("继续吗？") != string::npos);
            if (effects.front() == "icy armour") CHECK(shown.find("寒冰护甲") != string::npos);
            if (effects.back() == "frozen ramparts") CHECK(shown.find("冰霜壁垒") != string::npos);
            CHECK(shown.find("Your ") == string::npos);
        }
        Options.language = lang_t::EN;
        CHECK(m_ice_move_prompt(effects, original) == original);
    }
    CHECK(rng::peek_uint64() == state);
}

TEST_CASE("Hammer and scythe discoveries translate visible appearances without identifying them", "[localization]")
{
    init_properties(); init_mut_index();
    unwind_var<lang_t> language(Options.language, lang_t::ZH);
    const auto state = rng::peek_uint64();
    for (bool scythe : {false, true})
        for (int flags : vector<int>{0, ISFLAG_GLOWING, ISFLAG_RUNED, ISFLAG_IDENTIFIED})
        {
            item_def item;
            item.base_type = OBJ_WEAPONS; item.sub_type = scythe ? WPN_HALBERD : WPN_MACE;
            item.quantity = 1; item.flags = flags; item.plus = 7;
            item.props[ITEM_NAME_KEY] = scythe ? "scythe" : "hammer";
            const string original = item.name(DESC_A);
            Options.language = lang_t::ZH;
            const ui_arg name = ui_item_arg(item);
            REQUIRE(name.available);
            CHECK(name.text.find(scythe ? "长柄镰刀" : "锤子") != string::npos);
            const string found = ui_render("t.discovery", "Found %s.", "Found " + original + ".", {name});
            CHECK(found.find("发现") != string::npos);
            CHECK(found.find("Found ") == string::npos);
            CHECK(item.name(DESC_A) == original);
            if (!(flags & ISFLAG_IDENTIFIED)) CHECK(name.text.find("+7") == string::npos);
            Options.language = lang_t::EN;
            CHECK_FALSE(ui_item_arg(item).available);
            CHECK(ui_render("t.discovery", "Found %s.", "Found " + original + ".", {ui_item_arg(item)})
                == "Found " + original + ".");
        }
    Options.language = lang_t::ZH;
    item_def custom;
    custom.base_type = OBJ_WEAPONS; custom.sub_type = WPN_MACE; custom.quantity = 1;
    custom.props[ITEM_NAME_KEY] = "unknown custom hammer";
    CHECK_FALSE(ui_item_arg(custom).available);
    CHECK(rng::peek_uint64() == state);
}
