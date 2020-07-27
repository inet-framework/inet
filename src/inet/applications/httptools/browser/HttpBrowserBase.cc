//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "inet/applications/httptools/browser/HttpBrowserBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

namespace httptools {

HttpBrowserBase::HttpBrowserBase()
{
    m_bDisplayMessage = true;
    m_bDisplayResponseContent = true;
}

HttpBrowserBase::~HttpBrowserBase()
{
    delete rdProcessingDelay;
    delete rdActivityLength;
    delete rdInterRequestInterval;
    delete rdInterSessionInterval;
    delete rdRequestSize;
    delete rdReqInSession;

    cancelAndDelete(eventTimer);
}

void HttpBrowserBase::initialize(int stage)
{
    EV_DEBUG << "Initializing base HTTP browser component -- stage " << stage << endl;

    HttpNodeBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        cXMLElement *rootelement = par("config");
        if (rootelement == nullptr)
            throw cRuntimeError("Configuration file is not defined");

        cXMLAttributeMap attributes;
        rdObjectFactory rdFactory;

        // Activity period length -- the waking period
        cXMLElement *element = rootelement->getFirstChildWithTag("activityPeriod");
        if (element == nullptr) {
            rdActivityLength = nullptr;    // Disabled if this parameter is not defined in the file
        }
        else {
            attributes = element->getAttributes();
            rdActivityLength = rdFactory.create(attributes);
            if (rdActivityLength == nullptr)
                throw cRuntimeError("Activity period random object could not be created");
        }

        // Inter-session interval
        element = rootelement->getFirstChildWithTag("interSessionInterval");
        if (element == nullptr)
            throw cRuntimeError("Inter-request interval parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdInterSessionInterval = rdFactory.create(attributes);
        if (rdInterSessionInterval == nullptr)
            throw cRuntimeError("Inter-session interval random object could not be created");

        // Inter-request interval
        element = rootelement->getFirstChildWithTag("InterRequestInterval");
        if (element == nullptr)
            throw cRuntimeError("Inter-request interval parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdInterRequestInterval = rdFactory.create(attributes);
        if (rdInterRequestInterval == nullptr)
            throw cRuntimeError("Inter-request interval random object could not be created");

        // Request size
        element = rootelement->getFirstChildWithTag("requestSize");
        if (element == nullptr)
            throw cRuntimeError("Inter-request interval parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdRequestSize = rdFactory.create(attributes);
        if (rdRequestSize == nullptr)
            throw cRuntimeError("Request size random object could not be created");

        // Requests in session
        element = rootelement->getFirstChildWithTag("reqInSession");
        if (element == nullptr)
            throw cRuntimeError("requests in session parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdReqInSession = rdFactory.create(attributes);
        if (rdReqInSession == nullptr)
            throw cRuntimeError("Requests in session random object could not be created");

        // Processing delay
        element = rootelement->getFirstChildWithTag("processingDelay");
        if (element == nullptr)
            throw cRuntimeError("processing delay parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdProcessingDelay = rdFactory.create(attributes);
        if (rdProcessingDelay == nullptr)
            throw cRuntimeError("Processing delay random object could not be created");

        controller = getModuleFromPar<HttpController>(par("httpControllerModule"), this);

        httpProtocol = par("httpProtocol");

        logFileName = par("logFile").stdstringValue();
        enableLogging = logFileName != "";
        outputFormat = lf_short;

        // Initialize counters
        htmlRequested = 0;
        htmlReceived = 0;
        htmlErrorsReceived = 0;
        imgResourcesRequested = 0;
        imgResourcesReceived = 0;
        textResourcesRequested = 0;
        textResourcesReceived = 0;
        messagesInCurrentSession = 0;
        connectionsCount = 0;
        sessionCount = 0;

        reqInCurSession = 0;
        reqNoInCurSession = 0;

        eventTimer = new cMessage("eventTimer");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        EV_DEBUG << "Initializing base HTTP browser component -- phase 1\n";

        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        std::string scriptFile = par("scriptFile").stdstringValue();
        scriptedMode = !scriptFile.empty();
        if (scriptedMode) {
            EV_DEBUG << "Scripted mode. Script file: " << scriptFile << endl;
            readScriptedEvents(scriptFile.c_str());
        }
        else {
            double activationTime = par("activationTime");    // This is the activation delay. Optional
            if (rdActivityLength != nullptr)
                activationTime += std::max(0.0, 86400.0 - rdActivityLength->draw()) / 2.0; // First activate after half the sleep period
            EV_INFO << "Initial activation time is " << activationTime << endl;
            eventTimer->setKind(MSGKIND_ACTIVITY_START);
            scheduleAfter(activationTime, eventTimer);
        }
    }
}

void HttpBrowserBase::finish()
{
    EV_INFO << "Sessions: " << sessionCount << endl;
    EV_INFO << "HTML requested " << htmlRequested << "\n";
    EV_INFO << "HTML received " << htmlReceived << "\n";
    EV_INFO << "HTML errors received " << htmlErrorsReceived << "\n";
    EV_INFO << "Image resources requested " << imgResourcesRequested << "\n";
    EV_INFO << "Image resources received " << imgResourcesReceived << "\n";
    EV_INFO << "Text resources requested " << textResourcesRequested << "\n";
    EV_INFO << "Text resources received " << textResourcesReceived << "\n";

    recordScalar("session.count", sessionCount);
    recordScalar("html.requested", htmlRequested);
    recordScalar("html.received", htmlReceived);
    recordScalar("html.errors", htmlErrorsReceived);
    recordScalar("html.image.requested", imgResourcesRequested);
    recordScalar("html.image.received", imgResourcesReceived);
    recordScalar("html.text.requested", textResourcesRequested);
    recordScalar("html.text.received", textResourcesReceived);
}

void HttpBrowserBase::handleSelfMessages(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_ACTIVITY_START:
            handleSelfActivityStart();
            break;

        case MSGKIND_START_SESSION:
            handleSelfStartSession();
            break;

        case MSGKIND_NEXT_MESSAGE:
            handleSelfNextMessage();
            break;

        case MSGKIND_SCRIPT_EVENT:
            handleSelfScriptedEvent();
            break;

        case HTTPT_DELAYED_REQUEST_MESSAGE:
            handleSelfDelayedRequestMessage(msg);
            break;
    }
}

void HttpBrowserBase::handleSelfActivityStart()
{
    EV_DEBUG << "Starting new activity period @ T=" << simTime() << endl;
    eventTimer->setKind(MSGKIND_START_SESSION);
    messagesInCurrentSession = 0;
    reqNoInCurSession = 0;
    double activityPeriodLength = (rdActivityLength != nullptr) ? rdActivityLength->draw() : 0.0;    // Get the length of the activity period
    acitivityPeriodEnd = simTime() + activityPeriodLength;    // The end of the activity period
    EV_INFO << "Activity period starts @ T=" << simTime() << ". Activity period is " << (activityPeriodLength / 3600.0) << " hours." << endl;
    scheduleAfter(simtime_t(rdInterSessionInterval->draw()) / 2, eventTimer);
}

void HttpBrowserBase::handleSelfStartSession()
{
    EV_DEBUG << "Starting new session @ T=" << simTime() << endl;
    sessionCount++;
    messagesInCurrentSession = 0;
    reqInCurSession = 0;
    reqNoInCurSession = (int)rdReqInSession->draw();
    EV_INFO << "Starting session # " << sessionCount << " @ T=" << simTime() << ". Requests in session are " << reqNoInCurSession << "\n";
    sendRequestToRandomServer();
    scheduleNextBrowseEvent();
}

void HttpBrowserBase::handleSelfNextMessage()
{
    EV_DEBUG << "New browse event triggered @ T=" << simTime() << endl;
    EV_INFO << "Next message in session # " << sessionCount << " @ T=" << simTime() << ". "
            << "Current request is " << reqInCurSession << "/" << reqNoInCurSession << "\n";
    sendRequestToRandomServer();
    scheduleNextBrowseEvent();
}

void HttpBrowserBase::handleSelfScriptedEvent()
{
    EV_DEBUG << "Scripted browse event @ T=" << simTime() << "\n";
    sessionCount++;
    messagesInCurrentSession = 0;
    // Get the browse event
    if (browseEvents.empty())
        throw cRuntimeError("No event entry in queue");
    BrowseEvent be = browseEvents.back();
    browseEvents.pop_back();
    sendRequestToServer(be);
    // Schedule the next event
    if (!browseEvents.empty()) {
        be = browseEvents.back();
        EV_DEBUG << "Scheduling next event @ " << be.time << "\n";
        eventTimer->setKind(MSGKIND_SCRIPT_EVENT);
        scheduleAt(be.time, eventTimer);
    }
    else {
        EV_DEBUG << "No more browsing events\n";
    }
}

void HttpBrowserBase::handleSelfDelayedRequestMessage(cMessage *msg)
{
    EV_DEBUG << "Sending delayed message " << msg->getName() << " @ T=" << simTime() << endl;
    Packet *reqmsg = check_and_cast<Packet *>(msg);
    //HttpRequestMessage *reqmsg = check_and_cast<HttpRequestMessage *>(msg);
    reqmsg->setKind(HTTPT_REQUEST_MESSAGE);
    sendRequestToServer(reqmsg);
}

void HttpBrowserBase::handleDataMessage(Ptr<const HttpReplyMessage> appmsg)
{
    logResponse(appmsg);

    messagesInCurrentSession++;

    int serial = appmsg->getSerial();
    const char *pkName = "REPLY";  //TODO it's was the pk->getName()

    std::string senderWWW = appmsg->getOriginatorUrl();
    EV_DEBUG << "Handling received message from " << senderWWW << ": " << pkName << ". Received @T=" << simTime() << endl;

    if (appmsg->getResult() != 200 || appmsg->getContentType() == CT_UNKNOWN) {
        EV_INFO << "Result for " << pkName << " was other than OK. Code: " << appmsg->getResult() << endl;
        htmlErrorsReceived++;
        return;
    }
    else {
        switch (appmsg->getContentType()) {
            case CT_HTML:
                EV_INFO << "HTML Document received: " << pkName << "'. Size is " << appmsg->getChunkLength() << " and serial " << serial << endl;
                if (strlen(appmsg->getPayload()) != 0)
                    EV_DEBUG << "Payload of " << pkName << " is: " << endl << appmsg->getPayload()
                             << ", " << strlen(appmsg->getPayload()) << " bytes" << endl;
                else
                    EV_DEBUG << pkName << " has no referenced resources. No GETs will be issued in parsing" << endl;
                htmlReceived++;
                if (hasGUI())
                    bubble("Received a HTML document");
                break;

            case CT_TEXT:
                EV_INFO << "Text resource received: " << pkName << "'. Size is " << appmsg->getChunkLength() << " and serial " << serial << endl;
                textResourcesReceived++;
                if (hasGUI())
                    bubble("Received a text resource");
                break;

            case CT_IMAGE:
                EV_INFO << "Image resource received: " << pkName << "'. Size is " << appmsg->getChunkLength() << " and serial " << serial << endl;
                imgResourcesReceived++;
                if (hasGUI())
                    bubble("Received an image resource");
                break;

            case CT_UNKNOWN:
                EV_DEBUG << "UNKNOWN RESOURCE TYPE RECEIVED: " << appmsg->getContentType() << endl;
                if (hasGUI())
                    bubble("Received an unknown resource type");
                break;
        }
        // Parse the html page body
        if (appmsg->getContentType() == CT_HTML && strlen(appmsg->getPayload()) != 0) {
            EV_DEBUG << "Processing HTML document body:\n";
            cStringTokenizer lineTokenizer(appmsg->getPayload(), "\n");
            std::vector<std::string> lines = lineTokenizer.asVector();
            std::map<std::string, HttpRequestQueue> requestQueues;
            for (auto resourceLine : lines) {
                cStringTokenizer fieldTokenizer(resourceLine.c_str(), ";");
                std::vector<std::string> fields = fieldTokenizer.asVector();
                if (fields.size() < 1) {
                    EV_ERROR << "Invalid resource reference in received message: " << resourceLine << endl;
                    continue;
                }
                // Get the resource name -- this is mandatory for all references
                std::string resourceName = fields[0];

                std::string providerName = senderWWW;
                if (fields.size() > 1)
                    providerName = fields[1];

                double delay = 0.0;
                if (fields.size() > 2)
                    delay = safeatof(fields[2].c_str());

                bool bad = false;
                if (fields.size() > 3)
                    bad = safeatobool(fields[3].c_str());

                int refSize = 0;
                if (fields.size() > 4)
                    refSize = safeatoi(fields[4].c_str());

                EV_DEBUG << "Generating resource request: " << resourceName << ". Provider: " << providerName
                         << ", delay: " << delay << ", bad: " << bad << ", ref.size: " << refSize << endl;

                // Generate a request message and push on queue for the intended recipient
                Packet *reqmsg = generateResourceRequest(providerName, resourceName, serial++, bad, refSize);    // TODO: KVJ: CHECK HERE FOR XSITE
                if (delay == 0.0) {
                    requestQueues[providerName].push_front(reqmsg);
                }
                else {
                    reqmsg->setKind(HTTPT_DELAYED_REQUEST_MESSAGE);
                    scheduleAfter(delay, reqmsg);    // Schedule the message as a self message
                }
            }
            // Iterate through the list of queues (one for each recipient encountered) and submit each queue.
            // A single socket will thus be opened for each recipient for a rough HTTP/1.1 emulation.
            // This is only done for messages which are not delayed in the simulated page.
            auto i = requestQueues.begin();
            for ( ; i != requestQueues.end(); i++)
                sendRequestsToServer((*i).first, (*i).second);
        }
    }
}

Packet *HttpBrowserBase::generatePageRequest(std::string www, std::string pageName, bool bad, int size)
{
    EV_DEBUG << "Generating page request for URL " << www << ", page " << pageName << endl;

    if (www.size() + pageName.size() > MAX_URL_LENGTH) {
        EV_ERROR << "URL for site " << www << " exceeds allowed maximum size" << endl;
        return nullptr;
    }

    long requestLength = (long)rdRequestSize->draw();

    if (pageName.empty())
        pageName = "/";
    else if (pageName[0] != '/')
        pageName.insert(0, "/");

    char szReq[MAX_URL_LENGTH + 24];
    sprintf(szReq, "GET %s HTTP/1.1", pageName.c_str());
    Packet *outPk = new Packet(szReq, HTTPT_REQUEST_MESSAGE);
    const auto& msg = makeShared<HttpRequestMessage>();
    msg->setTargetUrl(www.c_str());
    msg->setProtocol(httpProtocol);
    msg->setHeading(szReq);
    msg->setSerial(0);
    msg->setChunkLength(B(requestLength + size));    // Add extra request size if specified
    msg->setKeepAlive(httpProtocol == 11);
    msg->setBadRequest(bad);    // Simulates willingly requesting a non-existing resource.
    outPk->insertAtBack(msg);

    logRequest(msg);
    htmlRequested++;

    return outPk;
}

Packet *HttpBrowserBase::generateRandomPageRequest(std::string www, bool bad, int size)
{
    EV_DEBUG << "Generating random page request, URL: " << www << endl;
    return generatePageRequest(www, "random_page.html", bad, size);
}

Packet *HttpBrowserBase::generateResourceRequest(std::string www, std::string resource, int serial, bool bad, int size)
{
    EV_DEBUG << "Generating resource request for URL " << www << ", resource: " << resource << endl;

    if (www.size() + resource.size() > MAX_URL_LENGTH) {
        EV_ERROR << "URL for site " << www << " exceeds allowed maximum size" << endl;
        return nullptr;
    }

    long requestLength = (long)rdRequestSize->draw() + size;

    if (resource.empty()) {
        EV_ERROR << "Unable to request resource -- empty resource string" << endl;
        return nullptr;
    }
    else if (resource[0] != '/')
        resource.insert(0, "/");

    std::string ext = trimLeft(resource, ".");
    HttpContentType rc = getResourceCategory(ext);
    if (rc == CT_IMAGE)
        imgResourcesRequested++;
    else if (rc == CT_TEXT)
        textResourcesRequested++;

    char szReq[MAX_URL_LENGTH + 24];
    sprintf(szReq, "GET %s HTTP/1.1", resource.c_str());

    Packet *outPk = new Packet(szReq, HTTPT_REQUEST_MESSAGE);
    const auto& msg = makeShared<HttpRequestMessage>();
    msg->setTargetUrl(www.c_str());
    msg->setProtocol(httpProtocol);
    msg->setHeading(szReq);
    msg->setSerial(serial);
    msg->setChunkLength(B(requestLength));    // Add extra request size if specified
    msg->setKeepAlive(httpProtocol == 11);
    msg->setBadRequest(bad);    // Simulates willingly requesting a non-existing resource.
    outPk->insertAtBack(msg);

    logRequest(msg);

    return outPk;
}

void HttpBrowserBase::scheduleNextBrowseEvent()
{
    if (eventTimer->isScheduled())
        cancelEvent(eventTimer);
    simtime_t nextEventTime;    // MIGRATE40: kvj
    if (++reqInCurSession >= reqNoInCurSession) {
        // The requests in the current round are done. Lets check what to do next.
        if (rdActivityLength == nullptr || simTime() < acitivityPeriodEnd) {
            // Scheduling next session start within an activity period.
            nextEventTime = simTime() + rdInterSessionInterval->draw();
            EV_INFO << "Scheduling a new session start @ T=" << nextEventTime << endl;
            messagesInCurrentSession = 0;
            eventTimer->setKind(MSGKIND_START_SESSION);
            scheduleAt(nextEventTime, eventTimer);
        }
        else {
            // Schedule the next activity period start. This corresponds to to a working day or home time, ie. time
            // when the user is near his workstation and periodically browsing the web. Inactivity periods then
            // correspond to sleep time or time away from the office
            simtime_t activationTime = simTime() + std::max(0.0, 86400.0 - rdActivityLength->draw());    // Sleep for a while
            EV_INFO << "Terminating current activity @ T=" << simTime() << ". Next activation time is " << activationTime << endl;
            eventTimer->setKind(MSGKIND_ACTIVITY_START);
            scheduleAt(activationTime, eventTimer);
        }
    }
    else {
        // Schedule another browse event in a while.
        nextEventTime = simTime() + rdInterRequestInterval->draw();
        EV_INFO << "Scheduling a browse event @ T=" << nextEventTime
                << ". Request " << reqInCurSession << " of " << reqNoInCurSession << endl;
        eventTimer->setKind(MSGKIND_NEXT_MESSAGE);
        scheduleAt(nextEventTime, eventTimer);
    }
}

void HttpBrowserBase::readScriptedEvents(const char *filename)
{
    EV_DEBUG << "Reading scripted events from " << filename << "\n";

    std::ifstream scriptfilestream;
    scriptfilestream.open(filename);
    if (!scriptfilestream.is_open())
        throw cRuntimeError("Could not open script file %s", filename);

    std::string line;
    std::string timepart;
    std::string wwwpart;
    int pos;
    simtime_t t;
    while (!std::getline(scriptfilestream, line).eof()) {
        line = trim(line);
        if (line.find("#") == 0)
            continue;

        pos = line.find(";");
        if (pos == -1)
            continue;
        timepart = line.substr(0, pos);
        wwwpart = line.substr(pos + 1, line.size() - pos - 1);

        try {
            t = simtime_t(atof(timepart.c_str()));
        }
        catch (...) {
            continue;
        }
        std::vector<std::string> path = parseResourceName(wwwpart);

        BrowseEvent be;
        be.time = t;
        be.wwwhost = extractServerName(wwwpart.c_str());
        be.resourceName = extractResourceName(wwwpart.c_str());
        be.serverModule = dynamic_cast<HttpNodeBase *>(controller->getServerModule(wwwpart.c_str()));
        if (be.serverModule == nullptr)
            throw cRuntimeError("Unable to locate server %s in the scenario", wwwpart.c_str());

        EV_DEBUG << "Creating scripted browse event @ T=" << t << ", " << be.wwwhost << " / " << be.resourceName << endl;
        browseEvents.push_front(be);
    }
    scriptfilestream.close();

    if (!browseEvents.empty()) {
        BrowseEvent be = browseEvents.back();
        eventTimer->setKind(MSGKIND_SCRIPT_EVENT);
        scheduleAt(be.time, eventTimer);
    }
}

} // namespace httptools

} // namespace inet

