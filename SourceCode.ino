#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// kiểu của sensor cảm biến (DHT11 HOAC DHT22)
#define DHTTYPE DHT11   // DHT 11
#define DHTPin 5    //D1
#define lamp 4      //D2
#define Led D3
#define Cambien D0

// Thông tin wifi để esp kết nối
const char* ssid = "Long";
const char* password = "ab12345678";

#define mqtt_server "mqtt.dioty.co" // Địa chỉ server
// Các topic sub và pub
#define mqtt_topic_pub1 "/vulong051999@gmail.com/room/humidity"
#define mqtt_topic_pub2 "/vulong051999@gmail.com/room/temperature"
#define mqtt_topic_sub "/vulong051999@gmail.com/room/lamp"
const uint16_t mqtt_port = 1883; //Port của CloudMQTT

//User và password của MQTT Broker
const char* User = "vulong051999@gmail.com";
const char* Pass = "392382e4";

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPin, DHTTYPE);

long now = millis();
long lastMeasure = 0;

// Hàm kết nối ESP8266 đến wifi
void setup_wifi() {
  delay(10);
  // Connecting WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// Thiết bị publishes message đến topic mà ESP8266 subscribed 
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  // Nếu có message được nhận ở topic room/lamp
  Serial.print("Changing Room lamp to ");
  if (messageTemp == "true") {
    digitalWrite(lamp, HIGH);
    Serial.print("On");
  }
  else if (messageTemp == "false") {
    digitalWrite(lamp, LOW);
    Serial.print("Off");
  }
  Serial.println();
}

// Hàm reconnects ESP8266 đến MQTT broker
void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client", User, Pass)) {
      Serial.println("connected");

      // Subscribe or resubscribe to a topic
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // đợi 5s
      delay(5000);
    }
  }
}

// Sets mqtt broker, sets callback function
// Hàm callback sẽ nhận message và điều khiển LEDs
void setup() {
  Serial.begin(115200);
  
  pinMode(lamp, OUTPUT);
  pinMode(Led,OUTPUT);
  pinMode(Cambien,INPUT);// nhận tín hiệu đầu vào cho cảm biên
  
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
  
  if (!!!client.connected()) {
    reconnect();
  }
  client.loop();
  
  int value = digitalRead(Cambien);
  digitalWrite(Led,value);
    
  now = millis();
  // Publishes nhiệt độ và độ ẩm mỗi 3s
  if (now - lastMeasure > 3000) {
    lastMeasure = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

     // Kiểm tra nếu sensor lỗi
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    char bufH[100], bufT[100];
    gcvt(h, 3, bufH);
    gcvt(t, 3, bufT);

    // Publishes Temperature and Humidity
    client.publish(mqtt_topic_pub1, bufH);
    client.publish(mqtt_topic_pub2, bufT);

    Serial.print("Publish message: ");
    Serial.print(bufH);
    Serial.print(" - ");
    Serial.println(bufT);
  }
}
