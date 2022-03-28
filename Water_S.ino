#include <TridentTD_LineNotify.h>

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#define WIFI_STA_NAME "Non" // ค าสั่งเชื่อมต่อ WiFi ใส่ ssid WiFi
#define WIFI_STA_PASS "nattphol1" // ใส่ password WiFi
#define DHTPIN 14  
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define LINE_TOKEN "1l4hlbjWzGbk7Km8MA9NQ8JSkd5nGLt1SHO38eGvWUP"
#define INTERVAL_MESSAGE1 14400000 //1 ชั่วโมง
BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);
char auth[] = "BTHeoVlK66VLEWwTN1PHcDjKpZhmuxHE";
float temp;
float hum;
float soil;
float moisture_percentage;
int sensor = A0;  /* Connect Soil moisture analog sensor pin to A0 of NodeMCU */
int Flowsensor = D4; 
int XHigh; // เอาไว้เก็บค่า pulse ช่วงบวก
int YLow; // เอาไว้เก็บค่า pulse ช่วงลบ
float Times; // เอาไว้เก็บค่าเวลา
float Frequency; // เอาไว้เก็บค่าความถี่
float WaterRate; // เอาไว้เก็บค่าอัตราการไหลของน้ำ ในหน่วย ลิตร/นาที
float Total; //เอาไว้เก็บค่าปริมาณการใช้น้ำ
float mLpMin; //เอาไว้เก็บค่าอัตราการไหลของน้ำ ในหน่วย มิลลิลิตร/นาที
float LpHr; //เอาไว้เก็บค่าอัตราการไหลของน้ำ ในหน่วย ลิตร/ชั่วโมง
//สวิตซ์
int ON;
int OFF;
int time_1;
//ขำรีเลย์
int relay = D2; // ประกาศใช้ขา D2 สำหรับ Relay
void setup()
{
Blynk.begin(auth, WIFI_STA_NAME, WIFI_STA_PASS, "blynk.kku.ac.th", 8080);
pinMode(A0, INPUT); // Soil moisture sensor เป็น Input ส่งค่าเข้ามา
dht.begin();
pinMode (Flowsensor, INPUT_PULLUP); //ตั้งค่าให้ Water flow sensor เป็น Input
pinMode (relay, OUTPUT); //ให้ Relay เป็น Output รอรับค าสั่ง
digitalWrite (relay, LOW); //ตั้งค่าให้ Relay OFF
Serial.begin(115200);


} 

void Temperature() 
{
hum = dht.readHumidity();
temp = dht.readTemperature();
Blynk.virtualWrite(V1, temp, "°C");
Blynk.virtualWrite(V6, hum, "% RH");//แสดงค่า T ที่ V1 ในแอพพลิเคชั่น Blynk
} 


void flow() 
{
XHigh = pulseIn(Flowsensor, HIGH);
YLow = pulseIn(Flowsensor, LOW);
Times = XHigh + YLow;
if(Times == 0)
{
WaterRate = 0;
mLpMin = 0;
LpHr = 0;
}
else
{
Frequency = 1000000 / Times;
WaterRate = Frequency / 20.4;
mLpMin = WaterRate*1000 ;
LpHr = WaterRate*60;
Total = Total+WaterRate/60;
}
Blynk.virtualWrite(V2, Total, "L"); //แสดงค่า Total ที่ V2 ในแอพพลิเคชั่น Blynk
Blynk.virtualWrite(V3, WaterRate, "L/min"); //แสดงค่า Total ที่ V3 ในแอพพลิเคชั่น Blynk
}
BLYNK_WRITE(V4) { // สร้างคำสั่ง V4 ในแอพพลิเคชั่น Blynk
ON = param.asInt(); //ค าสั่งกดปุ่ม
} //คำสั่งสร้างสวิตช์ ปุ่ม ON เพื่อใช้ในการเปิดน้ำ
BLYNK_WRITE(V5)
{ // สร้างคำสั่ง V5 ในแอพพลิเคชั่น Blynk
OFF = param.asInt(); //ค าสั่งกดปุ่ม
} //ค าสั่งสร้างสวิตช์ ปุ่ม OFF เพื่อใช้ในการปิดน้ำ

void work(){
if(ON==1)
{ //เมื่อกดปุ่ม V4 ใน Blynk ON=1
soil=analogRead(sensor);
moisture_percentage = (100- (soil/1023.00) * 100.00 ) ;
digitalWrite(relay, HIGH); //Relay การจ่ายแรงดัน Solenoid จะเปิดน ้า
flow();
Temperature ();
Serial.println("ON mode");

Blynk.virtualWrite(V10, moisture_percentage, "% RH");
}
//โหมดปิด  
else if(OFF==1)
{ //เมื่อกดปุ่ม V5 ใน Blynk OFF=1
  digitalWrite(relay, LOW); 
soil=analogRead(sensor);
moisture_percentage = (100- (soil/1023.00) * 100.00 ) ;
Blynk.virtualWrite(V10, moisture_percentage, "% RH");
flow();
Temperature ();
Serial.println("OFF mode");
}
else if(OFF==0 || ON  ==0){
flow();
Temperature ();
soil=analogRead(sensor);

moisture_percentage = (100- (soil/1023.00) * 100.00 ) ;
if (moisture_percentage  <50 && moisture_percentage  >5 ) 
  {
digitalWrite(relay, HIGH);
Serial.println("Solenoid ON");
Blynk.virtualWrite(V10, moisture_percentage, "% RH");
if(moisture_percentage > 51 || moisture_percentage  < 5)
{
 digitalWrite(relay, LOW); //Relay OFF
Serial.println("Solenoid OFF");
Blynk.virtualWrite(V10, moisture_percentage, "% RH");
 }
}
else if (moisture_percentage > 51) //solenoid valve OFF
{
digitalWrite(relay, LOW); //Relay OFF
Serial.println("Solenoid OFF");
Blynk.virtualWrite(V10, moisture_percentage, "% RH");

}
else
{
         
  digitalWrite(relay, LOW); //Relay OFF
Serial.println("Solenoid OFF");
  }
} 

}

WidgetBridge bridge1(V1);
WidgetBridge bridge2(V2);
WidgetBridge bridge3(V3);
WidgetBridge bridge4(V4);
WidgetBridge bridge5(V5);
WidgetBridge bridge6(V6);
WidgetBridge bridge10(V10);
static bool value = true;
void blynkAnotherDevice() // Here we will send HIGH or LOW once per second
{
    bridge1.virtualWrite(V1,temp );
    bridge2.virtualWrite(V2,Total );
    bridge3.virtualWrite(V3,WaterRate );
    bridge4.virtualWrite(V4,relay, HIGH );
    bridge5.virtualWrite(V5,relay, LOW );
    bridge6.virtualWrite(V6,hum );
    bridge10.virtualWrite(V10,moisture_percentage ); 
}
BLYNK_CONNECTED()
{
  bridge1.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge2.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge3.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge4.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge5.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge6.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
  bridge10.setAuthToken("MUk_6_sa53eo_heRzIUFGv2FWinl0Vu6"); // Place the AuthToken of the second hardware here
}

void loop(){
BLYNK_WRITE(V4);
BLYNK_WRITE(V5);
work();
flow(); 
Temperature ();
Serial.print("ควำมชื้น = ");
Serial.println(moisture_percentage);
Serial.print("อุณหภูมิ = ");
Serial.println(temp);
Serial.print("เวลาในการรดน้ำ =   ");
Serial.println(Times/60000);
Serial.print("ปริมำณน้ำที่ใช้ = ");
Serial.println(Total);
blynkAnotherDevice();
Blynk.run();
timer.run();
}
