#ifndef ByteStream_h
#define ByteStream_h

#include <Stream.h>

/**
 * Byte Stream which can hold '\0' unlike String
 * 
 * byte buffer[100];
 * ByteStream streamer(buffer);
 * 
 * inspired by https://gist.github.com/cmaglie/5883185
 */
class ByteStream : public Stream
{
public:
  ByteStream(uint8_t* pBuffer) : pBuffer_(pBuffer), position_(0), length_(0) { }

  // Stream methods
  virtual int available() { return length_ - position_; }
  virtual int read() { return position_ < length_ ? pBuffer_[position_++] : -1; }
  virtual int peek() { return position_ < length_ ? pBuffer_[position_] : -1; }
  virtual void flush() { };
  
  // Print methods
  virtual size_t write(uint8_t c) { pBuffer_[length_++] = c; return 1; };

private:
  uint8_t *pBuffer_;
  unsigned int position_; // position to read
  unsigned int length_;
};

#endif

