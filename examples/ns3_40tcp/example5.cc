#include <iomanip>

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/event-id.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"

using namespace ns3;

#define GLRAND_MAX  0x7ffffffeL  /* = 2**31-2 */

/**
 * Our RNG on [0,1), for the layouting library and other non-simulation purposes.
 * C's rand() is not to be trusted.
 */
class LCGRandom
{
    private:
        int32_t seed;
        uint64_t numDrawn = 0;

    public:
        LCGRandom(int32_t seed=1);

        int32_t getSeed() { return seed; }
        void setSeed(int32_t seed);

        double next01();
        uint32_t intRand();
        uint32_t intRand(uint32_t n);
        double doubleRand();

        int draw(int range);

        void selfTest();
};

LCGRandom::LCGRandom(int32_t seed)
{
    // do a self-test the very first time this class is used
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        selfTest();
    }

    setSeed(seed);
}

void LCGRandom::setSeed(int32_t seed)
{
    if (seed < 1 || seed > GLRAND_MAX)
        throw std::runtime_error("LCGRandom: Invalid seed");

    this->seed = seed;

    // consume some values, so that small seeds will work correctly
//    next01();
//    next01();
//    next01();
}

double LCGRandom::next01()
{
    const long int a = 16807, q = 127773, r = 2836;
    seed = a * (seed % q) - r * (seed / q);
    if (seed <= 0)
        seed += GLRAND_MAX + 1;

    return seed / (double)(GLRAND_MAX + 1);
}

uint32_t LCGRandom::intRand()
{
    numDrawn++;
    const long int a = 16807, q = 127773, r = 2836;
    seed = a * (seed % q) - r * (seed / q);
    if (seed <= 0)
        seed += GLRAND_MAX + 1;
    return seed - 1;  // shift range [1..2^31-2] to [0..2^31-3]
}

uint32_t LCGRandom::intRand(uint32_t n)
{
//    if (n > LCG32_MAX)
//        throw cRuntimeError("cLCG32: intRand(%u): Argument out of range 1..2^31-2", (unsigned)n);

    // code from MersenneTwister.h, Richard J. Wagner rjwagner@writeme.com
    // Find which bits are used in n
    uint32_t used = n - 1;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    // Draw numbers until one is found in [0,n]
    uint32_t i;
    do
        i = intRand() & used;  // toss unused bits to shorten search
    while (i >= n);
    return i;
}

double LCGRandom::doubleRand()
{
    return (double)intRand() * (1.0 / GLRAND_MAX);
}

int LCGRandom::draw(int range)
{
    double d = next01();
    return (int)floor(range*d);
}

void LCGRandom::selfTest()
{
    seed = 1;
    for (int i = 0; i < 10000; i++)
        next01();
    if (seed != 1043618065L)
        throw std::runtime_error("LCGRandom: Self test failed, please report this problem!");
}

class MyCustomApplication : public Application
{
public:
  MyCustomApplication ();
  virtual ~MyCustomApplication();

  void Setup (Address address);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  LCGRandom       random = LCGRandom(42);
};

MyCustomApplication::MyCustomApplication ()
  : m_socket (0),
    m_peer (),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyCustomApplication::~MyCustomApplication()
{
  m_socket = 0;
}

void MyCustomApplication::Setup (Address address)
{
  m_peer = address;
}

void MyCustomApplication::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      m_socket->Bind ();
      m_socket->Connect (m_peer);
      m_socket->SetAttribute ("TcpNoDelay", ns3::BooleanValue (false));
    }
  SendPacket ();
}

void MyCustomApplication::StopApplication (void)
{
  m_running = false;
  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }
  if (m_socket)
    {
      m_socket->Close ();
    }
}

void MyCustomApplication::SendPacket (void)
{
  ScheduleTx ();
  uint32_t packetSize = 1 + random.intRand((uint32_t)1000);
  Ptr<Packet> packet = Create<Packet> (packetSize);
  m_socket->Send (packet);
}

void MyCustomApplication::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (random.doubleRand() * 1E-3));
      m_sendEvent = Simulator::Schedule (tNext, &MyCustomApplication::SendPacket, this);
    }
}

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
    Config::Connect("/NodeList/0/$ns3::TcpL4Protocol/SocketList/*/HighestSequence", MakeCallback(&SndMaxChange));
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

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(536));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
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
  queue = CreateObjectWithAttributes<DropTailQueue<Packet>>("MaxSize", QueueSizeValue(QueueSize("12p")));
  p2pDevice->SetQueue(queue);

  p2pDevice = DynamicCast<PointToPointNetDevice>(dev2.Get(0));
  queue = CreateObjectWithAttributes<DropTailQueue<Packet>>("MaxSize", QueueSizeValue(QueueSize("12p")));
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
  Ptr<MyCustomApplication> app = CreateObject<MyCustomApplication> ();
  app->Setup (InetSocketAddress (iface2.GetAddress (1), remotePort));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (0.0));
  app->SetStopTime (Seconds (1.0));

  Address sinkAddress(InetSocketAddress(iface2.GetAddress(1), remotePort));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
  ApplicationContainer sinkApps = sinkHelper.Install(nodes.Get(2));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(1.0));

  Simulator::Schedule(MilliSeconds(1), &ConnectTraceFunctions);

  p2p1.EnablePcapAll("example5");

  Simulator::Stop (Seconds (1.0));
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

  std::ofstream ofs3("ns3-snd-max.dat", std::ios::out);
  for(auto &x : sndMaxOverTime)
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

