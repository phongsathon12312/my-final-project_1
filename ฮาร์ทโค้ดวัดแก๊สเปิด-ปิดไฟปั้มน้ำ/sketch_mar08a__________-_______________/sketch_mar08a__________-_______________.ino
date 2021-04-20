#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <TridentTD_LineNotify.h>
//--------------------------
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

// รับ Auth Token ในแอป Blynk
char auth[] = "1p6RS7EbuEympLNPkwa0g6Iaskhhj60t";

// ข้อมูลรับรอง WiFi ของเรา
// ตั้งรหัสผ่านเป็น "" สำหรับเครือข่าย
#define wifi_ssid "realme 5"
#define wifi_password "11111111"
#define mqtt_server "34.87.97.102"
#define LINE_TOKEN  "oM99PzIQOhrUyM4aXrk96Q8yGSicsNWMhpkqTaKpRGS"

#define LDR "sensor/mq2"

WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดคขาPin sensor วัดค่าGas
int ledPin = D2;
int buzzer = D3;
int smokeA0 = A0; //ประกาศตัวแปร ให้ smokeA0 แทนขา analog ขาที่ A0
//your threshole volue
int sensorThres = 700;

void setup(){
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(smokeA0, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println(LINE.getVersion());

  WiFi.begin(wifi_ssid, wifi_password);
  Serial.printf("WiFi connecting to %s\n",  wifi_ssid);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  Blynk.begin(auth, wifi_ssid, wifi_password);
  // เรายังสามารถระบุเซิร์ฟเวอร์
  //จบส่วนเชื่อมLine
}
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
    
      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;  
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);


    // Attempt to connect
    // If you do not want to use a username and password, change next line to
  if (client.connect((char*) clientName.c_str())) {
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
        reconnect();
      }
      client.loop();
      
      // Wait a few seconds between measurements.
      delay(1000);
      //-------------
    int sensorValue = analogRead(smokeA0);
    Serial.println(sensorValue);
    delay(1000);
      
      int analogSensor = analogRead(smokeA0);
  Serial.print("LPG = ");
  client.publish(LDR, String(sensorValue).c_str(), true);
  
  //เข้าเงื่อนไข
  if (analogSensor > sensorThres) {

    LINE.notify("ค่าแก๊สภายในโรงเพาะเห็ดสูงมาก ระวังไฟไหม้!!");
    LINE.notifyPicture("https://www.nicepng.com/png/detail/957-9570427_-.png");
  digitalWrite(buzzer, LOW);
  digitalWrite(ledPin, HIGH);
  }
  else{
  digitalWrite(buzzer, HIGH);
  //noTone(buzzer);
  digitalWrite(ledPin, LOW);
  }
  delay(1000);
  
  Blynk.run();
}
