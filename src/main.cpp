#include <Arduino.h>
#include <WEMOS_SHT3X.h>
#include "MessagePack.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "credentials.h"

SHT3X sht30(0x44);

WiFiClientSecure espClient;
PubSubClient client(espClient);


unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  // MQTT Connection
  espClient.setFingerprint(fingerprint);
  client.setServer(mqtt_broker, mqtt_port);

  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }

  client.subscribe("azenvy/incomming");
}


void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis;

    if(sht30.get()==0){
      client.publish("azenvy/meta/temperature", String(sht30.cTemp).c_str(), true);
      client.publish("azenvy/meta/humidity", String(sht30.humidity).c_str(), true);
    } else {
      Serial.println("Error!");
      client.publish(topic, "Error", true);
    }
  }

  client.loop();
}
