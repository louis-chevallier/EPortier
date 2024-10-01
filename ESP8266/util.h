#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED


String S;

long seko = millis();

void println(const String &ss) {
  Serial.println(ss);
}

#if NOEKO == 1
#define EKOT(x) 
#define EKOX(x) 
#define EKO() 
#else
#define EKOT(x) println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()
#endif
#endif
