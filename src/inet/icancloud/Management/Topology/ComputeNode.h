/**
 *
 * @class StorageNode StorageNode.cc StorageNode.h
 *
 *  Class to link to computeNode.ned
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-06-08
 */
#ifndef COMPUTENODE_H_
#define COMPUTENODE_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {

using namespace omnetpp;
class ComputeNode: public cSimpleModule{
protected:
    virtual ~ComputeNode();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* FSUSERDEFINITION_H_ */
