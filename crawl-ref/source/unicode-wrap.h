#pragma once

#include "unicode.h"
#include "format.h"

// Insert into the text operation itself, preserving foreground/background
// operations on either side of the boundary.
inline bool insert_wrap_break(formatted_string &text, size_t byte_offset)
{
    for (auto &op : text.ops)
    {
        if (op.type != FSOP_TEXT)
            continue;
        if (byte_offset <= op.text.size())
        {
            op.text.insert(byte_offset, "\n");
            return true;
        }
        byte_offset -= op.text.size();
    }
    return false;
}

// Byte offset of the last usable space or CJK boundary before a width limit.
// This deliberately leaves unspaced Latin words to the original ellipsis path.
inline int cjk_wrap_break(const char *text, int byte_limit)
{
    int result = -1;
    char32_t previous = 0;
    for (int offset = 0; text[offset] && offset <= byte_limit; )
    {
        char32_t current;
        const int length = utf8towc(&current, text + offset);
        const auto wide = [](char32_t c) {
            return c >= 0x2e80 && wcwidth(c) == 2;
        };
        const bool closing = current == U'，' || current == U'。'
            || current == U'！' || current == U'？' || current == U'：'
            || current == U'；' || current == U'、' || current == U'）'
            || current == U'》' || current == U'」' || current == U'”'
            || current == U'’' || current == U'】';
        const bool opening = previous == U'（' || previous == U'《'
            || previous == U'「' || previous == U'“' || previous == U'‘'
            || previous == U'【';
        if (current == U' ' || offset > 0 && !closing && !opening
            && (wide(previous) || wide(current)))
        {
            result = offset;
        }
        previous = current;
        offset += length;
    }
    return result;
}
