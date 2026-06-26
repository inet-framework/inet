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

std::string describe(const ProtocolTest& test)
{
    std::ostringstream os;
    os << "ProtocolTest \"" << test.name << "\":\n";
    for (size_t i = 0; i < test.steps.size(); i++) {
        const Step& step = test.steps[i];
        std::string clause;
        switch (step.type) {
            case StepType::Expect:    clause = renderPattern(step.pattern, "must"); break;
            case StepType::Optional:  clause = renderPattern(step.pattern, "may"); break;
            case StepType::ExpectNo:  clause = renderPattern(step.pattern, "must not"); break;
            case StepType::Unordered: clause = renderUnordered(step.group); break;
            case StepType::Inject:    clause = renderInject(step.injection); break;
        }
        os << "  " << (i + 1) << ". " << clause << "\n";
    }
    return os.str();
}

} // namespace protocoltest
} // namespace inet
