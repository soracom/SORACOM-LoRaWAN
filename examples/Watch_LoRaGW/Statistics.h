#ifndef STATISTICS_h
#define STATISTICS_h

class Statistics {
public:
  double getOkRate() {
    const int totalCount = getTotalCount();
    if (totalCount == 0)
      return -1;
    return static_cast<double>(okCount) / totalCount;
  }
  inline int getTotalCount() {
    return okCount + errCount;
  }
  inline int getOkCount() const {
    return okCount;
  }
  inline int getErrCount() const {
    return errCount;
  }
  inline int getSequenceCount() {
    return sequenceCount;
  }
  void addResult(bool isOk) {
    if (isOk) {
      ++okCount;
    } else {
      ++errCount;
    }

    if (isOk == isLastOk) {
      ++sequenceCount;
    } else {
      sequenceCount = 1;
    }

    isLastOk = isOk;  
  }
  inline bool getLastOk() {
    return isLastOk;
  }
  inline void reset() {
    okCount = 0;
    errCount = 0;
    sequenceCount = 0;
  }
private:
  bool isLastOk;
  int okCount = 0;
  int errCount = 0;
  int sequenceCount = 0;
};

#endif

