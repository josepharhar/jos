#ifndef SERIAL_H_
#define SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

void SerialInit();
void SerialWriteChar(char character);
void SerialWriteString(const char* string);
void SerialWrite(const char* buff, int len);

void SerialHandleInterrupt();

#ifdef __cplusplus
}
#endif

#endif  // SERIAL_H_
