//
// Module that implements a file that will be preloaded at file system.
// It is defined by a tenant in order to be loaded and modified by an application
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

#ifndef PRELOADFILESDEFINITION_H_
#define PRELOADFILESDEFINITION_H_

#include <omnetpp.h>

namespace inet {

namespace icancloud {

using namespace omnetpp;
class PreloadFilesDefinition: public cSimpleModule{
protected:
    virtual ~PreloadFilesDefinition();
    virtual void initialize() override;
    
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override {};
};

} // namespace icancloud
} // namespace inet

#endif /* PRELOADFILESDEFINITION_H_ */
