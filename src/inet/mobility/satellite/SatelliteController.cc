//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/satellite/SatelliteController.h"

#ifdef INET_WITH_SATELLITE_MOBILITY

#include <algorithm>

namespace inet {

Define_Module(SatelliteController);

simsignal_t SatelliteController::satelliteInsertedSignal = cComponent::registerSignal("satelliteInserted");
simsignal_t SatelliteController::satelliteRemovedSignal = cComponent::registerSignal("satelliteRemoved");

SatelliteController::~SatelliteController()
{
    cancelAndDelete(timer);
}

void SatelliteController::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        epoch = par("epoch").stringValue();
        nodeTypeName = par("nodeType").stringValue();
        nodeName = par("nodeName").stringValue();
        coordinateSystemModule = par("coordinateSystemModule").stringValue();
        nodeIcon = par("nodeIcon").stringValue();
        nodeIconRowX = par("nodeIconRowX");
        nodeIconRowY = par("nodeIconRowY");
        nodeIconSpacing = par("nodeIconSpacing");
        timer = new cMessage("satelliteEvent");
        loadTleFiles();
        scheduleEvents();
    }
    else if (stage == INITSTAGE_LAST)
        scheduleNextEvent();
}

void SatelliteController::loadTleFiles()
{
    cStringTokenizer tokenizer(par("tleFiles").stringValue(), " \t\n;,");
    while (tokenizer.hasMoreTokens()) {
        const char *fileName = tokenizer.nextToken();
        TleFile file;
        file.load(fileName);
        for (int i = 0; i < file.getNumSatellites(); i++) {
            SatelliteEntry entry;
            entry.tleFile = fileName;
            entry.index = i;
            entry.name = file.getRecord(i).name;
            entries.push_back(entry);
        }
    }
    if (entries.empty())
        throw cRuntimeError("No satellites found in the configured TLE files");
    EV_INFO << "SatelliteController loaded " << entries.size() << " satellites" << endl;
}

void SatelliteController::scheduleEvents()
{
    simtime_t insertionTime = par("insertionTime");
    // default: insert every satellite at insertionTime, never remove
    for (size_t i = 0; i < entries.size(); i++)
        events.push_back({insertionTime, true, (int)i});

    // optional per-satellite schedule overrides the defaults
    cXMLElement *schedule = par("schedule").xmlValue();
    if (schedule != nullptr) {
        for (cXMLElement *child : schedule->getChildrenByTagName("satellite")) {
            const char *name = child->getAttribute("name");
            const char *indexAttr = child->getAttribute("index");
            int entryIndex = -1;
            if (indexAttr != nullptr)
                entryIndex = atoi(indexAttr);
            else if (name != nullptr) {
                for (size_t i = 0; i < entries.size(); i++)
                    if (entries[i].name == name) { entryIndex = (int)i; break; }
            }
            if (entryIndex < 0 || entryIndex >= (int)entries.size())
                throw cRuntimeError("Schedule entry refers to unknown satellite (name='%s', index='%s')",
                        name ? name : "", indexAttr ? indexAttr : "");
            const char *insertAttr = child->getAttribute("insertTime");
            const char *removeAttr = child->getAttribute("removeTime");
            if (insertAttr != nullptr) {
                // replace the default insertion event for this entry
                for (auto& e : events)
                    if (e.entry == entryIndex && e.insert)
                        e.time = SimTime::parse(insertAttr);
            }
            if (removeAttr != nullptr)
                events.push_back({SimTime::parse(removeAttr), false, entryIndex});
        }
    }

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.time < b.time;
    });
}

void SatelliteController::scheduleNextEvent()
{
    if (nextEvent < events.size()) {
        cancelEvent(timer);
        scheduleAt(events[nextEvent].time, timer);
    }
}

void SatelliteController::handleMessage(cMessage *msg)
{
    ASSERT(msg == timer);
    simtime_t now = simTime();
    while (nextEvent < events.size() && events[nextEvent].time == now) {
        const Event& event = events[nextEvent];
        if (event.insert)
            insertSatellite(event.entry);
        else
            removeSatellite(event.entry);
        nextEvent++;
    }
    scheduleNextEvent();
}

void SatelliteController::insertSatellite(int entryIndex)
{
    SatelliteEntry& entry = entries[entryIndex];
    if (entry.module != nullptr)
        return; // already inserted
    cModuleType *moduleType = cModuleType::get(nodeTypeName.c_str());
    cModule *parentModule = getParentModule();

    // grow the submodule vector and create the new node (ScenarioManager pattern)
    int vectorSize = 0;
    for (cModule::SubmoduleIterator it(parentModule); !it.end(); ++it) {
        cModule *submodule = *it;
        if (submodule->isVector() && submodule->isName(nodeName.c_str()))
            if (vectorSize < submodule->getVectorSize())
                vectorSize = submodule->getVectorSize();
    }
    if (!parentModule->hasSubmoduleVector(nodeName.c_str()))
        parentModule->addSubmoduleVector(nodeName.c_str(), vectorSize + 1);
    else
        parentModule->setSubmoduleVectorSize(nodeName.c_str(), vectorSize + 1);
    cModule *module = moduleType->create(nodeName.c_str(), parentModule, vectorSize);
    module->finalizeParameters();
    module->buildInside();

    // inject the TLE selection and epoch into the satellite's mobility submodule
    cModule *mobility = module->getSubmodule("mobility");
    if (mobility == nullptr || !mobility->hasPar("tleFile") || !mobility->hasPar("satelliteIndex"))
        throw cRuntimeError("Node type '%s' must have a 'mobility' submodule of type SatelliteMobility "
                            "(set **.%s[*].mobility.typename = \"inet.mobility.satellite.SatelliteMobility\")",
                nodeTypeName.c_str(), nodeName.c_str());
    mobility->par("tleFile").setStringValue(entry.tleFile.c_str());
    mobility->par("satelliteIndex").setIntValue(entry.index);
    mobility->par("epoch").setStringValue(epoch.c_str());
    if (!coordinateSystemModule.empty() && mobility->hasPar("coordinateSystemModule"))
        mobility->par("coordinateSystemModule").setStringValue(coordinateSystemModule.c_str());

    // lay out the created node icons in a left-to-right row (e.g. below the map); set the
    // display-string position before initialization so the icon is placed correctly from the
    // start. SatelliteMobility has updateDisplayString=false, so this position is not overwritten.
    cDisplayString& displayString = module->getDisplayString();
    displayString.setTagArg("p", 0, (long)(nodeIconRowX + vectorSize * nodeIconSpacing));
    displayString.setTagArg("p", 1, (long)nodeIconRowY);
    if (!nodeIcon.empty())
        displayString.setTagArg("i", 0, nodeIcon.c_str());
    // show the TLE satellite name as text above the icon (the module name stays below it)
    if (!entry.name.empty()) {
        displayString.setTagArg("t", 0, entry.name.c_str());
        displayString.setTagArg("t", 1, "t"); // text position: top
    }

    // module add notifications are emitted automatically during creation;
    // initialize the new node (this triggers radio medium registration etc.)
    module->callInitialize();

    entry.module = module;
    EV_INFO << "Inserted satellite '" << entry.name << "' as " << module->getFullPath() << endl;
    emit(satelliteInsertedSignal, module);
}

void SatelliteController::removeSatellite(int entryIndex)
{
    SatelliteEntry& entry = entries[entryIndex];
    if (entry.module == nullptr)
        return; // not currently inserted
    cModule *module = entry.module;
    EV_INFO << "Removing satellite '" << entry.name << "' (" << module->getFullPath() << ")" << endl;
    emit(satelliteRemovedSignal, module);
    entry.module = nullptr;
    module->callFinish();
    module->deleteModule();
}

std::vector<cModule *> SatelliteController::getManagedSatellites() const
{
    std::vector<cModule *> result;
    for (const auto& entry : entries)
        result.push_back(entry.module);
    return result;
}

void SatelliteController::finish()
{
}

} // namespace inet

#endif // INET_WITH_SATELLITE_MOBILITY

