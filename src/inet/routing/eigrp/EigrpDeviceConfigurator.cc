//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */


#include "inet/routing/eigrp/EigrpDeviceConfigurator.h"
//#include "Ipv6Address.h"
//#include "IPv6InterfaceData.h"
//#include "Ipv4Address.h"
#include <errno.h>
namespace inet {
namespace eigrp {

//Define_Module(EigrpDeviceConfigurator);

using namespace std;


EigrpDeviceConfigurator::EigrpDeviceConfigurator() {
    //deviceId = NULL;
    //deviceType = NULL;
    configFile = NULL;
}

EigrpDeviceConfigurator::EigrpDeviceConfigurator(cXMLElement* confFile, IInterfaceTable* intf)
: configFile(confFile), ift(intf)
{
}

EigrpDeviceConfigurator::~EigrpDeviceConfigurator() {
    //deviceId = NULL;
    //deviceType = NULL;
    configFile = NULL;
}

cXMLElement * EigrpDeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

   // initial call of the method - find <Interfaces> and get first "Interface" node
   if (device != NULL){

      cXMLElement *ifaces = device->getFirstChildWithTag("Interfaces");
      if (ifaces == NULL)
         return NULL;

      iface = ifaces->getFirstChildWithTag("Interface");

   // repeated call - get another "Interface" sibling node
   }else if (iface != NULL){
      iface = iface->getNextSiblingWithTag("Interface");
   }else{
      iface = NULL;
   }

   return iface;
}

cXMLElement * EigrpDeviceConfigurator::GetIPv6Address(cXMLElement *addr, cXMLElement *iface){

   // initial call of the method - get first "Ipv6Address" child node
   if (iface != NULL){
      addr = iface->getFirstChildWithTag("IPv6Address");

   // repeated call - get another "Ipv6Address" sibling node
   }else if (addr != NULL){
      addr = addr->getNextSiblingWithTag("IPv6Address");
   }else{
      addr = NULL;
   }

   return addr;
}

/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool EigrpDeviceConfigurator::Str2Int(int *retValue, const char *str){

   if (retValue == NULL || str == NULL){
      return false;
   }

   char *tail = NULL;
   long value = 0;
   errno = 0;

   value = strtol(str, &tail, 0);

   if (*tail != '\0' || errno == ERANGE || errno == EINVAL || value < INT_MIN || value > INT_MAX){
      return false;
   }

   *retValue = (int) value;
   return true;
}

bool EigrpDeviceConfigurator::Str2Bool(bool *ret, const char *str){

   if (  (strcmp(str, "yes") == 0)
      || (strcmp(str, "enabled") == 0)
      || (strcmp(str, "on") == 0)
      || (strcmp(str, "true") == 0)){

      *ret = true;
      return true;
   }

   if (  (strcmp(str, "no") == 0)
      || (strcmp(str, "disabled") == 0)
      || (strcmp(str, "off") == 0)
      || (strcmp(str, "false") == 0)){

      *ret = false;
      return true;
   }

   int value;
   if (Str2Int(&value, str)){
      if (value == 1){
         *ret = true;
         return true;
      }

      if (value == 0){
         *ret = false;
         return true;
      }
   }

   return false;
}

bool EigrpDeviceConfigurator::wildcardToMask(const char *wildcard, Ipv4Address& result)
{
    result.set(wildcard);
    uint32_t wcNum = result.getInt();

    // convert wildcard to mask
    wcNum = ~wcNum;
    result.set(wcNum);

    if (!result.isValidNetmask())
        return false;

    return true;
}

EigrpNetwork<Ipv4Address> *EigrpDeviceConfigurator::isEigrpInterface(std::vector<EigrpNetwork<Ipv4Address> *>& networks, NetworkInterface *interface)
{
    Ipv4Address prefix;
    Ipv4Address mask;
    Ipv4Address ifAddress = interface->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
    Ipv4Address ifmask = interface->getProtocolData<Ipv4InterfaceData>()->getNetmask();
    vector<int> resultIfs;
    int maskLength, ifMaskLength;
    std::vector<EigrpNetwork<Ipv4Address> *>::iterator it;

    if (ifAddress.isUnspecified())
                return NULL;

    for (it = networks.begin(); it != networks.end(); it++)
    {

        prefix = (*it)->getAddress();
        mask = (*it)->getMask();

        maskLength = (mask.isUnspecified()) ? prefix.getNetworkMask().getNetmaskLength() : mask.getNetmaskLength();
        ifMaskLength = ifmask.getNetmaskLength();

        // prefix isUnspecified -> network = 0.0.0.0 -> all interfaces, or
        // mask is unspecified -> classful match or
        // mask is specified -> classless match
        if (prefix.isUnspecified() ||
                (mask.isUnspecified() && prefix.isNetwork(ifAddress) && maskLength <= ifMaskLength) ||
                (prefix.maskedAddrAreEqual(prefix, ifAddress, mask) && maskLength <= ifMaskLength))
        {
            return *it;
        }
    }

    return NULL;
}

void EigrpDeviceConfigurator::loadEigrpIPv4Networks(cXMLElement *processElem, IEigrpModule<Ipv4Address> *eigrpModule)
{
    const char *addressStr, *wildcardStr;
    Ipv4Address address, mask;
    std::vector<EigrpNetwork<Ipv4Address> *> networks;
    EigrpNetwork<Ipv4Address> *net;
    NetworkInterface* iface;

    cXMLElement *netoworkParentElem = processElem->getFirstChildWithTag("Networks");
    if (netoworkParentElem == NULL)
        return;
    cXMLElement *networkElem = GetEigrpIPv4Network(NULL, netoworkParentElem);

    while (networkElem != NULL)
    {
        // Get IP address
        if ((addressStr = GetNodeParamConfig(networkElem, "IPAddress", NULL)) == NULL)
        {// address is mandatory
            throw cRuntimeError("No IP address specified in the IPAddress node");
        }

        // Get wildcard
        wildcardStr = GetNodeParamConfig(networkElem, "Wildcard", NULL);

        // Create network address and mask
        address.set(addressStr);
        if (wildcardStr == NULL)
        {// wildcard is optional
            mask = Ipv4Address::UNSPECIFIED_ADDRESS;
            // classful network
            address = address.getNetwork();
        }
        else
        {
            // Accepts incorrectly specified wildcard as normal mask (Cisco)
            mask = Ipv4Address(wildcardStr);
            if (mask.isUnspecified() || !mask.isValidNetmask()) {
                if (!wildcardToMask(wildcardStr, mask))
                    throw cRuntimeError("Invalid wildcard in EIGRP network configuration");
            }
            address = address.doAnd(mask);
        }


        net = eigrpModule->addNetwork(address, mask);
        networks.push_back(net);

        networkElem = GetEigrpIPv4Network(networkElem, NULL);
    }


    // Find and store interfaces for networks
    for(int i = 0; i < ift->getNumInterfaces(); i++)
    {
        iface = ift->getInterface(i);
        net = isEigrpInterface(networks, iface);
        if (net != NULL)
            eigrpModule->addInterface(iface->getInterfaceId(), net->getNetworkId(), true);
    }
}

void EigrpDeviceConfigurator::loadEigrpIPv4Config(IEigrpModule<Ipv4Address> *eigrpModule)
{
    ASSERT(eigrpModule != NULL);

    // get access to device node from XML
    //const char *deviceType = par("deviceType");
    //const char *deviceId = par("deviceId");
    //const char *configFile = par("configFile");
    device = configFile;

    if (device == NULL)
    {
       EV_DEBUG << "No EIGRP configuration found for this device!" << endl;
       return;
    }

    loadEigrpProcessesConfig(device, eigrpModule);

    loadEigrpInterfacesConfig(device, eigrpModule);
}

void EigrpDeviceConfigurator::loadEigrpProcessesConfig(cXMLElement *device, IEigrpModule<Ipv4Address> *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *processElem = NULL;
    cXMLElementList procDetails;

    int asNum;              // converted AS number
    const char *asNumStr;   // string with AS number
    int tempNumber;
    bool success;

    processElem = GetEigrpProcess(processElem, device);
    if (processElem == NULL)
    {
        EV_DEBUG << "No EIGRP configuration found." << endl;
        return;
    }

    while (processElem != NULL)
    {
        // AS number of process
        if ((asNumStr = processElem->getAttribute("asNumber")) == NULL)
            throw cRuntimeError("No EIGRP autonomous system number specified");
        success = Str2Int(&asNum, asNumStr);
        if (!success || asNum < 1 || asNum > 65535)
            throw cRuntimeError("Bad value for EIGRP autonomous system number (<1, 65535>)");
        eigrpModule->setASNum(asNum);
        // Load networks and enable corresponding interfaces

        loadEigrpIPv4Networks(processElem, eigrpModule);
        procDetails = processElem->getChildren();
        for (cXMLElementList::iterator procElem = procDetails.begin(); procElem != procDetails.end(); procElem++)
        {
            std::string nodeName = (*procElem)->getTagName();
            if (nodeName == "MetricWeights")
            {
                EigrpKValues kval;
                kval.K1 = loadEigrpKValue((*procElem), "k1", "1");
                kval.K2 = loadEigrpKValue((*procElem), "k2", "0");
                kval.K3 = loadEigrpKValue((*procElem), "k3", "1");
                kval.K4 = loadEigrpKValue((*procElem), "k4", "0");
                kval.K5 = loadEigrpKValue((*procElem), "k5", "0");
                kval.K6 = loadEigrpKValue((*procElem), "k6", "0");
                eigrpModule->setKValues(kval);
            }
            else if (nodeName == "MaximumPath")
            {
                success = Str2Int(&tempNumber, (*procElem)->getNodeValue());
                if (!success || tempNumber < 1)
                    throw cRuntimeError("Bad value for EIGRP maximum paths for load balancing <1, 255>");
                eigrpModule->setMaximumPath(tempNumber);
            }
            else if (nodeName == "Variance")
            {
                success = Str2Int(&tempNumber, (*procElem)->getNodeValue());
                if (!success || tempNumber < 1 || tempNumber > 128)
                    throw cRuntimeError("Bad value for EIGRP variance (<1, 128>)");
                eigrpModule->setVariance(tempNumber);
            }
            else if (nodeName == "PassiveInterface")
            {
                // Get interface ID
                const char *ifaceName = (*procElem)->getNodeValue();
                NetworkInterface* iface = ift->findInterfaceByName(ifaceName);
                if (iface == NULL){
                    throw cRuntimeError("No interface called %s on this device", ifaceName);
                }
                int ifaceId = iface->getInterfaceId();
                eigrpModule->setPassive(true, ifaceId);
            }
            else if (nodeName == "Stub")
            {
                EigrpStub stub;
                stub.connectedRt = loadEigrpStubConf((*procElem), "connected");
                stub.leakMapRt = loadEigrpStubConf((*procElem), "leakMap");
                stub.recvOnlyRt = loadEigrpStubConf((*procElem), "receiveOnly");
                stub.redistributedRt = loadEigrpStubConf((*procElem), "redistributed");
                stub.staticRt = loadEigrpStubConf((*procElem), "static");
                stub.summaryRt = loadEigrpStubConf((*procElem), "summary");
                if (!(stub.connectedRt || stub.leakMapRt || stub.recvOnlyRt || stub.redistributedRt || stub.staticRt || stub.summaryRt))
                    stub.connectedRt = stub.summaryRt = true;   // Default values
                eigrpModule->setStub(stub);
            }
        }

        processElem = GetEigrpProcess(processElem, NULL);
    }
}

int EigrpDeviceConfigurator::loadEigrpKValue(cXMLElement *node, const char *attrName, const char *attrValue)
{
    int result;
    bool success;
    const char *kvalueStr = GetNodeAttrConfig(node, attrName, attrValue);

    success = Str2Int(&result, kvalueStr);
    if (!success || result < 0 || result > 255)
        throw cRuntimeError("Bad value for EIGRP metric weights (<0, 255>)");
    return result;
}

bool EigrpDeviceConfigurator::loadEigrpStubConf(cXMLElement *node, const char *attrName)
{
    bool result;
    bool success;
    const char *stubConf = GetNodeAttrConfig(node, attrName, NULL);
    if (stubConf == NULL)
        return false;

    success = Str2Bool(&result, stubConf);
    if (!success)
        throw cRuntimeError("Bad value for EIGRP stub configuration of parameter %s", attrName);
    return result;
}

void EigrpDeviceConfigurator::loadEigrpInterfacesConfig(cXMLElement *device, IEigrpModule<Ipv4Address> *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *eigrpIfaceElem = NULL;
    cXMLElement *ifaceElem = NULL;

    bool success;
    int tempNumber;

    if ((ifaceElem = GetInterface(ifaceElem, device)) == NULL)
        return;
    while (ifaceElem != NULL)
    {
        // Get interface ID
        const char *ifaceName = ifaceElem->getAttribute("name");
        NetworkInterface* iface = ift->findInterfaceByName(ifaceName);
        if (iface == NULL){
            throw cRuntimeError("No interface called %s on this device", ifaceName);
        }
        int ifaceId = iface->getInterfaceId();



        // Get EIGRP configuration for interface
        eigrpIfaceElem = ifaceElem->getFirstChildWithTag("EIGRP-IPv4");

        // Load EIGRP IPv4 configuration
        if (eigrpIfaceElem != NULL)
        {
            // Get EIGRP AS number
            const char *asNumStr;
            if ((asNumStr = eigrpIfaceElem->getAttribute("asNumber")) == NULL)
                throw cRuntimeError("No EIGRP autonomous system number specified in settings of interface %s", ifaceName);
            success = Str2Int(&tempNumber, asNumStr);
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP autonomous system number (<1, 65535>) on interface %s", ifaceName);
            // TODO vybrat podle AS spravny PDM a pro ten nastavovat nasledujici
            if( tempNumber == eigrpModule->getASNum())
            {
                loadEigrpInterface(eigrpIfaceElem, eigrpModule, ifaceId, ifaceName);

                if (loadEigrpInterfaceParams(eigrpIfaceElem, eigrpModule, ifaceId, ifaceName))
                    eigrpModule->updateInterface(ifaceId);
            }
        }
        ifaceElem = GetInterface(ifaceElem, NULL);
    }
}

bool EigrpDeviceConfigurator::loadEigrpInterfaceParams(cXMLElement *eigrpIface, IEigrpModule<Ipv4Address> *eigrpModule, int ifaceId, const char *ifaceName)
{
    int tempNumber;
    bool success, changed = false;

    cXMLElementList ifDetails = eigrpIface->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "Delay")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 16777215)
                throw cRuntimeError("Bad value for EIGRP Delay on interface %s", ifaceName);
            eigrpModule->setDelay(tempNumber, ifaceId);
            changed = true;
        }
        else if (nodeName == "Bandwidth")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 10000000)
                throw cRuntimeError("Bad value for EIGRP Bandwidth on interface %s", ifaceName);
            eigrpModule->setBandwidth(tempNumber, ifaceId);
            changed = true;
        }
        else if (nodeName == "Reliability")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 255)
                throw cRuntimeError("Bad value for EIGRP Reliability on interface %s", ifaceName);
            eigrpModule->setReliability(tempNumber, ifaceId);
            changed = true;
        }
        else if (nodeName == "Load")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 255)
                throw cRuntimeError("Bad value for EIGRP Load on interface %s", ifaceName);
            eigrpModule->setLoad(tempNumber, ifaceId);
            changed = true;
        }

    }
    return changed;
}

void EigrpDeviceConfigurator::loadEigrpInterface(cXMLElement *eigrpIface, IEigrpModule<Ipv4Address> *eigrpModule, int ifaceId, const char *ifaceName)
{
    int tempNumber;
    bool tempBool, success;

    cXMLElementList ifDetails = eigrpIface->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "HelloInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP Hello interval (<1, 65535>) on interface %s", ifaceName);
            eigrpModule->setHelloInt(tempNumber, ifaceId);
        }
        else if (nodeName == "HoldInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP Hold interval (<1, 65535>) on interface %s", ifaceName);
            eigrpModule->setHoldInt(tempNumber, ifaceId);
        }
        else if (nodeName == "SplitHorizon")
        {
            if (!Str2Bool(&tempBool, (*ifElemIt)->getNodeValue()))
                throw cRuntimeError("Bad value for EIGRP Split Horizon on interface %s", ifaceName);
            eigrpModule->setSplitHorizon(tempBool, ifaceId);
        }

    }
}

void EigrpDeviceConfigurator::loadEigrpIPv6Config(IEigrpModule<Ipv6Address> *eigrpModule)
{//TODO
    ASSERT(eigrpModule != NULL);

    // get access to device node from XML
    //const char *deviceType = par("deviceType");
    //const char *deviceId = par("deviceId");
    //const char *configFile = par("configFile");
    //cXMLElement *device = GetDevice(deviceType, deviceId, configFile);
    device = configFile;

    if (device == NULL)
    {
       EV_DEBUG << "No EIGRP configuration found for this device!" << endl;
       return;
    }

    loadEigrpProcesses6Config(device, eigrpModule);

    loadEigrpInterfaces6Config(device, eigrpModule);
}

void EigrpDeviceConfigurator::loadEigrpProcesses6Config(cXMLElement *device, IEigrpModule<Ipv6Address> *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *processElem = NULL;
    cXMLElementList procDetails;

    int asNum;              // converted AS number
    const char *asNumStr;   // string with AS number
    const char *rIdStr;     // string with routerID
    int tempNumber;
    bool success;

    processElem = GetEigrpProcess6(processElem, device);
    if (processElem == NULL)
    {
        EV_DEBUG << "No EIGRP configuration found." << endl;
        return;
    }

    // AS number of process
    if ((asNumStr = processElem->getAttribute("asNumber")) == NULL)
        throw cRuntimeError("No EIGRP autonomous system number specified");
    success = Str2Int(&asNum, asNumStr);
    if (!success || asNum < 1 || asNum > 65535)
        throw cRuntimeError("Bad value for EIGRP autonomous system number (<1, 65535>)");
    eigrpModule->setASNum(asNum);

    // routerID for process
    if ((rIdStr = processElem->getAttribute("routerId")) == NULL)
        throw cRuntimeError("No EIGRP routerID specified"); // routerID must be specified
    eigrpModule->setRouterId(Ipv4Address(rIdStr));

    procDetails = processElem->getChildren();
    for (cXMLElementList::iterator procElem = procDetails.begin(); procElem != procDetails.end(); procElem++)
    {
        std::string nodeName = (*procElem)->getTagName();

        if (nodeName == "MetricWeights")
        {
            EigrpKValues kval;
            kval.K1 = loadEigrpKValue((*procElem), "k1", "1");
            kval.K2 = loadEigrpKValue((*procElem), "k2", "0");
            kval.K3 = loadEigrpKValue((*procElem), "k3", "1");
            kval.K4 = loadEigrpKValue((*procElem), "k4", "0");
            kval.K5 = loadEigrpKValue((*procElem), "k5", "0");
            kval.K6 = loadEigrpKValue((*procElem), "k6", "0");
            eigrpModule->setKValues(kval);
        }
        else if (nodeName == "MaximumPath")
        {
            success = Str2Int(&tempNumber, (*procElem)->getNodeValue());
            if (!success || tempNumber < 1)
                throw cRuntimeError("Bad value for EIGRP maximum paths for load balancing <1, 255>");
            eigrpModule->setMaximumPath(tempNumber);
        }
        else if (nodeName == "Variance")
        {
            success = Str2Int(&tempNumber, (*procElem)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 128)
                throw cRuntimeError("Bad value for EIGRP variance (<1, 128>)");
            eigrpModule->setVariance(tempNumber);
        }
        else if (nodeName == "PassiveInterface")
        {
            // Get interface ID
            const char *ifaceName = (*procElem)->getNodeValue();
            NetworkInterface* iface = ift->findInterfaceByName(ifaceName);
            if (iface == NULL){
                throw cRuntimeError("No interface called %s on this device", ifaceName);
            }
            int ifaceId = iface->getInterfaceId();
            eigrpModule->setPassive(true, ifaceId);
        }
        else if (nodeName == "Stub")
        {
            EigrpStub stub;
            stub.connectedRt = loadEigrpStubConf((*procElem), "connected");
            stub.leakMapRt = loadEigrpStubConf((*procElem), "leakMap");
            stub.recvOnlyRt = loadEigrpStubConf((*procElem), "receiveOnly");
            stub.redistributedRt = loadEigrpStubConf((*procElem), "redistributed");
            stub.staticRt = loadEigrpStubConf((*procElem), "static");
            stub.summaryRt = loadEigrpStubConf((*procElem), "summary");
            if (!(stub.connectedRt || stub.leakMapRt || stub.recvOnlyRt || stub.redistributedRt || stub.staticRt || stub.summaryRt))
                stub.connectedRt = stub.summaryRt = true;   // Default values
            eigrpModule->setStub(stub);
        }
    }

    return;
}

void EigrpDeviceConfigurator::loadEigrpInterfaces6Config(cXMLElement *device, IEigrpModule<Ipv6Address> *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *eigrpIfaceElem = NULL;
    cXMLElement *ifaceElem = NULL;
    cXMLElement *ipv6AddrElem = NULL;

    bool success;
    int tempNumber;

    if ((ifaceElem = GetInterface(ifaceElem, device)) == NULL)
        return;

    while (ifaceElem != NULL)
    {
        // Get interface ID
        const char *ifaceName = ifaceElem->getAttribute("name");
        NetworkInterface* iface = ift->findInterfaceByName(ifaceName);
        if (iface == NULL){
            throw cRuntimeError("No interface called %s on this device", ifaceName);
        }

        auto int6data = iface->findProtocolDataForUpdate<Ipv6InterfaceData>();


        // for each IPv6 address - save info about network prefix
        ipv6AddrElem = GetIPv6Address(NULL, ifaceElem);
        while (ipv6AddrElem != NULL)
        {

            // get address string
            string addrFull = ipv6AddrElem->getNodeValue();
            Ipv6Address ipv6;
            int prefixLen;

            // check if it's a valid IPv6 address string with prefix and get prefix
            if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
            throw cRuntimeError("Unable to set IPv6 address %s on interface %s", addrFull.c_str(), ifaceName);
            }

            ipv6 = Ipv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());

            int6data->assignAddress(ipv6, false, SIMTIME_ZERO, SIMTIME_ZERO);

            //EV_DEBUG << "IPv6 address: " << ipv6 << "/" << prefixLen << " on iface " << ifaceName << endl;

            if(ipv6.getScope() != Ipv6Address::LINK)
            {// is not link-local -> add
                if(!eigrpModule->addNetPrefix(ipv6.getPrefix(prefixLen), prefixLen, iface->getInterfaceId())) //only saves information about prefix - does not enable in EIGRP
                {// failure - same prefix on different interfaces
                    //throw cRuntimeError("Same IPv6 network prefix (%s/%i) on different interfaces (%s)", ipv6.getPrefix(prefixLen).str().c_str(), prefixLen, ifaceName);
                    EV_DEBUG << "ERROR: Same IPv6 network prefix (" << ipv6.getPrefix(prefixLen) << "/" << prefixLen << ") on different interfaces (prefix ignored on " << ifaceName << ")" << endl;
                }
            }
            // get next IPv6 address
            ipv6AddrElem = GetIPv6Address(ipv6AddrElem, NULL);
        }

        int ifaceId = iface->getInterfaceId();

        // Get EIGRP configuration for interface
        eigrpIfaceElem = ifaceElem->getFirstChildWithTag("EIGRP-IPv6");

        // Load EIGRP IPv6 configuration
        if (eigrpIfaceElem != NULL)
        {
            // Get EIGRP AS number
            const char *asNumStr;
            if ((asNumStr = eigrpIfaceElem->getAttribute("asNumber")) == NULL)
                throw cRuntimeError("No EIGRP autonomous system number specified in settings of interface %s", ifaceName);
            success = Str2Int(&tempNumber, asNumStr);
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP autonomous system number (<1, 65535>) on interface %s", ifaceName);
            // TODO vybrat podle AS spravny PDM a pro ten nastavovat nasledujici

            if( tempNumber == eigrpModule->getASNum())
            {// same AS number of process
                loadEigrpInterface6(eigrpIfaceElem, eigrpModule, ifaceId, ifaceName);

                if (loadEigrpInterfaceParams6(eigrpIfaceElem, eigrpModule, ifaceId, ifaceName))
                    eigrpModule->updateInterface(ifaceId);

                eigrpModule->addInterface(iface->getInterfaceId(), true); // interface included to EIGRP process -> add

            }
        }

        ifaceElem = GetInterface(ifaceElem, NULL);
    }
}

bool EigrpDeviceConfigurator::loadEigrpInterfaceParams6(cXMLElement *eigrpIface, IEigrpModule<Ipv6Address> *eigrpModule, int ifaceId, const char *ifaceName)
{
    int tempNumber;
    bool success, changed=false;

    cXMLElementList ifDetails = eigrpIface->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "Delay")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 16777215)
                throw cRuntimeError("Bad value for EIGRP Delay on interface %s", ifaceName);
            eigrpModule->setDelay(tempNumber, ifaceId);
            changed=true;
        }
        else if (nodeName == "Bandwidth")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 10000000)
                throw cRuntimeError("Bad value for EIGRP Bandwidth on interface %s", ifaceName);
            eigrpModule->setBandwidth(tempNumber, ifaceId);
            changed=true;
        }
        else if (nodeName == "Reliability")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 255)
                throw cRuntimeError("Bad value for EIGRP Reliability on interface %s", ifaceName);
            eigrpModule->setReliability(tempNumber, ifaceId);
            changed=true;
        }
        else if (nodeName == "Load")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 255)
                throw cRuntimeError("Bad value for EIGRP Load on interface %s", ifaceName);
            eigrpModule->setLoad(tempNumber, ifaceId);
            changed=true;
        }

    }
    return changed;
}

void EigrpDeviceConfigurator::loadEigrpInterface6(cXMLElement *eigrpIface, IEigrpModule<Ipv6Address> *eigrpModule, int ifaceId, const char *ifaceName)
{
    int tempNumber;
    bool tempBool, success;

    cXMLElementList ifDetails = eigrpIface->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "HelloInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP Hello interval (<1, 65535>) on interface %s", ifaceName);
            eigrpModule->setHelloInt(tempNumber, ifaceId);
        }
        else if (nodeName == "HoldInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 65535)
                throw cRuntimeError("Bad value for EIGRP Hold interval (<1, 65535>) on interface %s", ifaceName);
            eigrpModule->setHoldInt(tempNumber, ifaceId);
        }
        else if (nodeName == "SplitHorizon")
        {
            if (!Str2Bool(&tempBool, (*ifElemIt)->getNodeValue()))
                throw cRuntimeError("Bad value for EIGRP Split Horizon on interface %s", ifaceName);
            eigrpModule->setSplitHorizon(tempBool, ifaceId);
        }
    }
}

cXMLElement *EigrpDeviceConfigurator::GetEigrpProcess6(cXMLElement *process, cXMLElement *device)
{
    // initial call of the method - get first "AS" child node in "EIGRP"
    if (device != NULL)
    {
        cXMLElement *routing = device->getFirstChildWithTag("Routing6");
        if (routing == NULL)
        {
            return NULL;
        }

        cXMLElement *eigrp = routing->getFirstChildWithTag("EIGRP");
        if (eigrp == NULL)
        {
            return NULL;
        }

        process = eigrp->getFirstChildWithTag("ProcessIPv6");

    // repeated call - get another "AS" sibling node
    }
    else if (process != NULL)
    {
        process = process->getNextSiblingWithTag("ProcessIPv6");
    }
    else
    {
        process = NULL;
    }

    return process;
}

const char *EigrpDeviceConfigurator::GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue)
{
    ASSERT(node != NULL);

    cXMLElement* paramElem = node->getElementByPath(paramName);
    if (paramElem == NULL)
        return defaultValue;

    const char *paramValue = paramElem->getNodeValue();
    if (paramValue == NULL)
        return defaultValue;

    return paramValue;
}

const char *EigrpDeviceConfigurator::GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue)
{
    ASSERT(node != NULL);

    const char *attrValue = node->getAttribute(attrName);
    if (attrValue == NULL)
        return defaultValue;

    return attrValue;
}

cXMLElement *EigrpDeviceConfigurator::GetEigrpProcess(cXMLElement *process, cXMLElement *device)
{
    // initial call of the method - get first "AS" child node in "EIGRP"
    if (device != NULL)
    {
        cXMLElement *routing = device->getFirstChildWithTag("Routing");


        if (routing == NULL)
        {
            return NULL;
        }

        cXMLElement *eigrp = routing->getFirstChildWithTag("EIGRP");
        if (eigrp == NULL)
        {
            return NULL;
        }

        process = eigrp->getFirstChildWithTag("ProcessIPv4");

    // repeated call - get another "AS" sibling node
    }
    else if (process != NULL)
    {
        process = process->getNextSiblingWithTag("ProcessIPv4");
    }
    else
    {
        process = NULL;
    }
    return process;
}

cXMLElement *EigrpDeviceConfigurator::GetEigrpIPv4Network(cXMLElement *network, cXMLElement *process)
{
    // initial call of the method - find first "Network" node in process
    if (process != NULL){

       network = process->getFirstChildWithTag("Network");

    // repeated call - get another "Network" sibling node
    }else if (network != NULL){
        network = network->getNextSiblingWithTag("Network");
    }else{
        network = NULL;
    }

    return network;
}
} //eigrp
} //inet
