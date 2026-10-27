"""Extract literal UI calls and retain manifest-backed dynamic display names."""
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def extract():
    for command in ('xgettext', 'msgcat'):
        if not shutil.which(command):
            raise SystemExit(command + ' not found: install GNU gettext')
    spec = json.loads((ROOT/'localization/manifests/ui-sample.json').read_text(encoding='utf-8'))
    files = set()
    template = ('msgid ""\nmsgstr ""\n'
                '"Project-Id-Version: DCSS UI\\n"\n'
                '"Content-Type: text/plain; charset=UTF-8\\n"\n\n')
    for entry in spec['messages']:
        origin = entry.get('module')
        if origin:
            files.add('crawl-ref/source/' + origin)
        source = entry['english']
        flags = '#, no-c-format\n' if entry.get('literal_percent') else '#, c-format\n' if re.search(r'%(?:[-+ .0-9]*[sdc])', source) else ''
        template += (flags + 'msgctxt ' + json.dumps(entry['context'], ensure_ascii=False)
                     + '\nmsgid ' + json.dumps(source, ensure_ascii=False) + '\nmsgstr ""\n\n')
    files.update('crawl-ref/source/' + name for name in ('main.cc','player.cc','melee-attack.cc'))
    with tempfile.TemporaryDirectory() as tmp:
        seed, scanned = Path(tmp)/'seed.pot', Path(tmp)/'scanned.pot'
        seed.write_text(template, encoding='utf-8')
        subprocess.run(['xgettext', '--language=C++', '--from-code=UTF-8', '--omit-header',
                        '--no-wrap', '--keyword=ui_translate:1c,2', '-o', str(scanned),
                        *sorted(files)], cwd=ROOT, check=True)
        subprocess.run(['msgcat', '--use-first', '--no-wrap', str(seed), str(scanned),
                        '-o', str(ROOT/'localization/templates/ui.pot')], check=True)


if __name__ == '__main__':
    extract()
