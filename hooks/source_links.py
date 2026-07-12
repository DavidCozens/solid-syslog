"""MkDocs build hook: rewrite repo-relative source links to canonical GitHub URLs.

The docs live under ``docs/`` but link to source files outside it
(``../Core/...``, ``../Platform/...``, ``../SECURITY.md``, and so on). Those
relative links are correct when the Markdown is read on GitHub, in a clone, in a
fork, or in the IDE, but they escape the built site's tree and would 404 on the
published pages.

This hook keeps the source Markdown relative and, at build time only, rewrites
any link that resolves *outside* ``docs/`` to
``{repo_url}/blob|tree/{ref}/{path}``. Links that stay within ``docs/``,
absolute URLs, in-page anchors, and mail/tel links are left untouched. It runs
in ``on_page_markdown`` (before MkDocs resolves and validates relative links),
so the site can keep strict ``not_found`` link validation.

A file target maps to ``/blob/``; a directory target (trailing slash) maps to
``/tree/``. The ref defaults to ``main`` and is pinned per build via the
``SOLIDSYSLOG_DOCS_REF`` environment variable (set it to a release tag or commit
SHA for a versioned or published build so the links stay self-consistent).
"""

import os
import posixpath
import re

# Inline Markdown link / image: [text](target) or ![alt](target "title").
_LINK = re.compile(r'(!?\[[^\]]*\])\(\s*(<[^>]+>|[^)\s]+)((?:\s+"[^"]*")?)\s*\)')
# Link reference definition: [id]: target "title"
_LINK_DEF = re.compile(r'^(\s*\[[^\]]+\]:\s*)(<[^>]+>|\S+)(.*)$')
# Fenced code block opener/closer.
_FENCE = re.compile(r'^\s*(`{3,}|~{3,})')
# Inline code span, masked out before link rewriting.
_INLINE_CODE = re.compile(r'`+[^`]*`+')
_LEADING_UP = re.compile(r'^(?:\.\./)+')
_MASK_RE = re.compile('\x00(\\d+)\x00')


def _ref():
    return os.environ.get('SOLIDSYSLOG_DOCS_REF', '').strip() or 'main'


def _rewrite(target, src_dir, repo_url):
    """Return the canonical URL for an out-of-docs target, or None to leave it."""
    inner = target
    if inner.startswith('<') and inner.endswith('>'):
        inner = inner[1:-1]
    inner = inner.strip()
    if not inner or inner.startswith(('http://', 'https://', '//', '#', 'mailto:', 'tel:')):
        return None
    path, sep, frag = inner.partition('#')
    if not path:
        return None
    resolved = posixpath.normpath(posixpath.join(src_dir, path))
    if not resolved.startswith('..'):
        return None  # resolves within docs/ -- a normal internal link
    repo_path = _LEADING_UP.sub('', resolved)
    kind = 'tree' if path.endswith('/') else 'blob'
    url = '{}/{}/{}/{}'.format(repo_url, kind, _ref(), repo_path)
    if sep:
        url += '#' + frag
    return url


def on_page_markdown(markdown, page, config, files, **kwargs):
    repo_url = (config.get('repo_url') or '').rstrip('/')
    if not repo_url:
        return markdown
    src = page.file.src_path.replace(os.sep, '/')
    src_dir = posixpath.dirname(src)

    def link_sub(match):
        new = _rewrite(match.group(2), src_dir, repo_url)
        if new is None:
            return match.group(0)
        return '{}({}{})'.format(match.group(1), new, match.group(3))

    def def_sub(match):
        new = _rewrite(match.group(2), src_dir, repo_url)
        if new is None:
            return match.group(0)
        return '{}{}{}'.format(match.group(1), new, match.group(3))

    def rewrite_line(line):
        spans = []

        def mask(match):
            spans.append(match.group(0))
            return '\x00{}\x00'.format(len(spans) - 1)

        masked = _INLINE_CODE.sub(mask, line)
        masked = _LINK.sub(link_sub, masked)
        masked = _LINK_DEF.sub(def_sub, masked)
        return _MASK_RE.sub(lambda m: spans[int(m.group(1))], masked)

    out = []
    fence = None
    for line in markdown.split('\n'):
        opener = _FENCE.match(line)
        if opener is not None:
            marker = opener.group(1)[0]
            if fence is None:
                fence = marker
            elif fence == marker:
                fence = None
            out.append(line)
        elif fence is not None:
            out.append(line)
        else:
            out.append(rewrite_line(line))
    return '\n'.join(out)
