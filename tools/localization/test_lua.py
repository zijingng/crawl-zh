"""Execute the reviewed embedded Lua branches with --lua PATH_TO_RUNNER."""
import argparse
import json
import re
import subprocess
import tempfile
from pathlib import Path
from build_textdb import DATA, parse_text


def test_lua(runner):
    program = ['local checked = 0']
    cases = [
        ('skills.txt', 'spellcasting', [('Djinni', '法术强度'), ('Human', '法术槽')]),
        ('skills.txt', 'invocations', [('Djinni', None), ('Human', '较高的一项')]),
        ('items.txt', 'javelin', [('Kobold', '无法使用'), ('Spriggan', '无法使用'), ('Human', None)]),
        ('status.txt', 'app status', [('Octopode', '额外触手攻击'), ('Human', '额外攻击')]),
        ('status.txt', 'contam status', [('0', None), ('97', '97')]),
    ]
    for file, key, branches in cases:
        body = parse_text((DATA/'descript/zh'/file).read_text(encoding='utf-8'))[0][key][1]
        blocks = re.findall(r'\{\{(.*?)\}\}', body, re.S)
        if len(blocks) != 1:
            raise ValueError('Expected one reviewed Lua block: ' + key)
        for race, expected in branches:
            setup = ('you = {contam_max_damage = function() return ' + race + ' end}'
                     if key == 'contam status' else
                     'you = {race = function() return "' + race + '" end}')
            program += [setup,
                        'local result = (function()' + blocks[0] + '\nend)()']
            program += ['assert(result and string.find(result, "' + expected + '", 1, true))'
                        if expected else 'assert(result == nil)']
            program += ['checked = checked + 1']
    # New help/branch blocks must work on both sides of every condition.
    extra = [
        ('database/zh/help.txt', 'level-map',
         [('wizard', 'false', None), ('wizard', 'true', '巫师命令')]),
        ('descript/zh/branches.txt', 'ice cave',
         [('branch', '"'+branch+'"', '冰川裂隙')
          for branch in ('Elf', 'Swamp', 'Snake', 'Shoals', 'Spider')]
         + [('branch', '"D"', '冰窟')]),
    ]
    for file, key, branches in extra:
        body = parse_text((DATA/file).read_text(encoding='utf-8'))[0][key][1]
        blocks = re.findall(r'\{\{(.*?)\}\}', body, re.S)
        if len(blocks) != 1:
            raise ValueError('Expected one reviewed Lua block: ' + key)
        for function, value, expected in branches:
            program += ['you = {' + function + ' = function() return ' + value + ' end}',
                        'local result = (function()' + blocks[0] + '\nend)()']
            program += ['assert(result and string.find(result, "' + expected + '", 1, true))'
                        if expected else 'assert(result == nil)']
            program += ['checked = checked + 1']
    # Exercise every embedded block in the nine-file review. Compare the
    # English and Chinese control-flow outcomes with identical API stubs.
    files = ['descript/'+name+'.txt' for name in
             ('features', 'hints', 'monsters', 'quotes', 'egos', 'monstatus',
              'passives', 'unrand')] + ['database/godname.txt']
    profiles = [
        ('Human', 'No God', 'D', 'berserk', False),
        ('Djinni', 'Trog', 'Crypt', 'magic', True),
        ('Merfolk', 'Jiyva', 'Tomb', 'ranger', False),
        ('Octopode', 'Ashenzari', 'Zot', 'magic', True),
    ]
    for file in files:
        path = Path(file)
        english = parse_text((DATA/path).read_text(encoding='utf-8'))[0]
        chinese = parse_text((DATA/path.parent/'zh'/path.name).read_text(encoding='utf-8'))[0]
        for key, (_, body) in english.items():
            original = re.findall(r'\{\{(.*?)\}\}', body, re.S)
            translated = re.findall(r'\{\{(.*?)\}\}', chinese[key][1], re.S)
            assert len(original) == len(translated), (file, key)
            for source, target in zip(original, translated):
                for race, god, branch, hint, flag in profiles:
                    boolean = 'true' if flag else 'false'
                    program += [
                        'do',
                        'you = {race=function() return '+json.dumps(race)+' end, '
                        'god=function() return '+json.dumps(god)+' end, '
                        'branch=function() return '+json.dumps(branch)+' end, '
                        'class=function() return "Conjurer" end, '
                        'genus=function() return "human" end, '
                        'can_smell=function() return '+boolean+' end, '
                        'is_useless_skill=function() return '+boolean+' end, '
                        'get_base_mutation_level=function() return '+('1' if flag else '0')+' end}',
                        'crawl={hints_type=function() return '+json.dumps(hint)+' end}',
                        'spells={memorised=function() return '+boolean+' end}',
                        'local original=(function() '+source+'\nend)()',
                        'local translated=(function() '+target+'\nend)()',
                        'assert(type(original)==type(translated))',
                        'assert((original==nil)==(translated==nil))',
                        'checked=checked+1',
                        'end',
                    ]
    # Complete-description batch: exercise every new conditional display block,
    # including the two explicitly reviewed raw-identifier display adapters.
    cases = [
        ('items.txt', 'book of spontaneous combustion',
         [('you={can_smell=function() return false end}', None),
          ('you={can_smell=function() return true end}', '硫磺')]),
        ('items.txt', 'book of the grave',
         [('you={can_smell=function() return false end}', None),
          ('you={can_smell=function() return true end}', '腐败')]),
        ('items.txt', 'book of the wilderness',
         [('you={can_smell=function() return false end}', None),
          ('you={can_smell=function() return true end}', '霉味')]),
        ('items.txt', 'large rock',
         [('you={race=function() return "'+race+'" end}', expected)
          for race, expected in [('Ogre', ''), ('Troll', ''), ('Human', '只有大型生物')]]),
        ('items.txt', 'riddle talisman',
         [('you={race=function() return "'+race+'" end}', expected)
          for race, expected in [('Felid', '体型太小'), ('Human', '能够穿戴披风和身甲')]]),
        ('items.txt', 'dragon-coil talisman',
         [('you={race=function() return "'+race+'" end}', expected)
          for race, expected in [('Red Draconian', '吐息'), ('Grey Draconian', '吐息'), ('Human', '金龙')]]),
        ('items.txt', 'eel talisman',
         [('you={hand=function() return "'+hand+'" end}', expected)
          for hand, expected in [('hand','双手'), ('paw','双爪'), ('tentacle','触手'),
                                 ('claw','利爪'), ('grasper','抓握肢'), ('tendril','触须'),
                                 ('unknown future anatomy','肢体')]]),
        ('status.txt', 'dragon status',
         [('you={race=function() return "'+race+'" end}', expected)
          for race, expected in [('Human','喷火巨龙'), ('Grey Draconian','颜色'),
                                ('Red Draconian','易受寒冷伤害'), ('White Draconian','易受火焰伤害'),
                                ('Green Draconian','颜色'), ('Black Draconian','毒素抗性')]]),
        ('status.txt', 'pact status',
         [('you={religion="Makhleb"}', '履行'), ('you={religion="No God"}', '战斗')]),
        ('status.txt', 'water status',
         [('you={race=function() return "'+race+'" end,flying=function() return '+flight+' end}; '
           'view={feature_at=function(x,y) assert(x==0 and y==0); return "'+feat+'" end}', expected)
          for race, flight, feat, expected in [
              ('Human','false','deep_water','站在深水中'),
              ('Human','false','shallow_water','站在浅水中'),
              ('Merfolk','false','deep_water','游在深水中'),
              ('Merfolk','false','shallow_water','游在浅水中'),
              ('Human','true','deep_water','飞在深水上方'),
              ('Merfolk','true','shallow_water','飞在浅水上方'),
              ('Human','false','floor','水域上方'),
              ('Human','true','unknown_future_feature','水域上方')]])]
    for file, key, branches in cases:
        english = parse_text((DATA/'descript'/file).read_text(encoding='utf-8'))[0][key][1]
        chinese = parse_text((DATA/'descript/zh'/file).read_text(encoding='utf-8'))[0][key][1]
        source, = re.findall(r'\{\{(.*?)\}\}', english, re.S)
        target, = re.findall(r'\{\{(.*?)\}\}', chinese, re.S)
        for setup, expected in branches:
            program += ['do', setup,
                        'local original=(function() '+source+'\nend)()',
                        'local translated=(function() '+target+'\nend)()',
                        'assert(type(original)==type(translated))',
                        'assert((original==nil)==(translated==nil))']
            if expected is not None:
                program += ['assert(string.find(translated, '+json.dumps(expected, ensure_ascii=False)+',1,true))',
                            'assert(not string.find(translated,"%a"))']
            program += ['checked=checked+1', 'end']
    program += ['print("Lua branches passed: " .. checked)']
    with tempfile.TemporaryDirectory() as tmp:
        script = Path(tmp)/'textdb-check.lua'
        script.write_text('\n'.join(program), encoding='utf-8')
        subprocess.run([str(runner), str(script)], check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--lua', required=True, type=Path)
    test_lua(parser.parse_args().lua.resolve())
