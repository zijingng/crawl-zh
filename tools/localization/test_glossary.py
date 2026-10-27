import re
import unittest

from audit_glossary import missing_terms


class GlossaryAuditTests(unittest.TestCase):
    def terms(self, english='Will', chinese='意志'):
        return [(english, chinese, re.compile(r'(?<![A-Za-z0-9_])' + re.escape(english) + r'(?![A-Za-z0-9_])', re.I))]

    def test_identifier_boundaries_and_case(self):
        self.assertEqual(list(missing_terms('willpower you.will_value()', '意志力', self.terms())), [])
        hits = list(missing_terms('Your WILL is weak.', '你很虚弱。', self.terms()))
        self.assertEqual(hits[0]['spans'], [[5, 9]])

    def test_chinese_hit_suppresses_candidate(self):
        self.assertEqual(list(missing_terms('Your will is weak.', '你的意志薄弱。', self.terms())), [])

    def test_overlapping_specific_names_are_retained(self):
        terms = self.terms('Pain', '痛苦术') + self.terms('Pain card', '痛苦牌')
        hits = list(missing_terms('Pain card', '痛苦牌', terms))
        self.assertEqual([hit['term'] for hit in hits], ['Pain'])

    def test_executable_examples_are_candidates_for_context_review(self):
        hits = list(missing_terms('<w>Will</w>', '<w>Will</w>', self.terms()))
        self.assertEqual(hits[0]['term'], 'Will')

    def test_slash_separated_senses_are_alternatives(self):
        terms = self.terms('slug', '鼻涕虫/弹丸')
        self.assertEqual(list(missing_terms('a slug hits you', '一枚弹丸击中你', terms)), [])
        hit = list(missing_terms('a slug hits you', '你被击中了', terms))[0]
        self.assertEqual(hit['accepted_translations'], ['鼻涕虫', '弹丸'])


if __name__ == '__main__':
    unittest.main()
