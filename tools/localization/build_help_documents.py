"""Export reviewed help documents, or current English when their source changes."""
import argparse
import json
from pathlib import Path
from python_compat import pgettext
from build_textdb import ROOT, digest, load_po


def generate(manifest, po, root=ROOT):
    spec = json.loads(manifest.read_text(encoding='utf-8'))
    translations = load_po(po)
    outputs = []
    for entry in spec['documents']:
        relative = Path(entry['source'])
        if (relative.parent.as_posix() != 'crawl-ref/docs'
                or relative.suffix != '.txt'):
            raise ValueError('Invalid help document path')
        english = (root/relative).read_text(encoding='utf-8').rstrip()
        key = entry['context'] + '\x04' + english
        reason = 'reviewed'
        if digest(english) != entry['source_sha256']:
            reason = 'source_changed'
        elif entry['review_status'] != 'reviewed':
            reason = 'unreviewed'
        elif key not in translations._catalog:
            reason = 'missing_or_fuzzy'
        target = pgettext(translations, entry['context'], english) if reason == 'reviewed' else english
        outputs.append((root/relative.parent/'zh'/relative.name, target+'\n',
                        {'source':entry['source'], 'status':reason}))
    # All documents must validate before any output is replaced.
    for path, body, _ in outputs:
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_text(encoding='utf-8') != body:
            with path.open('w', encoding='utf-8', newline='\n') as stream:
                stream.write(body)
    return [result for _, _, result in outputs]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=ROOT/'localization/manifests/help-documents.json')
    parser.add_argument('--po', type=Path, default=ROOT/'localization/zh_CN/ui.po')
    args = parser.parse_args()
    for result in generate(args.manifest, args.po):
        print(result['source'] + ': ' + result['status'])
