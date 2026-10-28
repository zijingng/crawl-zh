"""Regression checks for localization support on older Python versions."""
import gettext
from pathlib import Path
import unittest

from python_compat import is_relative_to, pgettext


class PythonCompatTest(unittest.TestCase):
    def test_context_lookup_and_english_fallback(self):
        translations = gettext.GNUTranslations()
        translations.plural = lambda n: int(n != 1)
        translations._catalog = {
            'menu\x04Open': '打开',
            'door\x04Open': '开启',
            'Open': 'Uncontextual translation',
        }
        self.assertEqual(pgettext(translations, 'menu', 'Open'), '打开')
        self.assertEqual(pgettext(translations, 'door', 'Open'), '开启')
        self.assertEqual(pgettext(translations, 'missing', 'Open'), 'Open')

    def test_containment_rejects_sibling_with_same_prefix(self):
        root = Path('/source').resolve()
        self.assertTrue(is_relative_to(root / 'file.cc', root))
        self.assertTrue(is_relative_to(root, root))
        self.assertFalse(is_relative_to(root.with_name('source-other') / 'file.cc', root))


if __name__ == '__main__':
    unittest.main()
