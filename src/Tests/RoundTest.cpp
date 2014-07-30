#include "DissentTest.hpp"
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

  typedef bool (*BadGuyCB)(Round *);

  template<typename T> bool TBadGuyCB(Round *pr)
  {
    T *pt = dynamic_cast<T *>(pr);
    if(pt) {
      return pt->Triggered();
    }
    return false;
  }

  void TestRoundBad(CreateRound good_cr, CreateRound bad_cr,
      const BadGuyCB &callback, bool client, bool will_finish)
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

    if(!client) {
      badguy = Random::GetInstance().GetInt(0, servers);
      badid = net.first[badguy]->GetId();
    }

    qDebug() << "Bad guy at" << badguy << badid;
    QSharedPointer<AsymmetricKey> key =
      sessions.private_keys[badid.ToString()];

    if(client) {
      ClientPointer cs = MakeSession<ClientSession>(
          net.second[badguy], key, sessions.keys, bad_cr);
      cs->SetSink(sessions.sink_multiplexers[servers + badguy].data());
      sessions.clients[badguy] = cs;
    } else {
      ServerPointer ss = MakeSession<ServerSession>(
          net.first[badguy], key, sessions.keys, bad_cr);
      ss->SetSink(sessions.sink_multiplexers[badguy].data());
      sessions.servers[badguy] = ss;
    }

    // Find a sender != badguy
    int sender = Random::GetInstance().GetInt(0, clients);
    if(client) {
      while(sender == badguy) {
        sender = Random::GetInstance().GetInt(0, clients);
      }
    }
    QByteArray msg(64, 0);
    CryptoRandom().GenerateBlock(msg);
    sessions.clients[sender]->Send(msg);

    qDebug() << "Starting sessions...";
    StartSessions(sessions);
    StartRound(sessions);

    QSharedPointer<Round> bad_round;
    if(client) {
      bad_round = sessions.clients[badguy]->GetRound();
    } else {
      bad_round = sessions.servers[badguy]->GetRound();
    }

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
    if(will_finish) {
      ASSERT_EQ(sc.GetCount(), clients + servers);
      ASSERT_EQ(bad_round->GetBadMembers().size(), 1);
      ASSERT_EQ(badid, bad_round->GetBadMembers()[0]);
    }

    if(!callback(bad_round.data())) {
      std::cout << "RoundTest_BadGuy was never triggered, "
        "consider rerunning." << std::endl;
    }

    StopNetwork(sessions.network);
    VerifyStoppedNetwork(sessions.network);
    ConnectionManager::UseTimer = true;
  }

  template <int N> class CSDCNetRoundBad : public CSDCNetRound, public Triggerable {
    public:
      explicit CSDCNetRoundBad(const Identity::Roster &clients,
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
        if(N >= 0 && N <= 1) {
          return CSDCNetRound::GenerateCiphertext();
        }

        QByteArray msg = CSDCNetRound::GenerateCiphertext();
        if(msg.size() == GetState()->base_msg_length) {
          qDebug() << "No damage done";
          return msg;
        }

        int offset = Random::GetInstance().GetInt(GetState()->base_msg_length + 1, msg.size());
        msg[offset] = msg[offset] ^ 0xff;

        if(IsServer()) {
          QSharedPointer<CSDCNetRound::State> cstate = GetState();
          QSharedPointer<CSDCNetRound::ServerState> state =
            cstate.dynamicCast<CSDCNetRound::ServerState>();
          int bc = Random::GetInstance().GetInt(0, state->anonymous_rngs.size());
          state->current_phase_log->my_sub_ciphertexts[bc][offset] =
            state->current_phase_log->my_sub_ciphertexts[bc][offset] ^ 0xff;
        }

        qDebug() << "up to no good";
        Triggerable::SetTriggered();
        return msg;
      }

      virtual void GenerateServerCiphertext()
      {
        switch(N) {
          case 0:
            GenerateBadServerCiphertext();
            break;
          case 1:
            GenerateBadClientCiphertext();
            break;
          default:
            CSDCNetRound::GenerateServerCiphertext();
        }
      }

    private:
      void GenerateBadServerCiphertext()
      {
        CSDCNetRound::GenerateServerCiphertext();
        QSharedPointer<CSDCNetRound::State> cstate = GetState();
        QSharedPointer<CSDCNetRound::ServerState> state =
          cstate.dynamicCast<CSDCNetRound::ServerState>();

        int size = state->my_ciphertext.size();
        if(size == GetState()->base_msg_length) {
          qDebug() << "No damage done";
          return;
        }

        int offset = Random::GetInstance().GetInt(GetState()->base_msg_length + 1, size);
        state->my_ciphertext[offset] = state->my_ciphertext[offset] ^ 0xff;
        state->my_commit = Hash().ComputeHash(state->my_ciphertext);
        qDebug() << "up to no good";
        Triggerable::SetTriggered();
      }

      /**
       * This attack currently succeeds:
       * See line 1341 in CSDCNetRound
       */
      void GenerateBadClientCiphertext()
      {
        QSharedPointer<CSDCNetRound::State> cstate = GetState();
        QSharedPointer<CSDCNetRound::ServerState> state =
          cstate.dynamicCast<CSDCNetRound::ServerState>();

        int mlen = state->msg_length;
        if(mlen == state->base_msg_length) {
          qDebug() << "No damage done";
          return CSDCNetRound::GenerateServerCiphertext();
        }

        int size = state->client_ciphertexts.size();
        if(size == 0) {
          qDebug() << "No damage done";
          return CSDCNetRound::GenerateServerCiphertext();
        }

        int tochange = Random::GetInstance().GetInt(0, size);
        QPair<int, QByteArray> &data = state->client_ciphertexts[tochange];
        int offset = Random::GetInstance().GetInt(GetState()->base_msg_length + 1, mlen);
        data.second[offset] = data.second[offset] ^ 0xff;
        state->current_phase_log->messages[data.first][offset] = data.second[offset];
        CSDCNetRound::GenerateServerCiphertext();

        qDebug() << "up to no good";
        Triggerable::SetTriggered();
      }

      void GenerateMatchingCiphertext()
      {
      }
  };

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

  TEST(CSDCNetRound, BadClient)
  {
    typedef CSDCNetRoundBad<-1> bad;
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NullRound>,
        TCreateDCNetRound<bad, NullRound>,
        TBadGuyCB<bad>, true, true);
  }

  TEST(CSDCNetRound, BadServerBadServerInputCiphertext)
  {
    typedef CSDCNetRoundBad<-1> bad;
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NullRound>,
        TCreateDCNetRound<bad, NullRound>,
        TBadGuyCB<bad>, false, true);
  }

  TEST(CSDCNetRound, BadServerBadServerCiphertext)
  {
    typedef CSDCNetRoundBad<0> bad;
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NullRound>,
        TCreateDCNetRound<bad, NullRound>,
        TBadGuyCB<bad>, false, false);
  }

  /*
   * This attack currently succeeds ... let's not test it
  TEST(CSDCNetRound, BadServerBadClientCiphertext)
  {
    typedef CSDCNetRoundBad<1> bad;
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NullRound>,
        TCreateDCNetRound<bad, NullRound>,
        TBadGuyCB<bad>, false, true);
  }
   */

  TEST(CSDCNetRound, BadClientNeff)
  {
    typedef CSDCNetRoundBad<-1> bad;
    TestRoundBad(TCreateDCNetRound<CSDCNetRound, NeffKeyShuffleRound>,
        TCreateDCNetRound<bad, NeffKeyShuffleRound>,
        TBadGuyCB<bad>, true, true);
  }
}
}
