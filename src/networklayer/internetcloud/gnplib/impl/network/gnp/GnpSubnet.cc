#include <gnplib/impl/network/gnp/GnpSubnet.h>

#include <gnplib/api/network/NetLayer.h>
#include <gnplib/impl/network/gnp/GeoLocationOracle.h>
#include <gnplib/impl/network/gnp/GnpLatencyModel.h>
#include <gnplib/impl/network/gnp/GnpNetLayer.h>

namespace oracle=gnplib::impl::network::gnp::GeoLocationOracle;
using gnplib::impl::network::gnp::GnpSubnet;

/**
 * 
 * @author Gerald Klunker
 * @version 0.1, 17.01.2008
 * 
 */

//static Logger GnpSubnet::log(SimLogger.getLogger(GnpSubnet.class));
//const Byte GnpSubnet::REALLOCATE_BANDWIDTH_PERIODICAL_EVENT(0);
//const Byte GnpSubnet::REALLOCATE_BANDWIDTH_EVENTBASED_EVENT(1);
//set<TransferProgress> GnpSubnet::obsoleteEvents();

GnpSubnet::GnpSubnet()
//: bandwidthManager(),
//pbaPeriod(1*Simulator.SECOND_UNIT),
//nextPbaTime(0),
//nextResheduleTime(-1),
//currentlyTransferedStreams(),
//currentStreams(),
//lastCommId(0),
: netLatencyModel(new GnpLatencyModel()),
layers()
{
    oracle::subnet=this;
    oracle::lm=netLatencyModel.get();
    //    this.obsoleteEvents=new HashSet<TransferProgress>(5000000);
}

void GnpSubnet::setLatencyModel(GnpLatencyModel* _netLatencyModel)
{
    netLatencyModel.reset(_netLatencyModel);
    oracle::lm=_netLatencyModel;
}

/**
 * Registers a NetWrapper in the SubNet.
 *
 * @param wrapper
 *            The NetWrapper.
 */
// 	@Override
void GnpSubnet::registerNetLayer(api::network::NetLayer& _netLayer)
{
    GnpNetLayer*const gnpNetLayer(dynamic_cast<GnpNetLayer*>(&_netLayer));
    //dynamic_cast<const IPv4NetID&>(
    layers[gnpNetLayer->getNetID()]=gnpNetLayer;
}

/**
 *
 */
// 	@Override
//void GnpSubnet::send(const NetMessage& msg)
//{
//    GnpNetLayer sender=this.layers.get(msg.getSender());
//    GnpNetLayer receiver=this.layers.get(msg.getReceiver());
//
//    // sender & receiver are registered in the SubNet
//    if (sender==null||receiver==null)
//        throw new IllegalStateException("Receiver or Sender is not registered");
//
//    if (msg.getPayload() instanceof UDPMessage)
//    {
//        double packetLossProb=this.netLatencyModel.getUDPerrorProbability(sender, receiver, (IPv4Message)msg);
//        if (Simulator.getRandom().nextDouble()<packetLossProb)
//        {
//            log.info("Packet loss occured while transfer \""+msg+"\" (packetLossProb: "+packetLossProb+")");
//            AbstractTransMessage transMsg=(AbstractTransMessage)msg.getPayload();
//            return;
//        }
//    }
//
//    sendMessage((IPv4Message)msg, sender, receiver);
//}

/**
 *
 * @param msg
 * @param sender
 * @param receiver
 */
//void GnpSubnet::sendMessage(const IPv4Message& msg, const GnpNetLayer& sender, const GnpNetLayer& receiver)
//{
//    long currentTime=Simulator.getCurrentTime();
//
//    int currentCommId=lastCommId;
//
//    AbstractTransMessage transMsg=(AbstractTransMessage)msg.getPayload();
//    if (transMsg.getCommId()== -1)
//    {
//        lastCommId++;
//        currentCommId=lastCommId;
//        transMsg.setCommId(currentCommId);
//    } else
//    {
//        currentCommId=transMsg.getCommId();
//    }
//
//    // Case 1: message only consists of 1 Segment => no bandwidth allocation
//    if (msg.getNoOfFragments()==1)
//    {
//        long propagationTime=netLatencyModel.getPropagationDelay(sender, receiver);
//        long transmissionTime=netLatencyModel.getTransmissionDelay(msg.getSize(), Math.min(sender.getMaxUploadBandwidth(), receiver.getMaxDownloadBandwidth()));
//        long sendingTime=Math.max(sender.getNextFreeSendingTime(), currentTime)+transmissionTime;
//        long arrivalTime=sendingTime+propagationTime;
//        sender.setNextFreeSendingTime(sendingTime);
//        TransferProgress newTp=new TransferProgress(msg, Double.POSITIVE_INFINITY, 0, currentTime);
//        Simulator.scheduleEvent(newTp, arrivalTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//    }
//        // Case 2: message consists minimum 2 Segments => bandwidth allocation
//    else
//    {
//
//        // Add streams to current transfers
//        double maximumRequiredBandwidth=sender.getMaxUploadBandwidth();
//        if (msg.getPayload() instanceof TCPMessage)
//        {
//            double tcpThroughput=netLatencyModel.getTcpThroughput(sender, receiver);
//            maximumRequiredBandwidth=Math.min(maximumRequiredBandwidth, tcpThroughput);
//        }
//        GnpNetBandwidthAllocation ba=bandwidthManager.addConnection(sender, receiver, maximumRequiredBandwidth);
//        TransferProgress newTp=new TransferProgress(msg, 0, msg.getSize(), currentTime);
//        if (!currentlyTransferedStreams.containsKey(ba))
//            currentlyTransferedStreams.put(ba, new HashSet<TransferProgress>());
//        currentlyTransferedStreams.get(ba).add(newTp);
//        currentStreams.put(currentCommId, newTp);
//
//        // Case 2a: Periodical Bandwidth Allocation
//        // Schedule the first Periodical Bandwidth Allocation Event
//        if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.PERIODICAL)
//        {
//            if (nextPbaTime==0)
//            {
//                nextPbaTime=Simulator.getCurrentTime()+pbaPeriod;
//                Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_PERIODICAL_EVENT, nextPbaTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//            }
//        }
//            // Case 2b: Eventbased Bandwidth Allocation
//            // Schedule an realocation Event after current timeunit
//        else if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.EVENT)
//        {
//            if (nextResheduleTime<=currentTime+1)
//            {
//                nextResheduleTime=currentTime+1;
//                Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_EVENTBASED_EVENT, nextResheduleTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//            }
//        }
//    }
//}

/**
 *
 * @param netLayer
 */
//void GnpSubnet::goOffline(const NetLayer& netLayer)
//{
//    if (bandwidthManager!=null&&bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.EVENT)
//    {
//        for (GnpNetBandwidthAllocation ba : bandwidthManager.removeConnections((AbstractNetLayer)netLayer))
//        {
//            obsoleteEvents.addAll(currentlyTransferedStreams.get(ba));
//            currentStreams.values().removeAll(currentlyTransferedStreams.get(ba));
//            currentlyTransferedStreams.remove(ba);
//
//        }
//        // Reschedule messages after current timeunit
//        long currentTime=Simulator.getCurrentTime();
//        if (nextResheduleTime<=currentTime+1)
//        {
//            nextResheduleTime=currentTime+1;
//            Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_EVENTBASED_EVENT, nextResheduleTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//        }
//    } else if (bandwidthManager!=null)
//    {
//        for (GnpNetBandwidthAllocation ba : bandwidthManager.removeConnections((AbstractNetLayer)netLayer))
//        {
//            currentStreams.values().removeAll(currentlyTransferedStreams.get(ba));
//            currentlyTransferedStreams.remove(ba);
//        }
//    }
//}

/**
 *
 * @param msg
 */
//void GnpSubnet::cancelTransmission(int commId)
//{
//    if (bandwidthManager!=null)
//    {
//
//        GnpNetLayer sender=layers.get(currentStreams.get(commId).getMessage().getSender());
//        GnpNetLayer receiver=layers.get(currentStreams.get(commId).getMessage().getReceiver());
//
//        // remove message from current transfers
//        double maximumRequiredBandwidth=sender.getMaxUploadBandwidth();
//        if (currentStreams.get(commId).getMessage().getPayload() instanceof TCPMessage)
//        {
//            double tcpThroughput=netLatencyModel.getTcpThroughput(sender, receiver);
//            maximumRequiredBandwidth=Math.min(maximumRequiredBandwidth, tcpThroughput);
//        }
//        bandwidthManager.removeConnection(sender, receiver, maximumRequiredBandwidth);
//
//        TransferProgress tp=currentStreams.get(commId);
//        currentStreams.remove(commId);
//
//        obsoleteEvents.add(tp);
//
//        // Reschedule messages after current timeunit
//        long currentTime=Simulator.getCurrentTime();
//        if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.EVENT&&nextResheduleTime<=currentTime+1)
//        {
//            nextResheduleTime=currentTime+1;
//            Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_EVENTBASED_EVENT, nextResheduleTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//        }
//    }
//}

/**
 * Processes a SimulationEvent. (Is called by the Scheduler.)
 *
 * @param se
 *            SimulationEvent that is returned by the Scheduler.
 */
//void GnpSubnet::eventOccurred(const SimulationEvent& se)
//{
//    long currentTime=Simulator.getCurrentTime();
//
//    /*
//     *
//     */
//    if (se.getData()==REALLOCATE_BANDWIDTH_PERIODICAL_EVENT)
//    {
//
//        log.debug("PBA Event at "+Simulator.getSimulatedRealtime());
//
//        nextPbaTime=Simulator.getCurrentTime()+pbaPeriod;
//        bandwidthManager.allocateBandwidth();
//        Set<GnpNetBandwidthAllocation> delete =new HashSet<GnpNetBandwidthAllocation>();
//        for (GnpNetBandwidthAllocation ba : currentlyTransferedStreams.keySet())
//        {
//            reschedulePeriodical(ba);
//            if (currentlyTransferedStreams.get(ba).isEmpty())
//                delete.add(ba);
//        }
//        currentlyTransferedStreams.keySet().removeAll(delete);
//
//        // Schedule next Periodic Event
//        if (currentlyTransferedStreams.size()>0)
//            Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_PERIODICAL_EVENT, nextPbaTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//        else
//            nextPbaTime=0;
//    }
//        /*
//         *
//         */
//    else if (se.getData()==REALLOCATE_BANDWIDTH_EVENTBASED_EVENT)
//    {
//        bandwidthManager.allocateBandwidth();
//        Set<GnpNetBandwidthAllocation> bas=bandwidthManager.getChangedAllocations();
//        for (GnpNetBandwidthAllocation ba : bas)
//            rescheduleEventBased(ba);
//    }
//        /*
//         *
//         */
//    else if (se.getType()==SimulationEvent.Type.MESSAGE_RECEIVED)
//    {
//
//        TransferProgress tp=(TransferProgress)se.getData();
//        IPv4Message msg=(IPv4Message)tp.getMessage();
//        GnpNetLayer sender=this.layers.get(msg.getSender());
//        GnpNetLayer receiver=this.layers.get(msg.getReceiver());
//
//        // Case 1: message only consists of 1 Segment => no bandwidth
//        // allocation
//        if (msg.getNoOfFragments()==1)
//        {
//            receiver.addToReceiveQueue(msg);
//        }
//            // Case 2: message consists minimum 2 Segments => bandwidth
//            // allocation
//        else
//        {
//
//            // Case 2a: Periodical Bandwidth Allocation
//            // Schedule the first Periodical Bandwidth Allocation Event
//            if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.PERIODICAL)
//            {
//                receiver.receive(msg);
//            }
//                // Case 2b: Eventbased Bandwidth Allocation
//                // Schedule an realocation Event after current timeunit
//            else if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.EVENT)
//            {
//                // Dropp obsolete Events
//                if (tp.obsolete||obsoleteEvents.contains(tp))
//                {
//                    obsoleteEvents.remove(tp);
//                    return;
//                } else
//                {
//                    receiver.receive(msg);
//                    // Reschedule messages after current timeunit
//                    if (nextResheduleTime<=currentTime+1)
//                    {
//                        nextResheduleTime=currentTime+1;
//                        Simulator.scheduleEvent(REALLOCATE_BANDWIDTH_EVENTBASED_EVENT, nextResheduleTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//                    }
//                }
//            }
//
//            // remove message from current transfers
//            double maximumRequiredBandwidth=sender.getMaxUploadBandwidth();
//            if (msg.getPayload() instanceof TCPMessage)
//            {
//                double tcpThroughput=netLatencyModel.getTcpThroughput(sender, receiver);
//                maximumRequiredBandwidth=Math.min(maximumRequiredBandwidth, tcpThroughput);
//            }
//            GnpNetBandwidthAllocation ba=bandwidthManager.removeConnection(sender, receiver, maximumRequiredBandwidth);
//            if (bandwidthManager.getBandwidthAllocationType()==BandwidthAllocation.EVENT)
//            {
//                if (currentlyTransferedStreams.get(ba)!=null)
//                {
//                    if (currentlyTransferedStreams.get(ba).size()<=1)
//                    {
//                        currentlyTransferedStreams.remove(ba);
//                    } else
//                    {
//                        currentlyTransferedStreams.get(ba).remove(tp);
//                    }
//                }
//                currentStreams.values().remove(tp);
//
//            } else
//            {
//                if (currentlyTransferedStreams.get(ba)!=null&&currentlyTransferedStreams.get(ba).isEmpty())
//                {
//                    currentlyTransferedStreams.remove(ba);
//                }
//            }
//
//        }
//    }
//}

/**
 * ToDo
 *
 * @param ba
 */
//void GnpSubnet::reschedulePeriodical(const GnpNetBandwidthAllocation& ba)
//{
//    Set<TransferProgress> oldIncomplete=currentlyTransferedStreams.get(ba);
//    Set<TransferProgress> newIncomplete=new HashSet<TransferProgress>(oldIncomplete.size());
//    GnpNetLayer sender=(GnpNetLayer)ba.getSender();
//    GnpNetLayer receiver=(GnpNetLayer)ba.getReceiver();
//    long currentTime=Simulator.getCurrentTime();
//    double leftBandwidth=ba.getAllocatedBandwidth();
//
//    Set<TransferProgress> temp=new HashSet<TransferProgress>();
//    temp.addAll(oldIncomplete);
//
//    oldIncomplete.removeAll(obsoleteEvents);
//    obsoleteEvents.removeAll(temp);
//    int leftStreams=oldIncomplete.size();
//
//    for (TransferProgress tp : oldIncomplete)
//    {
//
//        double remainingBytes=tp.getRemainingBytes(currentTime);
//        double bandwidth=leftBandwidth/leftStreams;
//        if (tp.getMessage().getPayload() instanceof TCPMessage)
//        {
//            double throughput=netLatencyModel.getTcpThroughput(sender, receiver);
//            if (throughput<bandwidth)
//                bandwidth=throughput;
//        }
//        leftBandwidth-=bandwidth;
//        leftStreams--;
//        long transmissionTime=this.netLatencyModel.getTransmissionDelay(remainingBytes, bandwidth);
//        TransferProgress newTp=new TransferProgress(tp.getMessage(), bandwidth, remainingBytes, currentTime);
//
//        if (currentTime+transmissionTime<nextPbaTime)
//        {
//            long propagationTime=this.netLatencyModel.getPropagationDelay(sender, receiver);
//            long arrivalTime=currentTime+transmissionTime+propagationTime;
//            Simulator.scheduleEvent(newTp, arrivalTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//        } else
//        {
//            newIncomplete.add(newTp);
//            int commId=((AbstractTransMessage)tp.getMessage().getPayload()).getCommId();
//            currentStreams.put(commId, newTp);
//        }
//    }
//    currentlyTransferedStreams.put(ba, newIncomplete);
//}

/**
 * ToDo
 *
 * @param ba
 */
//void GnpSubnet::rescheduleEventBased(const GnpNetBandwidthAllocation& ba)
//{
//    Set<TransferProgress> oldIncomplete=currentlyTransferedStreams.get(ba);
//    if (oldIncomplete==null)
//    {
//        return;
//    }
//    Set<TransferProgress> newIncomplete=new HashSet<TransferProgress>(oldIncomplete.size());
//    GnpNetLayer sender=(GnpNetLayer)ba.getSender();
//    GnpNetLayer receiver=(GnpNetLayer)ba.getReceiver();
//    long currentTime=Simulator.getCurrentTime();
//    double leftBandwidth=ba.getAllocatedBandwidth();
//
//    // Tesweise auskommentiert. muss aber wieder rein bzw ersetzt werden um
//    // abgbrochene streams zu entfernen
//    // oldIncomplete.removeAll(obsoleteEvents);
//    int leftStreams=oldIncomplete.size();
//
//    for (TransferProgress tp : oldIncomplete)
//    {
//        double remainingBytes=tp.getRemainingBytes(currentTime);
//
//        double bandwidth=leftBandwidth/leftStreams;
//        if (tp.getMessage().getPayload() instanceof TCPMessage)
//        {
//            double throughput=netLatencyModel.getTcpThroughput(sender, receiver);
//            if (throughput<bandwidth)
//                bandwidth=throughput;
//        }
//        leftBandwidth-=bandwidth;
//        leftStreams--;
//
//        long transmissionTime=this.netLatencyModel.getTransmissionDelay(remainingBytes, bandwidth);
//        long propagationTime=this.netLatencyModel.getPropagationDelay(sender, receiver);
//        long arrivalTime=currentTime+transmissionTime+propagationTime;
//
//        TransferProgress newTp=new TransferProgress(tp.getMessage(), bandwidth, remainingBytes, currentTime);
//        // SimulationEvent event = Simulator.scheduleEvent(newTp,
//        // arrivalTime, this,SimulationEvent.Type.MESSAGE_RECEIVED);
//        Simulator.scheduleEvent(newTp, arrivalTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//        // newTp.relatedEvent = event;
//        newIncomplete.add(newTp);
//        int commId=((AbstractTransMessage)tp.getMessage().getPayload()).getCommId();
//        currentStreams.put(commId, newTp);
//
//        newTp.firstSchedule=false;
//        if (tp.firstSchedule==false)
//        {
//            tp.obsolete=true;
//            // tp.relatedEvent.setData(null);
//        }
//
//    }
//    currentlyTransferedStreams.put(ba, newIncomplete);
//    // obsoleteEvents.addAll(oldIncomplete);
//}
