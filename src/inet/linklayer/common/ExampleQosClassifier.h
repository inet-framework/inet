//
// Copyright (C) 2010 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXAMPLEQOSCLASSIFIER_H
#define __INET_EXAMPLEQOSCLASSIFIER_H


namespace inet {

/**
 * An example packet classifier based on the UDP/TCP port number.
 */
class INET_API ExampleQosClassifier : public cSimpleModule
{
  protected:
    virtual int getUserPriority(cMessage *msg);

  public:
    ExampleQosClassifier() {}
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

