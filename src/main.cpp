#include "Arduino.h"
#include <secret.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define SENSOR 12
#define DASHBOARD 33

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
int lastStatus = 0;

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("");

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(int status)
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["window"] = status;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR, INPUT);
  pinMode(DASHBOARD, OUTPUT);
  connectAWS();
  lastStatus = digitalRead(SENSOR);
}

  // the loop function runs over and over again forever
void loop() {
  
  // println(digitalRead(SENSOR))
   int statusWindow =  digitalRead(SENSOR);
  if (lastStatus != statusWindow) {

    if(statusWindow == 1) {
      Serial.println("Window is closed");
      digitalWrite(DASHBOARD, LOW);
    }
    else {
      Serial.println("Window is open");
      digitalWrite(DASHBOARD, HIGH);
    }
    lastStatus = statusWindow;
   publishMessage(statusWindow);
  }
  client.loop();
  delay(100);                // wait for a second
}