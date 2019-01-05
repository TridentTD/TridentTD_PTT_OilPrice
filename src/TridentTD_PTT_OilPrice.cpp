#include "TridentTD_PTT_OilPrice.h"
#include <time.h>
#include <vector>
#include <algorithm>

static std::vector<PTT_OilPrice_t> OilList;
static String HTTP_POST_BODY = R"(<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
  <soap:Body>
    <CurrentOilPrice xmlns="http://www.pttplc.com/ptt_webservice/">
      <Language>string</Language>
    </CurrentOilPrice>
  </soap:Body>
</soap:Envelope>

)";

TridentTD_PTT_OilPrice::TridentTD_PTT_OilPrice(){
  OilList.clear();  
}

static String xml(String in, String key, int* start_index= NULL){
  String start_key = String("<") + key  + String(">");
  String end_key = String("</") + key  + String(">");

  int idx_start = in.indexOf(start_key, (start_index == NULL)? 0: *start_index )+start_key.length();  
  if( start_index != NULL) {
    if( idx_start < *start_index  ) {  *start_index = in.length();  return ""; }
  }
  
  int idx_stop  = in.indexOf(end_key, idx_start );

  if(start_index != NULL){ *start_index = idx_stop + end_key.length(); }

  String value = in.substring( idx_start, idx_stop); 
  value.trim();
  return value;
}

static String xmlcleanup(String &in ) {
  in.replace("&lt;" , "<");
  in.replace("&gt;" , ">");
  return in;
}

int str_td_split(char* buffer, size_t len, String delimiter, String** str_array) {
  if(*str_array != NULL) {
    delete[] *str_array;
    *str_array = NULL;
  }

  if(!buffer || len == 0) {
    *str_array = new String[1];
    (*str_array)[0] = "";
    return 0;
  }

  String input = String(buffer); 
  int token_size = 0;

  char *pChar = strtok( (char*)input.c_str(), (char*)delimiter.c_str());
  while ( pChar != NULL ) {
    pChar = strtok( NULL, (char*)delimiter.c_str());
    ++token_size;
  }
  
  if(token_size == 0) {
    *str_array = new String[1]; (*str_array)[0] = input;
    return 1;
  }

  *str_array = new String[token_size];
  input = String(buffer); 
  token_size = 0;

  pChar = strtok( (char*) input.c_str(), (char*)delimiter.c_str());
  while ( pChar != NULL ) {
    (*str_array)[token_size] = String(pChar);
    pChar = strtok( NULL, (char*)delimiter.c_str());
    ++token_size;
  }
  return token_size;
}

static String *str = NULL;
static time_t iso8601_timestamp(String iso8601_datetime_string) {
  int count = str_td_split((char*)iso8601_datetime_string.c_str(), iso8601_datetime_string.length(), "-T:+", &str);
  if(count != 8) return 0;

  struct tm tm;
  tm.tm_year = str[0].toInt() - 1900;
  tm.tm_mon  = str[1].toInt() - 1;
  tm.tm_mday = str[2].toInt();
  tm.tm_hour = str[3].toInt();
  tm.tm_min  = str[4].toInt();
  tm.tm_sec  = str[5].toInt();
  time_t timezone_sec = str[6].toInt()*3600 + str[7].toInt()*60;
  time_t ts = mktime(&tm); //+ timezone_sec;

  return ts;
}


static bool sort_compare(const _PTT_OilPrice_t a, const _PTT_OilPrice_t b){
   return ( strcmp(a.product, b.product)< 0); 
}

static void XML_parser(String data ) {
  OilList.clear();
  
  xmlcleanup(data);

  int idx = 0;
  while (idx < data.length()) {
    String DataAccess =  xml(data, "DataAccess", &idx);

    int idx2=0;
    String datetime_iso8601 = xml(DataAccess , "PRICE_DATE" , &idx2);
    
    time_t t  = iso8601_timestamp(datetime_iso8601);
    struct tm tm; localtime_r(&t,&tm);

    PTT_OilPrice_t new_data;
    
    new_data.timestamp  = t;
    sprintf(new_data.date , "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon+1, tm.tm_year + 1900);
    sprintf(new_data.time , "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(new_data.product, "%s", xml(DataAccess , "PRODUCT", &idx2 ).c_str());
    new_data.price      = xml(DataAccess , "PRICE", &idx2 ).toFloat();

    if( String(new_data.date) == "" || String(new_data.product) == "") continue;

    OilList.push_back(new_data);
  }
  // std::sort(OilList.begin(), OilList.end(), [](PTT_OilPrice_t &a, PTT_OilPrice_t& b){ return ( strcmp(a.product, b.product)< 0); });
  std::sort(OilList.begin(), OilList.end(), [](const _PTT_OilPrice_t a, const _PTT_OilPrice_t b){ return ( strcmp(a.product, b.product)< 0); });
}


int TridentTD_PTT_OilPrice::getCurrentOilPrice(PTT_OilPrice_t **oilprice) {
  HTTPClient http;
  size_t oilprice_size=0;

  if( !http.begin("http://www.pttplc.com/webservice/pttinfo.asmx")){
    return -1;  // can't connect
  }else{
    http.addHeader("Content-Type", "text/xml; charset=utf-8");
    http.addHeader("SOAPAction", "\"http://www.pttplc.com/ptt_webservice/CurrentOilPrice\"");
    int httpCode = http.POST(HTTP_POST_BODY);
    if( httpCode !=200) return -2;

    String payload = http.getString();
    http.end();

    XML_parser(payload);
    *oilprice = &OilList[0];
    oilprice_size = OilList.size(); 
  }
  return oilprice_size;
}

TridentTD_PTT_OilPrice PTT;
