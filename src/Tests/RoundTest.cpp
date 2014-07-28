#include "DissentTest.hpp"
#include "RoundTest.hpp"
#include "OverlayTest.hpp"
#include "SessionTest.hpp"

namespace Dissent {
namespace Tests {
  void TestRoundBasic(CreateRound create_round)
  {
    int servers = 3, clients = 10;
    ConnectionManager::UseTimer = false;
    Timer::GetInstance().UseVirtualTime();
    OverlayNetwork net = ConstructOverlay(servers, clients);
    VerifyStoppedNetwork(net);
    StartNetwork(net);
    VerifyNetwork(net);

    Sessions sessions = BuildSessions(net, create_round);
    qDebug() << "Starting sessions...";
    StartSessions(sessions);
    SendTest(sessions);
    SendTest(sessions);
    DisconnectServer(sessions, true);
    SendTest(sessions);
    DisconnectServer(sessions, false);
    SendTest(sessions);
    SendTest(sessions);
    StopSessions(sessions);

    StopNetwork(sessions.network);
    VerifyStoppedNetwork(sessions.network);
    ConnectionManager::UseTimer = true;
  }

  void TestRoundBad(CreateRound good_cr, CreateRound bad_cr, const BadGuyCB &callback)
  {
    int servers = 3, clients = 10;
    ConnectionManager::UseTimer = false;
    Timer::GetInstance().UseVirtualTime();
    OverlayNetwork net = ConstructOverlay(servers, clients);
    VerifyStoppedNetwork(net);
    StartNetwork(net);
    VerifyNetwork(net);

    Sessions sessions = BuildSessions(net, good_cr);
    // Find a bad guy and replace him...
    int badguy = Random::GetInstance().GetInt(0, clients);
    Id badid = net.second[badguy]->GetId();
    qDebug() << "Bad guy at" << badguy << badid;
    QSharedPointer<AsymmetricKey> key = sessions.private_keys[badid.ToString()];
    ClientPointer cs = MakeSession<ClientSession>(
        net.second[badguy], key, sessions.keys, bad_cr);
    cs->SetSink(sessions.sink_multiplexers[servers + badguy].data());
    sessions.clients[badguy] = cs;
    RoundCollector rc;
    QObject::connect(sessions.clients[badguy].data(),
        SIGNAL(RoundFinished(const QSharedPointer<Anonymity::Round> &)),
        &rc, SLOT(RoundFinished(const QSharedPointer<Anonymity::Round> &)));

    qDebug() << "Starting sessions...";
    StartSessions(sessions);

    // Find a sender != badguy
    int sender = Random::GetInstance().GetInt(0, clients);
    while(sender == badguy) {
      sender = Random::GetInstance().GetInt(0, clients);
    }
    QByteArray msg(64, 0);
    CryptoRandom().GenerateBlock(msg);
    sessions.clients[sender]->Send(msg);

    SignalCounter sc;
    for(int idx = 0; idx < servers; idx++) {
      QObject::connect(sessions.servers[idx].data(),
          SIGNAL(RoundFinished(const QSharedPointer<Anonymity::Round> &)),
          &sc, SLOT(Counter()));
    }

    for(int idx = 0; idx < clients; idx++) {
      QObject::connect(sessions.clients[idx].data(),
          SIGNAL(RoundFinished(const QSharedPointer<Anonymity::Round> &)),
          &sc, SLOT(Counter()));
    }

    RunUntil(sc, clients + servers);
    if(!callback(rc.rounds[0].data())) {
      std::cout << "RoundTest_BadGuy was never triggered, "
        "consider rerunning." << std::endl;
    }
/*
      QObject::connect(nodes[idx]->session.data(), SIGNAL(RoundFinished(const QSharedPointer<Round> &)),
          &rc, SLOT(RoundFinished(const QSharedPointer<Round> &)));
      QObject::connect(&nodes[idx]->sink, SIGNAL(DataReceived()),
          &sc, SLOT(Counter()));
      nodes[idx]->session->Start();
    }

    count -= 1;
    RunUntil(sc, count);

    if(!cb(nodes[badguy]->first_round.data())) {
      std::cout << "RoundTest_BadGuy was never triggered, "
        "consider rerunning." << std::endl;
    } else {
      for(int idx = 0; idx < nodes.size(); idx++) {
        TestNode *node = nodes[idx];
        QSharedPointer<Round> pr = node->first_round;
        if(node->ident.GetSuperPeer()) {
          EXPECT_EQ(pr->GetBadMembers().count(), 1);
        }
        EXPECT_FALSE(pr->Successful());

        if(idx == badguy) {
          continue;
        }

        EXPECT_FALSE(node->session->GetGroup().Contains(badid));
        EXPECT_TRUE(node->sink.Count() == 1);
        if(node->sink.Count() == 1) {
          EXPECT_EQ(node->sink.Last().second, msg);
        }
      }
    }
*/
    StopNetwork(sessions.network);
    VerifyStoppedNetwork(sessions.network);
    ConnectionManager::UseTimer = true;
  }

  TEST(NeffShuffleRound, Basic)
  {
    TestRoundBasic(TCreateRound<NeffShuffleRound>);
  }

  TEST(CSDCNetRound, Basic)
  {
    TestRoundBasic(TCreateDCNetRound<CSDCNetRound, NullRound>);
  }

  TEST(CSDCNetRound, Neff)
  {
    TestRoundBasic(TCreateDCNetRound<CSDCNetRound, NeffKeyShuffleRound>);
  }

  class CSDCNetRoundBadClient : public CSDCNetRound, public Triggerable {
    public:
      explicit CSDCNetRoundBadClient(const Identity::Roster &clients,
          const Identity::Roster &servers,
          const Identity::PrivateIdentity &ident,
          const QByteArray &nonce,
          const QSharedPointer<ClientServer::Overlay> &overlay,
          Messaging::GetDataCallback &get_data,
          CreateRound create_shuffle) :
        CSDCNetRound(clients, servers, ident, nonce, overlay, get_data,
            create_shuffle)
      {
      }

      inline virtual QString ToString() const
      {
        return CSDCNetRound::ToString() + " BAD!";
      }

    protected:
      virtual QByteArray GenerateCiphertext()
      {
        QByteArray msg = CSDCNetRound::GenerateCiphertext();
        if(msg.size() == GetState()->base_msg_length) {
          qDebug() << "No damage done";
          return msg;
        }

        int offset = Random::GetInstance().GetInt(GetState()->base_msg_length + 1, msg.size());
        msg[offset] = msg[offset] ^ 0xff;
        qDebug() << "up to no good";
        Triggerable::SetTriggered();
        return msg;
      }
  };

  TEST(CSDCNetRound, Bad)
  {
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NullRound>,
        TCreateDCNetRound<CSDCNetRoundBadClient, NullRound>,
        TBadGuyCB<CSDCNetRoundBadClient>);
  }

  TEST(CSDCNetRound, BadNeff)
  {
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NeffKeyShuffleRound>,
        TCreateDCNetRound<CSDCNetRoundBadClient, NeffKeyShuffleRound>,
        TBadGuyCB<CSDCNetRoundBadClient>);
  }
}
}
