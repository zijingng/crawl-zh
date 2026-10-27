#pragma once

// A display paragraph is committed only when all clauses and entity names
// are available. Canonical English is built by the original caller.
class hints_paragraph
{
public:
    explicit hints_paragraph(const char *message_context) : context(message_context) {}
    void add(const char *source, initializer_list<ui_arg> args = {})
    {
        if (args.size())
        {
            const string rendered = ui_render(context, source, "", args);
            available &= !rendered.empty();
            text += rendered;
        }
        else
        {
            const ui_arg rendered = ui_text_arg(context, source);
            available &= rendered.available;
            text += rendered.text;
        }
    }
    void append(const ui_arg &part)
    {
        available &= part.available;
        text += part.text;
    }
    ui_arg result() const { return {text, available}; }
private:
    const char *context;
    string text;
    bool available = true;
};

static string _hint_key(command_type cmd)
{
    string key = "%";
    insert_commands(key, {cmd});
    return key;
}

static ui_arg _hint_target_display(bool spells = false)
{
    hints_paragraph p("h.item");
    p.add("You will then enter target mode with the nearest monster or previous target selected. "
          "Cycle through visible hostile monsters with <w>+</w> or <w>-</w>. Press <w>f</w>, "
          "<w>Enter</w> or <w>.</w> to fire. If you miss, <w>%s</w> fires at the same target again.", {ui_literal_arg(_hint_key(spells ? CMD_CAST_SPELL : CMD_FIRE) + (spells ? "ap" : "f"))});
    return p.result();
}

static ui_arg _hint_ability_display(const item_def &item)
{
    hints_paragraph p("h.item");
    if (!item_is_equipped(item))
    {
        switch (item.base_type)
        {
        case OBJ_WEAPONS:
            p.add("First wield it with <w>%s</w>. ", {ui_literal_arg(_hint_key(CMD_WIELD_WEAPON))});
            break;
        case OBJ_ARMOUR:
            p.add("First wear it with <w>%s</w>. ", {ui_literal_arg(_hint_key(CMD_WEAR_ARMOUR))});
            break;
        case OBJ_JEWELLERY:
            p.add("First put it on with <w>%s</w>. ", {ui_literal_arg(_hint_key(CMD_WEAR_JEWELLERY))});
            break;
        default: return {"", false};
        }
    }
    p.add("Open the ability menu with <w>%s</w> and select the corresponding ability. Activation may "
          "fail, especially without training.", {ui_literal_arg(_hint_key(CMD_USE_ABILITY))});
    return p.result();
}

static ui_arg _hint_item_display(const item_def &item)
{
    hints_paragraph p("h.item");
    auto resist = [&](const char *source) {
        p.add(source, {ui_literal_arg(_hint_key(CMD_RESISTS_SCREEN))});
#ifdef USE_TILE
        p.add(" You can also right-click your avatar.");
#endif
    };
    switch (item.base_type)
    {
    case OBJ_WEAPONS:
        if (is_artefact(item) && item.is_identified())
        {
            if (gives_ability(item))
            {
                p.add("When wielded, some weapons (such as this one) offer abilities you can activate. ");
                p.append(_hint_ability_display(item));
            }
            else if (gives_resistance(item))
                resist("This weapon offers protection from certain damage sources. Press <w>%s</w> to view your "
                      "resistances and other attributes.");
            return p.result();
        }
        if (!item_is_equipped(item))
        {
            p.add("Wield this weapon with <w>%s</w>, or switch between weapons in slots a and b with "
                  "<w>%s</w>. Adjust item slots with <w>%s</w>i.", {ui_literal_arg(_hint_key(CMD_WIELD_WEAPON)), ui_literal_arg(_hint_key(CMD_WEAPON_SWAP)), ui_literal_arg(_hint_key(CMD_ADJUST_INVENTORY))});
            const skill_type current = item_attack_skill(item);
            skill_type best = is_range_weapon(item) ? SK_THROWING
                : best_skill(SK_SHORT_BLADES, SK_STAVES);
            if (you.skills[SK_UNARMED_COMBAT] > you.skills[best])
                best = SK_UNARMED_COMBAT;
            if (you.skills[current] + 2 < you.skills[best])
            {
                p.add("\nYou've trained <w>%s</w> for a while, so consider continuing with it instead of "
                      "<w>%s</w>. Press <w>%s</w> to see the skill values.", {ui_text_arg("display.skill", skill_name(best)), ui_text_arg("display.skill", skill_name(current)), ui_literal_arg(_hint_key(CMD_DISPLAY_SKILLS))});
            }
        }
        else if (is_range_weapon(item))
        {
#ifdef USE_TILE
            p.add("Left-click a monster to attack it. ");
#endif
            p.add("Press <w>%s</w> to fire your ranged weapon. ", {ui_literal_arg(_hint_key(CMD_PRIMARY_ATTACK))});
            p.append(_hint_target_display());
        }
        else
        p.add("To attack a monster, simply walk into it.");
        break;
    case OBJ_MISSILES:
        if (is_throwable(&you, item))
        {
            p.add("%s can be fired with <w>%s</w> without a launcher. ", {ui_item_arg(item), ui_literal_arg(_hint_key(CMD_FIRE))});
            if (item.slot)
            {
                p.add("Press <w>%s</w> to fire, then <w>%s</w> to select this item. ", {ui_literal_arg(_hint_key(CMD_FIRE)), ui_literal_arg(string(1, item.slot))});
            }
            else
            {
                p.add("Press <w>%s</w> to fire and select this item. ", {ui_literal_arg(_hint_key(CMD_FIRE))});
            }
            p.append(_hint_target_display());
        }
        break;
    case OBJ_ARMOUR:
    {
        bool wearable = true;
        if (you.get_innate_mutation_level(MUT_HORNS) > 0 && is_hard_helmet(item))
        {
                p.add("Your horns prevent you from wearing helmets. Press <w>%s</w> to view your mutations and innate abilities.", {ui_literal_arg(_hint_key(CMD_DISPLAY_MUTATIONS))});
            wearable = false;
        }
        else if (item.sub_type == ARM_BARDING)
        {
                p.add("Only nagas and armataurs can wear barding.");
            wearable = false;
        }
        else
        {
                p.add("Wear armour with <w>%s</w> and remove it with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_WEAR_ARMOUR)), ui_literal_arg(_hint_key(CMD_REMOVE_ARMOUR))});
#ifdef USE_TILE
                p.add(" Clicking its tile also performs either action.");
#endif
        }
        if (Hints.hints_type == HINT_MAGIC_CHAR
            && get_armour_slot(item) == SLOT_BODY_ARMOUR
            && !is_effectively_light_armour(&item))
        {
                p.add("\nBody armour with a high encumbrance rating may hinder spellcasting. Robes and leather "
                      "armour are generally safe for aspiring spellcasters.");
        }
        else if (Hints.hints_type == HINT_MAGIC_CHAR && is_shield(item))
        {
                p.add("\nShields hinder spellcasting until your Shields skill is high enough to remove the penalty.");
        }
        else if (Hints.hints_type == HINT_RANGER_CHAR && is_offhand(item))
        {
                p.add("\nMany ranged weapons require two hands and cannot be used with an offhand item.");
        }
        if (!item.is_identified() && (is_artefact(item)
            || get_equip_desc(item) != ISFLAG_NO_DESC))
        {
                p.add("\n\nWeapons and armour with unusual descriptions like this are more likely to have higher "
                      "enchantment or special properties, good or bad.");
        }
        if (wearable)
        {
            if (gives_resistance(item))
                resist("This armour offers protection from certain sources. Press <w>%s</w> to view your "
                      "resistances and other attributes.");
            if (gives_ability(item))
            {
                    p.add("\n\nWhen worn, some armour (such as this one) offers abilities you can activate with <w>%s</w>. ", {ui_literal_arg(_hint_key(CMD_USE_ABILITY))});
                p.append(_hint_ability_display(item));
            }
        }
        break;
    }
    case OBJ_WANDS:
        p.add("Evoke this wand with <w>%s</w> to release its magic.", {ui_literal_arg(_hint_key(CMD_EVOKE))});
#ifdef USE_TILE
    {
        string modifier;
#ifdef USE_TILE_WEB
        modifier = "Ctrl + Shift";
#else
#if defined(UNIX) && defined(USE_TILE_LOCAL)
        if (!tiles.is_fullscreen()) modifier = "Ctrl + Shift";
        else
#endif
        modifier = "Alt";
#endif
            p.add(" Alternatively, hold <w>%s</w> and <w>left-click</w> a monster (or yourself) and select a "
                  "wand, or <w>left-click</w> the wand tile and then the target.", {ui_literal_arg(modifier)});
        break;
    }
#else
        break;
#endif
    case OBJ_SCROLLS:
        p.add("Read this scroll with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_READ))});
#ifdef USE_TILE
        p.add(" You can also left-click it.");
#endif
        break;
    case OBJ_JEWELLERY:
        p.add("Put on jewellery with <w>%s</w> and remove it with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_WEAR_JEWELLERY)), ui_literal_arg(_hint_key(CMD_REMOVE_JEWELLERY))});
#ifdef USE_TILE
        p.add(" Clicking it in your inventory also performs either action.");
#endif
        if (gives_resistance(item))
        {
            if (item.sub_type < NUM_RINGS)
                resist("This ring offers protection from certain sources. Press <w>%s</w> to view your "
                      "resistances and other attributes.");
            else
                resist("This amulet offers protection from certain sources. Press <w>%s</w> to view your "
                      "resistances and other attributes.");
        }
        if (gives_ability(item))
        {
            p.add("\n\nWhen worn, some jewellery (such as this one) offers abilities you can activate with <w>%s</w>. ", {ui_literal_arg(_hint_key(CMD_USE_ABILITY))});
            p.append(_hint_ability_display(item));
        }
        break;
    case OBJ_POTIONS:
        p.add("Quaff this potion with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_QUAFF))});
#ifdef USE_TILE
        p.add(" You can also left-click it.");
#endif
        break;
    case OBJ_BOOKS:
        if (item.sub_type == BOOK_MANUAL)
        {
            p.add("A manual greatly helps train a skill. After you pick it up, the corresponding skill "
                  "trains more efficiently until the manual's contents are exhausted.");
        }
        else
        {
            p.add("\nPick up a spellbook to add its spells to your library. View the library with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_MEMORISE_SPELL))});
        }
        p.append(ui_literal_arg("\n"));
        break;
    case OBJ_CORPSES:
        p.add("Skeletons and corpses can be components for certain necromantic spells, but are otherwise largely useless.");
        break;
    case OBJ_STAVES:
        p.add("This staff enhances the power of spells of its related school.");
        if (gives_resistance(item))
            resist("It also offers protection from certain sources. Press <w>%s</w> to view your resistances "
                  "and other attributes.");
        else if (you_worship(GOD_TROG))
        {
            p.add("\n\nSince %s disapproves of magic, this staff is of little use to you. You may as well drop "
                  "it with <w>%s</w>.", {ui_text_arg("display.god", god_name(GOD_TROG, false)), ui_literal_arg(_hint_key(CMD_DROP))});
        }
        break;
    case OBJ_MISCELLANY:
        p.add("Some miscellaneous items contain magical powers you can harness by evoking them with <w>%s</w>.", {ui_literal_arg(_hint_key(CMD_EVOKE))});
        break;
    default: return {"", false};
    }
    return p.result();
}

static ui_arg _hint_memorise_display()
{
    hints_paragraph p("h.hints");
    p.add("This screen shows the spells in your spell library. From here you can memorise spells by "
          "selecting them, as well as view spell descriptions, search for them, and organize them. "
          "As a conjurer, you start with five memorisable spells, and one already memorised: Magic "
          "Dart. (To view memorised spells, you can exit this menu and select <w>I</w>.)");
    if (player_has_available_spells())
    {
        p.add("\n\nA spell that isn't <darkgray>grayed out</darkgray> or <lightred>forbidden</lightred> "
              "can be memorised right away by selecting it at this menu.");
    }
    else if (you.spell_no)
    {
        p.add("\n\nYou cannot memorise any more spells right now. This will change as you gain levels and "
              "Spellcasting proficiency. ");
    }
    else
    {
        p.add("\n\nYou cannot memorise any spells right now. This will change as you gain levels and "
              "Spellcasting proficiency. ");
    }
    if (you.spell_no)
    {
#ifdef USE_TILE
        p.add("\n\nTo cast a spell, hold <w>Control</w> and <w>left-click</w> a monster (or yourself), "
              "then select a spell. You can also switch to the spellcasting display by <w>clicking</w> "
              "its <w>tab</w>.");
#endif
        p.add("\n\nPress <w>%s</w> and choose a spell, e.g. <w>a</w> (check with <w>?</w>). For attack spells, ", {ui_literal_arg(_hint_key(CMD_CAST_SPELL))});
        p.append(_hint_target_display(true));
    }
    return p.result();
}

static ui_arg _hint_cloud_display(const cloud_struct &cloud)
{
    hints_paragraph p("h.hints");
    string display;
    const bool available = try_cloud_display_name(cloud.type, true, display);
    const ui_arg cloud_name = {display, available};
    if (is_harmless_cloud(cloud.type))
    {
        p.add("%s is harmless. ", {cloud_name});
    }
    else if (is_damaging_cloud(cloud.type, true))
    {
        p.add("%s is probably dangerous; stay out of it if you can. ", {cloud_name});
    }
    else
    {
        p.add("%s is currently harmless, but that could change. Check your resistances on the overview screen (<w>%s</w>).", {cloud_name, ui_literal_arg(_hint_key(CMD_RESISTS_SCREEN))});
    }
    if (is_opaque_cloud(cloud.type))
    {
        p.add("\nThis cloud is opaque. You cannot see a square if two or more opaque clouds lie between you and it.");
    }
    return p.result();
}

static ui_arg _hint_feature_display(const coord_def &where)
{
    hints_paragraph p("h.hints");
    const auto feat = env.grid(where);
    if (feat_is_trap(feat))
    {
        if (feat == DNGN_TRAP_SHAFT)
        { p.add("The dungeon contains a number of natural obstacles such as shafts, which lead one to "
              "three levels down. Once you know the shaft is there, you can safely step over it.\nIf you "
              "want to jump down there, use <w>></w> to do so. Be warned that getting back here might be "
              "difficult."); }
        else if (feat == DNGN_TRAP_WEB)
        { p.add("Some areas of the dungeon, such as the Spider Nest, may be strewn with giant webs that "
              "may ensnare you for a short time. Insects, oozes and incorporeal entities can navigate "
              "the webs safely."); }
        else
        { p.add("These nasty constructions can cause a range of unpleasant effects. You won't be able to "
              "avoid tripping traps by flying over them; their magic construction will cause them to be "
              "triggered anyway."); }
    }
    else if (feat == DNGN_EXIT_DUNGEON)
    { p.add("These stairs lead out of the dungeon. Following them will end the game. The only way to "
          "win is to transport the fabled Orb of Zot outside."); }
    else if (feat_is_stone_stair(feat))
    {
        if (feat_stair_direction(feat) == CMD_GO_DOWNSTAIRS)
        { p.add("You can enter the next (deeper) level by following them down (<w>></w>). To get back to "
              "this level again, press <w><<</w> while standing on the upstairs."); }
        else
        { p.add("You can enter the previous (shallower) level by following these up (<w><<</w>). This is "
              "ideal for retreating or finding a safe resting spot, since the previous level will have "
              "less monsters, and monsters on this level can't follow you up unless they're standing "
              "right next to you. To get back to this level again, press <w>></w> while standing on the "
              "downstairs."); }
#ifdef USE_TILE
        p.add(" In Tiles, you can achieve the same, in either direction, by clicking the <w>left mouse button</w>.");
#endif
        if (is_unknown_stair(where))
        { p.add("\n\nYou have not yet passed through this particular set of stairs. "); }
    }
    else if (feat_is_escape_hatch(feat))
    { p.add("Escape hatches can be used to quickly leave a level with <w><<</w> and <w>></w>, "
          "respectively. Note that you will usually be unable to return right away."); }
#if TAG_MAJOR_VERSION == 34
    else if (feat == DNGN_ENTER_PORTAL_VAULT)
        return {"", false}; // The existing TextDB portal paragraph owns this.
#endif
    else if (feat == DNGN_CLOSED_DOOR || feat == DNGN_CLOSED_CLEAR_DOOR)
    {
        if (!Hints.hints_explored)
        { p.add("\nTo avoid accidentally opening a door you'd rather remain closed during travel or "
              "autoexplore, you can mark it with an exclusion from the map view (<w>X</w>) with "
              "<w>ee</w> while your cursor is on the grid in question. Such an exclusion will prevent "
              "autotravel from ever entering that grid until you remove the exclusion with another press "
              "of <w>Xe</w>."); }
    }
    else if (feat_is_altar(feat))
    {
        const god_type altar = feat_altar_god(feat);
        if (you_worship(GOD_NO_GOD))
        {
            p.add("This is your chance to join a religion! In general, the gods will help their followers, "
                  "bestowing powers of all sorts upon them, but many of them demand a life of dedication, "
                  "constant tributes or entertainment in return.\n");
            if (altar == GOD_ECUMENICAL)
            { p.add("This particular altar is so ancient that you cannot make out which god it is dedicated "
                  "to! Converting here by pressing <w>></w> while standing on the altar will enter you into "
                  "service of a random god, who will grant you some additional piety in thanks."); }
            else
            {
                p.add("Stand on the altar and press <w>></w> for information about <w>%s</w>. You will be asked "
                      "to confirm before converting.", {ui_text_arg("display.god", god_name(altar))});
            }
        }
        else if (you_worship(altar))
            return p.result();
        else if (altar == GOD_ECUMENICAL)
        {
            p.add("This altar is too ancient to identify its god, and %s probably will not like a change of "
                  "allegiance. If you accept the risk, stand on it and press <w>></w> to serve a random god, "
                  "who will grant some extra piety.", {ui_text_arg("display.god", god_name(you.religion))});
        }
        else
        {
            p.add("%s probably will not like a change of allegiance, but looking is safe. Stand on the altar "
                  "and press <w>></w> for information about <w>%s</w>. You will be asked to confirm before "
                  "converting and abandoning your current faith.\nPress <w>^</w> to see your current standing "
                  "with %s.", {ui_text_arg("display.god", god_name(you.religion)), ui_text_arg("display.god", god_name(altar)), ui_text_arg("display.god", god_name(you.religion))});
#ifdef USE_TILE
            p.add(" You can also hold <w>Shift</w> and <w>right-click</w> your avatar.");
#endif
        }
    }
    else if (feat_is_branch_entrance(feat))
    {
        p.add("An entryway into one of the many dungeon side branches in Crawl. ");
        if (feat != DNGN_ENTER_TEMPLE)
        { p.add("Beware, sometimes these can be deadly!"); }
    }
    return p.result();
}

static ui_arg _hint_load_display()
{
    hints_paragraph p("h.event");
    p.add("Welcome back! If it has been a while, these screens can refresh your memory:\nInventory (<w>%s</w>)", {ui_literal_arg(_hint_key(CMD_DISPLAY_INVENTORY))});
    if (you.spell_no > 0)
    { p.add(", spells (<w>%s?</w>)", {ui_literal_arg(_hint_key(CMD_CAST_SPELL))}); }
    if (!your_talents().empty())
    { p.add(", abilities (<w>%s</w>)", {ui_literal_arg(_hint_key(CMD_USE_ABILITY))}); }
    if (Hints.hints_type != HINT_MAGIC_CHAR || you.has_any_mutations())
    { p.add(", mutations (<w>%s</w>)", {ui_literal_arg(_hint_key(CMD_DISPLAY_MUTATIONS))}); }
    if (!you_worship(GOD_NO_GOD))
    { p.add(", religious standing (<w>%s</w>)", {ui_literal_arg(_hint_key(CMD_DISPLAY_RELIGION))}); }
    p.add(", message history (<w>%s</w>), character overview (<w>%s</w>) and dungeon overview (<w>%s</w>).", {ui_literal_arg(_hint_key(CMD_REPLAY_MESSAGES)), ui_literal_arg(_hint_key(CMD_RESISTS_SCREEN)), ui_literal_arg(_hint_key(CMD_DISPLAY_OVERMAP))});
    return p.result();
}

static ui_arg _hint_excommunicate_display(god_type old_god, god_type new_god,
                                         int old_piety)
{
    hints_paragraph p("h.event");
    if (new_god == GOD_NO_GOD)
    {
        if (old_piety < 1)
        {
            p.add("Uh-oh, %s excommunicated you because your piety ran out. Perhaps you repeatedly broke the "
                  "religious rules, did not please your god often enough, or both. Find an altar to %s to "
                  "re-convert, and all will be well.", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(old_god))});
        }
        else
        {
            p.add("If abandoning %s no longer seems wise and you want to return to your old faith, find an "
                  "altar to %s and re-convert; all will be well.", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(old_god))});
        }
        if (god_hates_your_god(old_god, new_god))
        {
            p.add(" Otherwise you must endure this god's displeasure until their wrath is spent.");
        }
        return p.result();
    }
    bool angry = false;
    if (is_good_god(old_god))
    {
        if (is_good_god(new_god))
        {
            p.add("Fortunately, %s does not seem to mind your conversion to %s. ", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(new_god))});
            if (old_piety > piety_breakpoint(0))
            { p.add("You even kept some piety! "); }
            p.add("This alliance exists only among the three good gods; do not expect it to be the norm.");
        }
        else if (!god_hates_your_god(old_god))
        {
            p.add("Fortunately, %s does not seem to mind your conversion to %s. As one of the good gods, %s "
                  "usually forgives changes of faith, unless you take the path of evil, in which case "
                  "retribution can be nasty!", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(new_god)), ui_text_arg("display.god", god_name(old_god))});
        }
        else
        {
            p.add("%s did not appreciate your conversion to %s! What else could you expect from changing "
                  "from a good god to an evil one? %s would have been more forgiving towards a god on the "
                  "same side of the faith. ", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(new_god)), ui_text_arg("display.god", god_name(old_god))});
            angry = true;
        }
    }
    else if (god_hates_your_god(old_god))
    {
        p.add("%s did not appreciate your conversion to %s! Usually only the three good gods forgive "
              "this kind of faithlessness. ", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(new_god))});
        angry = true;
    }
    if (angry)
    {
        p.add("Unfortunately, converting back would appease %s but annoy %s, so you must suffer the "
              "wrath of one god or the other.", {ui_text_arg("display.god", god_name(old_god)), ui_text_arg("display.god", god_name(new_god))});
    }
    return p.result();
}
