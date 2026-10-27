"""Standalone help export must never retain stale or missing Chinese."""
import copy,json,tempfile,unittest
from pathlib import Path
from build_textdb import digest
from build_help_documents import generate


class HelpDocumentsTest(unittest.TestCase):
    def test_export_and_fallback_after_source_or_review_changes(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);source=root/'crawl-ref/docs/guide.txt'
            source.parent.mkdir(parents=True)
            source.write_text('English\n',encoding='utf8')
            spec={'documents':[{'source':'crawl-ref/docs/guide.txt','context':'help.document',
                                'source_sha256':digest('English'),'review_status':'reviewed'}]}
            mp=root/'manifest.json';po=root/'ui.po'
            header='msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
            entry='msgctxt "help.document"\nmsgid "English"\nmsgstr "中文"\n'
            output=root/'crawl-ref/docs/zh/guide.txt'
            def run():
                mp.write_text(json.dumps(spec),encoding='utf8')
                return generate(mp,po,root)[0]['status']
            po.write_text(header+entry,encoding='utf8')
            self.assertEqual(run(),'reviewed')
            self.assertEqual(output.read_text(encoding='utf8'),'中文\n')
            for state in ('changed','unreviewed','missing','fuzzy'):
                with self.subTest(state=state):
                    original=copy.deepcopy(spec)
                    source.write_text('New English\n' if state=='changed' else 'English\n',encoding='utf8')
                    if state=='unreviewed':spec['documents'][0]['review_status']='draft'
                    po.write_text(header+('' if state=='missing' else '#, fuzzy\n'+entry if state=='fuzzy' else entry),encoding='utf8')
                    self.assertNotEqual(run(),'reviewed')
                    self.assertEqual(output.read_text(encoding='utf8'),source.read_text(encoding='utf8'))
                    spec=original

    def test_rejects_paths_outside_documents(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);mp=root/'manifest.json';po=root/'ui.po'
            po.write_text('msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n',encoding='utf8')
            mp.write_text(json.dumps({'documents':[{'source':'../../guide.txt'}]}),encoding='utf8')
            with self.assertRaises(ValueError):generate(mp,po,root)


if __name__=='__main__':unittest.main()
