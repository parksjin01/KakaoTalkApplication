/*
+---------------------+         +----------------+
|   KakaoTalkClient   |<------->|   EchoServer   |
+---------------------+         +----------------+
 */

/********************* Header *********************/
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/log.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;

class KakaoTalkClient: public Application
{
public:
    KakaoTalkClient();
    virtual ~KakaoTalkClient();

    void setSocket (Ptr<Socket> socket);
    void setServer (Address address);

private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  Ptr<Packet> createPacket(std::string message);
  void sendPacket(Ptr<Packet> packet);
  void recvPacket(Ptr<Socket> socket);

  Ptr<Socket> socket;
  Address server_address;
};

KakaoTalkClient::KakaoTalkClient()
:socket(0),
server_address()
{
}

KakaoTalkClient::~KakaoTalkClient()
{
  socket = 0;
}

void KakaoTalkClient::setSocket(Ptr<Socket> sock)
{
  socket = sock;
}

void KakaoTalkClient::setServer(Address addr)
{
  server_address = addr;
}

void KakaoTalkClient::StartApplication(void)
{
  socket->Bind();
  socket->Connect(server_address);
  sendPacket(createPacket("Hello World"));
  socket->SetRecvCallback(MakeCallback(&KakaoTalkClient::recvPacket, this));
}

void KakaoTalkClient::StopApplication(void)
{
  if (socket) {
      socket->Close();
  }
}

Ptr<Packet> KakaoTalkClient::createPacket(std::string str)
{
  uint32_t size = str.size() + 1;
  uint8_t *data = new uint8_t[size];
  memcpy(data, str.c_str(), size);
  Ptr<Packet> packet = Create<Packet>(data, size);
  return packet;
}

void KakaoTalkClient::sendPacket(Ptr<Packet> packet)
{
  socket->Send(packet);
}

void KakaoTalkClient::recvPacket(Ptr<Socket> socket)
{
  Ptr<Packet> p;
  while ((p = socket->RecvFrom(server_address))) {
      std::ostringstream convert;
      uint8_t *buffer = new uint8_t[p->GetSize ()]; 
      p->CopyData (buffer, p->GetSize ());

      for(uint32_t i = 0; i < p->GetSize(); i++)
        convert << buffer[i];

      std::cout << "Packet: " << convert.str() << std::endl;
  }
}


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t sinkPort = 9;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  // PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  TcpEchoServerHelper echoServer (9);
  ApplicationContainer sinkApps = echoServer.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

  Ptr<KakaoTalkClient> app = CreateObject<KakaoTalkClient> ();
  app->setSocket (ns3TcpSocket);
  app->setServer (sinkAddress);
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

  pointToPoint.EnablePcapAll("Kakao_Message_reply");

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
