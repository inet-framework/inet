// ***************************************************************************
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
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
// ***************************************************************************

#include "HttpServerBase.h"

#include "ModuleAccess.h"
#include "NodeStatus.h"

void HttpServerBase::initialize(int stage)
{
    if (stage == 0)
    {
        ll = par("logLevel");

        EV_DEBUG << "Initializing server component\n";

        hostName = (const char*)par("hostName");
        if (hostName.empty())
        {
            hostName = "www.";
            hostName += getParentModule()->getFullName();
            hostName += ".com";
        }
        EV_DEBUG << "Initializing HTTP server. Using WWW name " << hostName << endl;
        port = par("port");

        logFileName = (const char*)par("logFile");
        enableLogging = logFileName!="";
        outputFormat = lf_short;

        httpProtocol = par("httpProtocol");

        cXMLElement *rootelement = par("config").xmlValue();
        if (rootelement==NULL)
            error("Configuration file is not defined");

        // Initialize the distribution objects for random browsing
        // @todo Skip initialization of random objects for scripted servers?
        cXMLAttributeMap attributes;
        cXMLElement *element;
        rdObjectFactory rdFactory;

        // The reply delay
        element = rootelement->getFirstChildWithTag("replyDelay");
        if (element==NULL)
            error("Reply delay parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdReplyDelay = rdFactory.create(attributes);
        if (rdReplyDelay==NULL)
            error("Reply delay random object could not be created");

        // HTML page size
        element = rootelement->getFirstChildWithTag("htmlPageSize");
        if (element==NULL)
            error("HTML page size parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdHtmlPageSize = rdFactory.create(attributes);
        if (rdHtmlPageSize==NULL)
            error("HTML page size random object could not be created");

        // Text resource size
        element = rootelement->getFirstChildWithTag("textResourceSize");
        if (element==NULL)
            error("Text resource size parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdTextResourceSize = rdFactory.create(attributes);
        if (rdTextResourceSize==NULL)
            error("Text resource size random object could not be created");

        // Image resource size
        element = rootelement->getFirstChildWithTag("imageResourceSize");
        if (element==NULL)
            error("Image resource size parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdImageResourceSize = rdFactory.create(attributes);
        if (rdImageResourceSize==NULL)
            error("Image resource size random object could not be created");

        // Number of resources per page
        element = rootelement->getFirstChildWithTag("numResources");
        if (element==NULL)
            error("Number of resources parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdNumResources = rdFactory.create(attributes);
        if (rdNumResources==NULL)
            error("Number of resources random object could not be created");

        // Text/Image resources ratio
        element = rootelement->getFirstChildWithTag("textImageResourceRatio");
        if (element==NULL)
            error("Text/image resource ratio parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdTextImageResourceRatio = rdFactory.create(attributes);
        if (rdTextImageResourceRatio==NULL)
            error("Text/image resource ratio random object could not be created");

        // Error message size
        element = rootelement->getFirstChildWithTag("errorMessageSize");
        if (element==NULL)
            error("Error message size parameter undefined in XML configuration");
        attributes = element->getAttributes();
        rdErrorMsgSize = rdFactory.create(attributes);
        if (rdErrorMsgSize==NULL)
            error("Error message size random object could not be created");

        activationTime = par("activationTime");
        EV_INFO << "Activation time is " << activationTime << endl;

        std::string siteDefinition = (const char*)par("siteDefinition");
        scriptedMode = !siteDefinition.empty();
        if (scriptedMode)
            readSiteDefinition(siteDefinition);

        // Register the server with the controller object
        registerWithController();

        // Initialize statistics
        htmlDocsServed = imgResourcesServed = textResourcesServed = badRequests = 0;

        // Initialize watches
        WATCH(htmlDocsServed);
        WATCH(imgResourcesServed);
        WATCH(textResourcesServed);
        WATCH(badRequests);

        updateDisplay();
    }
    else if (stage == 1)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void HttpServerBase::finish()
{
    EV_SUMMARY << "HTML documents served " << htmlDocsServed << "\n";
    EV_SUMMARY << "Image resources served " << imgResourcesServed << "\n";
    EV_SUMMARY << "Text resources served " << textResourcesServed << "\n";
    EV_SUMMARY << "Bad requests " << badRequests << "\n";

    recordScalar("HTML.served", htmlDocsServed);
    recordScalar("images.served", imgResourcesServed);
    recordScalar("text.served", textResourcesServed);
    recordScalar("bad.requests", badRequests);
}

void HttpServerBase::updateDisplay()
{
    if (ev.isGUI())
    {
        char buf[1024];
        sprintf(buf, "%ld", htmlDocsServed);
        getParentModule()->getDisplayString().setTagArg("t", 0, buf);

        if (activationTime<=simTime())
        {
            getParentModule()->getDisplayString().setTagArg("i2", 0, "status/up");
            getParentModule()->getDisplayString().setTagArg("i2", 1, "green");
        }
        else
        {
            getParentModule()->getDisplayString().setTagArg("i2", 0, "status/down");
            getParentModule()->getDisplayString().setTagArg("i2", 1, "red");
        }
    }
}

void HttpServerBase::handleMessage(cMessage *msg)
{
    // Override in derived classes
    updateDisplay();
}

cPacket* HttpServerBase::handleReceivedMessage(cMessage *msg)
{
    HttpRequestMessage *request = check_and_cast<HttpRequestMessage *>(msg);
    if (request==NULL)
        error("Message (%s)%s is not a valid request", msg->getClassName(), msg->getName());

    EV_DEBUG << "Handling received message " << msg->getName() << ". Target URL: " << request->targetUrl() << endl;

    logRequest(request);

    if (extractServerName(request->targetUrl()) != hostName)
    {
        // This should never happen but lets check
        error("Received message intended for '%s'", request->targetUrl()); // TODO: DEBUG HERE
        return NULL;
    }

    HttpReplyMessage* replymsg;

    // Parse the request string on spaces
    cStringTokenizer tokenizer = cStringTokenizer(request->heading(), " ");
    std::vector<std::string> res = tokenizer.asVector();
    if (res.size() != 3)
    {
        EV_ERROR << "Invalid request string: " << request->heading() << endl;
        replymsg = generateErrorReply(request, 400);
        logResponse(replymsg);
        return replymsg;
    }

    if (request->badRequest())
    {
        // Bad requests get a 404 reply.
        EV_ERROR << "Bad request - bad flag set. Message: " << request->getName() << endl;
        replymsg = generateErrorReply(request, 404);
    }
    else if (res[0] == "GET")
    {
        replymsg = handleGetRequest(request, res[1]); // Pass in the resource string part
    }
    else
    {
        EV_ERROR << "Unsupported request type " << res[0] << " for " << request->heading() << endl;
        replymsg = generateErrorReply(request, 400);
    }

    if (replymsg!=NULL)
        logResponse(replymsg);

    return replymsg;
}

HttpReplyMessage* HttpServerBase::handleGetRequest(HttpRequestMessage *request, std::string resource)
{
    EV_DEBUG << "Handling GET request " << request->getName() << " resource: " << resource << endl;

    resource = trimLeft(resource, "/");
    std::vector<std::string> req = parseResourceName(resource);
    if (req.size()!=3)
    {
        EV_ERROR << "Invalid GET request string: " << request->heading() << endl;
        return generateErrorReply(request, 400);
    }

    HttpContentType cat = getResourceCategory(req);

    if (cat==CT_HTML)
    {
        if (scriptedMode)
        {
            if (resource.empty() && htmlPages.find("root") != htmlPages.end())
            {
                EV_DEBUG << "Generating root resource" << endl;
                return generateDocument(request, "root");
            }
            if (htmlPages.find(resource) == htmlPages.end())
            {
                if (htmlPages.find("default") != htmlPages.end())
                {
                    EV_DEBUG << "Generating default resource" << endl;
                    return generateDocument(request, "default");
                }
                else
                {
                    EV_ERROR << "Page not found: " << resource << endl;
                    return generateErrorReply(request, 404);
                }
            }
        }
        return generateDocument(request, resource.c_str());
    }
    else if (cat==CT_TEXT || cat==CT_IMAGE)
    {
        if (scriptedMode && resources.find(resource)==resources.end())
        {
            EV_ERROR << "Resource not found: " << resource << endl;
            return generateErrorReply(request, 404);
        }
        return generateResourceMessage(request, resource, cat);
    }
    else
    {
        EV_ERROR << "Unknown or unsupported resource requested in " << request->heading() << endl;
        return generateErrorReply(request, 400);
    }
}

HttpReplyMessage* HttpServerBase::generateDocument(HttpRequestMessage *request, const char* resource, int size)
{
    EV_DEBUG << "Generating HTML document for request " << request->getName() << " from " << request->getSenderModule()->getName() << endl;

    char szReply[512];
    sprintf(szReply, "HTTP/1.1 200 OK (%s)", resource);
    HttpReplyMessage* replymsg = new HttpReplyMessage(szReply);
    replymsg->setHeading("HTTP/1.1 200 OK");
    replymsg->setOriginatorUrl(hostName.c_str());
    replymsg->setTargetUrl(request->originatorUrl());
    replymsg->setProtocol(request->protocol());
    replymsg->setSerial(request->serial());
    replymsg->setResult(200);
    replymsg->setContentType(CT_HTML); // Emulates the content-type header field
    replymsg->setKind(HTTPT_RESPONSE_MESSAGE);

    if (scriptedMode)
    {
        replymsg->setPayload(htmlPages[resource].body.c_str());
        size = htmlPages[resource].size;
    }
    else
    {
        replymsg->setPayload(generateBody().c_str());
    }

    if (size==0)
    {
        EV_DEBUG << "Using random distribution for page size" << endl;
        size = (int)rdHtmlPageSize->draw();
    }

    replymsg->setByteLength(size);
    EV_DEBUG << "Serving a HTML document of length " << replymsg->getByteLength() << " bytes" << endl;

    htmlDocsServed++;

    return replymsg;
}

HttpReplyMessage* HttpServerBase::generateResourceMessage(HttpRequestMessage *request, std::string resource, HttpContentType category)
{
    EV_DEBUG << "Generating resource message in response to request " << request->heading() << " with serial " << request->serial() << endl;

    if (category==CT_TEXT)
        textResourcesServed++;
    else if (category==CT_IMAGE)
        imgResourcesServed++;

    char szReply[512];
    sprintf(szReply, "HTTP/1.1 200 OK (%s)", resource.c_str());
    HttpReplyMessage* replymsg = new HttpReplyMessage(szReply);
    replymsg->setHeading("HTTP/1.1 200 OK");
    replymsg->setOriginatorUrl(hostName.c_str());
    replymsg->setTargetUrl(request->originatorUrl());
    replymsg->setProtocol(request->protocol()); // MIGRATE40: kvj
    replymsg->setSerial(request->serial());
    replymsg->setResult(200);
    replymsg->setContentType(category); // Emulates the content-type header field
    replymsg->setByteLength(resources[resource]); // Set the resource size
    replymsg->setKind(HTTPT_RESPONSE_MESSAGE);

    sprintf(szReply, "RESOURCE-BODY:%s", resource.c_str());
    return replymsg;
}

HttpReplyMessage* HttpServerBase::generateErrorReply(HttpRequestMessage *request, int code)
{
    char szErrStr[32];
    sprintf(szErrStr, "HTTP/1.1 %.3d %s", code, htmlErrFromCode(code).c_str());
    HttpReplyMessage* replymsg = new HttpReplyMessage(szErrStr);
    replymsg->setHeading(szErrStr);
    replymsg->setOriginatorUrl(hostName.c_str());
    replymsg->setTargetUrl(request->originatorUrl());
    replymsg->setProtocol(request->protocol());  // MIGRATE40: kvj
    replymsg->setSerial(request->serial());
    replymsg->setResult(code);
    replymsg->setByteLength((int)rdErrorMsgSize->draw());
    replymsg->setKind(HTTPT_RESPONSE_MESSAGE);

    badRequests++;
    return replymsg;
}

std::string HttpServerBase::generateBody()
{
    int numResources = (int)rdNumResources->draw();
    int numImages = (int)(numResources*rdTextImageResourceRatio->draw());
    int numText = numResources - numImages;

    std::string result;

    char tempBuf[128];
    for (int i=0; i<numImages; i++)
    {
        sprintf(tempBuf, "%s%.4d.%s\n", "IMG", i, "jpg");
        result.append(tempBuf);
    }
    for (int i=0; i<numText; i++)
    {
        sprintf(tempBuf, "%s%.4d.%s\n", "TEXT", i, "txt");
        result.append(tempBuf);
    }

    return result;
}

void HttpServerBase::registerWithController()
{
    // Find controller object and register
    cModule * controller = simulation.getSystemModule()->getSubmodule("controller");
    if (controller == NULL)
        error("Controller module not found");
    ((HttpController*)controller)->registerServer(getParentModule()->getFullName(), hostName.c_str(), port, INSERT_END, activationTime);
}

void HttpServerBase::readSiteDefinition(std::string file)
{
    EV_DEBUG << "Reading site definition file " << file << endl;

    std::ifstream tracefilestream;
    tracefilestream.open(file.c_str());
    if (tracefilestream.fail())
        error("Could not open site definition file %s", file.c_str());

    std::vector<std::string> siteFileSplit = splitFile(file);
    std::string line;
    std::string key;
    std::string htmlfile;
    std::string body;
    std::string value1;
    std::string value2;
    std::string sectionsub;
    int size;
    int linecount = 0;
    bool siteSection = false;
    bool resourceSection = false;

    while (!std::getline(tracefilestream, line).eof())
    {
        linecount++;
        line = trim(line);
        if (line.empty() || line[0]=='#')
            continue;
        sectionsub = getDelimited(line, "[", "]");
        if (!sectionsub.empty())
        {
            // Section
            siteSection = sectionsub == "HTML";
            resourceSection = sectionsub == "RESOURCES";
        }
        else
        {
            cStringTokenizer tokenizer = cStringTokenizer(line.c_str(), ";");
            std::vector<std::string> res = tokenizer.asVector();

            if (siteSection)
            {
                if (res.size()<2 || res.size()>3)
                    error("Invalid format of site configuration file '%s'. Site section, line (%d): %s",
                           file.c_str(), linecount, line.c_str());
                key = trimLeft(res[0], "/");
                if (key.empty())
                {
                    if (htmlPages.find("root")==htmlPages.end())
                        key = "root";
                    else
                        error("Second root page found in site definition file %s, line (%d): %s",
                              file.c_str(), linecount, line.c_str());
                }
                htmlfile = res[1];
                body = readHtmlBodyFile(htmlfile, siteFileSplit[0]); // Pass in the path of the definition file. Page defs are relative to that.
                size = 0;
                if (res.size()>2)
                {
                    try
                    {
                        size = atoi(res[2].c_str());
                    }
                    catch (...)
                    {
                        error("Invalid format of site configuration file '%s'. Resource section, size, line (%d): %s",
                              file.c_str(), linecount, line.c_str());
                    }
                }
                EV_DEBUG << "Adding html page definition " << key << ". The page size is " << size << endl;
                htmlPages[key].size = size;
                htmlPages[key].body = body;
            }
            else if (resourceSection)
            {
                if (res.size()<2 || res.size()>3)
                    error("Invalid format of site configuration file '%s'. Resource section, line (%d): %s",
                          file.c_str(), linecount, line.c_str());
                key = res[0];
                value1 = res[1];
                try
                {
                    size = atoi(value1.c_str());
                }
                catch (...)
                {
                    error("Invalid format of site configuration file '%s'. Resource section, size, line (%d): %s",
                          file.c_str(), linecount, line.c_str());
                }

                if (res.size() > 2)
                {
                    // The type parameter - skip this
                }

                resources[key] = size;
                EV_DEBUG << "Adding resource " << key << " of size " << size << endl;
            }
            else
            {
                error("Invalid format of site configuration file '%s'. Unknown section, line (%d): %s",
                      file.c_str(), linecount, line.c_str());
            }
        }
    }
    tracefilestream.close();
}

std::string HttpServerBase::readHtmlBodyFile(std::string file, std::string path)
{
    EV_DEBUG << "Reading HTML page definition file" << endl;

    std::string filePath = path;
    filePath += file;

    std::string line;
    std::string body = "";
    std::ifstream htmlfilestream;
    htmlfilestream.open(filePath.c_str());
    if (htmlfilestream.fail())
        error("Could not open page definition file '%s'", filePath.c_str());
    while (!std::getline(htmlfilestream, line).eof())
    {
        line = trim(line);
        if (line.empty() || line[0]=='#')
            continue;
        body += line;
        body += "\n";
    }
    htmlfilestream.close();
    return body;
}


