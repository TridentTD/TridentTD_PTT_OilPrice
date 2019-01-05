#include <TridentTD_PTT_OilPrice.h>

#define SSID			"--------------"
#define PASSWORD  "--------------"


PTT_OilPrice_t* OilPrices;

void setup(){
	Serial.begin(115200); Serial.println();

  Serial.println("WiFi Connecting....");
	WiFi.begin(SSID,PASSWORD);
	while( WiFi.status() != WL_CONNECTED ) { Serial.print("."); delay(400);}
	Serial.print("\nWiFi Connected. IP : ");
	Serial.println(WiFi.localIP());

	size_t product_count = PTT.getCurrentOilPrice(&OilPrices);

  Serial.println("----------------------------------------");
  Serial.println("PTT Current OilPrice List");
  Serial.println("----------------------------------------");
	for(int i = 0; i < product_count; ++i){
		Serial.print(OilPrices[i].date);
		Serial.print(" ");
		Serial.print(OilPrices[i].time);
		Serial.print("\t");
		Serial.print(OilPrices[i].product);
		Serial.print("\t");
		Serial.println(OilPrices[i].price);
	}
  Serial.println("----------------------------------------");
}

void loop(){
}


