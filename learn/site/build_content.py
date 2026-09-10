"""Rebuild the offline learning snapshot: python learn/site/build_content.py.

Requires Python-Markdown only when regenerating; browsing needs no dependencies.
"""
import hashlib
import json
import re
from pathlib import Path
from datetime import datetime, timezone
import markdown

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'learn/site'
lessons = json.loads((OUT / 'lessons.json').read_text(encoding='utf-8'))
paths = sorted((ROOT / 'learn').rglob('*.md'))
paths = [p for p in paths if 'site' not in p.parts and p.name != 'SITE-TASK.md']
paths += [ROOT / p for p in ['docs/VERIFICATION.md', 'docs/ARCHITECTURE.md', '.agents/BLASTER_ACCELERATED_LEARNING_PLAN.md']]
paths += [OUT / 'PLAN-RECONCILIATION.md', OUT / 'README.md']
known_docs = {p.relative_to(ROOT).as_posix() for p in paths if p.exists()}
known_sources = {ref[0] for lesson in lessons for ref in lesson['refs']}
docs = {}
for p in paths:
    if not p.exists():
        continue
    raw = p.read_text(encoding='utf-8-sig')
    key = p.relative_to(ROOT).as_posix()
    body = markdown.markdown(raw, extensions=['fenced_code', 'tables', 'sane_lists'])
    def link(m):
        href = m.group(1)
        if '://' in href or href.startswith('#'):
            return m.group(0)
        target = (p.parent / href.split('#')[0]).resolve()
        try:
            rel = target.relative_to(ROOT).as_posix()
        except ValueError:
            return m.group(0)
        if rel in known_docs:
            return 'href="#doc=' + rel + '"'
        if rel in known_sources:
            return 'href="#source=' + rel + '"'
        if target.is_file():
            return 'href="../' + rel + '"'
        return 'title="原文路径：' + href.replace('"', '&quot;') + '"'
    body = re.sub(r'href="([^"]+)"', link, body)
    docs[key] = {'title': next((s.lstrip('# ').strip() for s in raw.splitlines() if s.startswith('# ')), p.stem), 'html': body}
sources = {}
for lesson in lessons:
    for ref in lesson['refs']:
        path, symbol = ref
        p = ROOT / path
        if not p.is_file():
            raise ValueError(f'Missing source: {path}')
        try:
            raw = p.read_text(encoding='utf-8-sig')
        except UnicodeDecodeError:
            raw = p.read_text(encoding='gb18030')
        lines = raw.splitlines()
        matches = [i + 1 for i, line in enumerate(lines) if symbol in line]
        if not matches:
            raise ValueError(f'Missing anchor: {path}: {symbol}')
        ref.append(matches[0]) if len(ref) == 2 else None
        sources[path] = {'text': raw, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
    for doc in lesson['docs']:
        if doc not in docs:
            raise ValueError(f'Missing document: {doc}')
payload = {'builtAt': datetime.now(timezone.utc).isoformat(), 'lessons': lessons, 'docs': docs, 'sources': sources}
(OUT / 'content.js').write_text('window.COURSE = ' + json.dumps(payload, ensure_ascii=False).replace('</', '<\\/') + ';\n', encoding='utf-8')
print(f'Built {len(lessons)} lessons, {len(docs)} documents, {len(sources)} source files')
