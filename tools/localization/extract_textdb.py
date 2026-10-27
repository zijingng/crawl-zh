"""Refresh the TextDB POT from current English without approving translations."""
import json
from build_textdb import ROOT, DATA, digest, parse_text


def extract():
    spec = json.loads((ROOT/'localization/manifests/textdb-phase3.json').read_text(encoding='utf-8'))
    header = ('msgid ""\nmsgstr ""\n'
              '"Project-Id-Version: DCSS TextDB\\n"\n'
              '"Content-Type: text/plain; charset=UTF-8\\n"\n\n')
    output = header
    changes = []
    for file in spec['files']:
        entries, duplicates = parse_text((DATA/file['source']).read_text(encoding='utf-8'))
        approved = {e['key'].lower(): e for e in file['entries']}
        for key, (raw, body) in entries.items():
            context = 'textdb:' + file['source'] + ':' + key
            output += ('#: crawl-ref/source/dat/' + file['source'] + '\n'
                       'msgctxt ' + json.dumps(context, ensure_ascii=False) + '\n'
                       'msgid ' + json.dumps(body, ensure_ascii=False) + '\nmsgstr ""\n\n')
            old = approved.get(key)
            state = 'not_reviewed' if not old else 'source_changed' if old['source_sha256'] != digest(body) else 'source_matches'
            changes.append({'source':file['source'], 'key':raw, 'state':state,
                            'current_sha256':digest(body)})
        for key in set(approved) - set(entries):
            changes.append({'source':file['source'], 'key':key, 'state':'removed'})
    (ROOT/'localization/templates/textdb.pot').write_text(output, encoding='utf-8')
    (ROOT/'localization/manifests/textdb-source-changes.json').write_text(
        json.dumps({'baseline':spec['baseline'], 'entries':changes}, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')


if __name__ == '__main__':
    extract()
