#pragma once

#include "i18n.h"
#include "item-def.h"
#include "item-name.h"

// Presentation only: preserve the inventory letter and user inscription while
// retaining the same identification restrictions as the normal display name.
inline ui_arg ui_inventory_item_arg(const item_def &item,
                                    bool equipment_annotations = false)
{
    ui_arg result;
    result.available = try_item_display_name(item, result.text, true,
                                             equipment_annotations);
    return result;
}

// Reading or drinking consumes one member of a stack. This display snapshot
// neither changes the real quantity nor reveals an unidentified subtype.
inline ui_arg ui_single_item_arg(const item_def &item)
{
    item_def single = item;
    single.quantity = 1;
    return ui_item_arg(single);
}
