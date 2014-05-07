//
// Copyright (C) 2013 OpenSim Ltd.
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

#include <MultiThreadedRadioChannel2.h>

//#define DEBUG std::cout
#define DEBUG EV_DEBUG

#define TIME(CODE) { struct timespec timeStart, timeEnd; clock_gettime(CLOCK_REALTIME, &timeStart); CODE; clock_gettime(CLOCK_REALTIME, &timeEnd); \
    DEBUG << "Elapsed time is " << ((timeEnd.tv_sec - timeStart.tv_sec) + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1E+9) << "s during running " << #CODE << endl << endl; }

Define_Module(MultiThreadedRadioChannel2);

MultiThreadedRadioChannel2::~MultiThreadedRadioChannel2()
{
    terminateWorkers();
}

void MultiThreadedRadioChannel2::initialize(int stage)
{
    RadioChannel::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        initializeWorkers(1);
    }
}

void MultiThreadedRadioChannel2::initializeWorkers(int workerCount)
{
    DEBUG << "Main thread: starting " <<  workerCount << " workers..." << endl;
    mainThread = pthread_self();
    isWorkersEnabled = true;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cacheLock, &mutexattr);
    pthread_mutex_init(&computeCacheJobsLock, NULL);
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_cond_init(&computeCacheJobsCondition, &condattr);
    pthread_condattr_destroy(&condattr);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < workerCount; i++)
    {
        pthread_t *worker = new pthread_t();
        pthread_create(worker, &attr, callComputeCacheWorkerLoop, this);
        // TODO: non portable
//        cpu_set_t cpuset;
//        CPU_ZERO(&cpuset);
//        for (int j = 1; j < 8; j++)
//            CPU_SET(j, &cpuset);
//        pthread_setaffinity_np(*worker, sizeof(cpu_set_t), &cpuset);
        workers.push_back(worker);
    }
    pthread_attr_destroy(&attr);
    DEBUG << "Main thread: starting " << workerCount << " workers finished" << endl;
}

void MultiThreadedRadioChannel2::terminateWorkers()
{
    DEBUG << "Main thread: terminating " << workers.size() << " workers" << endl;
    isWorkersEnabled = false;
    pthread_mutex_lock(&computeCacheJobsLock);
    computeCacheJobs.clear();
    pthread_cond_signal(&computeCacheJobsCondition);
    pthread_mutex_unlock(&computeCacheJobsLock);
    for (std::vector<pthread_t *>::iterator it = workers.begin(); it != workers.end(); it++)
    {
        void *status;
        pthread_t *worker = *it;
        pthread_join(*worker, &status);
        delete worker;
    }
    pthread_cond_destroy(&computeCacheJobsCondition);
    pthread_mutex_destroy(&cacheLock);
    pthread_mutex_destroy(&computeCacheJobsLock);
    DEBUG << "Main thread: terminating " << workers.size() << " workers finished" << endl;
}

void *MultiThreadedRadioChannel2::callComputeCacheWorkerLoop(void *argument)
{
    return ((MultiThreadedRadioChannel2 *)argument)->computeCacheWorkerLoop(argument);
}

void *MultiThreadedRadioChannel2::computeCacheWorkerLoop(void *argument)
{
    assertWorkerThread();
    DEBUG << "Worker " << pthread_self() << ": running compute cache worker loop..." << endl;
    while (isWorkersEnabled)
    {
        pthread_mutex_lock(&computeCacheJobsLock);
        while (isWorkersEnabled && (computeCacheJobs.empty() || nextComputeCacheJobIndex == (int)computeCacheJobs.size()))
        {
            DEBUG << "Worker " << pthread_self() << ": sleeping while waiting for a compute cache job" << endl;
            pthread_cond_wait(&computeCacheJobsCondition, &computeCacheJobsLock);
            DEBUG << "Worker " << pthread_self() << ": woke up to look for a compute cache job" << endl;
        }
        if (!computeCacheJobs.empty())
        {
            ASSERT(0 <= nextComputeCacheJobIndex && nextComputeCacheJobIndex < (int)computeCacheJobs.size());
            const ComputeCacheJob computeCacheJob = computeCacheJobs[nextComputeCacheJobIndex++];
            pthread_mutex_unlock(&computeCacheJobsLock);
            if (nextComputeCacheJobIndex == 0)
            {
                pthread_mutex_lock(&cacheLock);
                transmissionsCopy = transmissions;
                pthread_mutex_unlock(&cacheLock);
            }
            if (computeCacheJob.receptionEndTime > simTime())
            {
                DEBUG << "Worker " << pthread_self() << ": running compute cache job... " << &computeCacheJob
                      << ", compute cache job count = " << computeCacheJobs.size()
                      << ", current time = " << simTime()
                      << ", reception start time = " << computeCacheJob.receptionStartTime
                      << ", reception end time = " << computeCacheJob.receptionEndTime << endl;
                const IRadioSignalReceptionDecision *decision = computeReceptionDecision(computeCacheJob.receiver, computeCacheJob.listening, computeCacheJob.transmission, &transmissionsCopy);
                setCachedDecision(computeCacheJob.receiver, computeCacheJob.transmission, decision);
                DEBUG << "Worker " << pthread_self() << ": running compute cache job finished" << &computeCacheJob << endl;
            }
        }
        else
        {
            DEBUG << "Worker " << pthread_self() << ": no runnable compute cache job found yet" << endl;
            pthread_mutex_unlock(&computeCacheJobsLock);
        }
    }
    DEBUG << "Worker " << pthread_self() << ": running compute cache worker loop finished" << endl;
    return NULL;
}

void MultiThreadedRadioChannel2::removeExpiredComputeCacheJobs() const
{
    while (!computeCacheJobs.empty())
    {
        const ComputeCacheJob &computeCacheJob = computeCacheJobs.top();
        if (computeCacheJob.receptionEndTime >= simTime())
            break;
        removeCachedDecision(computeCacheJob.receiver, computeCacheJob.transmission);
        computeCacheJobs.pop();
        nextComputeCacheJobIndex--;
    }
}

void MultiThreadedRadioChannel2::addRadio(const IRadio *radio)
{
    // TODO: extend cache
    RadioChannel::addRadio(radio);
}

void MultiThreadedRadioChannel2::removeRadio(const IRadio *radio)
{
    // TODO: remove from cache
    RadioChannel::removeRadio(radio);
}

void MultiThreadedRadioChannel2::transmitToChannel(const IRadio *transmitterRadio, const IRadioSignalTransmission *transmission)
{
    assertMainThread();
    DEBUG << "Main thread: transmitting signal..." << endl;
    pthread_mutex_lock(&cacheLock);
    RadioChannel::transmitToChannel(transmitterRadio, transmission);
    pthread_mutex_unlock(&cacheLock);
    DEBUG << "Main thread: transmitting signal finished" << endl;
}

void MultiThreadedRadioChannel2::sendToChannel(IRadio *transmitterRadio, const IRadioFrame *frame)
{
    assertMainThread();
    DEBUG << "Main thread: sending frames..." << endl;
    RadioChannel::sendToChannel(transmitterRadio, frame);
    pthread_mutex_lock(&computeCacheJobsLock);
    removeExpiredComputeCacheJobs();
    int i = 0;
    while (i < (int)computeCacheJobs.size())
    {
        const ComputeCacheJob &computeCacheJob = computeCacheJobs[i++];
        if (getCachedDecision(computeCacheJob.receiver, computeCacheJob.transmission))
            removeCachedDecision(computeCacheJob.receiver, computeCacheJob.transmission);
        else
            break;
    }
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    for (std::vector<const IRadio *>::iterator it = radios.begin(); it != radios.end(); it++) {
        const IRadio *receiverRadio = *it;
        // TODO: merge with sendToChannel in base class
        if (transmitterRadio != receiverRadio && isPotentialReceiver(receiverRadio, transmission)) {
            const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
            const IRadioSignalListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
            computeCacheJobs.push(ComputeCacheJob(receiverRadio, listening, transmission, arrival->getStartTime(), arrival->getEndTime()));
        }
    }
    nextComputeCacheJobIndex = 0;
    pthread_cond_signal(&computeCacheJobsCondition);
    pthread_mutex_unlock(&computeCacheJobsLock);
    DEBUG << "Main thread: sending frames finished" << endl;
}

const IRadioSignalReceptionDecision *MultiThreadedRadioChannel2::receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    assertMainThread();
    DEBUG << "Main thread: receiving from channel..." << endl;
    pthread_mutex_lock(&computeCacheJobsLock);
    removeExpiredComputeCacheJobs();
    pthread_cond_signal(&computeCacheJobsCondition);
    pthread_mutex_unlock(&computeCacheJobsLock);
    const IRadioSignalReceptionDecision *decision = RadioChannel::receiveFromChannel(radio, listening, transmission);
    DEBUG << "Main thread: receiving from channel finished" << endl;
    return decision;
}
