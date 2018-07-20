//
// Copyright (C) 2006-2012 Opensim Ltd
//  Author: Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// NOTE: This file is a near copy of the similar file in OMNeT++ 4.2, but under LGPL.
// Added here until the same functionality becomes available in OMNeT++ as public API.

#include "inet/common/PatternMatcher.h"

namespace inet {

// from common/stringutil.h
inline bool opp_isdigit(unsigned char c) { return isdigit(c); }
inline char opp_toupper(unsigned char c) { return toupper(c); }
static bool opp_stringendswith(const char *s, const char *ending)
{
    int slen = strlen(s);
    int endinglen = strlen(ending);
    return slen >= endinglen && strcmp(s + slen - endinglen, ending) == 0;
}

PatternMatcher::PatternMatcher()
{
}

PatternMatcher::PatternMatcher(const char *pattern, bool dottedpath, bool fullstring, bool casesensitive)
{
    setPattern(pattern, dottedpath, fullstring, casesensitive);
}

PatternMatcher::~PatternMatcher()
{
}

void PatternMatcher::setPattern(const char *patt, bool dottedpath, bool fullstring, bool casesensitive)
{
    pattern.clear();
    iscasesensitive = casesensitive;

    // "tokenize" pattern
    const char *s = patt;
    while (*s != '\0') {
        Elem e;
        switch (*s) {
            case '?':
                e.type = dottedpath ? COMMONCHAR : ANYCHAR;
                s++;
                break;

            case '[':
                if (pattern.empty() || pattern.back().type != LITERALSTRING || !parseNumRange(s, ']', e.fromnum, e.tonum))
                    parseLiteralString(s, e);
                else
                    e.type = NUMRANGE;
                break;

            case '{':
                if (parseNumRange(s, '}', e.fromnum, e.tonum)) {
                    e.type = NUMRANGE;
                    s++;
                }
                else
                    parseSet(s, e);
                break;

            case '*':
                if (*(s + 1) == '*') {
                    e.type = ANYSEQ;
                    s += 2;
                }
                else {
                    e.type = dottedpath ? COMMONSEQ : ANYSEQ;
                    s++;
                }
                break;

            default:
                parseLiteralString(s, e);
                break;
        }
        pattern.push_back(e);
    }

    if (!fullstring) {
        // for substring match, we add "**" at both ends of the pattern (unless already there)
        Elem e;
        e.type = ANYSEQ;
        if (pattern.empty() || pattern.back().type != ANYSEQ)
            pattern.push_back(e);
        if (pattern.front().type != ANYSEQ)
            pattern.insert(pattern.begin(), e);
    }
    Elem e;
    e.type = END;
    pattern.push_back(e);
}

void PatternMatcher::parseSet(const char *& s, Elem& e)
{
    s++;    // skip "{"
    e.type = SET;
    if (*s == '^') {
        e.type = NEGSET;
        s++;
    }
    // Note: to make "}" part of the set, it must be first within the braces
    const char *sbeg = s;
    while (*s && (*s != '}' || s == sbeg)) {
        char range[3];
        range[2] = 0;
        if (*(s + 1) == '-' && *(s + 2) && *(s + 2) != '}') {
            // store "A-Z" as "AZ"
            range[0] = *s;
            range[1] = *(s + 2);
            s += 3;
        }
        else {
            // store "X" as "XX"
            range[0] = range[1] = *s;
            s++;
        }
        if (!iscasesensitive) {
            // if one end of range is alpha and the other is not, funny things will happen
            range[0] = opp_toupper(range[0]);
            range[1] = opp_toupper(range[1]);
        }
        e.setchars += range;
    }
    if (!*s)
        throw cRuntimeError("unmatched '}' in expression");
    s++;    // skip "}"
}

void PatternMatcher::parseLiteralString(const char *& s, Elem& e)
{
    e.type = LITERALSTRING;
    while (*s && *s != '?' && *s != '{' && *s != '*') {
        long dummy;
        const char *s1;
        if (*s == '\\')
            e.literalstring += *(++s);
        else
            e.literalstring += *s;
        if (*s == '[' && parseNumRange((s1 = s), ']', dummy, dummy))
            break;
        s++;
    }
}

bool PatternMatcher::parseNumRange(const char *& str, char closingchar, long& lo, long& up)
{
    //
    // try to parse "[n..m]" or "{n..m}" and return true on success.
    // str should point at "[" or "{"; on success return it'll point to "]" or "}",
    // and on failure it'll be unchanged. n and m will be stored in lo and up.
    // They are optional -- if missing, lo or up will be set to -1.
    //
    lo = up = -1L;
    const char *s = str + 1;    // skip "[" or "{"
    if (opp_isdigit(*s)) {
        lo = atol(s);
        while (opp_isdigit(*s))
            s++;
    }
    if (*s != '.' || *(s + 1) != '.')
        return false;
    s += 2;
    if (opp_isdigit(*s)) {
        up = atol(s);
        while (opp_isdigit(*s))
            s++;
    }
    if (*s != closingchar)
        return false;
    str = s;
    return true;
}

std::string PatternMatcher::debugStrFrom(int from)
{
    std::string result;
    for (size_t k = from; k < pattern.size(); k++) {
        Elem& e = pattern[k];
        switch (e.type) {
            case LITERALSTRING:
                result = result + "\"" + e.literalstring + "\"";
                break;

            case ANYCHAR:
                result += "?!";
                break;

            case COMMONCHAR:
                result += "?";
                break;

            case SET:
                result = result + "SET(" + e.setchars + ")";
                break;

            case NEGSET:
                result = result + "NEGSET(" + e.setchars + ")";
                break;

            case NUMRANGE: {
                char buf[100];
                sprintf(buf, "%ld..%ld", e.fromnum, e.tonum);
                result += buf;
            } break;

            case ANYSEQ:
                result += "**";
                break;

            case COMMONSEQ:
                result += "*";
                break;

            case END:
                break;

            default:
                ASSERT(0);
                break;
        }
        result += " ";
    }
    return result;
}

bool PatternMatcher::isInSet(char c, const char *set)
{
    ASSERT((strlen(set) & 1) == 0);
    if (!iscasesensitive)
        c = opp_toupper(c); // set is already uppercase here
    while (*set) {
        if (c >= *set && c <= *(set + 1))
            return true;
        set += 2;
    }
    return false;
}

bool PatternMatcher::doMatch(const char *s, int k, int suffixlen)
{
    while (true) {
        Elem& e = pattern[k];
        long num;    // case NUMRANGE
        int len;    // case LITERALSTRING
        switch (e.type) {
            case LITERALSTRING:
                len = e.literalstring.length();
                // special case: last string literal with prefix match: allow s to be shorter
                if (suffixlen > 0 && k == (int)pattern.size() - 2)
                    len -= suffixlen;
                // compare
                if (iscasesensitive ?
                    strncmp(s, e.literalstring.c_str(), len) :
                    strncasecmp(s, e.literalstring.c_str(), len)
                    )
                    return false;
                s += len;
                break;

            case ANYCHAR:
                if (!*s)
                    return false;
                s++;
                break;

            case COMMONCHAR:
                if (!*s || *s == '.')
                    return false;
                s++;
                break;

            case SET:
                if (!*s)
                    return false;
                if (!isInSet(*s, e.setchars.c_str()))
                    return false;
                s++;
                break;

            case NEGSET:
                if (!*s)
                    return false;
                if (isInSet(*s, e.setchars.c_str()))
                    return false;
                s++;
                break;

            case NUMRANGE:
                if (!opp_isdigit(*s))
                    return false;
                num = atol(s);
                while (opp_isdigit(*s))
                    s++;
                if ((e.fromnum >= 0 && num < e.fromnum) || (e.tonum >= 0 && num > e.tonum))
                    return false;
                break;

            case ANYSEQ:
                // potential shortcuts: if pattern ends in ANYSEQ, rest of the input
                // can be anything; if pattern ends in ANYSEQ LITERAL, it's enough if
                // input ends in the literal string
                if (k == (int)pattern.size() - 2)
                    return true;
                if (k == (int)pattern.size() - 3 && pattern[k + 1].type == LITERALSTRING)
                    return opp_stringendswith(s, pattern[k + 1].literalstring.c_str());

                // general case
                while (true) {
                    if (doMatch(s, k + 1, suffixlen))
                        return true;
                    if (!*s)
                        return false;
                    s++;
                }
                break;    // at EOS

            case COMMONSEQ:
                while (true) {
                    if (doMatch(s, k + 1, suffixlen))
                        return true;
                    if (!*s || *s == '.')
                        return false;
                    s++;
                }
                break;

            case END:
                return !*s;

            default:
                ASSERT(0);
                break;
        }
        k++;
        ASSERT(k < (int)pattern.size());
    }
}

bool PatternMatcher::matches(const char *line)
{
    ASSERT(pattern[pattern.size() - 1].type == END);

    // shortcut: omnetpp.ini keys often begin with "*" or "**"
    // but end in a string literal. So it's usually a performance win to
    // to first check that the last string literal of the pattern matches
    // the end of the string. (We do the shortcut only in the case-sensitive
    // case. omnetpp.ini is case sensitive.)

    if (pattern.size() >= 2 && iscasesensitive) {
        Elem& e = pattern[pattern.size() - 2];
        if (e.type == LITERALSTRING) {
            // return if last 2 chars don't match
            int pattlen = e.literalstring.size();
            int linelen = strlen(line);
            if (pattlen >= 2 && linelen >= 2 && (line[linelen - 1] != e.literalstring.at(pattlen - 1) ||
                                                 line[linelen - 2] != e.literalstring.at(pattlen - 2))) //FIXME why doesn't work for pattlen==1 ?
                return false;
        }
    }

    // perform full-blown pattern matching
    return doMatch(line, 0, 0);
}

const char *PatternMatcher::patternPrefixMatches(const char *line, int suffixoffset)
{
    if (!iscasesensitive)
        throw cRuntimeError("PatternMatcher: patternPrefixMatches() doesn't support case-insensitive match");

    // pattern must end in a literal string...
    ASSERT(pattern[pattern.size() - 1].type == END);
    if (pattern.size() < 2)
        return nullptr;
    Elem& e = pattern[pattern.size() - 2];
    if (e.type != LITERALSTRING)
        return nullptr;

    // ...with the suffixlen characters at the end of 'line'
    const char *pattstring = e.literalstring.c_str();
    const char *p = strstr(pattstring, line + suffixoffset);
    if (!p)
        return nullptr;
    p += strlen(line + suffixoffset);
    rest = p;
    int pattsuffixlen = e.literalstring.size() - (p - pattstring);

    // pattern, if we cut off the 'rest', must exactly match 'line'
    return doMatch(line, 0, pattsuffixlen) ? rest.c_str() : nullptr;
}

bool PatternMatcher::containsWildcards(const char *pattern)
{
    return strchr(pattern, '?') || strchr(pattern, '*') ||
           strchr(pattern, '\\') || strchr(pattern, '{') ||
           strstr(pattern, "..");
}
};

