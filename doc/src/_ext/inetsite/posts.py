"""News and progress posts.

A post is a document whose file-wide metadata has a :category: and a :date: field. Each post gets its date
shown below the title, and the `postlist` directive lists the posts of a category, newest first.
"""

import datetime
import re

from docutils import nodes
from docutils.parsers.rst import Directive, directives

STYLES = ('titles', 'excerpts', 'full')
MONTHS = ('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October',
          'November', 'December')
CATEGORY_PAGES = {'news': ('News', 'All news'), 'progress': ('Progress', 'All progress updates')}


class postlist(nodes.General, nodes.Element):
    pass


class PostList(Directive):
    required_arguments = 1
    option_spec = {
        'limit': directives.positive_int,
        'style': lambda arg: directives.choice(arg, STYLES),
    }

    def run(self):
        env = self.state.document.settings.env
        env.inet_postlist_docs = getattr(env, 'inet_postlist_docs', set()) | {env.docname}
        node = postlist()
        node['category'] = self.arguments[0]
        node['limit'] = self.options.get('limit')
        node['style'] = self.options.get('style', 'titles')
        return [node]


def format_date(date, short=False):
    month = MONTHS[date.month - 1]
    return f'{month[:3] if short else month} {date.day}, {date.year}'


def parse_date(docname, value):
    try:
        return datetime.date.fromisoformat(value)
    except ValueError:
        raise ValueError(f'{docname}: invalid :date: {value!r}, expected YYYY-MM-DD') from None


def collect_post(app, doctree):
    env = app.env
    meta = env.metadata.get(env.docname, {})
    if 'category' not in meta:
        return
    if 'date' not in meta:
        raise ValueError(f'{env.docname}: post has a :category: but no :date:')
    date = parse_date(env.docname, meta['date'])
    section = next(iter(doctree.findall(nodes.section)), None)
    if section is None:
        raise ValueError(f'{env.docname}: post has no title')
    title = section[0].astext()
    body = [n.deepcopy() for n in section.children[1:]]
    if 'excerpt' in meta:
        excerpt = nodes.paragraph('', re.sub(r'<[^>]+>', '', meta['excerpt']))
    else:
        excerpt = next((n.deepcopy() for n in section.findall(nodes.paragraph)), nodes.paragraph())
    env.inet_posts = getattr(env, 'inet_posts', {})
    env.inet_posts[env.docname] = {
        'title': title, 'date': date, 'category': meta['category'], 'excerpt': excerpt, 'body': body,
    }
    section.insert(1, nodes.paragraph('', format_date(date), classes=['post-date']))
    page = CATEGORY_PAGES.get(meta['category'])
    if page:
        ref = nodes.reference('', page[1], internal=True, refuri=app.builder.get_relative_uri(env.docname, page[0]))
        section.append(nodes.paragraph('', '', ref, classes=['post-related']))


def purge_post(app, env, docname):
    getattr(env, 'inet_posts', {}).pop(docname, None)
    getattr(env, 'inet_postlist_docs', set()).discard(docname)


def merge_posts(app, env, docnames, other):
    env.inet_posts = getattr(env, 'inet_posts', {})
    env.inet_posts.update(getattr(other, 'inet_posts', {}))
    env.inet_postlist_docs = getattr(env, 'inet_postlist_docs', set()) | getattr(other, 'inet_postlist_docs', set())


def outdated_lists(app, env, added, changed, removed):
    if added or changed or removed:
        return list(getattr(env, 'inet_postlist_docs', set()))
    return []


def post_header(builder, fromdocname, docname, post, short_date):
    ref = nodes.reference('', post['title'], internal=True,
                          refuri=builder.get_relative_uri(fromdocname, docname))
    date = nodes.inline('', f' ({format_date(post["date"], short_date)})', classes=['post-date'])
    return ref, date


def render_lists(app, doctree, fromdocname):
    posts = getattr(app.env, 'inet_posts', {})
    for node in list(doctree.findall(postlist)):
        selected = sorted(((d, p) for d, p in posts.items() if p['category'] == node['category']),
                          key=lambda item: (item[1]['date'], item[0]), reverse=True)
        if node['limit']:
            selected = selected[:node['limit']]
        if node['style'] == 'titles':
            content = nodes.bullet_list(classes=['post-list'])
            for docname, post in selected:
                ref, date = post_header(app.builder, fromdocname, docname, post, True)
                content += nodes.list_item('', nodes.paragraph('', '', ref, date))
            node.replace_self(content)
            continue
        entries = []
        for docname, post in selected:
            ref, date = post_header(app.builder, fromdocname, docname, post, False)
            entry = nodes.container(classes=['post-entry'])
            entry += nodes.paragraph('', '', nodes.strong('', '', ref), date, classes=['post-title'])
            if node['style'] == 'excerpts':
                entry += post['excerpt'].deepcopy()
                more = nodes.reference('', 'More', internal=True,
                                       refuri=app.builder.get_relative_uri(fromdocname, docname))
                entry += nodes.paragraph('', '', more)
            else:
                entry.extend(n.deepcopy() for n in post['body'])
            entries.append(entry)
        node.replace_self(entries)


def setup(app):
    app.add_node(postlist)
    app.add_directive('postlist', PostList)
    app.connect('doctree-read', collect_post, priority=600)
    app.connect('env-purge-doc', purge_post)
    app.connect('env-merge-info', merge_posts)
    app.connect('env-get-outdated', outdated_lists)
    app.connect('doctree-resolved', render_lists)
