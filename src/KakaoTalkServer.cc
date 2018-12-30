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
#include "ns3/address-utils.h"

using namespace ns3;

class KakaoTalkServer: public Application
{
public:
  KakaoTalkServer();
  virtual ~KakaoTalkServer();

  void setSocket(Ptr<Socket> socket);
  void setLocal(InetSocketAddress address);

  void recvPacket(Ptr<Socket> socket);
  void accept(Ptr<Socket> socket, const Address& from);
  void successClose(Ptr<Socket> socket);

protected:
  virtual void doDispose(void);

private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  void readPacket(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  InetSocketAddress m_local;
  bool m_running;
};

KakaoTalkServer::KakaoTalkServer():
m_socket(0),
m_local((uint16_t)0),
m_running(false)
{
}

KakaoTalkServer::~KakaoTalkServer()
{
  m_socket = 0;
}

void KakaoTalkServer::setSocket(Ptr<Socket> socket)
{
  m_socket = socket;
}

void KakaoTalkServer::setLocal(InetSocketAddress addr)
{
  m_local = addr;
}

void KakaoTalkServer::doDispose(void)
{
  Application::DoDispose();
}

void KakaoTalkServer::StartApplication(void)
{
  m_running = true;
  if (m_socket != 0) {
    m_socket->Bind(m_local);
    m_socket->Listen();

    m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
  MakeCallback(&KakaoTalkServer::accept, this));
  }
}

void KakaoTalkServer::StopApplication(void)
{
  m_running = false;

  if (m_socket) {
    m_socket->Close();
    m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
  MakeNullCallback<void, Ptr<Socket>, const Address&>());
  }
}

void KakaoTalkServer::recvPacket(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;

  while (packet = socket->RecvFrom(from)) {
    if (packet->GetSize() > 0) {
      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      socket->Send(packet);
    }
  }
}

void KakaoTalkServer::accept(Ptr<Socket> socket, const Address& from)
{
  socket->SetRecvCallback(MakeCallback(&KakaoTalkServer::recvPacket, this));
  socket->SetCloseCallbacks(MakeCallback(&KakaoTalkServer::successClose, this), MakeNullCallback<void, Ptr<Socket>>());
}

void KakaoTalkServer::successClose(Ptr<Socket> socket)
{
  socket->Close();
  socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
  socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(), MakeNullCallback<void, Ptr<Socket>>());
}
