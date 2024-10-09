#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include "microTuple.h"

String S;

long seko = millis();
struct EKOPrinter {
  virtual void println(const String &ss) {
    Serial.println(ss);
  }
};

EKOPrinter *eko_printer = new EKOPrinter();
void println(const String &ss) { eko_printer->println(ss); }

#if NOEKO == 1
#define EKOT(x) 
#define EKOX(x) 
#define EKO() 
#else
#define EKOT(x) println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()
#endif



#define G(x) (S + "\"" + String(x) + "\"")
#define P(f,v) G(f) + " : " + G(v)
#define Acc(x) S + "{ " + x + " }"


MicroTuple<String, String> split(const String &mess, const String &sep = "?") {
  auto index = mess.indexOf(sep);
  return MicroTuple<String, String>(mess.substring(0, index), mess.substring(index+1));
} 

#endif
