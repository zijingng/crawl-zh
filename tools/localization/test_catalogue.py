"""Catalogue integration checks. Run with the same Python/PATH as the build."""
import importlib.util
import hashlib
from source_hash import text_file_sha256
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('catalogue', Path(__file__).with_name('build_catalogue.py'))
catalogue = importlib.util.module_from_spec(spec)
spec.loader.exec_module(catalogue)


class CatalogueTest(unittest.TestCase):
    def test_lf_and_crlf_dependencies_export_the_same_catalogue(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            folder = root/'localization/manifests'
            folder.mkdir(parents=True)
            source = root/'generated.h'
            manifest, po, output = folder/'ui.json', root/'ui.po', root/'catalogue.h'
            source.write_bytes(b'reviewed\ncode\n')
            entry = {'context':'test', 'english':'Original', 'review_status':'reviewed',
                     'source_dependencies':[{'source':'generated.h',
                                             'sha256':text_file_sha256(source)}]}
            manifest.write_text(json.dumps({'messages':[entry]}), encoding='utf-8')
            po.write_text('msgid ""\nmsgstr ""\n'
                          '"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
                          'msgctxt "test"\nmsgid "Original"\nmsgstr "译文"\n', encoding='utf-8')
            catalogue.generate(manifest, po, output)
            expected = output.read_bytes()
            self.assertIn('译文', expected.decode('utf-8'))
            source.write_bytes(b'reviewed\r\ncode\r\n')
            catalogue.generate(manifest, po, output)
            self.assertEqual(output.read_bytes(), expected)
            source.write_bytes(b'changed\r\ncode\r\n')
            catalogue.generate(manifest, po, output)
            self.assertNotIn('译文', output.read_text(encoding='utf-8'))

    def test_textdb_prose_cannot_be_duplicated_into_ui(self):
        entry={'context':'mon.cast.speech','english':'A database message.',
               'review_status':'reviewed'}
        with self.assertRaisesRegex(ValueError,'TextDB prose belongs in textdb.po'):
            self.compile(entry,'msgctxt "mon.cast.speech"\nmsgid "A database message."\nmsgstr "数据库消息。"\n')
    def test_shared_dependencies_are_read_once_without_caching_review_decisions(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root/'message.cc'
            source.write_text('reviewed code\n', encoding='utf-8')
            digest = text_file_sha256(source)
            folder = root/'localization/manifests'
            folder.mkdir(parents=True)
            manifest = folder/'ui.json'
            po, header = root/'ui.po', root/'catalogue.h'
            entries = [
                {'context':'test', 'english':english, 'review_status':'reviewed',
                 'source_dependencies':[{'source':'message.cc', 'sha256':expected}]}
                for english, expected in [('First', digest), ('Second', '0' * 64)]
            ]
            manifest.write_text(json.dumps({'messages':entries}), encoding='utf-8')
            po.write_text('msgid ""\nmsgstr ""\n'
                          '"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
                          'msgctxt "test"\nmsgid "First"\nmsgstr "第一条"\n\n'
                          'msgctxt "test"\nmsgid "Second"\nmsgstr "第二条"\n', encoding='utf-8')
            original_read = Path.read_bytes
            reads = []

            def read(path):
                if path == source:
                    reads.append(path)
                return original_read(path)

            with patch.object(Path, 'read_bytes', read):
                catalogue.generate(manifest, po, header)
                self.assertEqual(len(reads), 1)
                self.assertIn('第一条', header.read_text(encoding='utf-8'))
                self.assertNotIn('第二条', header.read_text(encoding='utf-8'))
                source.write_text('changed code\n', encoding='utf-8')
                catalogue.generate(manifest, po, header)
                self.assertEqual(len(reads), 2)
                self.assertNotIn('第一条', header.read_text(encoding='utf-8'))
                entries[0]['source_dependencies'] = [
                    {'source':'missing.cc', 'sha256':None}]
                manifest.write_text(json.dumps({'messages':entries}), encoding='utf-8')
                catalogue.generate(manifest, po, header)
                self.assertNotIn('第一条', header.read_text(encoding='utf-8'))

    def test_changed_dynamic_expression_or_missing_code_forces_english_fallback(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root/'message.cc'
            source.write_text('original expression\n', encoding='utf-8')
            folder = root/'localization/manifests'
            folder.mkdir(parents=True)
            manifest = folder/'ui.json'
            po, header = root/'ui.po', root/'catalogue.h'
            entry = {'context':'log.test', 'english':'Original %s',
                     'review_status':'reviewed', 'source_dependencies':[
                         {'source':'message.cc',
                          'sha256':text_file_sha256(source)}]}
            manifest.write_text(json.dumps({'messages':[entry]}), encoding='utf-8')
            po.write_text('msgid ""\nmsgstr ""\n'
                          '"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
                          '#, c-format\nmsgctxt "log.test"\n'
                          'msgid "Original %s"\nmsgstr "译文 %s"\n', encoding='utf-8')
            def compile():
                catalogue.generate(manifest, po, header)
                return header.read_text(encoding='utf-8')
            self.assertIn('译文', compile())
            source.write_text('changed state or grammar\n', encoding='utf-8')
            self.assertNotIn('译文', compile())
            source.unlink()
            self.assertNotIn('译文', compile())

    def compile(self, entry, body):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            manifest, po, header = [folder / x for x in ('manifest.json', 'ui.po', 'catalogue.h')]
            manifest.write_text(json.dumps({'messages': [entry]}), encoding='utf-8')
            po.write_text('msgid ""\nmsgstr ""\n'
                          '"Content-Type: text/plain; charset=UTF-8\\n"\n'
                          '"Language: zh_CN\\n"\n\n' + body, encoding='utf-8')
            catalogue.generate(manifest, po, header)
            return header.read_text(encoding='utf-8')

    def test_fuzzy_and_missing_entries_fall_back(self):
        entry = {'context': 'test', 'english': 'Original', 'review_status': 'reviewed'}
        for body in ('', '#, fuzzy\nmsgctxt "test"\nmsgid "Original"\nmsgstr "译文"\n'):
            self.assertNotIn('译文', self.compile(entry, body))

    def test_changed_source_is_not_reused(self):
        entry = {'context': 'test', 'english': 'Changed', 'review_status': 'reviewed'}
        self.assertNotIn('译文', self.compile(entry, 'msgctxt "test"\nmsgid "Original"\nmsgstr "译文"\n'))

    def test_unreviewed_entry_is_excluded(self):
        entry = {'context': 'test', 'english': 'Original', 'review_status': 'draft'}
        self.assertNotIn('译文', self.compile(entry, 'msgctxt "test"\nmsgid "Original"\nmsgstr "译文"\n'))

    def test_printf_mismatch_is_rejected(self):
        entry = {'context': 'test', 'english': 'Level %d', 'review_status': 'reviewed'}
        with self.assertRaises(subprocess.CalledProcessError):
            self.compile(entry, '#, c-format\nmsgctxt "test"\nmsgid "Level %d"\nmsgstr "等级 %s"\n')

    def test_reviewed_entry_is_generated(self):
        entry = {'context': 'test', 'english': 'Original', 'review_status': 'reviewed'}
        self.assertIn('译文', self.compile(entry, 'msgctxt "test"\nmsgid "Original"\nmsgstr "译文"\n'))

    def test_manual_key_examples_cannot_become_cpp_trigraphs(self):
        english = "??= ??/ ??' ??( ??) ??! ??< ??> ??-"
        chinese = '按键示例：' + english
        entry = {'context':'help.document', 'english':english,
                 'review_status':'reviewed'}
        body = ('msgctxt "help.document"\nmsgid '
                + json.dumps(english) + '\nmsgstr '
                + json.dumps(chinese, ensure_ascii=False) + '\n')
        header = self.compile(entry, body)
        self.assertNotIn('??', header)
        for suffix in "=/'()!<>-":
            self.assertIn(r'\?\?' + suffix, header)

    def test_colour_tags_are_preserved(self):
        entry = {'context': 'test', 'english': 'Press <w>?</w>', 'review_status': 'reviewed'}
        with self.assertRaises(ValueError):
            self.compile(entry, 'msgctxt "test"\nmsgid "Press <w>?</w>"\nmsgstr "按 ?"\n')

    def test_printf_mismatch_without_po_flag_is_rejected(self):
        entry = {'context': 'test', 'english': 'Level %d', 'review_status': 'reviewed'}
        with self.assertRaises(ValueError):
            self.compile(entry, 'msgctxt "test"\nmsgid "Level %d"\nmsgstr "等级 %s"\n')

    def test_percent_hotkey_is_literal_and_must_survive(self):
        entry = {'context':'test', 'english':'% - List aptitudes',
                 'review_status':'reviewed', 'literal_percent':True}
        valid = 'msgctxt "test"\nmsgid "% - List aptitudes"\nmsgstr "% - 查看资质"\n'
        self.assertIn('查看资质', self.compile(entry, valid))
        with self.assertRaises(ValueError):
            self.compile(entry, valid.replace('msgstr "% -', 'msgstr "'))


if __name__ == '__main__':
    unittest.main()
