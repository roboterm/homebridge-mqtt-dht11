#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"          // dht library

#define wifi_ssid "xxxxxx"
#define wifi_password "xxxxxxxxxxxx"

#define mqtt_server "192.168.2.106"

#define topic "home/bedroom/temphum"                    // overall topic
#define temperature_topic "home/bedroom/temperature"    // temperature topic
#define humidity_topic "home/bedroom/humidity"          // humidity topic
#define client_id "Bedroom air sensor"                  // client id

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

//Reconnect
void reconnect() {
    //Loop until reconnect is obtained
    while (!client.connected()) {
        Serial.print("Connecting to the MQTT server...");
        if (client.connect(client_id)) {
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

    // humidity reading
    float h = dht.readHumidity();
    // temperature reading
    float t = dht.readTemperature();

    //There's no point in going any further if the sensor doesn't return anything.
    if ( isnan(t) || isnan(h)) {
      Serial.println("Failed to read! Check your sensor");
      return;
    }
    
    // Allocate the JSON document
    //
    // Inside the brackets, 200 is the RAM allocated to this document.
    // Don't forget to change this value to match your requirement.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    DynamicJsonDocument  doc(100);

    // Add values in the document
    doc["temperature"] = t;
    doc["humidity"] = h;
    String output;
    serializeJson(doc, output);
    //if ( debug ) {
        Serial.print("temperature : ");
        Serial.print(t);
        Serial.print(" | humidity : ");
        Serial.println(h);
        Serial.print("json : ");
        Serial.println(output);
    //}  
    client.publish(temperature_topic, String(t).c_str(), true);   //Publish the temperature on the topic temperature_topic
    client.publish(humidity_topic, String(h).c_str(), true);      //And the humidity
    char buffer[256];
    serializeJson(doc, buffer);
    client.publish(topic, buffer, true);          //Overall topic
    delay(10000);
}
