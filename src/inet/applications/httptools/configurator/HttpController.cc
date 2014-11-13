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

#include "inet/applications/httptools/configurator/HttpController.h"

namespace inet {

namespace httptools {

Define_Module(HttpController);


HttpController::HttpController() :
    rdServerSelection(0)
{
}

HttpController::~HttpController()
{
    // Clean up the server references
    for (std::map<std::string, WebServerEntry *>::iterator it = webSiteList.begin(); it != webSiteList.end(); ++it)
        delete it->second;

    delete rdServerSelection;
}

void HttpController::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    EV_DEBUG << "Initializing stage " << stage << endl;

    if (stage == INITSTAGE_LOCAL) {
        EV_INFO << "Initializing HTTP controller. First stage" << endl;

        cXMLElement *rootelement = par("config").xmlValue();
        if (rootelement == NULL)
            throw cRuntimeError("Configuration file is not defined");

        cXMLAttributeMap attributes;
        cXMLElement *element;
        // Initialize the random object for random site selection
        rdObjectFactory rdFactory;
        element = rootelement->getFirstChildWithTag("serverPopularityDistribution");
        if (element == NULL)
            throw cRuntimeError("Server popularity distribution parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdServerSelection = rdFactory.create(attributes);
        if (rdServerSelection == NULL)
            throw cRuntimeError("Server popularity distribution random object could not be created");
        EV_INFO << "Using " << rdServerSelection->typeStr() << " for server popularity distribution." << endl;

        pspecial = 0.0;    // No special events by default
        totalLookups = 0;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // Two stages are required to finalize the initialization of the random object for the site selection
        // once the final number of web sites is known.

        EV_INFO << "Initializing HTTP controller. Second stage" << endl;
        EV_INFO << "Registered servers are " << webSiteList.size() << endl;
        // Finish initialization of the probability distribution objects which depend on the number of servers.
        if (rdServerSelection->getType() == dt_uniform)
            ((rdUniform *)rdServerSelection)->setEnd(webSiteList.size());
        else if (rdServerSelection->getType() == dt_zipf)
            ((rdZipf *)rdServerSelection)->setN(webSiteList.size());

        EV_DEBUG << "Server selection probability distribution: " << rdServerSelection->toString() << endl;

        std::string optionsfile = (const char *)par("events");
        std::string optionssection = (const char *)par("eventsSection");
        if (optionsfile.size() != 0)
            parseOptionsFile(optionsfile, optionssection);
    }
}

void HttpController::finish()
{
    EV_INFO << "Invoking finish on the controller. Total lookups " << totalLookups << endl;

    WebServerEntry *en;
    std::map<std::string, WebServerEntry *>::const_iterator iter;
    for (iter = webSiteList.begin(); iter != webSiteList.end(); ++iter) {
        en = (*iter).second;
        EV_INFO << "Server " << (*iter).first << ": Access count " << en->accessCount << endl;
    }

    // Clean up the server references
    for (iter = webSiteList.begin(); iter != webSiteList.end(); ++iter)
        delete (*iter).second;
    webSiteList.clear();
}

void HttpController::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        HttpServerStatusUpdateMsg *statusMsg = check_and_cast<HttpServerStatusUpdateMsg *>(msg);
        EV_DEBUG << "Handling a status change message @T=" << simTime() << " for www " << statusMsg->www() << endl;
        setSpecialStatus(statusMsg->www(), (ServerStatus)statusMsg->eventKind(), statusMsg->pvalue(), statusMsg->pamortize());
        delete statusMsg;
    }
    else {
        delete msg;
    }
}

void HttpController::registerServer(const char *objectName, const char *wwwName, int port, int rank, simtime_t activationTime)
{
    Enter_Method_Silent();

    std::string serverName = extractServerName(wwwName);

    EV_DEBUG << "Registering www server: " << objectName << ", " << wwwName
             << " (" << port << "). Activation time is " << activationTime << endl;

    if (webSiteList.find(wwwName) != webSiteList.end())
        EV_ERROR << "Server " << wwwName << " is already registered\n";

    WebServerEntry *en = new WebServerEntry;

    en->name = serverName;
    en->host = objectName;
    en->port = port;
    en->module = getTcpApp(objectName);
    en->activationTime = activationTime;
    en->statusSetTime = simTime();
    en->serverStatus = SS_NORMAL;
    en->pvalue = 0.0;
    en->pamortize = 0.0;
    en->accessCount = 0;

    if (en->module == NULL)
        throw cRuntimeError("Server %s does not have a WWW module", wwwName);

    webSiteList[en->name] = en;

    int pos;
    std::vector<WebServerEntry *>::iterator begin = pickList.begin();
    if (rank == INSERT_RANDOM) {
        if (pickList.size() == 0) {
            pickList.push_back(en);
        }
        else {
            pos = (int)uniform(0, pickList.size() - 1);
            pickList.insert(begin + pos, en);
        }
    }
    else if (rank == INSERT_MIDDLE) {
        pos = pickList.size() / 2;
        pickList.insert(begin + pos, en);
    }
    else if (rank == INSERT_END || rank >= (int)pickList.size()) {
        pickList.push_back(en);
    }
    else {
        pickList.insert(begin + rank, en);
    }
}

cModule *HttpController::getServerModule(const char *wwwName)
{
    Enter_Method_Silent();

    std::string serverUrl = extractServerName(wwwName);

    if (webSiteList.find(serverUrl) == webSiteList.end()) {    // The www name is not in the map
        EV_ERROR << "Could not find module name for " << wwwName << endl;
        return NULL;
    }

    WebServerEntry *en = webSiteList[serverUrl];
    EV_DEBUG << "Got module object for www name " << wwwName << ": " << en->host << " (" << en->port << ")\n";

    totalLookups++;
    en->accessCount++;

    if (en->module == NULL)
        EV_ERROR << "Undefined module for " << wwwName << endl;
    return en->module;
}

cModule *HttpController::getAnyServerModule()
{
    Enter_Method_Silent();

    if (webSiteList.size() == 0) {
        EV_WARN << "No modules registered. Cannot select a random module" << endl;
        return NULL;
    }

    if (pickList.size() == 0) {
        EV_ERROR << "No modules currently in the picklist. Cannot select a random module" << endl;
        return NULL;
    }

    EV_DEBUG << "Getting a random server module with pspecial=" << pspecial << endl;

    WebServerEntry *en = __getRandomServerInfo();
    EV_DEBUG << "Got a random www module: " << en->name << ", " << en->host << " (" << en->port << ")\n";

    totalLookups++;
    en->accessCount++;

    return en->module;
}

int HttpController::getServerInfo(const char *wwwName, char *module, int& port)
{
    Enter_Method_Silent();

    std::string serverUrl = extractServerName(wwwName);

    if (webSiteList.find(serverUrl) == webSiteList.end()) {    // The www name is not in the map
        EV_ERROR << "Could not find module name for " << wwwName << endl;
        return -1;
    }

    WebServerEntry *en = webSiteList[serverUrl];
    EV_DEBUG << "Got module object for www name " << wwwName << ": " << en->host << " (" << en->port << ")\n";

    totalLookups++;
    en->accessCount++;

    strcpy(module, en->host.c_str());
    port = en->port;

    return 0;
}

int HttpController::getAnyServerInfo(char *wwwName, char *module, int& port)
{
    Enter_Method_Silent();

    if (webSiteList.size() == 0) {
        EV_WARN << "No modules registered. Cannot select a random module" << endl;
        return -1;
    }

    if (pickList.size() == 0) {
        EV_ERROR << "No modules currently in the picklist. Cannot select a random module" << endl;
        return -2;
    }

    EV_DEBUG << "Getting a random server module with pspecial=" << pspecial << endl;

    WebServerEntry *en = __getRandomServerInfo();
    EV_DEBUG << "Got a random www module: " << en->name << ", " << en->host << " (" << en->port << ")\n";

    totalLookups++;
    en->accessCount++;

    strcpy(wwwName, en->name.c_str());
    strcpy(module, en->host.c_str());
    port = en->port;

    return 0;
}

cModule *HttpController::getTcpApp(std::string node)
{
    int pos = node.find("[");
    int rpos = node.rfind("]");
    cModule *receiverModule = NULL;
    if (pos > -1 && rpos > -1) {
        std::string id = node.substr(pos + 1, pos - rpos - 1);
        std::string name = node.substr(0, pos);
        int numid = atoi(id.c_str());
        receiverModule = simulation.getSystemModule()->getSubmodule(name.c_str(), numid);
    }
    else {
        receiverModule = simulation.getSystemModule()->getSubmodule(node.c_str());
    }

    return receiverModule->getSubmodule("tcpApp", 0);    // TODO: CHECK INDEX
}

void HttpController::setSpecialStatus(const char *www, ServerStatus status, double p, double amortize)
{
    if (webSiteList.find(www) == webSiteList.end()) {    // The www name is not in the map
        EV_ERROR << "Could not find module name for " << www << ". Cannot set special status" << endl;
        return;
    }

    EV_DEBUG << "Setting special status for " << www << ", p=" << p << " and amortize=" << amortize << endl;

    WebServerEntry *en = webSiteList[www];

    en->statusSetTime = simTime();
    en->serverStatus = status;
    en->pvalue = p;
    en->pamortize = amortize;

    specialList.push_front(en);

    pspecial += p;
}

void HttpController::cancelSpecialStatus(const char *www)
{
    if (specialList.size() == 0)
        return;
    WebServerEntry *en;
    std::list<WebServerEntry *>::iterator i;
    for (i = specialList.begin(); i != specialList.end(); i++) {
        en = (*i);
        if (strcmp(en->name.c_str(), www) == 0) {
            pspecial -= en->pvalue;
            en->statusSetTime = simTime();
            en->serverStatus = SS_NORMAL;
            en->pvalue = 0.0;
            en->pamortize = 0.0;
            specialList.erase(i);
            EV_DEBUG << "Special status for " << www << " cancelled" << endl;
            break;
        }
    }
    if (pspecial < 0.0)
        pspecial = 0.0;
    if (specialList.size() == 0)
        pspecial = 0.0;
    EV_DEBUG << "Size of special list is now " << specialList.size() << endl;
}

HttpController::WebServerEntry *HttpController::selectFromSpecialList()
{
    if (specialList.size() == 0) {
        EV_ERROR << "No entries in special list. Cannot select server with special probability" << endl;
        return NULL;
    }

    WebServerEntry *en = NULL;

    if (specialList.size() == 1) {
        en = specialList.front();
    }
    else {
        double p = uniform(0, 1);
        double pcumulative = 0.0;
        std::list<WebServerEntry *>::iterator i;
        for (i = specialList.begin(); i != specialList.end(); i++) {
            en = (*i);
            pcumulative += en->pvalue;
            if (pcumulative / pspecial > p)
                break;
        }
    }

    if (en->pamortize > 0.0) {
        double newp = en->pvalue - en->pamortize;
        if (newp > 0.0) {
            en->pvalue = newp;
            pspecial -= en->pamortize;
            EV_DEBUG << "Amortizing special probability for " << en->name << ". Now at " << en->pvalue << endl;
        }
        else {
            pspecial -= en->pvalue;
            en->pvalue = 0.0;
            cancelSpecialStatus(en->name.c_str());
            EV_DEBUG << "Cancelling special status for " << en->name << endl;
        }
    }

    return en;
}

std::string HttpController::listRegisteredServers()
{
    std::ostringstream str;
    WebServerEntry *en;
    std::map<std::string, WebServerEntry *>::const_iterator iter;
    for (iter = webSiteList.begin(); iter != webSiteList.end(); ++iter) {
        en = (*iter).second;
        str << (*iter).first << ";" << en->host << ";" << en->port << endl;
    }
    return str.str();
}

std::string HttpController::listSpecials()
{
    std::ostringstream str;
    WebServerEntry *en;
    std::list<WebServerEntry *>::iterator i;
    for (i = specialList.begin(); i != specialList.end(); i++) {
        en = (*i);
        str << en->name << ";" << en->host << ";" << en->port << ";" << en->serverStatus
            << ";" << en->pvalue << ";" << en->pamortize << endl;
    }
    return str.str();
}

std::string HttpController::listPickOrder()
{
    std::ostringstream str;
    WebServerEntry *en;
    std::vector<WebServerEntry *>::iterator i;
    for (i = pickList.begin(); i != pickList.end(); i++) {
        en = (*i);
        str << en->name << ";" << en->host << ";" << en->port << ";" << en->serverStatus
            << ";" << en->pvalue << ";" << en->pamortize << endl;
    }
    return str.str();
}

void HttpController::parseOptionsFile(std::string file, std::string section)
{
    bool bSectionFound = false;
    std::ifstream tracefilestream;
    tracefilestream.open(file.c_str());
    if (!tracefilestream.is_open())
        throw cRuntimeError("Could not open events file %s", file.c_str());

    if (section.size() == 0)
        bSectionFound = true; // Grab first section

    double pval;
    double amortizeval;
    simtime_t activationtime;
    std::string line;
    int linecount = 0;
    HttpServerStatusUpdateMsg *statusChange;
    while (!std::getline(tracefilestream, line).eof()) {
        linecount++;
        if (line.empty() || line[0] == '#')
            continue;
        if (line[0] == '[') {
            // Section
            bSectionFound = false;
            std::string sectionsub = line.substr(1, line.size() - 2);
            bSectionFound = sectionsub == section;
        }
        else {
            if (bSectionFound) {
                // Format: {time};{www name};{event kind};{p value};{amortization factor}
                // Event kind is not used at the present

                cStringTokenizer tokenizer = cStringTokenizer(line.c_str(), ";");
                std::vector<std::string> res = tokenizer.asVector();
                if (res.size() != 5)
                    throw cRuntimeError("Invalid format of event config line in '%s' Line: '%s'", file.c_str(), line.c_str());
                try {
                    activationtime = (simtime_t)atof(res[0].c_str());
                    pval = atof(res[3].c_str());
                    amortizeval = atof(res[4].c_str());
                }
                catch (...) {
                    throw cRuntimeError("Invalid format of event config line in '%s' Line: '%s'", file.c_str(), line.c_str());
                }
                EV_DEBUG << "Scheduling a status change for " << res[1] << " @ T=" << activationtime << ". Parameters: " << line << endl;
                statusChange = new HttpServerStatusUpdateMsg();
                statusChange->setWww(res[1].c_str());
                statusChange->setEventKind(1);
                statusChange->setPvalue(pval);
                statusChange->setPamortize(amortizeval);
                scheduleAt(activationtime, statusChange);
            }
        }
    }
    tracefilestream.close();
}

HttpController::WebServerEntry *HttpController::__getRandomServerInfo()
{
    WebServerEntry *en;
    int selected = 0;
    // @todo Reimplement! This is a ugly hack to enable easy activation of servers - can lead to problems if no servers active!!!
    do {
        if (pspecial > 0.0 && bernoulli(pspecial)) {
            // Pick from the special list. Each node can have different probabilities
            en = selectFromSpecialList();
            EV_DEBUG << "Selecting from special list. Got node " << en->name << endl;
        }
        else {
            // Pick from the probability distribution which applies to the general population.
            selected = (int)rdServerSelection->draw();
            en = pickList[selected];
            if (en == NULL)
                throw cRuntimeError("Invalid node selected at index %d", selected);
            EV_DEBUG << "Selecting from normal list. Got node " << en->name << endl;
        }
        EV_DEBUG << "Activation time of the node is " << en->activationTime << " and the current time is " << simTime() << endl;
    } while (en->activationTime > simTime());
    return en;
}

} // namespace httptools

} // namespace inet

