// Renders the navigation tree of the NED reference (api/navtree.json, written by _ext/inetsite/apinav.py) under
// the "NED Reference" entry of the sidebar, in the markup of the theme's own nested navigation. Group children
// are only rendered when a group is first expanded, so a page carries a few hundred entries, not thousands.
// A type is listed in several groups (e.g. Packages and Modules), but has a single page: the page opens in the
// group whose entry was clicked, so that the navigation keeps its context.
(function () {
  const script = document.currentScript;
  if (!script) return;
  const apiBase = new URL('../api/', script.src).href;
  const GROUP = 'inet-apinav.group';
  let tree = null;
  let counter = 0;

  function strip(url) {
    return url.split(/[?#]/)[0];
  }

  function element(tag, className, attrs) {
    const e = document.createElement(tag);
    if (className) e.className = className;
    for (const [k, v] of Object.entries(attrs || {})) e.setAttribute(k, v);
    return e;
  }

  function link(entry, active) {
    const a = element('a', 'md-nav__link' + (active ? ' md-nav__link--active' : ''), {href: entry.href, title: entry.title});
    const span = element('span', 'md-ellipsis');
    entry.title.split(/(?<=\.|::)/).forEach((part, i) => {
      if (i) span.appendChild(element('wbr'));
      span.appendChild(document.createTextNode(part));
    });
    a.appendChild(span);
    return a;
  }

  function isPage(entry, page) {
    return !entry.href.includes('#') && strip(entry.href) === page;
  }

  function contains(entry, page) {
    return isPage(entry, page) || (entry.children || []).some(c => contains(c, page));
  }

  function resolve(entries) {
    for (const e of entries) {
      e.href = new URL(e.url, apiBase).href;
      if (e.children) resolve(e.children);
    }
  }

  // Entries outside the group of the context are neither active nor open.
  function item(entry, level, page, group) {
    const inContext = group === undefined || group === entry;
    const current = inContext && isPage(entry, page);
    if (!entry.children) {
      const li = element('li', 'md-nav__item inet-apinav');
      li.appendChild(link(entry, current));
      return li;
    }
    const open = inContext && (entry.follow !== false || group === entry) && contains(entry, page);
    const id = '__apinav_' + (++counter);
    const li = element('li', 'md-nav__item md-nav__item--nested inet-apinav' + (open ? ' md-nav__item--active' : ''));
    if (level === groupLevel) li.dataset.apinavGroup = entry.title;
    const toggle = element('input', 'md-nav__toggle md-toggle', {type: 'checkbox', id: id});
    toggle.checked = open;
    li.appendChild(toggle);
    const container = element('div', 'md-nav__link md-nav__container');
    container.appendChild(link(entry, current));
    const label = element('label', 'md-nav__link', {for: id, tabindex: '0'});
    label.appendChild(element('span', 'md-nav__icon md-icon'));
    container.appendChild(label);
    li.appendChild(container);
    const nav = element('nav', 'md-nav', {'data-md-level': String(level), 'aria-expanded': String(open)});
    const title = element('label', 'md-nav__title', {for: id});
    title.appendChild(element('span', 'md-nav__icon md-icon'));
    title.appendChild(document.createTextNode(entry.title));
    nav.appendChild(title);
    const list = element('ul', 'md-nav__list', {'data-md-scrollfix': ''});
    nav.appendChild(list);
    li.appendChild(nav);
    const fill = () => {
      if (list.childElementCount) return;
      for (const child of entry.children)
        list.appendChild(item(child, level + 1, page, entry === reference ? context : undefined));
    };
    if (open) fill();
    toggle.addEventListener('change', () => {
      nav.setAttribute('aria-expanded', String(toggle.checked));
      if (toggle.checked) fill();
    });
    return li;
  }

  let reference = null;
  let context = undefined;
  let groupLevel = 0;

  // The group of the last click in the navigation of the NED reference, if it lists the page below its own.
  function chooseContext(page) {
    let title = null;
    try { title = sessionStorage.getItem(GROUP); } catch (err) { }
    const group = tree.find(g => g.title === title && (g.children || []).some(c => contains(c, page)));
    if (group) return group;
    return tree.find(g => g.follow !== false && contains(g, page)) || null;
  }

  document.addEventListener('click', e => {
    const a = e.target.closest && e.target.closest('li.inet-apinav a.md-nav__link[href]');
    if (!a) return;
    const group = a.closest('li[data-apinav-group]');
    try {
      if (group) sessionStorage.setItem(GROUP, group.dataset.apinavGroup);
      else sessionStorage.removeItem(GROUP);
    } catch (err) { }
  }, true);

  function referenceItem(primary) {
    const index = apiBase + 'index.html';
    for (const a of primary.querySelectorAll('a.md-nav__link')) {
      const li = a.closest('li.md-nav__item');
      if (strip(a.href) === index && !li.classList.contains('md-nav__current-toc')) return li;
    }
    return null;
  }

  function activateAncestors(li, primary) {
    let top = li;
    for (let e = li.parentElement; e && e !== primary; e = e.parentElement) {
      if (e.matches('li.md-nav__item')) {
        e.classList.add('md-nav__item--active');
        const toggle = e.querySelector(':scope > input.md-nav__toggle');
        if (toggle) toggle.checked = true;
        top = e;
      }
    }
    const first = top.querySelector('a.md-nav__link[href]');
    if (!first) return;
    for (const tab of document.querySelectorAll('.md-tabs__link')) {
      if (strip(tab.href) === strip(first.href)) tab.closest('.md-tabs__item').classList.add('md-tabs__item--active');
    }
  }

  function render() {
    const page = strip(location.href);
    if (!page.startsWith(apiBase)) return;
    const primary = document.querySelector('.md-sidebar--primary .md-nav--primary');
    if (!primary || primary.dataset.apinav) return;
    const old = referenceItem(primary);
    if (!old) return;
    primary.dataset.apinav = 'done';
    const levelNav = old.closest('nav[data-md-level]');
    const level = levelNav ? Number(levelNav.dataset.mdLevel) + 1 : 1;
    const titleLink = old.querySelector('a.md-nav__link');
    reference = {title: titleLink.textContent.trim(), href: apiBase + 'index.html', children: tree};
    groupLevel = level + 1;
    context = chooseContext(page) || undefined;
    const li = item(reference, level, page);
    old.replaceWith(li);
    activateAncestors(li, primary);
    const active = li.querySelector('.md-nav__link--active');
    const wrap = primary.closest('.md-sidebar__scrollwrap');
    if (active && wrap) {
      const offset = active.getBoundingClientRect().top - wrap.getBoundingClientRect().top;
      wrap.scrollTop += offset - wrap.clientHeight / 3;
    }
  }

  const loaded = fetch(apiBase + 'navtree.json')
    .then(r => r.json())
    .then(t => { resolve(t); tree = t; });

  function update() {
    loaded.then(render);
  }

  if (window.document$) window.document$.subscribe(update);
  else document.addEventListener('DOMContentLoaded', update);
})();
