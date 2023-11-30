#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/tcp-socket-base.h"

using namespace ns3;

std::vector<std::pair<double, uint32_t>> bytesInFlightOverTime;

void BytesInFlightTrace(std::string context, uint32_t oldValue, uint32_t newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    bytesInFlightOverTime.push_back(std::make_pair(simTime, newValue));
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

std::vector<std::pair<double, ns3::SequenceNumber32>> sndMaxOverTime;

void SndMaxChange(std::string context, ns3::SequenceNumber32 oldValue, ns3::SequenceNumber32 newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    sndMaxOverTime.push_back(std::make_pair(simTime, newValue));
}

std::vector<std::pair<double, ns3::SequenceNumber32>> lastAckedSeqOverTime;

void LastAckedSeqChange(std::string context, ns3::SequenceNumber32 oldValue, ns3::SequenceNumber32 newValue)
{
    double simTime = ns3::Simulator::Now().GetSeconds();
    lastAckedSeqOverTime.push_back(std::make_pair(simTime, newValue));
}

void ConnectTraceFunctions()
{
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow", MakeCallback(&CwndChange));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/HighestSequence", MakeCallback(&SndMaxChange));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/CongState", MakeCallback(&CongStateTrace));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/BytesInFlight", MakeCallback(&BytesInFlightTrace));
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/LastAckedSeq", MakeCallback(&LastAckedSeqChange));
}

int main(int argc, char *argv[])
{
  Time::SetResolution(Time::PS);

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(536));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  Config::SetDefault("ns3::PcapFileWrapper::NanosecMode", ns3::BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1000000));
  Config::SetDefault("ns3::BulkSendApplication::SendSize", UintegerValue(100));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t remotePort = 1000;

  BulkSendHelper source("ns3::TcpSocketFactory",
                         InetSocketAddress(interfaces.GetAddress(1), remotePort));
  source.SetAttribute("MaxBytes", UintegerValue(1000000));
  ApplicationContainer sourceApps = source.Install(nodes.Get(0));
  sourceApps.Start(Seconds(0.0));
  sourceApps.Stop(Seconds(10.0));

  PacketSinkHelper sink("ns3::TcpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), remotePort));
  ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  Simulator::Schedule(MilliSeconds(1), &ConnectTraceFunctions);

  pointToPoint.EnablePcapAll("example1");

  Simulator::Run();
  Simulator::Destroy();

  std::ofstream ofs2("ns3-cwnd.dat", std::ios::out);
  for(auto &x : cwndOverTime)
    ofs2 << x.first << " " << x.second << std::endl;
  ofs2.close();

  std::ofstream ofs3("ns3-seq-num.dat", std::ios::out);
  for(auto &x : sndMaxOverTime)
    ofs3 << x.first << " " << x.second << std::endl;
  ofs3.close();

  std::ofstream ofs4("ns3-cong-state.dat", std::ios::out);
  for(auto &x : congStateOverTime)
    ofs4 << x.first << " " << x.second << std::endl;
  ofs4.close();

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

