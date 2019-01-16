//
// Copyright (C) 2013 OpenSim Ltd.
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
// Author: Benjamin Seregi
//

#ifndef __INET_L2NETWORKCONFIGURATOR_H
#define __INET_L2NETWORKCONFIGURATOR_H

#include <algorithm>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * Computes L2 configuration of the network. See the NED definition for details.
 */
class INET_API L2NetworkConfigurator : public cSimpleModule
{
  public:
    L2NetworkConfigurator() { }
    typedef Ieee8021dInterfaceData::PortInfo PortInfo;

  protected:
    class InterfaceInfo;

    /**
     * Represents a node in the network.
     */
    class Node : public Topology::Node
    {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        std::vector<InterfaceInfo *> interfaceInfos;

      public:
        Node(cModule *module) : Topology::Node(module->getId()) { this->module = module; interfaceTable = nullptr; }
        ~Node() { for (size_t i = 0; i < interfaceInfos.size(); i++) delete interfaceInfos[i]; }
    };

    /**
     * Represents an interface in the network.
     */
    class InterfaceInfo : public cObject
    {
      public:
        Node *node;
        Node *childNode;
        InterfaceEntry *interfaceEntry;
        PortInfo portData;

      public:
        InterfaceInfo(Node *node, Node *childNode, InterfaceEntry *interfaceEntry);
        virtual std::string getFullPath() const override { return interfaceEntry->getInterfaceFullPath(); }
    };

    class Matcher
    {
      protected:
        bool matchesany;
        std::vector<inet::PatternMatcher *> matchers;    // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();

        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

    class Link : public Topology::Link
    {
      public:
        InterfaceInfo *sourceInterfaceInfo;
        InterfaceInfo *destinationInterfaceInfo;

      public:
        Link() { sourceInterfaceInfo = nullptr; destinationInterfaceInfo = nullptr; }
    };

    class L2Topology : public Topology
    {
      protected:
        virtual Node *createNode(cModule *module) override { return new L2NetworkConfigurator::Node(module); }
        virtual Link *createLink() override { return new L2NetworkConfigurator::Link(); }
    };

    cXMLElement *configuration = nullptr;
    L2Topology topology;
    Node *rootNode = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates vertices from modules having @networkNode property.
     * Creates edges from connections (wired and wireless) between network interfaces.
     */
    virtual void extractTopology(L2Topology& topology);

    /**
     * Computes the Layer 2 network configuration for all nodes in the network.
     * The result of the computation is only stored in the network configurator.
     */
    virtual void computeConfiguration();

    // helper functions
    virtual bool linkContainsMatchingHostExcept(InterfaceInfo *currentInfo, Matcher& hostMatcher, cModule *exceptModule);
    void ensureConfigurationComputed(L2Topology& topology);
    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    void configureInterface(InterfaceInfo *interfaceInfo);

  public:
    /**
     * Reads interface elements from the configuration file and stores result.
     */
    virtual void readInterfaceConfiguration(Node *rootNode);

    /**
     * Configures the provided interface based on the current network configuration.
     */
    virtual void configureInterface(InterfaceEntry *interfaceEntry);
};

} // namespace inet

#endif // ifndef __INET_L2NETWORKCONFIGURATOR_H

