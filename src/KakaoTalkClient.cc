/********************* Header *********************/
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

Class KakaoTalkClient: public Application
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
}

KakaoTalkClient::KakaoTalkClient():
socket(0),
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
