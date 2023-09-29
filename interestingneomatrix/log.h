#ifdef SERIAL
  #define LOG Serial
#else
  #include <TelnetStream.h>
  #define LOG TelnetStream
#endif
