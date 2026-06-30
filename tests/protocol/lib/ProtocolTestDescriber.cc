//
// Protocol Test Framework for INET -- English description generator (plan §13).
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTestDescriber.h"

#include <cctype>
#include <cstring>
#include <sstream>
#include <vector>

#include "inet/common/Protocol.h"

#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

static std::string formatTime(simtime_t t)
{
    std::ostringstream os;
    os << t << "s";
    return os.str();
}

static std::string capitalize(std::string s)
{
    if (!s.empty())
        s[0] = std::toupper(s[0]);
    return s;
}

// "<verb> a packet <locus>" for the observed event kind.
static std::string kindPhrase(const EventPattern& /*p*/)
{
    return "have a packet event";
}

// "BindingUpdate" -> "Binding Update"; "homeInitCookie" -> "home init cookie" (lowercased).
static std::string spaceCamel(const std::string& s, bool lower)
{
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (i > 0 && std::isupper((unsigned char)c) && !std::isupper((unsigned char)s[i - 1]))
            out += ' ';
        out += lower ? (char)std::tolower((unsigned char)c) : c;
    }
    return out;
}

static std::string protocolHumanName(const std::string& name)
{
    if (auto protocol = Protocol::findProtocol(name.c_str()))
        return protocol->getDescriptiveName();
    return name;
}

static std::string firstComponent(const std::string& path)
{
    auto dot = path.find('.');
    return dot == std::string::npos ? path : path.substr(0, dot);
}

static std::string lastComponent(const std::string& path)
{
    auto dot = path.rfind('.');
    return dot == std::string::npos ? path : path.substr(dot + 1);
}

static std::string trimSpace(const std::string& s)
{
    auto a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    auto b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}

// Translate a PacketFilter expression into a noun phrase, e.g.
// "a Binding Update (home registration flag set, ack flag set)". Returns false (caller falls
// back to the raw expression) for anything it can't cleanly parse.
static bool translateExpr(const std::string& expr, std::string& out)
{
    if (expr.find("||") != std::string::npos)
        return false;
    std::vector<std::string> clauses;
    size_t start = 0;
    while (true) {
        size_t amp = expr.find("&&", start);
        clauses.push_back(expr.substr(start, amp == std::string::npos ? std::string::npos : amp - start));
        if (amp == std::string::npos) break;
        start = amp + 2;
    }
    std::string chunkNoun;
    std::vector<std::string> phrases;
    static const char *ops[] = {"==", "!=", ">=", "<=", ">", "<"};
    for (const auto& rawClause : clauses) {
        std::string clause = trimSpace(rawClause);
        size_t opPos = std::string::npos, opLen = 0;
        std::string op;
        for (auto o : ops) {
            size_t pos = clause.find(o);
            if (pos != std::string::npos && (opPos == std::string::npos || pos < opPos)) { opPos = pos; op = o; opLen = std::strlen(o); }
        }
        if (opPos == std::string::npos) return false;
        std::string lhs = trimSpace(clause.substr(0, opPos));
        std::string rhs = trimSpace(clause.substr(opPos + opLen));
        size_t dot = lhs.find('.');
        if (dot == std::string::npos) return false;
        std::string chunk = lhs.substr(0, dot);
        std::string field = lhs.substr(dot + 1);
        if (field.find('.') != std::string::npos) return false;
        if (chunk.empty() || !std::isupper((unsigned char)chunk[0])) return false; // need a CamelCase class noun
        if (chunkNoun.empty()) chunkNoun = spaceCamel(chunk, false);
        std::string f = spaceCamel(field, true);
        std::string phrase;
        if (op == "==" && rhs == "true") phrase = f + " set";
        else if (op == "==" && rhs == "false") phrase = f + " clear";
        else if (op == ">=" && rhs == "0") phrase = ""; // presence-only check
        else if (!rhs.empty() && rhs[0] == '{') {
            size_t close = rhs.find('}');
            if (close == std::string::npos) return false;
            std::string cap = rhs.substr(1, close - 1);
            if (rhs != "{" + cap + "}" || op != "==") return false; // arithmetic etc. -> fall back
            phrase = f + " equal to the remembered " + cap;
        }
        else if (op == "==") phrase = f + " " + rhs;
        else phrase = f + " " + op + " " + rhs;
        if (!phrase.empty()) phrases.push_back(phrase);
    }
    if (chunkNoun.empty()) return false;
    out = "a " + chunkNoun;
    if (!phrases.empty()) {
        out += " (";
        for (size_t i = 0; i < phrases.size(); i++) out += (i ? ", " : "") + phrases[i];
        out += ")";
    }
    return true;
}

// The message/value phrase: an explicit description wins, else the translated expression,
// else the raw expression / predicate.
static std::string contentNoun(const EventPattern& p)
{
    if (!p.description.empty()) return p.description;
    std::string noun;
    if (!p.selExpr.empty() && translateExpr(p.selExpr, noun)) return noun;
    if (!p.selExpr.empty()) return "a packet matching \"" + p.selExpr + "\"";
    if (p.predicate) return "a packet matching a custom predicate";
    return "a packet";
}

// New-vocabulary rendering: subject (the attributed counterpart module, named via its
// protocol; else the honest source) + verb (from the signal, flipped under attributeTo) + the
// translated message phrase.
static std::string renderNewPattern(const EventPattern& p, const char *modal)
{
    std::ostringstream os;
    if (p.selHasWithin)
        os << "within " << formatTime(p.selWithin) << ", ";

    bool isDropped = p.selSignal.find("Dropped") != std::string::npos;
    bool isSent = p.selSignal.find("Sent") != std::string::npos;

    if (p.selHasValue) {
        // Scalar / state-signal assertion: "<module>'s <signal> must reach <value>".
        std::string subject = !p.selSource.empty() ? p.selSource
                            : (p.selNode.empty() ? "some module" : p.selNode);
        os << subject << "'s " << p.selSignal << " " << modal << " reach ";
        if (!p.description.empty())
            os << p.description << " (value " << p.selValue << ")";
        else
            os << "value " << p.selValue;
    }
    else if (!p.attributeToPath.empty()) {
        // Narrate from the attributed (up/down counterpart) module: it does the opposite of
        // the emitting layer (a packet the layer "received from upper" was *sent* by the module
        // above it).
        std::string node = firstComponent(p.attributeToPath);
        std::string name = !p.selProtocol.empty() ? protocolHumanName(p.selProtocol) : lastComponent(p.attributeToPath);
        const char *verb = isDropped ? "drop" : (isSent ? "receive" : "send");
        os << node << "'s " << name << " " << modal << " " << verb << " " << contentNoun(p);
    }
    else {
        // Honest source point of view.
        std::string subject = !p.selSource.empty() ? p.selSource
                            : (p.selNode.empty() ? "some module" : p.selNode);
        os << subject << " " << modal << " ";
        if (isDropped)
            os << "drop " << contentNoun(p);
        else {
            bool isUpper = p.selSignal.find("Upper") != std::string::npos;
            os << (isSent ? "send " : "receive ") << contentNoun(p)
               << (isSent ? " to the " : " from the ") << (isUpper ? "upper" : "lower") << " layer";
        }
    }

    if (!p.selIface.empty())
        os << " on interface " << p.selIface;
    if (!p.captures.empty()) {
        os << ", remembering ";
        for (size_t i = 0; i < p.captures.size(); i++)
            os << (i ? ", " : "") << p.captures[i].first;
    }
    if (p.selHasNotBefore)
        os << " (but not before " << formatTime(p.selNotBefore) << " after the previous step)";
    return capitalize(os.str()) + ".";
}

static std::string renderPattern(const EventPattern& p, const char *modal)
{
    if (!p.selSignal.empty())   // new orthogonal vocabulary (signal()-based)
        return renderNewPattern(p, modal);

    std::ostringstream os;
    if (p.selHasWithin)
        os << "within " << formatTime(p.selWithin) << ", ";
    os << (p.selNode.empty() ? "some node" : p.selNode) << " " << modal << " " << kindPhrase(p);
    if (!p.selIface.empty())
        os << " on interface " << p.selIface;

    if (!p.description.empty())
        os << " -- " << p.description;
    else if (!p.selExpr.empty())
        os << " matching \"" << p.selExpr << "\"";
    else if (p.predicate)
        os << " matching a custom predicate";

    if (!p.captures.empty()) {
        os << ", remembering ";
        for (size_t i = 0; i < p.captures.size(); i++)
            os << (i ? ", " : "") << p.captures[i].first;
    }
    if (p.selHasNotBefore)
        os << " (but not before " << formatTime(p.selNotBefore) << " after the previous step)";

    return capitalize(os.str()) + ".";
}

static std::string renderUnordered(const std::vector<EventPattern>& group)
{
    std::ostringstream os;
    os << "In any order: ";
    for (size_t i = 0; i < group.size(); i++) {
        std::string clause = renderPattern(group[i], "must");
        if (!clause.empty() && clause.back() == '.')
            clause.pop_back();
        os << (i ? "; " : "") << clause;
    }
    return os.str() + ".";
}

static std::string stripPeriod(std::string s)
{
    if (!s.empty() && s.back() == '.')
        s.pop_back();
    return s;
}

static std::string renderAnyOf(const std::vector<EventPattern>& group)
{
    std::ostringstream os;
    os << "One of: ";
    for (size_t i = 0; i < group.size(); i++)
        os << (i ? "; or " : "") << stripPeriod(renderPattern(group[i], "must"));
    return os.str() + ".";
}

static std::string renderExactlyTimes(const EventPattern& p, int count)
{
    return std::to_string(count) + " times: " + stripPeriod(renderPattern(p, "must")) + ".";
}

// Greedy count range [lo, hi] (hi < 0 = unbounded), as produced by the *Times builders.
static std::string renderCount(const EventPattern& p, int lo, int hi)
{
    std::string times;
    if (hi < 0 && lo == 0)      times = "any number of times";
    else if (hi < 0 && lo == 1) times = "one or more times";
    else if (hi < 0)            times = "at least " + std::to_string(lo) + " times";
    else if (lo == 0)           times = "at most " + std::to_string(hi) + " times";
    else if (lo == hi)          times = std::to_string(lo) + " times";
    else                        times = "between " + std::to_string(lo) + " and " + std::to_string(hi) + " times";
    return capitalize(times) + ": " + stripPeriod(renderPattern(p, "must")) + ".";
}

static std::string contentPhrase(const EventPattern& p)
{
    if (!p.description.empty()) return p.description;
    if (!p.selExpr.empty()) return "a packet matching \"" + p.selExpr + "\"";
    return "a packet";
}

static std::string renderDelivery(const EventPattern& from, const EventPattern& to)
{
    std::ostringstream os;
    os << "Within " << (to.selHasWithin ? formatTime(to.selWithin) : std::string("the window")) << ", "
       << contentPhrase(from) << " sent by " << (from.selNode.empty() ? "some node" : from.selNode)
       << " is received by " << (to.selNode.empty() ? "some node" : to.selNode) << " (same packet).";
    return os.str();
}

static std::string renderInject(const Injection& inj)
{
    std::ostringstream os;
    if (inj.hasAtTime)
        os << "at " << formatTime(inj.atTime) << ", ";
    else if (inj.hasAfter)
        os << formatTime(inj.afterDelay) << " after the previous step, ";
    else
        os << "then, ";
    os << "inject " << (inj.description.empty() ? "a crafted packet" : inj.description)
       << " into " << inj.nodeName << "'s " << inj.moduleSubPath << " (" << inj.gateName << ")";
    return capitalize(os.str()) + ".";
}

static std::string renderInterception(const Interception& i)
{
    std::ostringstream os;
    os << "on tap '" << i.tapName << "', ";
    if (i.action == "delay")       os << "delay (by " << formatTime(i.delayTime) << ") ";
    else if (i.action == "mutate") os << "mutate ";
    else                           os << "drop ";
    if (!i.description.empty())          os << i.description;
    else if (!i.matchExpression.empty()) os << "frames matching \"" << i.matchExpression << "\"";
    else                                 os << "every frame";
    if (i.minimumBytes > 0)
        os << " (≥ " << i.minimumBytes << " bytes)";
    if (i.occurrence > 0)
        os << ", occurrence " << i.occurrence;
    return capitalize(os.str()) + ".";
}

std::string describe(const ProtocolTest& test)
{
    std::ostringstream os;
    os << "ProtocolTest \"" << test.name << "\":\n";
    for (const auto& interception : test.interceptions)
        os << "  * Fault injection: " << renderInterception(interception) << "\n";
    for (size_t i = 0; i < test.steps.size(); i++) {
        const Step& step = test.steps[i];
        std::string clause;
        switch (step.type) {
            case StepType::Once:         clause = renderPattern(step.pattern, "must"); break;
            case StepType::AtMostOnce:   clause = renderPattern(step.pattern, "may"); break;
            case StepType::Never:        clause = renderPattern(step.pattern, "must not"); break;
            case StepType::Unordered:    clause = renderUnordered(step.group); break;
            case StepType::AnyOf:        clause = renderAnyOf(step.group); break;
            case StepType::ExactlyTimes: clause = renderExactlyTimes(step.pattern, step.count); break;
            case StepType::Count:        clause = renderCount(step.pattern, step.cardMin, step.cardMax); break;
            case StepType::Delivery:  clause = renderDelivery(step.pattern, step.pattern2); break;
            case StepType::Inject:    clause = renderInject(step.injection); break;
        }
        os << "  " << (i + 1) << ". " << clause << "\n";
    }
    return os.str();
}

} // namespace protocoltest
} // namespace inet
