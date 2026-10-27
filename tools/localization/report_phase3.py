"""Report the managed TextDB scope and all remaining English fallback keys."""
import json
from pathlib import Path
from build_textdb import ROOT, DATA, parse_text, digest


def report():
    coverage = json.loads((ROOT/'localization/manifests/phase3-coverage.json').read_text(encoding='utf-8'))
    ui = json.loads((ROOT/'localization/manifests/ui-sample.json').read_text(encoding='utf-8'))
    audit = json.loads((ROOT/'docs/localization/phase3-textdb-line-review.json').read_text(encoding='utf-8'))
    owned = {entry['source'] for entry in coverage['files']}
    text = '# 阶段 3 覆盖与缺译清单\n\n'
    text += '基线：0.34.1。条目按 TextDB 规范化后的唯一键计数，UI 按上下文与原句计数。\n\n'
    reviewed_ui = sum(e['review_status'] == 'reviewed' for e in ui['messages'])
    text += f'UI 清单：{len(ui["messages"])} 条，其中已审 {reviewed_ui} 条，其余暂缓或待审条目不进入中文目录。TextDB 已导出：{sum(f["exported"] for f in coverage["files"])} 条。\n\n'
    documents = [e for e in ui['messages'] if e.get('document_source')]
    if documents:
        text += f'UI 数量中包含 {len(documents)} 份独立帮助文档，整篇各计一项，不按段落或字符计数。\n\n'
    text += '| TextDB 来源 | 当前英文键数 | 已审导出 | 英文回退 |\n| --- | ---: | ---: | ---: |\n'
    for file in coverage['files']:
        text += f'| `{file["source"]}` | {file["source_entries"]} | {file["exported"]} | {len(file["english_fallback"])} |\n'
    text += '\n## 逐行审查记录的有效性\n\n'
    stale = []
    for entry in audit['entries'] + audit.get('reviewed_references', []):
        source = parse_text((DATA/entry['source']).read_text(encoding='utf-8'))[0]
        current = source.get(entry['key'].lower())
        if not current or digest(current[1]) != entry['source_sha256']:
            stale.append(entry['source'] + ': ' + entry['key'])
    text += ('来源已变化，须重新校对：\n\n' + '\n'.join('- '+s for s in stale) if stale else '本批逐行记录的来源指纹全部与当前英文正文一致。') + '\n'
    text += '\n## 管理文件中的英文回退键\n'
    for file in coverage['files']:
        if file['english_fallback']:
            text += '\n### '+file['source']+'\n\n'
            text += '\n'.join('- `'+key+'`' for key in file['english_fallback'])+'\n'
    text += '\n## 未纳入本批 PO 管理的旧中文文件\n\n以下文件仍沿用上游已有数据，本批没有声称逐行校对它们。正式发布前须另行复核或停用。\n\n'
    for path in sorted(DATA.glob('*/zh/*.txt')):
        rel = path.parent.parent.relative_to(DATA) / path.name
        if rel.as_posix() not in owned:
            text += '- `'+path.relative_to(DATA).as_posix()+'`\n'
    if documents:
        text += '\n## 独立帮助文档\n\n| 来源 | 当前英文指纹 |\n| --- | --- |\n'
        for entry in documents:
            source = (ROOT/entry['document_source']).read_text(encoding='utf-8').rstrip()
            state = '匹配已审来源' if digest(source) == entry['source_sha256'] else '来源已变更，回退英文'
            text += '| `' + entry['document_source'] + '` | ' + state + ' |\n'
        text += '\n游戏显示按完整英文匹配；未收录文档继续使用原英文。中文阅读副本见 `crawl-ref/docs/zh/`。\n'
    text += '\n## UI 目录模块\n\n'
    modules = {}
    for entry in ui['messages']:
        module = entry.get('module', '阶段 1 最小样例')
        modules[module] = modules.get(module, 0) + 1
    text += '\n'.join('- '+k+'：'+str(v)+' 条' for k,v in sorted(modules.items()))+'\n'
    text += '\nUI 数量不等于界面全部字符串或实际调用点数量；对象名称、复杂动态消息及未接入字符串仍可能为英文。\n'
    (ROOT/'docs/localization/phase3-coverage.md').write_text(text, encoding='utf-8')


if __name__ == '__main__':
    report()
