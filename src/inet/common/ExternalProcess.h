//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXTERNALPROCESS_H
#define __INET_EXTERNALPROCESS_H

#include "inet/common/scheduler/RealTimeScheduler.h"

namespace inet {

class ExternalProcess : public cSimpleModule, public RealTimeScheduler::ICallback
{
  private:
    // parameters
    bool printStdout = false;
    bool printStderr = false;
    const char *command = nullptr;

    // context
    RealTimeScheduler *rtScheduler = nullptr;
    cMessage *startTimer = nullptr;

    // process information
    int processStdout = -1;
    int processStderr = -1;
    pid_t pid = -1;

    // logging state
    std::stringstream stdoutBuffer;
    std::stringstream stderrBuffer;

  public:
    virtual ~ExternalProcess();

    virtual bool notify(int fd) override;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    void startProcess();
    void stopProcess();

    void handleProcessExit();

    std::string removeLinesFromBuffer(std::stringstream& buffer);

    bool waitForEofOrTimeout(int fd, int timeoutSeconds);
};

}

#endif
