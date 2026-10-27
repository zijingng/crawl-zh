/**
 * @file
 * @brief Code for the god menu.
 * @todo The god menu.
 */

#include "AppHdr.h"
#include "i18n.h"

#include "god-menu.h"

#include "colour.h"
#include "libutil.h"
#include "religion.h"
#include "terrain.h"
#include "tilepick.h"
#include "tileview.h"

GodMenuEntry::GodMenuEntry(god_type god_, bool long_name) :
    MenuEntry(god_name(god_, long_name), MEL_ITEM, 1, 0),
    god(god_)
{
    if (god == GOD_SHINING_ONE)
        hotkeys.push_back('1');
    else
    {
        hotkeys.push_back(text.at(0));
        hotkeys.push_back(toalower(text.at(0)));
    }
    int c = god_message_altar_colour(god);
    colour_text = colour_to_str(c);
    data = &text;

    const dungeon_feature_type feat = altar_for_god(god_);
    if (feat)
    {
        const tileidx_t idx = tileidx_feature_base(feat);
        add_tile(tile_def(pick_dngn_tile(idx, ui_random(INT_MAX))));
    }
}

string GodMenuEntry::get_text() const
{
    // Keep text/data and ASCII hotkeys canonical for selection and scripts.
    const string canonical = god_name(god);
    const string display = ui_translate("display.god", canonical.c_str());
    if (display == canonical)
        return MenuEntry::get_text();
    return _get_text_preface() + display + " (" + text + ")";
}

string GodMenuEntry::_get_text_preface() const
{
    if (level == MEL_ITEM && hotkeys.size())
    {
        char buf[300];
        // XX this probably breaks local tiles hotkey handling?
        snprintf(buf, sizeof buf, " <%s>%c</%s> - ",  colour_text.c_str(),
                 hotkeys[0], colour_text.c_str());
        return string(buf);
    }
    return "";
}
