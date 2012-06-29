#include <gnplib/impl/network/gnp/GnpNetLayer.h>

#include <gnplib/api/common/HostProperties.h>
#include <gnplib/impl/network/gnp/GnpSubnet.h>
#include <gnplib/impl/network/gnp/topology/GnpPosition.h>

using std::string;
using gnplib::impl::network::gnp::GnpNetLayer;
using gnplib::impl::network::gnp::topology::GnpPosition;

GnpNetLayer::GnpNetLayer(GnpSubnet& subNet, const IPv4NetID& netID, const GnpPosition& netPosition, const GeoLocation& geoLoc,
                         double downBandwidth, double upBandwidth, double accessLatency)
//: SimulationEventHandler(),
: AbstractNetLayer(downBandwidth, upBandwidth, accessLatency, new GnpPosition(netPosition)),
geoLocation(geoLoc),
subnet(subNet)
//nextFreeSendingTime(0),
//nextFreeReceiveTime(0),
//connections(new HashMap<GnpNetLayer, GnpNetBandwidthAllocation>())
{
    myID.reset(new IPv4NetID(netID));
    //    this.online=true;
    subNet.registerNetLayer(*this);
}

/**
 *
 * @return first time sending is possible (line is free)
 */
//long GnpNetLayer::getNextFreeSendingTime()
//{
//    return nextFreeSendingTime;
//}

/**
 *
 * @param time
 *            first time sending is possible (line is free)
 */
//void GnpNetLayer::setNextFreeSendingTime(long time)
//{
//
//    nextFreeSendingTime=time;
//}

/**
 *
 * @param netLayer
 * @return
 */
//boolean GnpNetLayer::isConnected(GnpNetLayer netLayer)
//{
//    return connections.containsKey(netLayer);
//}

/**
 *
 * @param netLayer
 * @param allocation
 */
//void GnpNetLayer::addConnection(GnpNetLayer netLayer, GnpNetBandwidthAllocation allocation)
//{
//    connections.put(netLayer, allocation);
//}

/**
 *
 * @param netLayer
 * @return
 */
//GnpNetBandwidthAllocation GnpNetLayer::getConnection(GnpNetLayer netLayer)
//{
//    return connections.get(netLayer);
//}

/**
 *
 * @param netLayer
 */
//void GnpNetLayer::removeConnection(GnpNetLayer netLayer)
//{
//    connections.remove(netLayer);
//}

/**
 *
 * @param msg
 */
//void GnpNetLayer::addToReceiveQueue(IPv4Message msg)
//{
//    long receiveTime=subnet.getLatencyModel().getTransmissionDelay(msg.getSize(), getMaxDownloadBandwidth());
//    long currenTime=Simulator.getCurrentTime();
//    long arrivalTime=nextFreeReceiveTime+receiveTime;
//    if (arrivalTime<=currenTime)
//    {
//        nextFreeReceiveTime=currenTime;
//        receive(msg);
//    } else
//    {
//        nextFreeReceiveTime=arrivalTime;
//        Simulator.scheduleEvent(msg, arrivalTime, this, SimulationEvent.Type.MESSAGE_RECEIVED);
//    }
//}


// 	@Override
//boolean GnpNetLayer::isSupported(TransProtocol transProtocol)
//{
//
//    return (transProtocol.equals(TransProtocol.UDP)||transProtocol.equals(TransProtocol.TCP));
//}

//void GnpNetLayer::send(Message msg, NetID receiver, NetProtocol netProtocol)
//{
//
//    TransProtocol usedTransProtocol=((AbstractTransMessage)msg).getProtocol();
//    if (this.isSupported(usedTransProtocol))
//    {
//        NetMessage netMsg=new IPv4Message(msg, receiver, this.myID);
//        log.info(Simulator.getSimulatedRealtime()+" Sending "+netMsg);
//        Simulator.getMonitor().netMsgEvent(netMsg, myID, Reason.SEND);
//        this.subnet.send(netMsg);
//    } else
//        throw new IllegalArgumentException("Transport protocol "+usedTransProtocol+" not supported by this NetLayer implementation.");
//
//}



// 	@Override
//string GnpNetLayer::toString() const
//{
//    return getNetID().toString()+" ( "+getHost().getProperties().getGroupID()+" )";
//}

/*
 * (non-Javadoc)
 *
 * @see de.tud.kom.p2psim.api.simengine.SimulationEventHandler#eventOccurred(de.tud.kom.p2psim.api.simengine.SimulationEvent)
 */
//void GnpNetLayer::eventOccurred(SimulationEvent se)
//{
//
//    if (se.getType()==SimulationEvent.Type.MESSAGE_RECEIVED)
//        receive((NetMessage)se.getData());
//    else if (se.getType()==SimulationEvent.Type.TEST_EVENT)
//    {
//        Object[] msgInfo=(Object[])se.getData();
//        send((Message)msgInfo[0], (NetID)msgInfo[1], (NetProtocol)msgInfo[2]);
//    } else if (se.getType()==SimulationEvent.Type.SCENARIO_ACTION&&se.getData()==null)
//    {
//        goOffline();
//    } else if (se.getType()==SimulationEvent.Type.SCENARIO_ACTION)
//    {
//        System.out.println("ERROR"+se.getData());
//        cancelTransmission((Integer)se.getData());
//    }
//}

//void GnpNetLayer::goOffline()
//{
//    super.goOffline();
//    subnet.goOffline(this);
//}

//void GnpNetLayer::cancelTransmission(int commId)
//{
//    subnet.cancelTransmission(commId);
//}


// for JUnit Test

//void GnpNetLayer::goOffline(long time)
//{
//    Simulator.scheduleEvent(null, time, this, SimulationEvent.Type.SCENARIO_ACTION);
//}

//void GnpNetLayer::cancelTransmission(int commId, long time)
//{
//    Simulator.scheduleEvent(new Integer(commId), time, this, SimulationEvent.Type.SCENARIO_ACTION);
//}

//void GnpNetLayer::send(Message msg, NetID receiver, NetProtocol netProtocol, long sendTime)
//{
//
//    Object[] msgInfo=new Object[3];
//    msgInfo[0]=msg;
//    msgInfo[1]=receiver;
//    msgInfo[2]=netProtocol;
//    Simulator.scheduleEvent(msgInfo, sendTime, this, SimulationEvent.Type.TEST_EVENT);
//}
