#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
//-------------------
#define BLYNK_PRINT Serial
#include <TridentTD_LineNotify.h> //แจ้งเตือนผ่านไลน์
#include <BlynkSimpleEsp8266.h>
//--------------------
#include <NewPing.h> //วัดระยะ

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); //โชว์บนLCD
#include <SimpleDHT.h>
int pinDHT11 = D4;
SimpleDHT11 dht11(pinDHT11);

#include "DHT.h"
#define DHTPIN D4
//------กำหนดขาวัดระยะ
#define TRIGGER_PIN   D7 
#define ECHO_PIN      D3 
#define MAX_DISTANCE 100
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
int led1 = D6; //สีเขียว
int led2 = D5; //สีเหลือง
int trigPin = D7;
int echoPin = D3;
//-----------------------------------
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

#define auth "1p6RS7EbuEympLNPkwa0g6Iaskhhj60t"
#define wifi_ssid "realme 5"
#define wifi_password "11111111"
#define mqtt_server "35.240.193.162"

#define SOI "sensor/soil"
#define humid "sensor/humid"
#define tempc "sensor/tempc"
#define tempf "sensor/tempf"
#define LINE_TOKEN  "oM99PzIQOhrUyM4aXrk96Q8yGSicsNWMhpkqTaKpRGS"

WiFiClient espClient;
PubSubClient client(espClient);
//--------------จบส่วน เชื่อมต่อBlynk และ เชื่อมต่อ NOdeRED

int ledPin = D0;
int ledPin8 = D8;
int SoilPin = A0; //ประกาศตัวแปร ให้ analogPin แทนขา analog ขาที่5
int val = 0;
//---------จบส่วนกำหนดขา Pin ต่างๆ ต่อมากำหนดเชื่อมต่อขาPinกับBlynk
// กำหนดให้เซอเวออ่านค่า Virtual Pin 0 จะส่งค่า A0 กลับไป
BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0,analogRead(A0));
}
void setup() {
  pinMode(ledPin, OUTPUT);  // sets the pin as output
  pinMode(ledPin8, OUTPUT);  // sets the pin as output
  pinMode(SoilPin, INPUT);
  //-----------------ส่วนเป็นเป็นของ Soil
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode (led1,OUTPUT);
  pinMode (led2,OUTPUT);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println(LINE.getVersion());
  //------------
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Humidity=");
  lcd.setCursor(4, 1);
  lcd.print("Temp = ");
  //-------------------LCD
  Blynk.begin(auth, wifi_ssid, wifi_password); //น่าจะอยู่ข้างบน มั่ง!
  Serial.printf("WiFi connecting to %s\n",  wifi_ssid);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);
  //---------------------ข้างบนมาจาก Soil
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}
  //--------------------------------
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
      delay(500);
    }
  }
}
void loop() {
  // start working...
  //-----------------ในบล็อคนี้มาจาก Soil -----------------
  if (!client.connected()) {
        reconnect();
      }
      client.loop();
      
      // Wait a few seconds between measurements.
      delay(1000);
      //-------------
      int val = analogRead(SoilPin);
  Serial.print("Soil = ");
  client.publish(SOI, String(val).c_str(), true);
  //--------------อ่านค่าความแห้งในดิน--------------
  
  val = analogRead(SoilPin);  //อ่านค่าสัญญาณ analog ขา5 ที่ต่อกับ Soil Moisture Sensor Module v1
  Serial.print("val = "); // พิมพ์ข้อมความส่งเข้าคอมพิวเตอร์ "val = "
  Serial.println(val); // พิมพ์ค่าของตัวแปร val
  if (val > 800) { 
    digitalWrite(ledPin, LOW); // สั่งให้ LED ที่ Pin1 ดับ
    digitalWrite(ledPin8, HIGH); // สั่งให้ LED ที่ Pin8 ติดสว่าง

    LINE.notify("ความชื้นในดินเริ่มจะแห้ง!!รดน้ำให้โรงเพาะเห็ดด้วยนะครับ");
    LINE.notifyPicture("https://inwfile.com/s-fy/f8h0yd.jpg");
  } //ดินมีความแห้งเยอะ=ค่า1024เยอะ
  else {
    digitalWrite(ledPin, HIGH); // สั่งให้ LED ที่ Pin2 ติดสว่าง
    digitalWrite(ledPin8, LOW); // สั่งให้ LED ที่ Pin8 ดับ
  }
  Blynk.run();
  delay(500);
  //----------------ถึงนี่-----------------------
Serial.println("=================================");
  Serial.println("Sample DHT11...");

// read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(500);
    return;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" H");
  lcd.print(" ");
  // DHT11 sampling rate is 1HZ.
  delay(500);

  //Output on LCD
  lcd.setCursor(0,0);
  lcd.print("Humidity=");
  lcd.print(humidity + String("%"));
  lcd.setCursor(0,1);
  lcd.print("Temp = ");
  lcd.print(temperature + String("C"));
  //-----------------------LCD
  delay(500);
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm());
  Serial.println(" cm");

  if (sonar.ping_cm() <= 20) {  
  digitalWrite(led1,LOW);//เขียว
  digitalWrite(led2,HIGH);//สีเเดง
 }
if (sonar.ping_cm() <= 40 and sonar.ping_cm() > 21 ) { 
  digitalWrite(led1,HIGH);//เขียว
  digitalWrite(led2,LOW);//สีเเดง
}
  if (sonar.ping_cm() > 40) { 
  digitalWrite(led1,LOW);//เขียว
  digitalWrite(led2,LOW);//สีเเดง
}
  delay(500);
  //-----------ข้างบนเป็นส่วนของ วัดระยะ
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      // Wait a few seconds between measurements.
      delay(500);
      
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);
      
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
      }

      Serial.print("Temperature:");
      Serial.println(String(t).c_str());
      client.publish(tempc, String(t).c_str(), true);

      Serial.print("Humidity:");
      Serial.println(String(h).c_str());
      client.publish(humid, String(h).c_str(), true);
