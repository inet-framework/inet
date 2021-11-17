# -*- coding: utf-8 -*-

import os
import xml.etree.ElementTree as ET
import urllib.parse
from collections import namedtuple

from docutils import nodes, utils
from sphinx.util.nodes import split_explicit_title
from sphinx.util.console import bold, standout
from sphinx import __version__ as sphinx_version

if sphinx_version >= '1.6.0':
    from sphinx.util.logging import getLogger

from ..doxylink import __version__
from .parsing import normalise, ParseException

Entry = namedtuple('Entry', ['kind', 'file'])


def report_info(env, msg, docname=None, lineno=None):
    '''Convenience function for logging an informational

    Args:
        msg (str): Message of the warning
        docname (str): Name of the document on which the error occured
        lineno (str): Line number in the document on which the error occured
    '''
    if sphinx_version >= '1.6.0':
        logger = getLogger(__name__)
        if lineno is not None:
            logger.info(msg, location=(docname, lineno))
        else:
            logger.info(msg, location=docname)
    else:
        env.info(docname, msg, lineno=lineno)

def report_warning(env, msg, docname=None, lineno=None):
    '''Convenience function for logging a warning

    Args:
        msg (str): Message of the warning
        docname (str): Name of the document on which the error occured
        lineno (str): Line number in the document on which the error occured
    '''
    if sphinx_version >= '1.6.0':
        logger = getLogger(__name__)
        if lineno is not None:
            logger.warning(msg, location=(docname, lineno))
        else:
            logger.warning(msg, location=docname)
    else:
        env.warn(docname, msg, lineno=lineno)


class FunctionList:
    """A FunctionList maps argument lists to specific entries"""
    def __init__(self):
        self.kind = 'function_list'
        self._arglist = {}  # type: MutableMapping[str, str]

    def __getitem__(self, arglist: str) -> Entry:
        # If the user has requested a specific function through specifying an arglist then get the right anchor
        if arglist:
            try:
                filename = self._arglist[arglist]
            except KeyError:
                # TODO Offer fuzzy suggestion
                raise LookupError('Argument list match not found')
        else:
            # Otherwise just return the first entry (if they don't care they get whatever comes first)
            filename = list(self._arglist.values())[0]

        return Entry(kind='function', file=filename)

    def add_overload(self, arglist: str, file: str) -> None:
        self._arglist[arglist] = file


class SymbolMap:
    """A SymbolMap maps symbols to Entries or FunctionLists"""
    def __init__(self, xml_doc: ET.ElementTree) -> None:
        self._mapping = parse_tag_file(xml_doc)

    def _get_symbol_match(self, symbol: str) -> str:
        if self._mapping.get(symbol):
            return symbol

        piecewise_list = match_piecewise(self._mapping.keys(), symbol)

        # If there is only one match, return it.
        if len(piecewise_list) == 1:
            return list(piecewise_list)[0]

        # If there is more than one item in piecewise_list then there is an ambiguity
        # Often this is due to the symbol matching the name of the constructor as well as the class name itself
        # We will prefer the class
        classes_list = {s for s in piecewise_list if self._mapping[s].kind == 'class'}

        # If there is only one by here we return it.
        if len(classes_list) == 1:
            return list(classes_list)[0]

        # Now, to disambiguate between ``PolyVox::Array< 1, ElementType >::operator[]`` and ``PolyVox::Array::operator[]`` matching ``operator[]``,
        # we will ignore templated (as in C++ templates) tag names by removing names containing ``<``
        no_templates_list = {s for s in piecewise_list if '<' not in s}

        if len(no_templates_list) == 1:
            return list(no_templates_list)[0]

        # If not found by now, return the shortest match, assuming that's the most specific
        if no_templates_list:
            # TODO return a warning here?
            return min(no_templates_list, key=len)

        # TODO Offer fuzzy suggestion
        raise LookupError('Could not find a match')

    def __getitem__(self, item: str) -> Entry:
        symbol, normalised_arglist = normalise(item)

        matched_symbol = self._get_symbol_match(symbol)
        entry = self._mapping[matched_symbol]

        if isinstance(entry, FunctionList):
            entry = entry[normalised_arglist]

        return entry


def parse_tag_file(doc: ET.ElementTree) -> dict:
    """
    Takes in an XML tree from a Doxygen tag file and returns a dictionary that looks something like:

    .. code-block:: python

        {'PolyVox': Entry(...),
         'PolyVox::Array': Entry(...),
         'PolyVox::Array1DDouble': Entry(...),
         'PolyVox::Array1DFloat': Entry(...),
         'PolyVox::Array1DInt16': Entry(...),
         'QScriptContext::throwError': FunctionList(...),
         'QScriptContext::toString': FunctionList(...)
         }

    Note the different form for functions. This is required to allow for 'overloading by argument type'.

    :Parameters:
        doc : xml.etree.ElementTree
            The XML DOM object

    :return: a dictionary mapping fully qualified symbols to files
    """

    mapping = {}  # type: MutableMapping[str, Union[Entry, FunctionList]]
    function_list = []  # This is a list of function to be parsed and inserted into mapping at the end of the function.
    for compound in doc.findall('./compound'):
        compound_kind = compound.get('kind')
        if compound_kind not in {'namespace', 'class', 'struct', 'file', 'define', 'group'}:
            continue

        compound_name = compound.findtext('name')
        compound_filename = compound.findtext('filename')

        # TODO The following is a hack bug fix I think
        # Doxygen doesn't seem to include the file extension to <compound kind="file"><filename> entries
        # If it's a 'file' type, check if it _does_ have an extension, if not append '.html'
        if compound_kind == 'file' and not os.path.splitext(compound_filename)[1]:
            compound_filename = compound_filename + '.html'

        # If it's a compound we can simply add it
        mapping[compound_name] = Entry(kind=compound_kind, file=compound_filename)

        for member in compound.findall('member'):
            # If the member doesn't have an <anchorfile> element, use the parent compounds <filename> instead
            # This is the way it is in the qt.tag and is perhaps an artefact of old Doxygen
            anchorfile = member.findtext('anchorfile') or compound_filename
            member_symbol = compound_name + '.' + member.findtext('name')
            member_kind = member.get('kind')
            arglist_text = member.findtext('./arglist')  # If it has an <arglist> then we assume it's a function. Empty <arglist> returns '', not None. Things like typedefs and enums can have empty arglists

            if arglist_text and member_kind not in {'variable', 'typedef', 'enumeration'}:
                function_list.append((member_symbol, arglist_text, member_kind, join(anchorfile, '#', member.findtext('anchor'))))
            else:
                mapping[member_symbol] = Entry(kind=member.get('kind'), file=join(anchorfile, '#', member.findtext('anchor')))

    for member_symbol, arglist, kind, anchor_link in function_list:
        try:
            normalised_arglist = normalise(member_symbol + arglist)[1]
        except ParseException as e:
            print('Skipping %s %s%s. Error reported from parser was: %s' % (kind, member_symbol, arglist, e))
        else:
            if mapping.get(member_symbol) and isinstance(mapping[member_symbol], FunctionList):
                mapping[member_symbol].add_overload(normalised_arglist, anchor_link)
            else:
                mapping[member_symbol] = FunctionList()
                mapping[member_symbol].add_overload(normalised_arglist, anchor_link)

    return mapping


def match_piecewise(candidates: set, symbol: str, sep: str='.') -> set:
    """
    Match the requested symbol reverse piecewise (split on ``::``) against the candidates.
    This allows you to under-specify the base namespace so that ``"MyClass"`` can match ``my_namespace::MyClass``

    Args:
        candidates: set of possible matches for symbol
        symbol: the symbol to match against
        sep: the separator between identifier elements

    Returns:
        set of matches
    """
    piecewise_list = set()
    for item in candidates:
        split_symbol = symbol.split(sep)
        split_item = item.split(sep)

        split_symbol.reverse()
        split_item.reverse()

        min_length = len(split_symbol)

        split_item = split_item[:min_length]

        if split_symbol == split_item:
            piecewise_list.add(item)

    return piecewise_list


def join(*args):
    return ''.join(args)


def create_role(app, tag_filename, rootdir):
    # Tidy up the root directory path
    if not rootdir.endswith(('/', '\\')):
        rootdir = join(rootdir, os.sep)

    try:
        tag_file = ET.parse(tag_filename)

        cache_name = os.path.basename(tag_filename)

        report_info(app.env, bold('Checking tag file cache for %s: ' % cache_name))
        if not hasattr(app.env, 'doxylink_cache'):
            # no cache present at all, initialise it
            report_info(app.env, 'No cache at all, rebuilding...')
            mapping = SymbolMap(tag_file)
            app.env.doxylink_cache = {cache_name: {'mapping': mapping, 'mtime': os.path.getmtime(tag_filename)}}
        elif not app.env.doxylink_cache.get(cache_name):
            # Main cache is there but the specific sub-cache for this tag file is not
            report_info(app.env, 'Sub cache is missing, rebuilding...')
            mapping = SymbolMap(tag_file)
            app.env.doxylink_cache[cache_name] = {'mapping': mapping, 'mtime': os.path.getmtime(tag_filename)}
        elif app.env.doxylink_cache[cache_name]['mtime'] < os.path.getmtime(tag_filename):
            # tag file has been modified since sub-cache creation
            report_info(app.env, 'Sub-cache is out of date, rebuilding...')
            mapping = SymbolMap(tag_file)
            app.env.doxylink_cache[cache_name] = {'mapping': mapping, 'mtime': os.path.getmtime(tag_filename)}
        elif not app.env.doxylink_cache[cache_name].get('version') or app.env.doxylink_cache[cache_name].get('version') != __version__:
            # sub-cache doesn't have a version or the version doesn't match
            report_info(app.env, 'Sub-cache schema version doesn\'t match, rebuilding...')
            mapping = SymbolMap(tag_file)
            app.env.doxylink_cache[cache_name] = {'mapping': mapping, 'mtime': os.path.getmtime(tag_filename)}
        else:
            # The cache is up to date
            report_info(app.env, 'Sub-cache is up-to-date')
    except FileNotFoundError:
        tag_file = None
        report_warning(app.env, standout('Could not find tag file %s. Make sure your `doxylink` config variable is set correctly.' % tag_filename))

    def find_doxygen_link(name, rawtext, text, lineno, inliner, options={}, content=[]):
        # from :name:`title <part>`
        has_explicit_title, title, part = split_explicit_title(text)
        part = utils.unescape(part)
        warning_messages = []
        if not tag_file:
            warning_messages.append('Could not find match for `%s` because tag file not found' % part)
            return [nodes.inline(title, title)], []

        try:
            url = app.env.doxylink_cache[cache_name]['mapping'][part]
        except LookupError as error:
            inliner.reporter.warning('Could not find match for `%s` in `%s` tag file. Error reported was %s' % (part, tag_filename, error), line=lineno)
            pnode = nodes.inline(title, title)
            pnode.set_class('reference-'+name)
            return [pnode], []
        except ParseException as error:
            inliner.reporter.warning('Error while parsing `%s`. Is not a well-formed C++ function call or symbol.'
                                     'If this is not the case, it is a doxylink bug so please report it.'
                                     'Error reported was: %s' % (part, error), line=lineno)
            return [nodes.inline(title, title)], []

        # If it's an absolute path then the link will work regardless of the document directory
        # Also check if it is a URL (i.e. it has a 'scheme' like 'http' or 'file')
        if os.path.isabs(rootdir) or urllib.parse.urlparse(rootdir).scheme:
            full_url = join(rootdir, url.file)
        # But otherwise we need to add the relative path of the current document to the root source directory to the link
        else:
            relative_path_to_docsrc = os.path.relpath(app.env.srcdir, os.path.dirname(inliner.document.attributes['source']))
            full_url = join(relative_path_to_docsrc, '/', rootdir, url.file)  # We always use the '/' here rather than os.sep since this is a web link avoids problems like documentation/.\../library/doc/ (mixed slashes)

        if url.kind == 'function' and app.config.add_function_parentheses and normalise(title)[1] == '' and not has_explicit_title:
            title = join(title, '()')

        pnode = nodes.reference(title, title, internal=False, refuri=full_url)
        pnode.set_class('reference-'+name)
        return [pnode], []

    return find_doxygen_link


def setup_doxylink_roles(app):
    for name, (tag_filename, rootdir) in app.config.doxylink.items():
        app.add_role(name, create_role(app, tag_filename, rootdir))
