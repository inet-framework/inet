#include <iomanip>

#include "ns3/drop-tail-queue.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

std::vector<std::pair<double, uint32_t>> bytesInFlightOverTime;

void BytesInFlightTrace(std::string context, uint32_t oldValue, uint32_t newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    bytesInFlightOverTime.push_back(std::make_pair(simTime, newValue));
}

std::vector<std::pair<double, uint32_t>> ssthreshOverTime;

void SSThreshTrace(std::string context, uint32_t oldSsthresh, uint32_t newSsthresh)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    if(newSsthresh > UINT32_MAX / 2)
        newSsthresh = 0;
    ssthreshOverTime.push_back(std::make_pair(simTime, newSsthresh));
}

std::vector<std::pair<double, TcpSocketState::TcpCongState_t>> congStateOverTime;

void CongStateTrace(std::string context, TcpSocketState::TcpCongState_t oldState, TcpSocketState::TcpCongState_t newState)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    congStateOverTime.push_back(std::make_pair(simTime, newState));
}

std::vector<std::pair<double, uint32_t>> cwndOverTime;

void CwndChange(std::string context, uint32_t oldCwnd, uint32_t newCwnd)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    cwndOverTime.push_back(std::make_pair(simTime, newCwnd));
}

std::vector<std::pair<double, ns3::SequenceNumber32>> seqNumOverTime;

void SeqNumChange(std::string context, ns3::SequenceNumber32 oldValue, ns3::SequenceNumber32 newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    seqNumOverTime.push_back(std::make_pair(simTime, newValue));
}

std::vector<std::pair<double, ns3::SequenceNumber32>> lastAckedSeqOverTime;

void LastAckedSeqChange(std::string context, ns3::SequenceNumber32 oldValue, ns3::SequenceNumber32 newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    lastAckedSeqOverTime.push_back(std::make_pair(simTime, newValue));
}

void PacketDropTrace(Ptr<const Packet> p)
{
//  NS_LOG_UNCOND("Packet drop at time " << std::setprecision(15) << Simulator::Now().GetSeconds());
}

std::vector<std::pair<double, uint32_t>> queueLengthOverTime;

void TcQueueTrace(uint32_t oldValue, uint32_t newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    queueLengthOverTime.push_back(std::make_pair(simTime, newValue));
}

void ConnectTraceFunctions()
{
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow", MakeCallback(&CwndChange));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/NextTxSequence", MakeCallback(&SeqNumChange));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/CongState", MakeCallback(&CongStateTrace));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/SlowStartThreshold", MakeCallback(&SSThreshTrace));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/BytesInFlight", MakeCallback(&BytesInFlightTrace));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/LastAckedSeq", MakeCallback(&LastAckedSeqChange));
}

int main(int argc, char *argv[]) {
  Time::SetResolution(Time::PS);

  NodeContainer nodes;
  nodes.Create(3);

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(536));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));
  Config::SetDefault("ns3::PcapFileWrapper::NanosecMode", ns3::BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1000000));
  Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(100));

  PointToPointHelper p2p1, p2p2;
  p2p1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p1.SetChannelAttribute("Delay", StringValue("2ms"));
  p2p1.SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("10p")));
  p2p2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  p2p2.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer dev1, dev2;
  dev1 = p2p1.Install(nodes.Get(0), nodes.Get(1));
  dev2 = p2p2.Install(nodes.Get(1), nodes.Get(2));

  Ptr<PointToPointNetDevice> p2pDevice;
  Ptr<Queue<Packet>> queue;

  p2pDevice = DynamicCast<PointToPointNetDevice>(dev1.Get(1));
  queue = CreateObjectWithAttributes<DropTailQueue<Packet>>("MaxSize", QueueSizeValue(QueueSize("10p")));
  p2pDevice->SetQueue(queue);

  p2pDevice = DynamicCast<PointToPointNetDevice>(dev2.Get(0));
  queue = CreateObjectWithAttributes<DropTailQueue<Packet>>("MaxSize", QueueSizeValue(QueueSize("10p")));
  queue->TraceConnectWithoutContext("PacketsInQueue", MakeCallback(&TcQueueTrace));
  queue->TraceConnectWithoutContext("Drop", MakeCallback(&PacketDropTrace));
  p2pDevice->SetQueue(queue);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface1 = ipv4.Assign(dev1);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iface2 = ipv4.Assign(dev2);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t remotePort = 1000;

  BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(iface2.GetAddress(1), remotePort));
  source.SetAttribute("MaxBytes", UintegerValue(1000000));
  ApplicationContainer sourceApps = source.Install(nodes.Get(0));
  sourceApps.Start(Seconds(0.0));
  sourceApps.Stop(Seconds(10.0));

  Address sinkAddress(InetSocketAddress(iface2.GetAddress(1), remotePort));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
  ApplicationContainer sinkApps = sinkHelper.Install(nodes.Get(2));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  Simulator::Schedule(MilliSeconds(1), &ConnectTraceFunctions);

  p2p1.EnablePcapAll("example3");

  Simulator::Run();
  Simulator::Destroy();

  std::ofstream ofs1("ns3-queue-length.dat", std::ios::out);
  for(auto &x : queueLengthOverTime)
    ofs1 << x.first << " " << x.second << std::endl;
  ofs1.close();

  std::ofstream ofs2("ns3-cwnd.dat", std::ios::out);
  for(auto &x : cwndOverTime)
    ofs2 << x.first << " " << x.second << std::endl;
  ofs2.close();

  std::ofstream ofs3("ns3-seq-num.dat", std::ios::out);
  for(auto &x : seqNumOverTime)
    ofs3 << x.first << " " << x.second << std::endl;
  ofs3.close();

  std::ofstream ofs4("ns3-cong-state.dat", std::ios::out);
  for(auto &x : congStateOverTime)
    ofs4 << x.first << " " << x.second << std::endl;
  ofs4.close();

  std::ofstream ofs5("ns3-ssthresh.dat", std::ios::out);
  for(auto &x : ssthreshOverTime)
    ofs5 << x.first << " " << x.second << std::endl;
  ofs5.close();

  std::ofstream ofs6("ns3-bytesInFlight.dat", std::ios::out);
  for(auto &x : bytesInFlightOverTime)
    ofs6 << x.first << " " << x.second << std::endl;
  ofs6.close();

  std::ofstream ofs7("ns3-last-acked-seq.dat", std::ios::out);
  for(auto &x : lastAckedSeqOverTime)
    ofs7 << x.first << " " << x.second << std::endl;
  ofs7.close();

  return 0;
}

