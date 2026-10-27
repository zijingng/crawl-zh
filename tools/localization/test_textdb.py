"""TextDB export regression checks; run with GNU msgfmt on PATH."""
import copy
import json
from pathlib import Path
import tempfile
import unittest

from build_textdb import generate, parse_text, digest, validate


class TextDBTest(unittest.TestCase):
    def test_lua_literals_require_an_exported_reviewed_parent(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);data=root/'data';(data/'database').mkdir(parents=True)
            folder=root/'localization/manifests';folder.mkdir(parents=True)
            manifest=folder/'test.json';po=root/'test.po'
            source='{{ return "Hello." }}\n'
            (data/'database/test.txt').write_text('%%%%\nprogram\n\n'+source,encoding='utf8')
            entry=dict(key='program',context='program',review_status='reviewed',source_sha256=digest(source))
            literal=dict(context='lua',english='Hello.',keys=['program'],review_status='reviewed')
            spec=dict(files=[dict(source='database/test.txt',entries=[entry],lua_results=[literal])])
            header='msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
            def run():
                manifest.write_text(json.dumps(spec),encoding='utf8')
                po.write_text(header+'msgctxt "program"\nmsgid '+json.dumps(source)+'\nmsgstr '+json.dumps(source)+'\n\nmsgctxt "lua"\nmsgid "Hello."\nmsgstr "你好。"\n',encoding='utf8')
                return generate(manifest,po,data)[0],(data/'database/zh/test.txt').read_text(encoding='utf8')
            self.assertIn('你好。',run()[1])
            entry['review_status']='draft'
            result,text=run();self.assertEqual(result['exported'],0);self.assertNotIn('你好。',text)
            entry['review_status']='reviewed';entry['source_sha256']='stale'
            self.assertNotIn('你好。',run()[1])
            entry['source_sha256']=digest(source);literal['review_status']='draft'
            self.assertNotIn('你好。',run()[1])
            literal['review_status']='reviewed'
            code=root/'database.cc';code.write_text('reviewed code',encoding='utf8')
            entry['source_dependencies']=[dict(source='database.cc',sha256=digest('reviewed code'))]
            self.assertIn('你好。',run()[1])
            code.write_text('changed code',encoding='utf8')
            result,text=run();self.assertEqual(result['entries'][0]['status'],'code_changed');self.assertNotIn('你好。',text)

    def test_unowned_database_translation_cannot_enter_runtime(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'database/zh').mkdir(parents=True)
            (root/'database/zh/unreviewed.txt').write_text('%%%%\nkey\n未审内容\n',encoding='utf8')
            manifest=root/'manifest.json';manifest.write_text('{"files":[]}',encoding='utf8')
            po=root/'test.po';po.write_text('msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n',encoding='utf8')
            with self.assertRaisesRegex(ValueError,'Unreviewed database translation file'):
                generate(manifest,po,root)

    def test_lua_display_adapter_requires_exact_reviewed_pair(self):
        source = '{{ return you.hand() .. "s" }}\n'
        target = '{{ local names = {hand="双手"}; return names[you.hand()] or "肢体" }}\n'
        with self.assertRaises(ValueError):
            validate(source, target)
        adapter = {'block': 0, 'source_sha256': digest(' return you.hand() .. "s" '),
                   'translation_sha256': digest(' local names = {hand="双手"}; return names[you.hand()] or "肢体" '),
                   'reason': 'Reviewed display-only mapping of canonical anatomy.'}
        validate(source, target, lua_display_adapters=[adapter])
        for original, translated, adapters in (
                (source.replace('you.hand()', 'you.race()'), target, [adapter]),
                (source, target.replace('双手', '错误'), [adapter]),
                (source, target, [{**adapter, 'block': 1}]),
                (source, target, [adapter, adapter]),
                (source, target, [{**adapter, 'reason': ''}])):
            with self.assertRaises(ValueError):
                validate(original, translated, lua_display_adapters=adapters)

    def test_random_choice_weights_and_boundaries_are_protected(self):
        source = 'w:3\nFirst\n\nw:1\nSecond\n'
        translated = 'w:3\n第一\n\nw:1\n第二\n'
        validate(source, translated)
        for invalid in (translated.replace('w:3', 'w:4'),
                        translated.replace('w:3', 'w:0').replace('w:1', 'w:3').replace('w:0', 'w:1'),
                        translated.replace('\n\n', '\n')):
            with self.assertRaises(ValueError):
                validate(source, invalid)

    def test_hints_keep_numbered_and_item_placeholders(self):
        original = '$1 sees $2: <w>$item[wand]</w>'
        validate(original, '$1 看见 $2：<w>$item[wand]</w>')
        for invalid in ('$1：<w>$item[wand]</w>',
                        '$1 $2：<w>$item[魔杖]</w>'):
            with self.assertRaises(ValueError):
                validate(original, invalid)

    def test_reviewed_backend_closing_tag_repair(self):
        original = '<tiles>Mouse<tiles> Keyboard'
        chinese = '<tiles>鼠标</tiles> 键盘'
        with self.assertRaises(ValueError):
            validate(original, chinese)
        repair = {'old':'<tiles>', 'new':'</tiles>', 'occurrence':2}
        validate(original, chinese, source_markup_repairs=[repair])
        for invalid in ({**repair, 'old':'Mouse', 'new':'鼠标'},
                        {**repair, 'occurrence':3}):
            with self.assertRaises(ValueError):
                validate(original, chinese, source_markup_repairs=[invalid])

    def test_same_export_dependency_uses_new_child_not_old_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'descript').mkdir()
            (root/'descript/test.txt').write_text('%%%%\nparent\n\nParent [[child]]\n%%%%\nchild\n\nChild\n', encoding='utf-8')
            dependency = {'source':'descript/test.txt', 'key':'child',
                          'source_sha256':digest('Child\n'),
                          'translation_sha256':digest('子\n')}
            spec = {'baseline':'test', 'files':[{'source':'descript/test.txt', 'entries':[
                {'key':'parent', 'context':'parent', 'review_status':'reviewed',
                 'source_sha256':digest('Parent [[child]]\n'), 'dependencies':[dependency]},
                {'key':'child', 'context':'child', 'review_status':'reviewed',
                 'source_sha256':digest('Child\n')}]}]}
            manifest = root/'manifest.json'
            manifest.write_text(json.dumps(spec), encoding='utf-8')
            po = root/'test.po'
            body = ('msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
                    'msgctxt "parent"\nmsgid "Parent [[child]]\\n"\nmsgstr "父 [[child]]\\n"\n\n'
                    'msgctxt "child"\nmsgid "Child\\n"\nmsgstr "子\\n"\n')
            po.write_text(body, encoding='utf-8')
            self.assertEqual(generate(manifest, po, root)[0]['exported'], 2)
            po.write_text(body.replace('子', '新译'), encoding='utf-8')
            result = generate(manifest, po, root)[0]
            self.assertEqual(result['exported'], 1)
            self.assertEqual(result['entries'][0]['status'], 'dependency_changed')
            self.assertNotIn('父', (root/'descript/zh/test.txt').read_text(encoding='utf-8'))

    def test_tutorial_commands_and_layout_directive_are_protected(self):
        source = ':nowrap\nUse <input>$cmd[CMD_GO_DOWNSTAIRS]</input>.\n'
        validate(source, ':nowrap\n按 <input>$cmd[CMD_GO_DOWNSTAIRS]</input> 下楼。\n')
        for target in (source.replace('CMD_GO_DOWNSTAIRS', 'CMD_GO_UPSTAIRS'),
                       source.replace(':nowrap\n', ''),
                       source + '$cmd[CMD_GO_DOWNSTAIRS]\n'):
            with self.assertRaises(ValueError):
                validate(source, target)

    def test_quoted_at_shortcut_is_not_a_substitution_marker(self):
        validate("Press '@' while armed. '@' works unarmed too.\n",
                 "装备武器时按 '@'；徒手时也能按 '@'。\n")
        with self.assertRaises(ValueError):
            validate('Value: @amount@\n', '数值：@other@\n')
        with self.assertRaises(ValueError):
            validate("Press '@'.\n", '按键。\n')

    def test_changed_reference_removes_reviewed_parent_translation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'descript/zh').mkdir(parents=True)
            source = 'Parent [[child]]\n'
            (root/'descript/test.txt').write_text('%%%%\nParent\n\n'+source, encoding='utf-8')
            child = root/'descript/child.txt'
            translated_child = root/'descript/zh/child.txt'
            child.write_text('%%%%\nchild\n\nEnglish\n', encoding='utf-8')
            translated_child.write_text('%%%%\nchild\n\n中文\n', encoding='utf-8')
            entry = {'key':'Parent', 'context':'test', 'review_status':'reviewed',
                     'source_sha256':digest(source), 'dependencies':[
                         {'source':'descript/child.txt', 'key':'child',
                          'source_sha256':digest('English\n'),
                          'translation_sha256':digest('中文\n')}]}
            manifest = root/'manifest.json'
            manifest.write_text(json.dumps({'files':[{'source':'descript/test.txt', 'entries':[entry]}]}), encoding='utf-8')
            po = root/'test.po'
            po.write_text('msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
                          'msgctxt "test"\nmsgid "Parent [[child]]\\n"\nmsgstr "父条目 [[child]]\\n"\n', encoding='utf-8')
            self.assertEqual(generate(manifest, po, root)[0]['exported'], 1)
            for changed in (child, translated_child):
                original = changed.read_text(encoding='utf-8')
                changed.write_text(original+'Changed\n', encoding='utf-8')
                result = generate(manifest, po, root)[0]
                self.assertEqual(result['entries'][0]['status'], 'dependency_changed')
                self.assertNotIn('父条目', (root/'descript/zh/test.txt').read_text(encoding='utf-8'))
                changed.write_text(original, encoding='utf-8')

    def test_parser_matches_last_wins_and_preserves_runtime_newlines(self):
        entries, duplicates = parse_text('ignored\n%%%%\nName\n\nFirst\n%%%%\nNAME\n# comment\n\nSecond  \n\n')
        self.assertEqual(entries['name'], ('NAME', 'Second\n\n'))
        self.assertEqual(duplicates, ['name'])

    def test_lua_identifiers_and_control_flow_are_protected(self):
        source = '{{if you.race() == "Djinni" then return "Hello" end}}\n'
        target = source.replace('Hello', '你好')
        validate(source, target, [1])
        for invalid in (target.replace('Djinni', '巨灵'),
                        target.replace('==', '~='), target.replace('end', '')):
            with self.assertRaises(ValueError):
                validate(source, invalid, [1])

    def test_references_tags_and_textdb_boundaries(self):
        for original, invalid in (('[[gem]]\n', '[[宝石]]\n'),
                                  ('<Breath status>\n', '<吐息状态>\n'),
                                  ('<w>Enter</w>\n', 'Enter\n'),
                                  ('text\n', '%%%%\ntext\n'),
                                  ('text\n', '#hidden\ntext\n')):
            with self.assertRaises(ValueError):
                validate(original, invalid)

    def test_export_filters_and_removes_stale_outputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'descript/zh').mkdir(parents=True)
            english = 'Hello\n'
            (root/'descript/test.txt').write_text('%%%%\nKey\n\n'+english, encoding='utf-8')
            spec = {'baseline': 'test', 'files': [{'source':'descript/test.txt', 'entries':[
                {'key':'Key','context':'test','source_sha256':digest(english), 'review_status':'reviewed'}]}]}
            manifest = root/'manifest.json'
            po = root/'test.po'
            header = 'msgid ""\nmsgstr ""\n"Content-Type: text/plain; charset=UTF-8\\n"\n\n'
            entry = 'msgctxt "test"\nmsgid "Hello\\n"\nmsgstr "你好\\n"\n'
            po.write_text(header+entry, encoding='utf-8')
            output = root/'descript/zh/test.txt'
            def run():
                manifest.write_text(json.dumps(spec), encoding='utf-8')
                return generate(manifest, po, root)
            self.assertEqual(run()[0]['exported'], 1)
            initial = output.read_bytes()
            self.assertEqual(run()[0]['exported'], 1)
            self.assertEqual(initial, output.read_bytes())
            for state in ('fuzzy', 'missing', 'unreviewed', 'changed'):
                current = copy.deepcopy(spec)
                po.write_text(header+('#, fuzzy\n'+entry if state=='fuzzy' else '' if state=='missing' else entry), encoding='utf-8')
                if state=='unreviewed': spec['files'][0]['entries'][0]['review_status']='draft'
                if state=='changed': spec['files'][0]['entries'][0]['source_sha256']='outdated'
                self.assertEqual(run()[0]['exported'], 0, state)
                self.assertNotIn('你好', output.read_text(encoding='utf-8'))
                spec = current


if __name__ == '__main__':
    unittest.main()
