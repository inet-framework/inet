//
// Protocol Test Framework for INET -- English description generator (plan §13).
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTestDescriber.h"

#include <sstream>

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
static std::string kindPhrase(const EventPattern& p)
{
    if (!p.selHasKind)
        return "have a packet event";
    switch (p.selKind) {
        case EventKind::SentToLower: return "send a packet to the lower layer";
        case EventKind::ReceivedFromLower: return "receive a packet from the lower layer";
        case EventKind::SentToUpper: return "send a packet to the upper layer";
        case EventKind::ReceivedFromUpper: return "receive a packet from the upper layer";
        case EventKind::SentToPeer: return "send a packet to the peer";
        case EventKind::ReceivedFromPeer: return "receive a packet from the peer";
        case EventKind::Dropped: return "drop a packet";
        default: return "have a packet event";
    }
}

static std::string renderPattern(const EventPattern& p, const char *modal)
{
    std::ostringstream os;
    if (p.selHasWithin)
        os << "within " << formatTime(p.selWithin) << ", ";
    os << (p.selNode.empty() ? "some node" : p.selNode) << " " << modal << " " << kindPhrase(p);
    if (p.selHasLayer)
        os << " at the " << getLayerName(p.selLayer) << " layer";
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
