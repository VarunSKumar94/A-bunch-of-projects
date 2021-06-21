#include <WiFi.h>

const char* ssid = "MTNL-bb-2097";
const char* password = "2613361400";
int numberparts=0;

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
 
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client){
    int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 4095) since it's a 12 bit ADC to a voltage 
    //(0 - 5V):
    float voltage = sensorValue * (5.0 / 4095.0);
    if ( voltage >  0 ){
    ++numberparts;
    } 
  
    while(client.connected()){
    client.println("counted parts ");
    client.println(numberparts);
    delay(1000);
    }
  }
}
