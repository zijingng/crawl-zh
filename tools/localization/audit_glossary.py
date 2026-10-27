#!/usr/bin/env python3
"""Report English glossary hits whose expected Chinese text is absent.

This is a candidate finder, not a translator or a pass/fail gate. Matching is
case-insensitive with ASCII identifier boundaries; inflections are not inferred.
Overlapping terms, code examples and quotation prose are deliberately retained.
"""
import argparse
import csv
import hashlib
import json
import re
from pathlib import Path

from build_textdb import load_po

ROOT = Path(__file__).resolve().parents[2]


def missing_terms(english, translation, terms):
    for term, chinese, pattern in terms:
        matches = list(pattern.finditer(english))
        alternatives = [text.strip() for text in chinese.split('/') if text.strip()]
        if matches and not any(text in translation for text in alternatives):
            yield {'term': term, 'expected': chinese,
                   'accepted_translations': alternatives,
                   'spans': [list(match.span()) for match in matches]}


def audit(glossary, catalogues):
    with glossary.open(encoding='utf-8-sig', newline='') as stream:
        rows = list(csv.DictReader(stream))
    terms = [(row['Term'], row['Translation'],
              re.compile(r'(?<![A-Za-z0-9_])' + re.escape(row['Term'])
                         + r'(?![A-Za-z0-9_])', re.I))
             for row in rows if row['Term'] and row['Translation']]
    entries = []
    scanned = 0
    for path in catalogues:
        for key, translation in load_po(path)._catalog.items():
            if not isinstance(key, str) or '\x04' not in key:
                continue
            context, english = key.split('\x04', 1)
            scanned += 1
            issues = list(missing_terms(english, translation, terms))
            if issues:
                entries.append({'catalogue': path.stem, 'context': context,
                                'english': english, 'translation': translation,
                                'source_sha256': hashlib.sha256(english.encode()).hexdigest(),
                                'translation_sha256': hashlib.sha256(translation.encode()).hexdigest(),
                                'issues': issues})
    return {'glossary_sha256': hashlib.sha256(glossary.read_bytes()).hexdigest(),
            'glossary_terms': len(terms), 'scanned_entries': scanned,
            'candidate_pairs': sum(len(entry['issues']) for entry in entries),
            'candidate_entries': len(entries),
            'matching': 'case-insensitive ASCII identifier boundaries; Term column; no inferred inflections',
            'entries': entries}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--glossary', type=Path, default=ROOT / 'localization/glossary.csv')
    parser.add_argument('--po', type=Path, action='append')
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    result = audit(args.glossary, args.po or [ROOT / f'localization/zh_CN/{name}.po' for name in ('ui', 'textdb')])
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f"Scanned {result['scanned_entries']} entries against {result['glossary_terms']} terms; "
          f"{result['candidate_pairs']} candidate pairs in {result['candidate_entries']} entries.")
