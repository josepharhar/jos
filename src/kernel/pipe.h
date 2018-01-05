#ifndef PIPE_H_
#define PIPE_H_

class Pipe {
 public:
  virtual int Write(const uint8_t* source_buffer, int write_size) = 0;
  virtual int Read(uint8_t* dest_buffer, int read_size) = 0;
  //virtual void Close() = 0;
  
  /*enum Mode {
    WRITE_ONLY = 1;
    READ_ONLY = 2;
    READ_AND_WRITE = 3;
  };
  virtual Mode GetMode() = 0;*/
};

#endif  // PIPE_H_
