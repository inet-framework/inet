//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/ResultRecorders.h"

#include "inet/common/ModuleAccess.h"

#include <sstream>

namespace inet {

Register_ResultRecorder("groupCount", GroupCountRecorder);
Register_ResultRecorder("weightedHistogram", WeightedHistogramRecorder);

void GroupCountRecorder::collect(std::string value) {
    groupcounts[value]++;
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) {
    collect(b ? "true" : "false");
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) {
    collect(v.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) {
    collect(s);
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) {
    collect(obj->getFullPath());
}

void GroupCountRecorder::finish(cResultFilter *prev) {
    opp_string_map attributes = getStatisticAttributes();

    for (auto& elem : groupcounts) {
        std::stringstream name;
        name << getResultName().c_str() << ":" << elem.first;
        getEnvir()->recordScalar(getComponent(), name.str().c_str(), elem.second, &attributes); // note: this is NaN if count==0
    }
}

Register_ResultRecorder("elapsedTime", ElapsedTimeRecorder);

ElapsedTimeRecorder::ElapsedTimeRecorder()
{
    startTime = clock();
}

void ElapsedTimeRecorder::finish(cResultFilter *prev)
{
    clock_t t = clock();
    double elapsedTime = (t - startTime) / (double)CLOCKS_PER_SEC;
    opp_string_map attributes = getStatisticAttributes();
    getEnvir()->recordScalar(getComponent(), getResultName().c_str(), elapsedTime, &attributes);
}

WeightedHistogramRecorder::WeightedHistogramRecorder()
{
}

WeightedHistogramRecorder::~WeightedHistogramRecorder()
{
    dropAndDelete(statistic);
}

void WeightedHistogramRecorder::forEachChild(cVisitor *v)
{
    v->visit(statistic);
    cNumericResultRecorder::forEachChild(v);
}

void WeightedHistogramRecorder::setStatistic(cStatistic *stat)
{
    ASSERT(statistic == nullptr);
    statistic = stat;
    take(statistic);
}

void WeightedHistogramRecorder::collect(simtime_t_cref t, double value, cObject *details)
{
    statistic->collectWeighted(value, check_and_cast<cWeight *>(details)->getWeight());
}

void WeightedHistogramRecorder::finish(cResultFilter *prev)
{
    opp_string_map attributes = getStatisticAttributes();
    getEnvir()->recordStatistic(getComponent(), getResultName().c_str(), statistic, &attributes);
}

void WeightedHistogramRecorder::init(Context *ctx)
{
    cNumericResultRecorder::init(ctx);
    setStatistic(new cHistogram("histogram", true));
}

std::string WeightedHistogramRecorder::str() const
{
    std::stringstream os;
    os << getResultName() << ": " << getStatistic()->str();
    return os.str();
}

Register_ResultRecorder("channelOwner", ChannelOwnerRecorder);

void ChannelOwnerRecorder::init(Context *ctx)
{
    VectorRecorder::init(ctx);
    std::set<cModule *> visitedModules;
    collectNetworkNodes(check_and_cast<cModule *>(ctx->component), visitedModules);
    _enum = new omnetpp::cEnum();
    _enum->registerNames("UNKNOWN,IDLE,COLLISION");
    _enum->registerValues<int>({0, 1, 2});
	int value = 3;
    for (auto networkNode : networkNodeNames)
        _enum->insert(value++, networkNode.c_str());
    omnetpp::internal::enums.getInstance()->add(_enum);
    event = new SetChannelOwnerEvent("updateChannelOwner", this);
}

void ChannelOwnerRecorder::collectNetworkNodes(cModule *module, std::set<cModule *>& visitedModules)
{
    visitedModules.insert(module);
    for (cModule::GateIterator it(module); !it.end(); ++it) {
        cGate *gate = *it;
        auto endGate = gate->getPathEndGate();
        if (endGate != nullptr) {
            auto ownerModule = endGate->getOwnerModule();
            auto networkNodeModule = findContainingNode(ownerModule);
            if (networkNodeModule != nullptr)
                networkNodeNames.insert(networkNodeModule->getFullName());
            else if (visitedModules.find(ownerModule) == visitedModules.end())
                collectNetworkNodes(ownerModule, visitedModules);
        }
    }
}

opp_string_map ChannelOwnerRecorder::getStatisticAttributes()
{
    auto result = VectorRecorder::getStatisticAttributes();
    std::string names;
    for (auto& entry :_enum->getNameValueMap())
        names += entry.first + ",";
    names.pop_back();
    result["enum"] = names.c_str();
    result["type"] = "enum";
    result["recordingmode"] = "vector";
    return result;
}

void ChannelOwnerRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref time, cObject *object, cObject *details)
{
    auto cpacket = check_and_cast<cPacket *>(object);
    auto networkNodeModule = findContainingNode(cpacket->getSenderModule());
    if (networkNodeModule == nullptr && cpacket->findPar("originalSender") != -1)
        networkNodeModule = findContainingNode(static_cast<cModule *>(cpacket->par("originalSender").pointerValue()));
    if (networkNodeModule != nullptr) {
        intval_t value = _enum->lookup(networkNodeModule->getFullName(), 0);
        if (value != lastValue)
            collect(time, value, details);
        auto fes = getSimulation()->getFES();
        if (event->isScheduled())
            fes->remove(event);
        event->setArrivalTime(time + cpacket->getRemainingDuration());
        fes->insert(event);
    }
}

} // namespace inet

