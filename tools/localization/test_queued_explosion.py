"""Compile the production explosion scheduler and catalogue without game effects."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


class QueuedExplosionTest(unittest.TestCase):
    def test_production_scheduler_translates_fixed_messages_and_preserves_unknowns(self):
        compiler = shutil.which("g++")
        if not compiler:
            self.skipTest("g++ required")
        root = Path(__file__).resolve().parents[2] / "crawl-ref/source"

        def function(filename, signature):
            source = (root / filename).read_text(encoding="utf-8")
            return re.search(r"^" + signature + r".*?^\}", source, re.M | re.S)[0]

        code = r'''
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "i18n-catalogue.h"
using std::string;
enum class lang_t { EN, ZH };
struct { lang_t language; } Options;
struct actor {};
struct bolt { string name = "canonical blast", aux_source = "canonical cause"; };
enum explosion_fineff_type { EXPLOSION_FINEFF_GENERIC, EXPLOSION_FINEFF_INNER_FLAME,
    EXPLOSION_FINEFF_CONCUSSION, EXPLOSION_FINEFF_PYROMANIA };
struct explosion_fineff {
    bolt beam;
    string boom, sanct, poof;
    explosion_fineff_type typ;
    const actor *agent;
    explosion_fineff(const bolt &b, string m, string s, explosion_fineff_type t,
                     const actor *a, string p)
        : beam(b), boom(m), sanct(s), poof(p), typ(t), agent(a) {}
};
std::unique_ptr<explosion_fineff> queued;
void _schedule_final_effect(explosion_fineff *effect) { queued.reset(effect); }
'''
        code += function("i18n.cc", r"static const char \*find_ui_translation\(") + "\n"
        code += function("i18n.cc", r"const char \*ui_translate\(") + "\n"
        code += function("fineff.cc", r"void schedule_explosion_fineff\(") + "\n"
        code += r'''
int main() {
    bolt beam;
    actor agent;
    const string sanct = "By Zin's power, the fiery explosion is contained.";
    int checked = 0;
    for (const auto &entry : zh_ui_catalogue) {
        if (strcmp(entry.context, "b.explosion") || strchr(entry.english, '%')) continue;
        for (auto language : {lang_t::EN, lang_t::ZH}) {
            Options.language = language;
            for (int i = 0; i < 4; ++i) {
                auto type = static_cast<explosion_fineff_type>(i);
                schedule_explosion_fineff(beam, entry.english, sanct, type, &agent, "chosen residue");
                assert(queued->boom == (language == lang_t::ZH ? entry.chinese : entry.english));
                assert(queued->sanct == (language == lang_t::ZH
                    ? "辛的力量遏制了火焰爆炸。" : sanct));
                assert(queued->beam.name == beam.name && queued->beam.aux_source == beam.aux_source);
                assert(queued->typ == type && queued->agent == &agent);
                assert(queued->poof == "chosen residue");
                ++checked;
            }
        }
    }
    Options.language = lang_t::ZH;
    schedule_explosion_fineff(beam, "The cloud of blastmotes explodes!", sanct,
                              EXPLOSION_FINEFF_CONCUSSION, nullptr, "");
    assert(queued->boom == "爆裂微灵云爆炸了！");
    Options.language = lang_t::EN;
    assert(queued->boom == "爆裂微灵云爆炸了！"); // Completed event-time text.
    for (auto language : {lang_t::EN, lang_t::ZH}) {
        Options.language = language;
        for (const string text : {"Unknown custom monster explodes!", "中文怪物爆炸了！"}) {
            schedule_explosion_fineff(beam, text, "Unknown protection.",
                                      EXPLOSION_FINEFF_GENERIC, nullptr, "");
            assert(queued->boom == text && queued->sanct == "Unknown protection.");
        }
    }
    assert(checked > 0);
    std::cout << "queued explosion messages passed\n";
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source, exe = Path(tmp) / "scheduler.cc", Path(tmp) / "scheduler.exe"
            source.write_text(code, encoding="utf-8")
            subprocess.run([compiler, "-std=c++17", "-I", str(root), str(source), "-o", str(exe)], check=True)
            result = subprocess.check_output([str(exe)], text=True)
        self.assertEqual(result.strip(), "queued explosion messages passed")
