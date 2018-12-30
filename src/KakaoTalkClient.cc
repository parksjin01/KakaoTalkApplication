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
