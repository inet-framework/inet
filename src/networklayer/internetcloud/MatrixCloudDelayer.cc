//
// Copyright (C) 2012 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#include "MatrixCloudDelayer.h"

#include "InterfaceTableAccess.h"
#include "PatternMatcher.h"
#include "XMLUtils.h"

Define_Module(MatrixCloudDelayer);

namespace {

inline bool isEmpty(const char *s)
{
    return !s || !s[0];
}

//TODO suggestion: add to XMLUtils
bool getBoolAttribute(const cXMLElement &element, const char *name, const bool *defaultValue = NULL)
{
    const char* s = element.getAttribute(name);
    if (isEmpty(s))
    {
        if (defaultValue)
            return *defaultValue;
        throw cRuntimeError("Required attribute %s of <%s> missing at %s", name, element.getTagName(),
                element.getSourceLocation());
    }
    if (strcasecmp(s, "true") == 0 || strcmp(s, "1") == 0)
        return true;
    if (strcasecmp(s, "false") == 0 || strcmp(s, "0") == 0)
        return false;
    throw cRuntimeError("Invalid boolean attribute %s = '%s' at %s", name, s, element.getSourceLocation());
}

} // namespace


//FIXME modified copy of 'Matcher' class from IPv4NetworkConfigurator
MatrixCloudDelayer::Matcher::Matcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
    {
        const char *token = tokenizer.nextToken();
        matchers.push_back(new inet::PatternMatcher(token, true, true, true));
        if (*token != '*')
        {
            // add "*.token" too
            std::string subtoken("*.");
            subtoken += token;
            matchers.push_back(new inet::PatternMatcher(subtoken.c_str(), true, true, true));
        }
    }
}

MatrixCloudDelayer::Matcher::~Matcher()
{
    for (int i = 0; i < (int) matchers.size(); i++)
        delete matchers[i];
}

bool MatrixCloudDelayer::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (int i = 0; i < (int) matchers.size(); i++)
        if (matchers[i]->matches(s))
            return true;
    return false;
}


MatrixCloudDelayer::MatrixEntry::MatrixEntry(cXMLElement *trafficEntity, bool defaultSymmetric) :
        srcMatcher(trafficEntity->getAttribute("src")), destMatcher(trafficEntity->getAttribute("dest"))
{
    const char *delayAttr = trafficEntity->getAttribute("delay");
    const char *datarateAttr = trafficEntity->getAttribute("datarate");
    const char *dropAttr = trafficEntity->getAttribute("drop");
    symmetric = getBoolAttribute(*trafficEntity, "symmetric", &defaultSymmetric);
    try {
        delayPar.parse(delayAttr);
    } catch (std::exception& e) { throw cRuntimeError("parser error '%s' in 'delay' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation()); }

    try {
        dataratePar.parse(datarateAttr);
    } catch (std::exception& e) { throw cRuntimeError("parser error '%s' in 'datarate' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation()); }

    try {
        dropPar.parse(dropAttr);
    } catch (std::exception& e) { throw cRuntimeError("parser error '%s' in 'drop' attribute of '%s' entity at %s", e.what(), trafficEntity->getTagName(), trafficEntity->getSourceLocation()); }
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
    for (MatrixEntryPtrVector::iterator i=matrixEntries.begin(); i != matrixEntries.end(); ++i)
        delete *i;
    matrixEntries.clear();
}

void MatrixCloudDelayer::initialize()
{
    host = findContainingNode(this);
    ift = InterfaceTableAccess().get(this);
    cXMLElement *configEntity = par("config").xmlValue();
    // parse XML config
    if (strcmp(configEntity->getTagName(), "internetCloud"))
        error("Cannot read internetCloud configuration, unaccepted '%s' entity at %s", configEntity->getTagName(),
                configEntity->getSourceLocation());
    bool defaultSymmetric = getBoolAttribute(*configEntity, "symmetric");
    const cXMLElement *parameterEntity = getUniqueChild(configEntity, "parameters");
    cXMLElementList trafficEntities = parameterEntity->getChildrenByTagName("traffic");
    for (int i = 0; i < (int) trafficEntities.size(); i++)
    {
        cXMLElement *trafficEntity = trafficEntities[i];
        MatrixEntry *matrixEntry = new MatrixEntry(trafficEntity, defaultSymmetric);
        matrixEntries.push_back(matrixEntry);
    }
}

void MatrixCloudDelayer::calculateDropAndDelay(const cMessage *msg, int srcID, int destID,
        bool& outDrop, simtime_t& outDelay)
{
    Descriptor *descriptor = getOrCreateDescriptor(srcID, destID);
    outDrop = descriptor->dropPar->boolValue(this);
    outDelay = SIMTIME_ZERO;
    if (!outDrop)
    {
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

MatrixCloudDelayer::Descriptor* MatrixCloudDelayer::getOrCreateDescriptor(int srcID, int destID)
{
    IDPair idPair(srcID, destID);
    IDPairToDescriptorMap::iterator it = idPairToDescriptorMap.find(idPair);
    if (it != idPairToDescriptorMap.end())
        return &(it->second);

    std::string src = getPathOfConnectedNodeOnIfaceID(srcID);
    std::string dest = getPathOfConnectedNodeOnIfaceID(destID);

    // find first matching node in XML
    MatrixEntry *reverseMatrixEntry = NULL;
    for (unsigned int i = 0; i < matrixEntries.size(); i++)
    {
        MatrixEntry *matrixEntry = matrixEntries[i];
        if (matrixEntry->matches(src.c_str(), dest.c_str()))
        {
            MatrixCloudDelayer::Descriptor& descriptor = idPairToDescriptorMap[idPair];
            descriptor.delayPar = &matrixEntry->delayPar;
            descriptor.dataratePar = &matrixEntry->dataratePar;
            descriptor.dropPar = &matrixEntry->dropPar;
            descriptor.lastSent = simTime();
            if (matrixEntry->symmetric)
            {
                if (reverseMatrixEntry) // existing previous asymmetric entry which matching to (dest,src)
                    throw cRuntimeError("Inconsistent xml config between '%s' and '%s' nodes (at %s and %s)",
                            src.c_str(), dest.c_str(), matrixEntry->entity->getSourceLocation(),
                            reverseMatrixEntry->entity->getSourceLocation());
                IDPair reverseIdPair(destID, srcID);
                MatrixCloudDelayer::Descriptor& rdescriptor = idPairToDescriptorMap[reverseIdPair];
                rdescriptor = descriptor;
            }
            return &descriptor;
        }
        else if (!matrixEntry->symmetric && !reverseMatrixEntry && matrixEntry->matches(dest.c_str(), src.c_str()))
        {
            // store first matched asymmetric reverse entry to reverseMatrixEntry
            reverseMatrixEntry = matrixEntry;
        }
    }
    throw cRuntimeError("The 'traffic' xml entity not found for communication from '%s' to '%s' node", src.c_str(),
            dest.c_str());
}

std::string MatrixCloudDelayer::getPathOfConnectedNodeOnIfaceID(int id)
{
    InterfaceEntry *ie = ift->getInterfaceById(id);
    if (!ie)
        throw cRuntimeError("The interface id=%i not found in interfacetable", id);

    int gateId;
    cGate *connectedGate = NULL;

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

