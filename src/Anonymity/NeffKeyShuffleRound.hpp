//#ifdef FAST_NEFF_SHUFFLE
//#include "FastNeffKeyShuffleRound.hpp"
//#else
#ifndef DISSENT_ANONYMITY_NEFF_KEY_SHUFFLE_ROUND_H_GUARD
#define DISSENT_ANONYMITY_NEFF_KEY_SHUFFLE_ROUND_H_GUARD

#include "Crypto/DsaPublicKey.hpp"
#include "NeffShuffleRound.hpp"

namespace Dissent {
namespace Anonymity {

  /**
   * Wrapper around NeffShuffleRound to make keys easier to access.
   * Also API compatible with the old NeffKeyShuffle
   */
  class NeffKeyShuffleRound : public NeffShuffleRound {
    public:
      /**
       * Constructor
       * @param clients the list of clients in the round
       * @param servers the list of servers in the round
       * @param ident this participants private information
       * @param nonce Unique round id (nonce)
       * @param overlay handles message sending
       * @param get_data requests data to share during this session
       */
      explicit NeffKeyShuffleRound(const Identity::Roster &clients,
          const Identity::Roster &servers,
          const Identity::PrivateIdentity &ident,
          const QByteArray &nonce,
          const QSharedPointer<ClientServer::Overlay> &overlay,
          Messaging::GetDataCallback &get_data) :
        NeffShuffleRound(clients, servers, ident, nonce, overlay, get_data,
            true, 1024),
        _parsed(false),
        _key_index(-1) { }

      /**
       * Destructor
       */
      virtual ~NeffKeyShuffleRound() {}

      /**
       * Returns the anonymized private key
       */
      QSharedPointer<Crypto::AsymmetricKey> GetKey() const
      {
        if(const_cast<NeffKeyShuffleRound *>(this)->Parse()) {
          return GetState()->private_key;
        } else {
          return QSharedPointer<Crypto::AsymmetricKey>();
        }
      }

      /**
       * Returns the list of shuffled keys
       */
      QVector<QSharedPointer<Crypto::AsymmetricKey> > GetKeys() const
      {
        if(const_cast<NeffKeyShuffleRound *>(this)->Parse()) {
          return _keys;
        } else {
          return QVector<QSharedPointer<Crypto::AsymmetricKey> >();
        }
      }

      /**
       * Returns the index in the shuffle for the anonymized proivate key
       */
      int GetKeyIndex() const
      {
        if(const_cast<NeffKeyShuffleRound *>(this)->Parse()) {
          return _key_index;
        } else {
          return -1;
        }
      }

    private:
      bool Parse()
      {
        if(_parsed) {
          return true;
        } else if(!Successful()) {
          return false;
        }

        QSharedPointer<Crypto::DsaPublicKey> my_key(GetState()->private_key->
            GetPublicKey().dynamicCast<Crypto::DsaPublicKey>());
        Integer modulus = my_key->GetModulus();
        Integer subgroup = my_key->GetSubgroupOrder();
        Integer generator = my_key->GetGenerator();

        for(int idx = 0; idx < GetState()->cleartext.size(); idx++) {
          const QByteArray  &ct = GetState()->cleartext[idx];
          Integer public_element(ct);
          QSharedPointer<Crypto::AsymmetricKey> key(new Crypto::DsaPublicKey(
                modulus, subgroup, generator, public_element));
          _keys.append(key);
          if(key == my_key) {
            _key_index = idx;
          }
        }

        _parsed = true;
        return true;
      }

      bool _parsed;
      QVector<QSharedPointer<Crypto::AsymmetricKey> > _keys;
      int _key_index;
  };
}
}

#endif
//#endif
