//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MATRIXCLOUDDELAYER_H
#define __INET_MATRIXCLOUDDELAYER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/internetcloud/CloudDelayerBase.h"

namespace inet {

class IInterfaceTable;
class PatternMatcher;

/**
 * Implementation of MatrixCloudDelayer. See NED file for details.
 */
class INET_API MatrixCloudDelayer : public CloudDelayerBase
{
  protected:
    // FIXME modified copy of 'Matcher' class from Ipv4NetworkConfigurator
    class INET_API Matcher {
      private:
        bool matchesany;
        std::vector<inet::PatternMatcher *> matchers; // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();
        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

    class INET_API MatrixEntry {
      public:
        Matcher srcMatcher;
        Matcher destMatcher;
        bool symmetric = false;
        cDynamicExpression delayPar;
        cDynamicExpression dataratePar;
        cDynamicExpression dropPar;
        cXMLElement *entity = nullptr;

      public:
        MatrixEntry(cXMLElement *trafficEntity, bool defaultSymmetric);
        ~MatrixEntry() {}
        bool matches(const char *src, const char *dest);
    };

    class INET_API Descriptor {
      public:
        cDynamicExpression *delayPar = nullptr;
        cDynamicExpression *dataratePar = nullptr;
        cDynamicExpression *dropPar = nullptr;
        simtime_t lastSent;

      public:
        Descriptor() {}
    };

    typedef std::pair<int, int> IdPair;
    typedef std::map<IdPair, Descriptor> IdPairToDescriptorMap;
    typedef std::vector<MatrixEntry *> MatrixEntryPtrVector;

    MatrixEntryPtrVector matrixEntries;
    IdPairToDescriptorMap idPairToDescriptorMap;

    ModuleRefByPar<IInterfaceTable> ift;
    cModule *host = nullptr;

  protected:
    virtual ~MatrixCloudDelayer();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /**
     * returns isDrop and delay for this msg
     */
    virtual void calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay) override;

    MatrixCloudDelayer::Descriptor *getOrCreateDescriptor(int srcID, int destID);

    /// returns path of connected node for the interface specified by 'id'
    std::string getPathOfConnectedNodeOnIfaceID(int id);
};

} // namespace inet

#endif

