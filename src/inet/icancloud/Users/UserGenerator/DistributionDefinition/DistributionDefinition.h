//
// Module that implements a distribution definition
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

#ifndef DISTRIBUTIONDEFINITION_H_
#define DISTRIBUTIONDEFINITION_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {

using namespace omnetpp;
class DistributionDefinition: public cSimpleModule{
protected:
    virtual ~DistributionDefinition();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* DISTRIBUTIONDEFINITION_H_ */
