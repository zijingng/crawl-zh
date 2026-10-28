"""Exercise the unchanged WebTiles server with two ordinary dump messages."""
import ast
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from types import SimpleNamespace
import unittest


class LegacyDumpLinksTest(unittest.TestCase):
    def adapter(self, prefix="https://example.test/morgue/Player/"):
        source = Path(__file__).resolve().parents[2]/"crawl-ref/source/webserver/webtiles/process_handler.py"
        tree = ast.parse(source.read_text(encoding="utf-8"))
        method = next(n for n in ast.walk(tree)
                      if isinstance(n, ast.FunctionDef) and n.name == "_on_socket_message")
        namespace = {"json_decode": json.loads}
        exec(compile(ast.Module(body=[method], type_ignores=[]), str(source), "exec"), namespace)
        calls = []
        server = SimpleNamespace(process=None, username="Player", exit_dump_url="unchanged",
                                 game_params=SimpleNamespace(templated=lambda *a, **kw: prefix),
                                 send_to_all=lambda msg, **kw: calls.append((msg, kw)))
        def receive(filename, kind="command"):
            namespace[method.name](server, "*" + json.dumps(
                {"msg": "dump", "type": kind, "filename": filename}))
        return receive, server, calls

    def test_two_ordinary_notifications_produce_two_distinct_links(self):
        receive, server, calls = self.adapter()
        receive("Player")
        receive("Player.zh")
        self.assertEqual(calls, [
            ("dump", {"url": "https://example.test/morgue/Player/Player"}),
            ("dump", {"url": "https://example.test/morgue/Player/Player.zh"})])
        self.assertEqual(server.exit_dump_url, "unchanged")

    def test_english_only_has_no_inferred_chinese_link(self):
        receive, _, calls = self.adapter()
        receive("Player")
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1]["url"], "https://example.test/morgue/Player/Player")

    def test_save_and_death_keep_canonical_exit_url(self):
        for kind in ["save", "morgue"]:
            receive, server, calls = self.adapter()
            receive("Player", kind)
            self.assertEqual(calls, [])
            self.assertEqual(server.exit_dump_url, "https://example.test/morgue/Player/Player")

    def test_unconfigured_public_directory_sends_no_link(self):
        receive, server, calls = self.adapter(prefix=None)
        receive("Player")
        receive("Player.zh")
        self.assertEqual(calls, [])
        self.assertEqual(server.exit_dump_url, "unchanged")

    def test_real_sender_preserves_suffix_after_filename_sanitization(self):
        compiler = shutil.which("g++")
        if not compiler:
            self.skipTest("g++ required for isolated sender protocol check")
        root = Path(__file__).resolve().parents[2]/"crawl-ref/source"
        def function(filename, signature):
            source = (root/filename).read_text(encoding="utf-8")
            return re.search(r"^" + signature + r".*?^\}", source, re.M | re.S)[0]
        # Compile the actual production sender and sanitizer with only the socket
        # writes captured. No WebTiles SDK or server installation is needed.
        code = r'''
#include <string>
#include <iostream>
#include <cassert>
using std::string;
#define ASSERT assert
class TilesFramework {
    string output;
public:
    void write_message(const char *text) { output += text; }
    void write_message_escaped(const string &text) { output += text; }
    void finish_message() { std::cout << output << '\n'; output.clear(); }
    void send_dump_info(const string &, const string &, bool = false);
};
'''
        code += function("stringutil.cc", r"string replace_all_of\(") + "\n"
        code += function("stringutil.cc", r"string strip_filename_unsafe_chars\(") + "\n"
        code += function("tileweb.cc", r"void TilesFramework::send_dump_info\(") + "\n"
        code += r'''
int main() {
    TilesFramework tiles;
    tiles.send_dump_info("command", "A.B ;@");
    tiles.send_dump_info("command", "A.B ;@", true);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source, exe = Path(tmp)/"sender.cc", Path(tmp)/"sender.exe"
            source.write_text(code, encoding="utf-8")
            subprocess.run([compiler, "-std=c++11", str(source), "-o", str(exe)], check=True)
            lines = subprocess.check_output([str(exe)], text=True).splitlines()
        self.assertEqual([json.loads(line[1:]) for line in lines], [
            {"msg": "dump", "type": "command", "filename": "AB"},
            {"msg": "dump", "type": "command", "filename": "AB.zh"}])
