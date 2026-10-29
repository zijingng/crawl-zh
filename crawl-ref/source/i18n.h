#pragma once

#include <initializer_list>
#include <string>
#include <vector>

class actor;
struct item_def;

// Presentation adapters: never pass rendered English grammar to a Chinese
// template. Failure (including English mode) leaves output untouched.
bool try_ui_actor_name(const actor &who, std::string &output);
bool try_ui_actor_log(const char *context, const char *english,
                      const actor &who, std::string &output);
// Canonical projectile/zap name only, never a rendered message or kill cause.
bool try_ui_projectile_name(const std::string &english, std::string &output);
bool try_ui_floor_item_list(const std::vector<const item_def*> &items,
                            std::string &output);
bool try_ui_item_log(const char *english, const item_def &item,
                     std::string &output, int quantity = -1);
bool try_ui_melee_miss(const char *source, const actor &attacker, const actor &defender,
                       const std::string &adverb, const item_def *weapon,
                       std::string &output);
bool try_ui_melee_hit(const char *source, const actor &attacker, const actor &defender,
                     const std::string &verb, const std::string &degree,
                     const item_def *weapon, bool charged, bool distant,
                     bool stealth, const std::string &debug,
                     const std::string &punctuation, std::string &output);

// Display text only. Internal names, TextDB keys and commands stay in English.
const char *ui_translate(const char *context, const char *english);

// Typed display inputs: names come from original entities, text keys are
// canonical nouns, and literals are only numbers, punctuation or proper names.
struct ui_arg
{
    std::string text;
    bool available;
};
// Only call for typed, procedurally generated proper names, never player input
// or a rendered English sentence. Does not generate names or consume RNG.
ui_arg ui_generated_name_arg(const std::string &canonical);
ui_arg ui_actor_arg(const actor &who);
ui_arg ui_corrode_source_arg(const actor *source, const std::string &canonical);
ui_arg ui_knockback_cause_arg(const std::string &canonical);
ui_arg ui_target_arg(const actor &who, const actor &subject);
ui_arg ui_item_arg(const item_def &item);
ui_arg ui_text_arg(const char *context, const std::string &canonical);
ui_arg ui_literal_arg(const std::string &text);
std::string ui_render(const char *context, const char *source,
                      const std::string &original,
                      std::initializer_list<ui_arg> args);

struct ui_message
{
    const char *context;
    const char *english;
};

// A dynamic expression is translated only when every required piece exists.
// Decide requirements before rendering; otherwise use the original expression.
class ui_translation_group
{
public:
    ui_translation_group(std::initializer_list<ui_message> required = {});
    void require(const char *context, const char *english);
    bool available() const { return m_available; }
    const char *get(const char *context, const char *english) const;
private:
    bool m_available;
};
