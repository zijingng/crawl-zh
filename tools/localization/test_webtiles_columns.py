"""Compile the real WebTiles writer and serializer without a running server."""
from pathlib import Path
import re
import os
import shutil
import subprocess
import tempfile
import unittest


class WebColumnsTest(unittest.TestCase):
    def test_wide_columns_and_incremental_updates(self):
        compiler = shutil.which('g++')
        if not compiler:
            self.skipTest('g++ required')
        root = Path(__file__).resolve().parents[2] / 'crawl-ref/source'
        writer = re.search(r'^void TilesFramework::put_ucs_string\(.*?^}',
                           (root/'tileweb.cc').read_text('utf8'), re.M | re.S)[0]
        serializer = (root/'tileweb-text.cc').read_text('utf8')
        serializer = re.sub(r'^#.*$', '', serializer, flags=re.M)
        header = (root/'tileweb-text.h').read_text('utf8')
        header = re.sub(r'^#.*$', '', header, flags=re.M)
        code = r'''
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <codecvt>
#include <locale>
#include <regex>
#include <iostream>
using namespace std;
#define ASSERT_RANGE(v,l,h) assert((v)>=(l) && (v)<(h))
int test_width(char32_t c) { return c == 0 ? 0 : c >= 0x4e00 ? 2 : 1; }
#define wcwidth test_width
int wctoutf8(char *buf, char32_t c) {
    string s=wstring_convert<codecvt_utf8<char32_t>,char32_t>().to_bytes(c);
    copy(s.begin(),s.end(),buf); return s.size();
}
string make_stringf(const char *fmt, ...) {
    char buf[2048]; va_list ap; va_start(ap,fmt);
    vsnprintf(buf,sizeof buf,fmt,ap); va_end(ap); return buf;
}
'''+header+r'''
class TilesFramework {
public:
    WebTextArea *m_print_area=nullptr;
    int m_print_x=0,m_print_y=0,m_print_fg=7,m_print_bg=0;
    string output;
    void put_ucs_string(char32_t *);
    void write_message(const char *fmt, ...) {
        char buf[8192]; va_list ap; va_start(ap,fmt);
        vsnprintf(buf,sizeof buf,fmt,ap); va_end(ap); output+=buf;
    }
    void json_write_comma() {}
    void finish_message() {}
} tiles;
'''+serializer+writer+r'''
int main() {
    WebTextArea area("menu"); area.resize(40,2); tiles.m_print_area=&area;
    for (auto label : {U"战斗", U"短兵器", U"投掷", U"Fire"}) {
        area.clear(); tiles.output.clear(); tiles.m_print_x=0; tiles.m_print_y=0;
        u32string text(label);
        tiles.put_ucs_string(&text[0]);
        assert(tiles.m_print_x == (text[0] < 128 ? 4 : int(text.size())*2));
        // A fixed numeric column must not move with the translated name.
        tiles.m_print_x=12; char32_t value[]=U"27"; tiles.put_ucs_string(value);
        area.send(true);
        auto &html=tiles.output;
        assert(html.find("\\u0000")==string::npos && html.find('\0')==string::npos);
        const string expected=wstring_convert<codecvt_utf8<char32_t>,char32_t>().to_bytes(text)
            +string(12-(text[0]<128 ? 4 : text.size()*2),' ')+"27";
        const string plain=regex_replace(html,regex("<[^>]*>"),"");
        assert(plain.find(expected)!=string::npos);
        cout << html << endl;
    }
    area.clear(); tiles.output.clear(); tiles.m_print_x=0; tiles.m_print_y=0;
    char32_t cr[]=U"A\rB"; tiles.put_ucs_string(cr); assert(tiles.m_print_x==2);
    area.send(); assert(tiles.output.find("AB")!=string::npos);
    tiles.output.clear(); area.send(); assert(tiles.output.empty());
    area.put_character(' ',7,0,0,0); area.put_character(' ',7,0,1,0);
    area.send(); assert(!tiles.output.empty());
    area.clear(); tiles.m_print_x=39; tiles.m_print_y=0;
    char32_t wide[]=U"战"; tiles.put_ucs_string(wide);
    assert(tiles.m_print_y==1 && tiles.m_print_x==2);
    tiles.output.clear(); area.send();
    area.put_character('X',7,0,1,1); tiles.output.clear(); area.send();
    assert(tiles.output.find("战")==string::npos);
    assert(tiles.output.find("X")!=string::npos);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source, exe = Path(tmp)/'columns.cc', Path(tmp)/'columns.exe'
            source.write_text(code, encoding='utf8')
            subprocess.run([compiler,'-std=c++11',str(source),'-o',str(exe)],check=True)
            output = subprocess.check_output([str(exe)],timeout=10)
            if os.environ.get('WEBTILES_COLUMNS_OUTPUT'):
                Path(os.environ['WEBTILES_COLUMNS_OUTPUT']).write_bytes(output)


if __name__ == '__main__':
    unittest.main()
