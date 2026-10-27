#pragma once

#include "i18n.h"
#include <map>

// Capture typed alternatives during one callback invocation. Never rerun a
// callback to obtain its other language, or parse its rendered English.
class target_description_capture
{
public:
    target_description_capture();
    ~target_description_capture();
    target_description_capture(const target_description_capture&) = delete;
    target_description_capture &operator=(const target_description_capture&) = delete;
    ui_arg argument(const std::string &canonical) const;
    void record(const std::string &canonical, const ui_arg &display);
private:
    target_description_capture *previous;
    std::map<std::string, ui_arg> alternatives;
};

std::string target_description_text(const std::string &canonical, const ui_arg &display);
std::string target_description_text(const char *context, const char *canonical);
std::string target_description_format(const char *context, const char *source, ...);
std::string target_description_render(const char *context, const char *source,
    const std::string &canonical, std::initializer_list<ui_arg> arguments);

// Canonical prefix/body survive any missing translation; no entity lookup or RNG.
std::string target_prompt_display(const char *prefix, const std::string &canonical,
                                 const ui_arg &display, bool colour);
