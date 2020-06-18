#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"          // dht library

#define wifi_ssid "xxxxxx"
#define wifi_password "xxxxxxxxxxxx"

#define mqtt_server "192.168.2.106"

#define topic "home/livingroom/temphum" // overall topic
#define temperature_topic "home/childrensroom/temperature"  // temperature topic
#define humidity_topic "home/childrensroom/humidity"        // humidity topic

//Buffer for decoding received MQTT messages MQTT
char message_buff[100];

long lastMsg = 0;   //Timestamp of the last message published on MQTT
long lastRecu = 0;
bool debug = false;  //Displays on the console if True

#define DHTPIN 13    // Pin to which the DHT is connected

// Uncomment the line that corresponds to your sensor. 
//#define DHTTYPE DHT11       // DHT 11 
#define DHTTYPE DHT22         // DHT 22  (AM2302)

//Create objects
DHT dht(DHTPIN, DHTTYPE);     
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);     //Optional for debugging
  pinMode(D7,INPUT_PULLUP);     //Pin 2 
  setup_wifi();           //We connect to the wifi network
  client.setServer(mqtt_server, 1883);    //Configuring the connection to the MQTT server 
  dht.begin();
}

//Connection to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connect to ");
  Serial.println(wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connect to WiFi ");
  Serial.print("=> IP address : ");
  Serial.print(WiFi.localIP());
}

//Reconnexion
void reconnect() {
  //Loop until a reconnection is obtained
  while (!client.connected()) {
    Serial.print("Connecting to the MQTT server...");
    if (client.connect("Kinderzimmer Luftsensor")) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" We wait five seconds before we do it again.");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  // Send one message per minute
  if (now - lastMsg > 1000 * 10) {
    lastMsg = now;
    // humidity reading
    float h = dht.readHumidity();
    // temperature reading
    float t = dht.readTemperature();

    //There's no point in going any further if the sensor doesn't return anything.
    /*
    if ( isnan(t) || isnan(h)) {
      Serial.println("Failed to read! Check your HVL sensor");
      return;
    }
    */
    // Allocate the JSON document
    //
    // Inside the brackets, 200 is the RAM allocated to this document.
    // Don't forget to change this value to match your requirement.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<200> doc;
  
    // StaticJsonObject allocates memory on the stack, it can be
    // replaced by DynamicJsonDocument which allocates in the heap.
    //
    // DynamicJsonDocument  doc(200);

    // Add values in the document
    //
    doc["temperature"] = String(t).c_str();
    doc["humidity"] = String(h).c_str();
  
    //if ( debug ) {
      Serial.print("temperature : ");
      Serial.print(t);
      Serial.print(" | humidity : ");
      Serial.println(h);
    //}  
    client.publish(temperature_topic, String(t).c_str(), true);   //Publish the temperature on the topic temperature_topic
    client.publish(humidity_topic, String(h).c_str(), true);      //And the humidity
    client.publish(topic, doc, true);                             //Overall topic
  }
}
