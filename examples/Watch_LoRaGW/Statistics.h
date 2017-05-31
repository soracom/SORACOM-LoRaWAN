#ifndef STATISTICS_h
#define STATISTICS_h

class Statistics {
public:
  // @return -1 if totalCount() is 0
  double getOkRate() const {
    const unsigned int totalCount = getTotalCount();
    if (totalCount == 0)
      return -1;
    return static_cast<double>(okCount) / totalCount;
  }
  inline unsigned int getTotalCount() const {
    return okCount + errCount;
  }
  inline unsigned int getOkCount() const {
    return okCount;
  }
  inline unsigned int getErrCount() const {
    return errCount;
  }
  inline unsigned int getSequenceCount() const {
    return sequenceCount;
  }
  void addResult(bool isOk) {
    if (isOk) {
      ++okCount;
      if (!isLastOk)
        maxErrMillis = max(maxErrMillis, lastErrMillis - startErrMillis);
    } else {
      ++errCount;
      lastErrMillis = millis();
      if (isLastOk)
        startErrMillis = lastErrMillis;
    }

    if (isOk == isLastOk) {
      ++sequenceCount;
    } else {
      sequenceCount = 1;
    }

    isLastOk = isOk;  
  }
  inline double getMaxErrSecs() const {
    return static_cast<double>(maxErrMillis) / 1000;
  }
  inline double getMaxErrMins() const {
    return static_cast<double>(maxErrMillis) / 1000 / 60;
  }
  inline bool getLastOk() const {
    return isLastOk;
  }
  inline void reset() {
    okCount = 0;
    errCount = 0;
    sequenceCount = 0;
    maxErrMillis = 0;
  }
private:
  bool isLastOk;
  // If you add fields, review reset() and consider if the new fields should also be reset in it.
  unsigned int okCount = 0;
  unsigned int errCount = 0;
  unsigned int sequenceCount = 0;
  unsigned long startErrMillis;
  unsigned long lastErrMillis;
  unsigned long maxErrMillis = 0;
};

#endif

