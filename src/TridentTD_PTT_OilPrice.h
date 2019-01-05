#ifndef __TRIDENTTD_PTTOIL_H__
#define __TRIDENTTD_PTTOIL_H__

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined (ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

typedef struct _PTT_OilPrice_t {
  time_t timestamp;  // UTC timestamp
  char  date[11];
  char  time[9];
  char  product[30];
  float  price;
} PTT_OilPrice_t;

class TridentTD_PTT_OilPrice {
  public:
    TridentTD_PTT_OilPrice();
    static int getCurrentOilPrice(PTT_OilPrice_t **oilprice);
  private:
};

extern TridentTD_PTT_OilPrice PTT;
#endif
