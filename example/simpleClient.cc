/*
+---------------------+         +----------------------+
|   KakaoTalkClient   |<------->|   PacketSinkHelper   |
+---------------------+         +----------------------+
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
      std::cout << "Packet: " << std::endl;
      p->Print(std::cout);
  }
}


int
main (int argc, char *argv[])
{

  // CommandLine ?~]체를 ?~]??~Z??~U~X면 커맨?~S~\?~]??~]??~]~D ?~F??~U? ?~L~L?~]?미?~D?를 ?~[?|  ?~B??~Z??~U|  ?~H~XX
 ?~^~H?~K?.
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // ?~D??~J??~[~L?~A? ?~F| ?~O??~\?~@?~W~P ?~O??~U??~P~X?~J~T 2?~\?~]~X ?~E??~S~\를 ?~L?~S| ?~K?.
  NodeContainer nodes;
  nodes.Create (2);

  /*
  2?~\?~]~X ?~E??~S~\를 ?~W?결?~U??~D point-to-point ?~D?~D~P?~]~D ?~L?~S| ?~K?.
  point-to-point ?~D?~D~P?~]~@ 5Mbps?~]~X DataRate를 ?~@?~@?| , 2ms?~]~X Delay를 ?~V?~J~T?~K?.
  */
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  /*
  ?~E??~S~\ 2?~\를 point-to-point?~\ ?~W?결?~U~\?~K?.
  point-to-point?~\ ?~E??~S~\를 ?~W?결?~U~X?~L ?~P~X면 NIC?~W~P ?~U??~K??~U~X?~J~T device ?~]체를 ?~V??~]~D ?~H~X ?~^~HH
?~K?.
  */
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  /*
  point-to-point ?~D?~D~P?~W~P ErrorRate를 주기 ?~\~D?~U? ?~]체를 ?~D| ?~V??~U~\?~K?.
  ErrorRate를 0.001%?~\ ?~D??| ~U?~U~X?|  ?~D~\?~D쪽?~W~P ErrorRate를 ?~D??| ~U?~U??~@?~K?.
  */
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  // TCP/IP ?~J??~C~]?~]~D 2?~\?~]~X ?~E??~S~\?~W~P ?~D??~X?~U??~@?~K?.
  InternetStackHelper stack;
  stack.Install (nodes);

  // 2?~\?~]~X ?~E??~S~\?~W~P ?~D~\?~L?~D? ?~U~D?~]??~T~T?~@ "10.1.1.0", ?~D~\?~L?~D? ?~H?~J??~A??~@ "255.255.255.0"?~]?
ip를 ?~U| ?~K??~U??~@?~K?.
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  /*
  ?~D~\?~D?~W~P?~D~\ ?~B??~Z??~U|  SinkApplication?~]~D ?~L?~S??~V??~@?~K?.
  SinkApplication?~]~X ?~O??~J??~Y~@ ip를 ?~D??| ~U?~U??~@?~K?.
  ?~V??~T~L리?~@?~]??~E~X ?~F~L?~S?~]~@ TCP ?~F~L?~S?~]~D ?~B??~Z??~U|  ?~C?~]??~]??|  ?~D~X겨?~@?~K?.
  SinkApplication?~]~@ 0?~H?~W~P ?~K??~V~I?~U??~D~\ 20?~H?~W~P ?~E?~L?~U~\?~K?.
  */
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  /*
  ?~]? ?~K~\?~B~X리?~X??~W~P?~D~\ ?~B??~Z??~^~P?~@ ?| ~U?~]~X?~U~\ ?~V??~T~L리?~@?~]??~E~X?~]? ?~B??~Z??~U|  TCP ?~F~L?~S??
~]~D ?~L?~S| ?~K?.
  ?~L?~S|  TCP ?~F~L?~S ?~]체?~W~P Tracing?~]~D ?~T?~@?~U~X?~W? CWND ?~@?~Y~T를 ?~\?| ??~U~\?~K?.
  */
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

    /*
    ?~U~^?~D~\ ?| ~U?~]~X?~U~\ ?~V??~T~L리?~@?~]??~E~X?~]~X ?~]체를 ?~L?~S??| , ?~V??~T~L리?~@?~]??~E~X?~]~X property를?
   ?~D??| ~U?~U??~@?~K?.
    ?~V??~T~L리?~@?~]??~E~X?~]~D 첫?~H째 ?~E??~S~\?~W~P ?~D??~X?~U~X?|  1?~H?~W~P ?~V??~T~L리?~@?~]??~E~X?~]~D ?~K~\?~^^
  ~Q?~U~X?|  20?~H?~W~P ?~V??~T~L리?~@?~]??~E~X?~]~D ?~E?~L?~U~\?~K?.
    */
  Ptr<KakaoTalkClient> app = CreateObject<KakaoTalkClient> ();
  app->setSocket (ns3TcpSocket);
  app->setServer (sinkAddress);
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

  pointToPoint.EnablePcapAll("Kakao_Message");

    /*
    point-to-point ?~D?~D~P?~]~D ?~]??~Z??~U~X?~W? ?~Q~P ?~E??~S~\를 ?~W?결?~U~X면 ?~V??~]~D ?~H~X ?~^~H?~J~T Device ?~]체??
  ~W~P Tracing?~]~D ?~T?~@?~U~X?~W? packet loss를 ?~\?| ??~U~\?~K?.
    */
  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
