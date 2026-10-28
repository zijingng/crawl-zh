"""Export reviewed TextDB PO entries; stale/fuzzy/missing entries fall back to English.

Only manifest-owned files are replaced. English keys stay canonical. Lua control
code is protected, except for separately reviewed display adapters pinned to the
exact source and translated blocks. Run from any directory; no network required.
"""
import argparse
from collections import Counter
import gettext
import hashlib
import json
from pathlib import Path
from python_compat import pgettext, is_relative_to
import re
import shutil
import subprocess
import tempfile
from source_hash import text_file_sha256

ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / 'crawl-ref/source/dat'


def parse_text(text):
    entries = {}
    duplicates = []
    key = None
    body = []
    active = False
    def finish():
        if key:
            if key.lower() in entries:
                duplicates.append(key.lower())
            entries[key.lower()] = (key, '\n'.join(body).lstrip('\n') + '\n')
    for line in text.splitlines():
        if line.startswith('#'):
            continue
        if line.startswith('%%%%'):
            finish()
            active, key, body = True, None, []
        elif active and key is None:
            if line.strip():
                key = line.strip()
        elif active:
            body.append(line.rstrip())
    finish()
    return entries, duplicates


def digest(text):
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def validate(source, target, lua_text_indices=(), source_markup_repairs=(),
             lua_display_adapters=()):
    if any(line.startswith(('%%%%', '#')) for line in target.splitlines()):
        raise ValueError('Translation contains TextDB delimiter/comment')
    weights = re.findall(r'(?m)^w:\d+\s*$', source)
    if weights:
        if ([s.strip() for s in weights]
                != [s.strip() for s in re.findall(r'(?m)^w:\d+\s*$', target)]
                or len(re.split(r'\n\s*\n', source.strip()))
                != len(re.split(r'\n\s*\n', target.strip()))):
            raise ValueError('Changed random-text weights or choice boundaries')
    # A reviewed source typo may be repaired in the translation without editing
    # upstream English. Only a misplaced closing backend tag is allowed here.
    markup_source = source
    for repair in source_markup_repairs:
        old, new = repair['old'], repair['new']
        if (old not in ('<tiles>', '<console>', '<nomouse>', '<localtiles>')
                or new != '</' + old[1:] or repair['occurrence'] < 1):
            raise ValueError('Invalid reviewed markup repair')
        matches = list(re.finditer(re.escape(old), markup_source))
        if len(matches) < repair['occurrence']:
            raise ValueError('Reviewed markup repair no longer matches source')
        match = matches[repair['occurrence'] - 1]
        markup_source = markup_source[:match.start()] + new + markup_source[match.end():]
    # Includes, format tags, and doubled angle escapes must survive unchanged.
    for pattern in (r'\[\[[^\]]+\]\]', r'(?<!<)</?[A-Za-z_][^<>\n]*>',
                    r'<<', r'@', r'''@[^@\n'"]+@''', r'\$cmd\[[A-Z_]+\]',
                    r'\$item\[[^\]]+\]|\$[0-9]+', r'(?m)^:nowrap[ \t]*$'):
        if Counter(re.findall(pattern, markup_source)) != Counter(re.findall(pattern, target)):
            raise ValueError('Changed TextDB reference/markup')
    blocks = re.findall(r'\{\{(.*?)\}\}', source, re.S)
    translated = re.findall(r'\{\{(.*?)\}\}', target, re.S)
    if len(blocks) != len(translated) or target.count('{{') != target.count('}}'):
        raise ValueError('Changed Lua block boundaries')
    index = 0
    literal = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
    adapters = {a['block']: a for a in lua_display_adapters}
    if len(adapters) != len(lua_display_adapters) or any(
            not isinstance(i, int) or i < 0 or i >= len(blocks) for i in adapters):
        raise ValueError('Invalid reviewed Lua display adapter index')
    for block_index, (original, chinese) in enumerate(zip(blocks, translated)):
        left, right = list(literal.finditer(original)), list(literal.finditer(chinese))
        if block_index in adapters:
            adapter = adapters[block_index]
            if (not adapter.get('reason')
                    or digest(original) != adapter['source_sha256']
                    or digest(chinese) != adapter['translation_sha256']):
                raise ValueError('Reviewed Lua display adapter no longer matches')
            # A separately reviewed display program may map canonical raw values
            # to Chinese. Both complete blocks are pinned, never auto-approved.
            index += len(left)
            continue
        if len(left) != len(right):
            raise ValueError('Changed Lua string count')
        for a, b in zip(left, right):
            if index not in lua_text_indices and a.group() != b.group():
                raise ValueError('Changed protected Lua literal (e.g. species key)')
            index += 1
        if literal.sub('"TEXT"', original) != literal.sub('"TEXT"', chinese):
            raise ValueError('Changed Lua control code')
    if any(i >= index or i < 0 for i in lua_text_indices):
        raise ValueError('Invalid Lua literal index')


def load_po(path):
    msgfmt = shutil.which('msgfmt')
    if not msgfmt:
        raise SystemExit('msgfmt not found: install GNU gettext')
    with tempfile.TemporaryDirectory() as tmp:
        mo = Path(tmp) / 'textdb.mo'
        subprocess.run([msgfmt, '--check', '--check-format', '-o', str(mo), str(path)], check=True)
        with mo.open('rb') as stream:
            return gettext.GNUTranslations(stream)


def generate(manifest, po, data=DATA, report=None):
    spec = json.loads(manifest.read_text(encoding='utf-8'))
    translations = load_po(po)
    results = []
    # Validate all files before writing any output.
    outputs = []
    candidates = {}
    dependencies = {}
    dependency_hashes = {}
    source_root = manifest.resolve().parents[2]
    owned = {Path(file['source']).name for file in spec['files']
             if file['source'].startswith('database/')}
    for path in (data/'database/zh').glob('*.txt'):
        if path.name not in owned:
            raise ValueError('Unreviewed database translation file: '+path.name)
    for file in spec['files']:
        relative = Path(file['source'])
        if relative.is_absolute() or '..' in relative.parts or relative.parts[0] not in ('descript', 'database'):
            raise ValueError('Invalid TextDB source path')
        entries, duplicates = parse_text((data / relative).read_text(encoding='utf-8'))
        output = '# Generated from localization/zh_CN/textdb.po; do not edit.\n'
        passed = set()
        details = []
        for entry in file['entries']:
            key = entry['key'].lower()
            if key in passed:
                raise ValueError('Duplicate manifest key: ' + key)
            passed.add(key)
            current = entries.get(key)
            reason = 'reviewed'
            if not current or digest(current[1]) != entry['source_sha256']:
                reason = 'source_changed'
            elif entry['review_status'] != 'reviewed':
                reason = 'unreviewed'
            elif not code_dependencies_match(entry.get('source_dependencies', []),
                                             source_root, dependency_hashes):
                reason = 'code_changed'
            else:
                target = pgettext(translations, entry['context'], current[1])
                if not target or entry['context'] + '\x04' + current[1] not in translations._catalog:
                    reason = 'missing_or_fuzzy'
                else:
                    validate(current[1], target, entry.get('lua_text_indices', []),
                             entry.get('source_markup_repairs', []),
                             entry.get('lua_display_adapters', []))
                    output += '%%%%\n' + current[0] + '\n\n' + target.rstrip('\n') + '\n'
                    candidates[(relative.as_posix(), key)] = (current[0], target)
                    dependencies[(relative.as_posix(), key)] = entry.get('dependencies', [])
            details.append({'key': key, 'status': reason})
        exported = {e['key'] for e in details if e['status'] == 'reviewed'}
        result = {'source': relative.as_posix(), 'source_entries': len(entries),
                  'exported': len(exported), 'english_fallback': sorted(set(entries) - exported),
                  'source_duplicate_keys': duplicates, 'entries': details}
        results.append(result)
        outputs.append((data / relative.parent / 'zh' / relative.name, output))
    # Resolve references against this export, never a previous generated file.
    # Iterate so a rejected child also invalidates its parents in the same run.
    while True:
        pending = {path: text for path, text in outputs}
        rejected = {key for key in candidates
                    if not dependencies_match(dependencies[key], data, pending)}
        if not rejected:
            break
        for key in rejected:
            del candidates[key]
        outputs = []
        for file, result in zip(spec['files'], results):
            relative = Path(file['source'])
            output = '# Generated from localization/zh_CN/textdb.po; do not edit.\n'
            for detail in result['entries']:
                key = (relative.as_posix(), detail['key'])
                if key in rejected:
                    detail['status'] = 'dependency_changed'
                if key in candidates:
                    raw, target = candidates[key]
                    output += '%%%%\n' + raw + '\n\n' + target.rstrip('\n') + '\n'
            exported = {e['key'] for e in result['entries'] if e['status'] == 'reviewed'}
            result['exported'] = len(exported)
            source = parse_text((data/relative).read_text(encoding='utf-8'))[0]
            result['english_fallback'] = sorted(set(source) - exported)
            outputs.append((data/relative.parent/'zh'/relative.name, output))
    # Literal Lua results live in the same PO. They never execute a Chinese
    # program: callers may use them only after one reviewed canonical execution.
    for file in spec['files']:
        relative = Path(file['source'])
        for literal in file.get('lua_results', []):
            parents = [(relative.as_posix(), key.lower()) for key in literal['keys']]
            if not any(key in candidates for key in parents):
                continue
            source = literal['english']
            if '\n' in source or '\r' in source:
                raise ValueError('Lua result lookup key must be a single line')
            target = pgettext(translations, literal['context'], source)
            if (literal.get('review_status') != 'reviewed'
                    or literal['context']+'\x04'+source not in translations._catalog):
                continue
            validate(source, target)
            kind = literal.get('kind', 'display')
            if kind not in ('display', 'fragment'):
                raise ValueError('Invalid Lua result kind')
            key = '__lua_' + kind + '__' + source.lower()
            identity = (relative.as_posix(), key)
            if identity in candidates:
                raise ValueError('Duplicate Lua result key')
            candidates[identity] = (key, target)
            path = data/relative.parent/'zh'/relative.name
            outputs = [(p, text + ('%%%%\n'+key+'\n\n'+target.rstrip('\n')+'\n'
                                   if p == path else '')) for p, text in outputs]
    # Derive Lua results from already approved child prose: never maintain a
    # second PO copy of a child used by a conditional reference.
    for file in spec['files']:
        relative = Path(file['source'])
        source_entries = parse_text((data/relative).read_text(encoding='utf-8'))[0]
        for alias in file.get('lua_aliases', []):
            child = (relative.as_posix(), alias['key'].lower())
            parents = [(relative.as_posix(), key.lower()) for key in alias['keys']]
            if (alias.get('review_status') != 'reviewed' or child not in candidates
                    or not any(key in candidates for key in parents)):
                continue
            kind = alias['kind']
            if kind not in ('display', 'fragment'):
                raise ValueError('Invalid derived Lua result kind')
            source = source_entries[child[1]][1]
            target = candidates[child][1]
            left = re.split(r'\n\s*\n', source.strip())
            right = re.split(r'\n\s*\n', target.strip())
            if len(left) != len(right):
                raise ValueError('Changed derived Lua result choices')
            for original, display in zip(left, right):
                original = re.sub(r'^w:\d+\n', '', original).strip()
                display = re.sub(r'^w:\d+\n', '', display).strip()
                if alias.get('unescape_single_quote'):
                    original = original.replace("\\'", "'")
                    display = display.replace("\\'", "'")
                suffix = alias.get('suffix', '')
                original += suffix
                display += suffix.lstrip()
                if any(c in original for c in ('\n','\r','[',']','{','}')):
                    raise ValueError('Derived Lua result must be a literal single line')
                validate(original, display)
                key = '__lua_' + kind + '__' + original.lower()
                identity = (relative.as_posix(), key)
                if identity in candidates:
                    raise ValueError('Duplicate derived Lua result key')
                candidates[identity] = (key, display)
                path = data/relative.parent/'zh'/relative.name
                outputs = [(p, text + ('%%%%\n'+key+'\n\n'+display+'\n'
                                      if p == path else '')) for p, text in outputs]
    for path, text in outputs:
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_text(encoding='utf-8') != text:
            with path.open('w', encoding='utf-8', newline='\n') as stream:
                stream.write(text)
    if report:
        report.write_text(json.dumps({'baseline': spec['baseline'], 'files': results},
                                    ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return results


def code_dependencies_match(dependencies, source_root, hashes):
    for dependency in dependencies:
        source = dependency['source']
        if source not in hashes:
            path = (source_root/source).resolve()
            hashes[source] = (text_file_sha256(path)
                              if is_relative_to(path, source_root) and path.is_file() else None)
        if hashes[source] != dependency['sha256']:
            return False
    return True


def dependencies_match(dependencies, data, pending=None):
    for dependency in dependencies:
        relative = Path(dependency['source'])
        if (relative.is_absolute() or '..' in relative.parts
                or relative.parts[0] not in ('descript', 'database')):
            raise ValueError('Invalid TextDB dependency path')
        english = data / relative
        chinese = data / relative.parent / 'zh' / relative.name
        for path, fingerprint in ((english, dependency['source_sha256']),
                                  (chinese, dependency['translation_sha256'])):
            if pending is not None and path in pending:
                text = pending[path]
            elif path.exists():
                text = path.read_text(encoding='utf-8')
            else:
                return False
            body = parse_text(text)[0].get(dependency['key'].lower())
            if not body or digest(body[1]) != fingerprint:
                return False
    return True


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=ROOT / 'localization/manifests/textdb-phase3.json')
    parser.add_argument('--po', type=Path, default=ROOT / 'localization/zh_CN/textdb.po')
    parser.add_argument('--report', type=Path, default=ROOT / 'localization/manifests/phase3-coverage.json')
    args = parser.parse_args()
    generate(args.manifest, args.po, report=args.report)
