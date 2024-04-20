//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/ExternalProcess.h"

#include "inet/common/NetworkNamespaceContext.h"

#include <fcntl.h>

namespace inet {

// we could call setsid() to make this process the group leader and
// use pkill -s to kill all child processes if the current approach fails for some reason
// search for setsid and pkill

Define_Module(ExternalProcess);

ExternalProcess::~ExternalProcess()
{
    stopProcess();
    cancelAndDelete(startTimer);
}

void ExternalProcess::initialize()
{
    printStdout = par("printStdout");
    printStderr = par("printStderr");
    command = par("command");
    rtScheduler = check_and_cast<RealTimeScheduler *>(getSimulation()->getScheduler());
    startTimer = new cMessage("startTimer");
    scheduleAt(par("startTime"), startTimer);
}

void ExternalProcess::handleMessage(cMessage *msg)
{
    if (msg == startTimer)
        startProcess();
    else
        throw cRuntimeError("Unknown message: %s", msg->getFullName());
}

bool ExternalProcess::notify(int fd)
{
    Enter_Method("notify");
    char buffer[4096];
    int size = read(fd, buffer, 4096 - 1);
    if (size == 0)
        handleProcessExit();
    else if (size > 0) {
        buffer[size] = 0;
        if (fd == processStdout) {
            stdoutBuffer << buffer;
            auto text = removeLinesFromBuffer(stdoutBuffer);
            if (!text.empty()) {
                EV_INFO << text;
                if (printStdout)
                    std::cout << text;
            }
        }
        else if (fd == processStderr) {
            stderrBuffer << buffer;
            auto text = removeLinesFromBuffer(stderrBuffer);
            if (!text.empty()) {
                EV_ERROR << text;
                if (printStderr)
                    std::cerr << text;
            }
        }
        else
            throw cRuntimeError("Unknown file descriptor: %d", fd);
    }
    return false;
}

void ExternalProcess::startProcess()
{
#ifdef __linux__
    NetworkNamespaceContext context(par("namespace"));
    EV_DEBUG << "Starting process: " << command << std::endl;
    int stdout_pipe[2];
    int stderr_pipe[2];
    if (pipe(stdout_pipe) != 0)
        throw cRuntimeError("cannot open pipe");
    if (pipe(stderr_pipe) != 0)
        throw cRuntimeError("cannot open pipe");
    pid = fork();
    if (pid < 0) {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        throw cRuntimeError("Failed to fork");
    }
    else if (pid == 0) { // child process
        // we could use setsid(), see top comment
        close(stdout_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO); // Redirect stdout to the pipe
        close(stderr_pipe[0]);
        dup2(stderr_pipe[1], STDERR_FILENO); // Redirect stderr to the pipe
        cStringTokenizer tokenizer(command, " ", cStringTokenizer::HONOR_QUOTES);
        std::vector<char*> args;
        while (tokenizer.hasMoreTokens()) {
            char *token = const_cast<char *>(tokenizer.nextToken());
            if (*token == '\'') {
                *(token + strlen(token) - 1) = 0;
                token++;
            }
            args.push_back(const_cast<char *>(token));
        }
        args.push_back(nullptr);
        execvp(args[0], &args[0]);
        throw cRuntimeError("Failed to execute command");
    }
    else { // parent process
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        processStdout = stdout_pipe[0];
        processStderr = stderr_pipe[0];
        fcntl(processStdout, F_SETFL, O_NONBLOCK);
        fcntl(processStderr, F_SETFL, O_NONBLOCK);
        rtScheduler->addCallback(processStdout, this);
        rtScheduler->addCallback(processStderr, this);
    }
#else
    throw cRuntimeError("External processes are only supported on Linux");
#endif
}

void ExternalProcess::stopProcess()
{
    EV_DEBUG << "Stopping process: " << command << std::endl;
    if (pid != -1) {
        rtScheduler->removeCallback(processStdout, this);
        rtScheduler->removeCallback(processStderr, this);
        std::string prefix = !strncmp("sudo", command, 4) ? "sudo " : "";
        // we could use pkill, see top comment
        std::string killCommand = prefix + "/bin/kill -SIGTERM " + std::to_string(pid);
        if (std::system(killCommand.c_str()) != 0)
            EV_WARN << "Failed to send TERM signal to process: " << command << std::endl;
        bool ret = waitForEofOrTimeout(processStdout, 1);
        if (ret) {
            std::string killCommand = prefix + "/bin/kill -SIGKILL " + std::to_string(pid);
            if (std::system(killCommand.c_str()) != 0)
                EV_WARN << "Failed to send KILL signal to process: " << command << std::endl;
        }
        close(processStdout);
        close(processStderr);
    }
}

void ExternalProcess::handleProcessExit()
{
    std::string onExit = par("onExit");
    if (onExit == "ignore")
        ; // void
    else if (onExit == "terminateSimulation")
        throw cTerminationException("External process finished");
    else if (onExit == "relaunch") {
        stopProcess();
        simtime_t relaunchDelay = par("relaunchDelay");
        if (relaunchDelay > 0)
            scheduleAfter(relaunchDelay, startTimer);
        else
            startProcess();
    }
    else
        throw cRuntimeError("Unknown onExit parameter value: %s", onExit.c_str());
}

std::string ExternalProcess::removeLinesFromBuffer(std::stringstream& buffer)
{
    std::string result;
    std::string bufferContent = buffer.str();
    size_t lastNewlinePos = bufferContent.rfind('\n');
    if (lastNewlinePos != std::string::npos) {
        result = bufferContent.substr(0, lastNewlinePos + 1);
        buffer.str("");
        buffer << bufferContent.substr(lastNewlinePos + 1);
    }
    return result;
}

bool ExternalProcess::waitForEofOrTimeout(int fd, int timeoutSeconds)
{
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;
    while (true) {
        fd_set fds = readfds;
        int ret = select(fd + 1, &fds, nullptr, nullptr, &timeout);
        if (ret == -1)
            return false; // error occurred
        else if (ret == 0)
            return false; // timeout occurred, EOF not reached
        else {
            if (FD_ISSET(fd, &fds)) {
                // file descriptor is ready; check if it's at EOF by attempting to read
                char buffer[1];
                int bytesRead = read(fd, buffer, 1);
                if (bytesRead == 0)
                    return true; // EOF reached
                else if (bytesRead < 0)
                    return false; // error occurred during read
                else {
                    // data was read, indicating that EOF has not been reached
                    // if the loop should continue checking for EOF, you might need to handle the data or seek back
                }
            }
        }
    }
}

} // namespace inet
