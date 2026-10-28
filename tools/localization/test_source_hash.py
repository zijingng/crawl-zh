"""Platform line endings are equivalent; real source edits still invalidate review."""
from pathlib import Path
import hashlib
import os
import subprocess
import sys
import tempfile
import unittest

from source_hash import text_file_sha256
from build_textdb import code_dependencies_match


class SourceHashTest(unittest.TestCase):
    def test_platform_endings_share_one_review_but_content_changes_do_not(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root/'generated.h'
            expected = hashlib.sha256('第一行\nsecond line\n'.encode('utf-8')).hexdigest()
            dependencies = [{'source': 'generated.h', 'sha256': expected}]
            for text in ['第一行\nsecond line\n', '第一行\r\nsecond line\r\n']:
                source.write_bytes(text.encode('utf-8'))
                self.assertEqual(text_file_sha256(source), expected)
                self.assertTrue(code_dependencies_match(dependencies, root, {}))
            for text in ['第一行\nchanged line\n', '第一行\nsecond line \n',
                         '第一行\rsecond line\r']:
                source.write_bytes(text.encode('utf-8'))
                self.assertFalse(code_dependencies_match(dependencies, root, {}))
            source.unlink()
            self.assertFalse(code_dependencies_match(dependencies, root, {}))

class GeneratorPortabilityTest(unittest.TestCase):
    def test_species_output_is_lf_and_independent_of_hash_seed(self):
        source = Path(__file__).resolve().parents[2]/'crawl-ref/source'
        names = ['species-data.h', 'aptitudes.h', 'species-groups.h', 'species-type.h']
        with tempfile.TemporaryDirectory() as tmp:
            outputs = [str(Path(tmp)/name) for name in names]
            command = [sys.executable, 'util/species-gen.py', 'dat/species',
                       'util/species-gen', *outputs]
            snapshots = []
            for seed in ['1', '97']:
                subprocess.run(command, cwd=source, check=True,
                               env=dict(os.environ, PYTHONHASHSEED=seed))
                snapshots.append([Path(p).read_bytes() for p in outputs])
            self.assertEqual(*snapshots)
            for data in snapshots[0]:
                self.assertNotIn(b'\r\n', data)
