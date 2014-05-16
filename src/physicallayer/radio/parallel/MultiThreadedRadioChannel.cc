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

#include <MultiThreadedRadioChannel.h>

//#define DEBUG std::cout
#define DEBUG EV_DEBUG

#define TIME(CODE) { struct timespec timeStart, timeEnd; clock_gettime(CLOCK_REALTIME, &timeStart); CODE; clock_gettime(CLOCK_REALTIME, &timeEnd); \
    DEBUG << "Elapsed time is " << ((timeEnd.tv_sec - timeStart.tv_sec) + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1E+9) << "s during running " << #CODE << endl << endl; }

Define_Module(MultiThreadedRadioChannel);

MultiThreadedRadioChannel::~MultiThreadedRadioChannel()
{
    terminateWorkers();
}

void MultiThreadedRadioChannel::initialize(int stage)
{
    RadioChannel::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        initializeWorkers(3);
    }
}

void MultiThreadedRadioChannel::initializeWorkers(int workerCount)
{
    DEBUG << "Main thread: starting " <<  workerCount << " workers..." << endl;
    mainThread = pthread_self();
    isWorkersEnabled = true;
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cacheLock, &mutexattr);
    pthread_mutex_init(&computeCacheJobsLock, NULL);
    pthread_mutex_init(&invalidateCacheJobsLock, NULL);
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_cond_init(&computeCacheJobsCondition, &condattr);
    pthread_cond_init(&invalidateCacheJobsCondition, &condattr);
    pthread_condattr_destroy(&condattr);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < workerCount; i++)
    {
        pthread_t *worker = new pthread_t();
        pthread_create(worker, &attr, i == 0 ? callInvalidateCacheWorkerLoop : callComputeCacheWorkerLoop, this);
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

void MultiThreadedRadioChannel::terminateWorkers()
{
    DEBUG << "Main thread: terminating " << workers.size() << " workers" << endl;
    isWorkersEnabled = false;
    pthread_mutex_lock(&invalidateCacheJobsLock);
    invalidateCacheJobs.clear();
    pthread_cond_signal(&invalidateCacheJobsCondition);
    pthread_mutex_unlock(&invalidateCacheJobsLock);
    pthread_mutex_lock(&computeCacheJobsLock);
    computeCacheJobs.clear();
    pthread_cond_broadcast(&computeCacheJobsCondition);
    pthread_mutex_unlock(&computeCacheJobsLock);
    for (std::vector<pthread_t *>::iterator it = workers.begin(); it != workers.end(); it++)
    {
        void *status;
        pthread_t *worker = *it;
        pthread_join(*worker, &status);
        delete worker;
    }
    pthread_cond_destroy(&computeCacheJobsCondition);
    pthread_cond_destroy(&invalidateCacheJobsCondition);
    pthread_mutex_destroy(&cacheLock);
    pthread_mutex_destroy(&computeCacheJobsLock);
    pthread_mutex_destroy(&invalidateCacheJobsLock);
    DEBUG << "Main thread: terminating " << workers.size() << " workers finished" << endl;
}

void *MultiThreadedRadioChannel::callInvalidateCacheWorkerLoop(void *argument)
{
    return ((MultiThreadedRadioChannel *)argument)->invalidateCacheWorkerLoop(argument);
}

void *MultiThreadedRadioChannel::callComputeCacheWorkerLoop(void *argument)
{
    return ((MultiThreadedRadioChannel *)argument)->computeCacheWorkerLoop(argument);
}

void *MultiThreadedRadioChannel::invalidateCacheWorkerLoop(void *argument)
{
    assertWorkerThread();
    DEBUG << "Worker " << pthread_self() << ": running invalidate cache worker loop..." << endl;
    while (isWorkersEnabled)
    {
        pthread_mutex_lock(&invalidateCacheJobsLock);
        while (isWorkersEnabled && (invalidateCacheJobs.empty() || runningComputeCacheJobCount != 0))
        {
            DEBUG << "Worker " << pthread_self() << ": sleeping while waiting for an invalidate cache job" << endl;
            pthread_cond_wait(&invalidateCacheJobsCondition, &invalidateCacheJobsLock);
            DEBUG << "Worker " << pthread_self() << ": woke up to look for an invalidate cache job" << endl;
        }
        if (!invalidateCacheJobs.empty() && runningComputeCacheJobCount == 0)
        {
            // NOTE: when execution reaches this point there *ARE NO* concurrently running compute cache worker threads
            const InvalidateCacheJob invalidateCacheJob = invalidateCacheJobs.front();
            invalidateCacheJobs.pop_front();
            runningInvalidateCacheJobCount++;
            pthread_mutex_unlock(&invalidateCacheJobsLock);
            DEBUG << "Worker " << pthread_self() << ": running invalidate cache job... " << &invalidateCacheJob << endl;
            invalidateCachedDecisions(invalidateCacheJob.transmission);
            DEBUG << "Worker " << pthread_self() << ": running invalidate cache job finished " << &invalidateCacheJob << endl;
            pthread_mutex_lock(&invalidateCacheJobsLock);
            runningInvalidateCacheJobCount--;
            pthread_cond_signal(&invalidateCacheJobsCondition);
            pthread_mutex_unlock(&invalidateCacheJobsLock);
            if (invalidateCacheJobs.size() == 0 && computeCacheJobs.size() != 0)
            {
                pthread_mutex_lock(&computeCacheJobsLock);
                pthread_cond_broadcast(&computeCacheJobsCondition);
                pthread_mutex_unlock(&computeCacheJobsLock);
            }
        }
        else
        {
            DEBUG << "Worker " << pthread_self() << ": no runnable invalidate cache job found yet" << endl;
            pthread_mutex_unlock(&invalidateCacheJobsLock);
        }
    }
    DEBUG << "Worker " << pthread_self() << ": running invalidate cache worker loop finished" << endl;
    return NULL;
}

void *MultiThreadedRadioChannel::computeCacheWorkerLoop(void *argument)
{
    assertWorkerThread();
    DEBUG << "Worker " << pthread_self() << ": running compute cache worker loop..." << endl;
    while (isWorkersEnabled)
    {
        pthread_mutex_lock(&computeCacheJobsLock);
        while (isWorkersEnabled && (computeCacheJobs.empty() || !invalidateCacheJobs.empty() || runningInvalidateCacheJobCount != 0))
        {
            DEBUG << "Worker " << pthread_self() << ": sleeping while waiting for a compute cache job" << endl;
            pthread_cond_wait(&computeCacheJobsCondition, &computeCacheJobsLock);
            DEBUG << "Worker " << pthread_self() << ": woke up to look for a compute cache job" << endl;
        }
        if (!computeCacheJobs.empty() && invalidateCacheJobs.empty() && runningInvalidateCacheJobCount == 0)
        {
            const ComputeCacheJob computeCacheJob = computeCacheJobs.top();
            computeCacheJobs.pop();
            runningComputeCacheJobCount++;
            pthread_mutex_unlock(&computeCacheJobsLock);
            if (computeCacheJob.receptionEndTime > simTime())
            {
                DEBUG << "Worker " << pthread_self() << ": running compute cache job... " << &computeCacheJob
                      << ", compute cache job count = " << computeCacheJobs.size()
                      << ", current time = " << simTime()
                      << ", reception start time = " << computeCacheJob.receptionStartTime
                      << ", reception end time = " << computeCacheJob.receptionEndTime << endl;
                const IRadioSignalReceptionDecision *decision = computeReceptionDecision(computeCacheJob.receiver, computeCacheJob.listening, computeCacheJob.transmission, &transmissionsCopy);
                setCachedDecision(computeCacheJob.receiver, computeCacheJob.transmission, decision);
                delete computeCacheJob.listening;
                DEBUG << "Worker " << pthread_self() << ": running compute cache job finished" << &computeCacheJob << endl;
            }
            pthread_mutex_lock(&computeCacheJobsLock);
            runningComputeCacheJobCount--;
            pthread_mutex_unlock(&computeCacheJobsLock);
            if (runningComputeCacheJobCount == 0 && invalidateCacheJobs.size() != 0)
            {
                pthread_mutex_lock(&invalidateCacheJobsLock);
                pthread_cond_signal(&invalidateCacheJobsCondition);
                pthread_mutex_unlock(&invalidateCacheJobsLock);
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

void MultiThreadedRadioChannel::waitForInvalidateCacheJobsToComplete() const
{
    if (!invalidateCacheJobs.empty() || runningInvalidateCacheJobCount != 0)
    {
        DEBUG << "Main thread: waiting for invalidate cache jobs to complete..." << endl;
        pthread_mutex_lock(&invalidateCacheJobsLock);
        while (!invalidateCacheJobs.empty() || runningInvalidateCacheJobCount != 0)
            pthread_cond_wait(&invalidateCacheJobsCondition, &invalidateCacheJobsLock);
        pthread_mutex_unlock(&invalidateCacheJobsLock);
        DEBUG << "Main thread: waiting for invalidate cache jobs to complete finished" << endl;
    }
}

//const IRadioSignalArrival *MultiThreadedRadioChannel::getCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
//{
//    const IRadioSignalArrival *arrival = NULL;
//    pthread_mutex_lock(&cacheLock);
//    arrival = RadioChannel::getCachedArrival(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//    return arrival;
//}
//
//void MultiThreadedRadioChannel::setCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalArrival *arrival) const
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::setCachedArrival(radio, transmission, arrival);
//    pthread_mutex_unlock(&cacheLock);
//}
//
//void MultiThreadedRadioChannel::removeCachedArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::removeCachedArrival(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//}

//const IRadioSignalReception *MultiThreadedRadioChannel::getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
//{
//    const IRadioSignalReception *reception = NULL;
//    pthread_mutex_lock(&cacheLock);
//    reception = RadioChannel::getCachedReception(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//    return reception;
//}
//
//void MultiThreadedRadioChannel::setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception) const
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::setCachedReception(radio, transmission, reception);
//    pthread_mutex_unlock(&cacheLock);
//}
//
//void MultiThreadedRadioChannel::removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::removeCachedReception(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//}

//const IRadioSignalReceptionDecision *MultiThreadedRadioChannel::getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const
//{
//    const IRadioSignalReceptionDecision *decision = NULL;
//    pthread_mutex_lock(&cacheLock);
//    decision = RadioChannel::getCachedDecision(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//    return decision;
//}
//
//void MultiThreadedRadioChannel::setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision)
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::setCachedDecision(radio, transmission, decision);
//    pthread_mutex_unlock(&cacheLock);
//}
//
//void MultiThreadedRadioChannel::removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission)
//{
//    pthread_mutex_lock(&cacheLock);
//    RadioChannel::removeCachedDecision(radio, transmission);
//    pthread_mutex_unlock(&cacheLock);
//}

void MultiThreadedRadioChannel::removeNonInterferingTransmission(const IRadioSignalTransmission *transmission)
{
    for (ComputeCacheJobQueue::iterator it = computeCacheJobs.begin(); it != computeCacheJobs.end();)
    {
        if (it->transmission == transmission)
            it = computeCacheJobs.erase(it);
        else
            it++;
    }
    computeCacheJobs.sort();
}

void MultiThreadedRadioChannel::invalidateCachedDecisions(const IRadioSignalTransmission *transmission)
{
    pthread_mutex_lock(&cacheLock);
    transmissionsCopy = transmissions;
    RadioChannel::invalidateCachedDecisions(transmission);
    pthread_mutex_unlock(&cacheLock);
}

void MultiThreadedRadioChannel::invalidateCachedDecision(const IRadioSignalReceptionDecision *decision)
{
    const IRadioSignalReception *reception = decision->getReception();
//    pthread_mutex_lock(&cacheLock);
    RadioChannel::invalidateCachedDecision(decision);
//    pthread_mutex_unlock(&cacheLock);
    const IRadio *radio = reception->getReceiver();
    const IRadioSignalTransmission *transmission = reception->getTransmission();
    const IRadioSignalListening *listening = radio->getReceiver()->createListening(radio, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
    simtime_t receptionStartTime = reception->getStartTime();
    simtime_t receptionEndTime = reception->getEndTime();
    pthread_mutex_lock(&computeCacheJobsLock);
    computeCacheJobs.push(ComputeCacheJob(radio, listening, transmission, receptionStartTime, receptionEndTime));
    pthread_mutex_unlock(&computeCacheJobsLock);
}

void MultiThreadedRadioChannel::addRadio(const IRadio *radio)
{
    // TODO: extend cache
    RadioChannel::addRadio(radio);
}

void MultiThreadedRadioChannel::removeRadio(const IRadio *radio)
{
    // TODO: remove from cache
    RadioChannel::removeRadio(radio);
}

void MultiThreadedRadioChannel::transmitToChannel(const IRadio *transmitterRadio, const IRadioSignalTransmission *transmission)
{
    assertMainThread();
    DEBUG << "Main thread: transmitting signal..." << endl;
    pthread_mutex_lock(&cacheLock);
//    RadioChannel::transmitToChannel(transmitterRadio, transmission);
    transmissionCount++;
    transmissions.push_back(transmission);
    for (std::vector<const IRadio *>::const_iterator it = radios.begin(); it != radios.end(); it++)
    {
        const IRadio *receiverRadio = *it;
        if (receiverRadio != transmitterRadio)
        {
            const IRadioSignalArrival *arrival = propagation->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            setCachedArrival(receiverRadio, transmission, arrival);
        }
    }
    pthread_mutex_unlock(&cacheLock);
    DEBUG << "Main thread: transmitting signal finished" << endl;
}

void MultiThreadedRadioChannel::sendToChannel(IRadio *transmitterRadio, const IRadioFrame *frame)
{
    assertMainThread();
    DEBUG << "Main thread: sending frames..." << endl;
    RadioChannel::sendToChannel(transmitterRadio, frame);
    pthread_mutex_lock(&invalidateCacheJobsLock);
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    invalidateCacheJobs.push_back(InvalidateCacheJob(transmission));
    pthread_cond_signal(&invalidateCacheJobsCondition);
    pthread_mutex_unlock(&invalidateCacheJobsLock);
    pthread_mutex_lock(&computeCacheJobsLock);
    for (std::vector<const IRadio *>::iterator it = radios.begin(); it != radios.end(); it++) {
        const IRadio *receiverRadio = *it;
        // TODO: merge with sendToChannel in base class
        if (transmitterRadio != receiverRadio && isPotentialReceiver(receiverRadio, transmission)) {
            const IRadioSignalArrival *arrival = getArrival(receiverRadio, transmission);
            const IRadioSignalListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
            computeCacheJobs.push(ComputeCacheJob(receiverRadio, listening, transmission, arrival->getStartTime(), arrival->getEndTime()));
        }
    }
    pthread_cond_broadcast(&computeCacheJobsCondition);
    pthread_mutex_unlock(&computeCacheJobsLock);
    DEBUG << "Main thread: sending frames finished" << endl;
}

const IRadioSignalListeningDecision *MultiThreadedRadioChannel::listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const
{
    assertMainThread();
    DEBUG << "Main thread: listening on channel..." << endl;
    waitForInvalidateCacheJobsToComplete();
    const IRadioSignalListeningDecision *decision = RadioChannel::listenOnChannel(radio, listening);
    DEBUG << "Main thread: listening on channel finished" << endl;
    return decision;
}

const IRadioSignalReceptionDecision *MultiThreadedRadioChannel::receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const
{
    assertMainThread();
    DEBUG << "Main thread: receiving from channel..." << endl;
    waitForInvalidateCacheJobsToComplete();
    const IRadioSignalReceptionDecision *decision = RadioChannel::receiveFromChannel(radio, listening, transmission);
    DEBUG << "Main thread: receiving from channel..." << endl;
    return decision;
}
