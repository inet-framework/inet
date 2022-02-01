//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/internetcloud/MatrixCloudDelayer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(MatrixCloudDelayer);

// FIXME modified copy of 'Matcher' class from Ipv4NetworkConfigurator
MatrixCloudDelayer::Matcher::Matcher(const char *pattern)
{
    matchesany = opp_isempty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        matchers.push_back(new inet::PatternMatcher(token, true, true, true));
        if (*token != '*') {
            // add "*.token" too
            std::string subtoken("*.");
            subtoken += token;
            matchers.push_back(new inet::PatternMatcher(subtoken.c_str(), true, true, true));
        }
    }
}

MatrixCloudDelayer::Matcher::~Matcher()
{
    for (auto& elem : matchers)
        delete elem;
}

bool MatrixCloudDelayer::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (auto& elem : matchers)
        if (elem->matches(s))
            return true;

    return false;
}

MatrixCloudDelayer::MatrixEntry::MatrixEntry(cXMLElement *trafficEntity, bool defaultSymmetric) :
    srcMatcher(trafficEntity->getAttribute("src")), destMatcher(trafficEntity->getAttribute("dest"))
{
    const char *delayAttr = trafficEntity->getAttribute("delay");
    const char *datarateAttr = trafficEntity->getAttribute("datarate");
    const char *dropAttr = trafficEntity->getAttribute("drop");
    symmetric = xmlutils::getAttributeBoolValue(trafficEntity, "symmetric", defaultSymmetric);
    try {
        delayPar.parse(delayAttr);
    }
    catch (std::exception& e) {
        throw cRuntimeError("parser error '%s' in 'delay' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation());
    }
    try {
        dataratePar.parse(datarateAttr);
    }
    catch (std::exception& e) {
        throw cRuntimeError("parser error '%s' in 'datarate' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation());
    }
    try {
        dropPar.parse(dropAttr);
    }
    catch (std::exception& e) {
        throw cRuntimeError("parser error '%s' in 'drop' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation());
    }
}

bool MatrixCloudDelayer::MatrixEntry::matches(const char *src, const char *dest)
{
    if (srcMatcher.matches(src) && destMatcher.matches(dest))
        return true;
    if (symmetric && srcMatcher.matches(dest) && destMatcher.matches(src))
        return true;
    return false;
}

MatrixCloudDelayer::~MatrixCloudDelayer()
{
    for (auto& elem : matrixEntries)
        delete elem;
    matrixEntries.clear();
}

void MatrixCloudDelayer::initialize(int stage)
{
    using namespace xmlutils;

    CloudDelayerBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift.reference(this, "interfaceTableModule", true);
        cXMLElement *configEntity = par("config");
        // parse XML config
        if (strcmp(configEntity->getTagName(), "internetCloud"))
            throw cRuntimeError("Cannot read internetCloud configuration, unaccepted '%s' entity at %s", configEntity->getTagName(),
                    configEntity->getSourceLocation());
        bool defaultSymmetric = getAttributeBoolValue(configEntity, "symmetric");
        const cXMLElement *parameterEntity = getUniqueChild(configEntity, "parameters");
        cXMLElementList trafficEntities = parameterEntity->getChildrenByTagName("traffic");
        for (auto& trafficEntitie : trafficEntities) {
            cXMLElement *trafficEntity = trafficEntitie;
            MatrixEntry *matrixEntry = new MatrixEntry(trafficEntity, defaultSymmetric);
            matrixEntries.push_back(matrixEntry);
        }
    }
}

void MatrixCloudDelayer::calculateDropAndDelay(const cMessage *msg, int srcID, int destID,
        bool& outDrop, simtime_t& outDelay)
{
    Descriptor *descriptor = getOrCreateDescriptor(srcID, destID);
    outDrop = descriptor->dropPar->boolValue(this);
    outDelay = SIMTIME_ZERO;
    if (!outDrop) {
        outDelay = descriptor->delayPar->doubleValue(this, "s");
        double datarate = descriptor->dataratePar->doubleValue(this, "bps");
        ASSERT(outDelay >= 0);
        ASSERT(datarate > 0.0);
        simtime_t curTime = simTime();
        if (curTime + outDelay < descriptor->lastSent)
            outDelay = descriptor->lastSent - curTime;

        const cPacket *pk = dynamic_cast<const cPacket *>(msg);
        if (pk)
            outDelay += pk->getBitLength() / datarate;

        descriptor->lastSent = curTime + outDelay;
    }
}

MatrixCloudDelayer::Descriptor *MatrixCloudDelayer::getOrCreateDescriptor(int srcID, int destID)
{
    IdPair idPair(srcID, destID);
    auto it = idPairToDescriptorMap.find(idPair);
    if (it != idPairToDescriptorMap.end())
        return &(it->second);

    std::string src = getPathOfConnectedNodeOnIfaceID(srcID);
    std::string dest = getPathOfConnectedNodeOnIfaceID(destID);

    // find first matching node in XML
    MatrixEntry *reverseMatrixEntry = nullptr;
    for (auto& elem : matrixEntries) {
        MatrixEntry *matrixEntry = elem;
        if (matrixEntry->matches(src.c_str(), dest.c_str())) {
            MatrixCloudDelayer::Descriptor& descriptor = idPairToDescriptorMap[idPair];
            descriptor.delayPar = &matrixEntry->delayPar;
            descriptor.dataratePar = &matrixEntry->dataratePar;
            descriptor.dropPar = &matrixEntry->dropPar;
            descriptor.lastSent = simTime();
            if (matrixEntry->symmetric) {
                if (reverseMatrixEntry) // existing previous asymmetric entry which matching to (dest,src)
                    throw cRuntimeError("Inconsistent xml config between '%s' and '%s' nodes (at %s and %s)",
                            src.c_str(), dest.c_str(), matrixEntry->entity->getSourceLocation(),
                            reverseMatrixEntry->entity->getSourceLocation());
                IdPair reverseIdPair(destID, srcID);
                MatrixCloudDelayer::Descriptor& rdescriptor = idPairToDescriptorMap[reverseIdPair];
                rdescriptor = descriptor;
            }
            return &descriptor;
        }
        else if (!matrixEntry->symmetric && !reverseMatrixEntry && matrixEntry->matches(dest.c_str(), src.c_str())) {
            // store first matched asymmetric reverse entry to reverseMatrixEntry
            reverseMatrixEntry = matrixEntry;
        }
    }
    throw cRuntimeError("The 'traffic' xml entity not found for communication from '%s' to '%s' node", src.c_str(),
            dest.c_str());
}

std::string MatrixCloudDelayer::getPathOfConnectedNodeOnIfaceID(int id)
{
    NetworkInterface *ie = ift->getInterfaceById(id);
    if (!ie)
        throw cRuntimeError("The interface id=%i not found in interfacetable", id);

    int gateId;
    cGate *connectedGate = nullptr;

    if ((gateId = ie->getNodeOutputGateId()) != -1)
        connectedGate = host->gate(gateId)->getPathEndGate();
    else if ((gateId = ie->getNodeInputGateId()) != -1)
        connectedGate = host->gate(gateId)->getPathStartGate();

    if (!connectedGate)
        throw cRuntimeError("Interface '%s' (id=%i) not connected", ie->getFullName(), id);

    cModule *connNode = findContainingNode(connectedGate->getOwnerModule());
    if (!connNode)
        throw cRuntimeError("The connected node is unknown at interface '%s' (id=%i)", ie->getFullName(), id);

    return connNode->getFullPath();
}

} // namespace inet

