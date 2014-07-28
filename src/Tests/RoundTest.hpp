#ifndef DISSENT_TEST_ROUND_TEST_H_GUARD
#define DISSENT_TEST_ROUND_TEST_H_GUARD

#include "DissentTest.hpp"

namespace Dissent {
namespace Tests {
  typedef bool (*BadGuyCB)(Round *);

  template<typename T> bool TBadGuyCB(Round *pr)
  {
    T *pt = dynamic_cast<T *>(pr);
    if(pt) {
      return pt->Triggered();
    }
    return false;
  }

  class RoundCollector : public QObject {
    Q_OBJECT

    public:
      QVector<QSharedPointer<Anonymity::Round> > rounds;

    public slots:
      void RoundFinished(const QSharedPointer<Anonymity::Round> &round)
      {
        rounds.append(round);
      }
  };
}
}

#endif
