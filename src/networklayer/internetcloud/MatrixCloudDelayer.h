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

#ifndef __INET_INTERNETCLOUD_MATRIXCLOUDDELAYER_H
#define __INET_INTERNETCLOUD_MATRIXCLOUDDELAYER_H


#include "INETDefs.h"

#include "CloudDelayerBase.h"

class IInterfaceTable;
namespace inet { class PatternMatcher; }

/**
 * Implementation of MatrixCloudDelayer. See NED file for details.
 */
class INET_API MatrixCloudDelayer : public CloudDelayerBase
{
  protected:
    //FIXME modified copy of 'Matcher' class from IPv4NetworkConfigurator
    class Matcher
    {
      private:
        bool matchesany;
        std::vector<inet::PatternMatcher *> matchers; // TODO replace with a MatchExpression once it becomes available in OMNeT++
      public:
        Matcher(const char *pattern);
        ~Matcher();
        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

    class MatrixEntry
    {
      public:
        Matcher srcMatcher;
        Matcher destMatcher;
        bool symmetric;
        cDynamicExpression delayPar;
        cDynamicExpression dataratePar;
        cDynamicExpression dropPar;
        cXMLElement *entity;
      public:
        MatrixEntry(cXMLElement *trafficEntity, bool defaultSymmetric);
        ~MatrixEntry() {}
        bool matches(const char *src, const char *dest);
    };

    class Descriptor
    {
      public:
        cDynamicExpression *delayPar;
        cDynamicExpression *dataratePar;
        cDynamicExpression *dropPar;
        simtime_t lastSent;
      public:
        Descriptor() : delayPar(NULL), dataratePar(NULL), dropPar(NULL), lastSent(SIMTIME_ZERO) {}
    };

    typedef std::pair<int,int> IDPair;
    typedef std::map<IDPair,Descriptor> IDPairToDescriptorMap;
    typedef std::vector<MatrixEntry*> MatrixEntryPtrVector;

    MatrixEntryPtrVector matrixEntries;
    IDPairToDescriptorMap idPairToDescriptorMap;

    IInterfaceTable *ift;
    cModule *host;

  protected:
    virtual ~MatrixCloudDelayer();
    virtual int numInitStages() const { return 2; }
    virtual void initialize(int stage);

    /**
     * returns isDrop and delay for this msg
     */
    virtual void calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay);

    MatrixCloudDelayer::Descriptor* getOrCreateDescriptor(int srcID, int destID);

    /// returns path of connected node for the interface specified by 'id'
    std::string getPathOfConnectedNodeOnIfaceID(int id);
};


#endif  // __INET_INTERNETCLOUD_MATRIXCLOUDDELAYER_H

