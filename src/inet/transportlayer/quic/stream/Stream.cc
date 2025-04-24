//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Stream.h"
#include "../packet/QuicStreamFrame.h"

namespace inet {
namespace quic {

#define minvalue(x,y,z) (x < y ? (x < z ? x : z) : (y < z ? y : z))

Stream::Stream(uint64_t id, Connection *connection, Statistics *connStats) {
    this->id = id;
    this->connection = connection;
    this->streamCreatedTime = simTime();

    this->stats = new Statistics(connStats, "_sid=" + std::to_string(id));
    sendBufferUnsentDataStat = stats->createStatisticEntry("sendBufferUnsentData");
    streamRcvDataBytesStat = stats->createStatisticEntry("streamRcvDataBytes");
    streamTotalRcvDataBytesStat = stats->createStatisticEntry("streamTotalRcvDataBytes");
    stats->getMod()->emit(streamRcvDataBytesStat, (unsigned long)totalStreamRcvDataBytes);

    auto tp = connection->getTransportParameters();
    streamFlowController = new StreamFlowController(id, tp->initial_max_stream_data_bidi_remote, stats);
    streamFlowControlResponder = new StreamFlowControlResponder(this, tp->initial_max_stream_data_bidi_remote, connection->getMaxStreamDataFrameThreshold(), connection->getRoundConsumedDataValue(), stats);
    receiveQueue = new StreamRcvQueue(this, stats);

    connectionFlowController = connection->getConnectionFlowController();
    connectionFlowControlResponder = connection->getConnectionFlowControlResponder();
    controlQueue = connection->getControlQueue();
}

Stream::~Stream() {
    delete streamFlowController;
    delete streamFlowControlResponder;
    delete receiveQueue;
    delete stats;
}

void Stream::enqueueDataFromApp(Ptr<const Chunk> data)
{
    sendQueue.addData(data);
    stats->getMod()->emit(sendBufferUnsentDataStat, (unsigned long)sendQueue.getTotalDataLengthToSend());
}

uint64_t Stream::getSendQueueLength()
{
    return sendQueue.getTotalDataLength();
}

Ptr<StreamFrameHeader> Stream::createHeader(StreamSndQueue::Region region)
{
    Ptr<StreamFrameHeader> header = makeShared<StreamFrameHeader>();
    header->setStreamId(this->id);
    header->setOffset(B(region.offset).get());
    header->setLength(B(region.length).get());
    header->calcChunkLength();
    return header;
}

/**
 * Calculates the size in bytes of the next stream frame from this stream.
 * \param maxFrameSize Gives the limit for the frame.
 * \return The size in bytes equal or smaller the given maxFrameSize.
 * Returns 0 if the stream is unable to generate a stream frame with the given maxFrameSize.
 */
uint64_t Stream::getNextStreamFrameSize(uint64_t maxFrameSize)
{
    StreamSndQueue::Region region = sendQueue.getNextSendRegion();
    if (region.offset == b(-1)) {
        // no data to send
        return 0;
    }

    // check flow control windows
    b availRwnd = B(checkAndGetAvailableRwnd());
    if (availRwnd == b(0)) {
        // cannot send data because flow control window is 0
        return 0;
    }
    if (region.length > availRwnd) {
        region.length = availRwnd;
    }

    auto header = createHeader(region);
    b headerLength = header->getChunkLength();
    b maxFrameBytes = B(maxFrameSize);
    if (maxFrameBytes <= headerLength) {
        // not enough space for the header
        return 0;
    }

    b frameBytes = region.length + headerLength;
    if (maxFrameBytes <= frameBytes) {
        return maxFrameSize;
    }
    return B(frameBytes).get();
}

QuicFrame *Stream::generateStreamFrame(uint64_t offset, uint64_t length)
{
    StreamSndQueue::Region region = sendQueue.getSendRegion(B(offset), B(length));
    if (region.offset == b(-1)) {
        // no data to send
        return nullptr;
    }

    // check flow control windows
    b availRwnd = B(checkAndGetAvailableRwnd());
    if (availRwnd == b(0)) {
        // cannot send data because flow control window is 0
        return nullptr;
    }
    if (region.length > availRwnd) {
        region.length = availRwnd;
    }

    auto header = createHeader(region);
    sendQueue.addOutstandingRegion(region);
    QuicStreamFrame *frame = new QuicStreamFrame(this);
    frame->setHeader(header);

    stats->getMod()->emit(sendBufferUnsentDataStat, (unsigned long)sendQueue.getTotalDataLengthToSend());

    /* TODO: How to count retransmitted data?
    streamFlowController->onStreamFrameSent(B(region.length).get());
    connectionFlowController->onStreamFrameSent(B(region.length).get());
    */
    return frame;
}

/**
 * Generates the next stream frame from this stream.
 * \param maxFrameSize Gives the limit for the frame
 * \return The generated stream frame.
 * throws cRuntimeError if this stream is unable to generate a stream frame.
 */
QuicFrame *Stream::generateNextStreamFrame(uint64_t maxFrameSize)
{
    StreamSndQueue::Region region = sendQueue.getNextSendRegion();
    if (region.offset == b(-1)) {
        throw cRuntimeError("Attempt to generate stream frame failed because no data are available.");
    }

    // check flow control windows
    b availRwnd = B(checkAndGetAvailableRwnd());
    if (availRwnd == b(0)) {
        // cannot send data because flow control window is 0
        return nullptr;
    }
    if (region.length > availRwnd) {
        region.length = availRwnd;
    }

    b maxFrameBytes = B(maxFrameSize);
    if (region.length > maxFrameBytes) {
        region.length = B(maxFrameSize - StreamFrameHeader::MIN_HEADER_SIZE);
    }

    auto header = createHeader(region);
    b headerLength = header->getChunkLength();
    EV_DEBUG << "region (" << region.offset << ", " << region.length << ") headerLength = " << headerLength << endl;
    if (maxFrameBytes <= headerLength) {
        throw cRuntimeError("Attempt to generate stream frame failed because maxFrameSize is too small for the stream header.");
    }

    if (maxFrameBytes < region.length + headerLength) {
        region.length = maxFrameBytes - headerLength;
        header = createHeader(region);
        EV_DEBUG << "new region (" << region.offset << ", " << region.length << ") headerLength = " << header->getChunkLength() << endl;
    }

    sendQueue.addOutstandingRegion(region);
    QuicStreamFrame *frame = new QuicStreamFrame(this);
    frame->setHeader(header);

    stats->getMod()->emit(sendBufferUnsentDataStat, (unsigned long)sendQueue.getTotalDataLengthToSend());

    streamFlowController->onStreamFrameSent(B(region.length).get());
    connectionFlowController->onStreamFrameSent(B(region.length).get());

    return frame;
}

/**
 * While a QuicStreamFrame stores only offset and length, this function returns the actual data.
 * \param offset Offset in bytes.
 * \param length Length in bytes.
 * \return data chunk
 * throws cRuntimeError if the data are not available.
 */
const Ptr<const Chunk> Stream::getDataToSend(uint64_t offset, uint64_t length)
{
    auto data = sendQueue.getData(B(offset), B(length));
    if (data == nullptr) {
        throw cRuntimeError("Unable to fetch data from send buffer");
    }
    return data;
}

void Stream::streamDataLost(uint64_t offset, uint64_t length)
{
    sendQueue.dataLost(B(offset), B(length));
    streamFlowController->onStreamFrameLost(length);
    connectionFlowController->onStreamFrameLost(length);
    stats->getMod()->emit(sendBufferUnsentDataStat, (unsigned long)sendQueue.getTotalDataLengthToSend());
}

void Stream::streamDataAcked(uint64_t offset, uint64_t length)
{
    sendQueue.dataAcked(B(offset), B(length));
}

void Stream::bufferReceivedData(Ptr<const Chunk> data, uint64_t offset)
{
    //Handling unordered stream frames
    receiveQueue->push(data, offset);
}

void Stream::processFlowControlResponder(uint64_t dataSize)
{
    streamFlowControlResponder->updateConsumedData(dataSize);

    if (streamFlowControlResponder->isSendMaxDataFrame()){
        auto maxStreamDataFrame = streamFlowControlResponder->generateMaxDataFrame();
        controlQueue->push_back(maxStreamDataFrame);
     }
}

void Stream::updateHighestRecievedOffset(uint64_t offset){
    streamFlowControlResponder->updateHighestRecievedOffset(offset);
}

bool Stream::hasDataForApp()
{
    return receiveQueue->hasDataForApp();
}

uint64_t Stream::getAvailableDataSizeForApp()
{
    return B(receiveQueue->reorderBuffer.getAvailableDataLength()).get();
}

Ptr<const Chunk> Stream::getDataForApp(B dataSize)
{
    Ptr<const Chunk> chunk = receiveQueue->pop(dataSize);
    auto chunkSize = B(chunk->getChunkLength()).get();

    connectionFlowControlResponder->updateConsumedData(chunkSize);

    if (connectionFlowControlResponder->isSendMaxDataFrame()){
        auto maxDataFrame = connectionFlowControlResponder->generateMaxDataFrame();
        controlQueue->push_back(maxDataFrame);
    }

    processFlowControlResponder(chunkSize);
    return chunk;
}

/**
 * Checks the connection and stream flow control. If one is 0, it queues a DATA_BLOCKED or STREAM_DATA_BLOCKED frame.
 * \return The minimum of both flow control available receiver windows
 */
uint64_t Stream::checkAndGetAvailableRwnd()
{
    //check if connection flow control allowed to send Data
    if(connectionFlowController->getAvailableRwnd()==0  && !connectionFlowController->isDataBlockedFrameWasSend()){
        auto blockedFrame = connectionFlowController->generateDataBlockFrame();
        controlQueue->push_back(blockedFrame);
    }

    //check if stream flow control allowed to send Data
     if (streamFlowController->getAvailableRwnd() == 0 && !streamFlowController->isDataBlockedFrameWasSend()){
         auto blockedFrame = streamFlowController->generateDataBlockFrame();
         controlQueue->push_back(blockedFrame);
     }
    EV_DEBUG << "checkAndGetAvailableRwnd for stream " << id << " - remaining flow control credits: connection: " << connectionFlowController->getAvailableRwnd() << ", stream: " << streamFlowController->getAvailableRwnd() << endl;
    return (std::min(connectionFlowController->getAvailableRwnd(), streamFlowController->getAvailableRwnd()));
}

bool Stream::isAllowedToReceivedData(uint64_t dataSize)
{
    if(connection->getRoundConsumedDataValue()){
        auto maxStreamDataSizeInQuicPacket = connection->getPath()->getMaxQuicPacketSize() - OneRttPacketHeader::SIZE;
        if(connectionFlowControlResponder->getRcvwnd() + maxStreamDataSizeInQuicPacket >= dataSize && streamFlowControlResponder->getRcvwnd() + maxStreamDataSizeInQuicPacket >= dataSize) return true;
        else {throw cRuntimeError("FLOW_CONTROL_ERROR");}
    }else{
        if(connectionFlowControlResponder->getRcvwnd() >= dataSize && streamFlowControlResponder->getRcvwnd() >= dataSize) return true;
        else {throw cRuntimeError("FLOW_CONTROL_ERROR");}
    }
}

void Stream::onDataBlockedFrameReceived(uint64_t streamDataLimit)
{
    streamFlowControlResponder->onDataBlockedFrameReceived(streamDataLimit);
}

void Stream::onMaxStreamDataFrameReceived(uint64_t maxStreamData)
{
    streamFlowController->onMaxFrameReceived(maxStreamData);
}

void Stream::onMaxStreamDataFrameLost()
{
    auto maxStreamDataFrame = streamFlowControlResponder->onMaxDataFrameLost();
    controlQueue->push_back(maxStreamDataFrame);

// send retransmission of FC update immediately
//    this->connection->sendPackets();
}

void Stream::measureStreamRcvDataBytes(uint64_t dataLength)
{
    totalStreamRcvDataBytes +=dataLength;
    stats->getMod()->emit(streamTotalRcvDataBytesStat, (unsigned long)totalStreamRcvDataBytes);
    stats->getMod()->emit(streamRcvDataBytesStat, (unsigned long)dataLength);
}

} /* namespace quic */
} /* namespace inet */
