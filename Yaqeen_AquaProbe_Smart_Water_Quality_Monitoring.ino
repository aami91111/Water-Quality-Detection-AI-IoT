#define BLYNK_TEMPLATE_ID "TMPL656rP2x4Y"
#define BLYNK_TEMPLATE_NAME "My"
#define BLYNK_AUTH_TOKEN    "**************"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// مكتبات حساس الحرارة الرقمي 
#include <OneWire.h>
#include <DallasTemperature.h>

char ssid[] = "your_wifi_name";     
char pass[] = "***************";   

BlynkTimer timer;

// تعريف منفذ حساس الحرارة الرقمي  25
#define ONE_WIRE_BUS 25
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void sendSensorData() {
  // 1. فصل الواي فاي مؤقتاً لأجزاء من الثانية ليفك تعليق المنافذ التناظرية
  WiFi.disconnect();
  delay(150); // وقت إضافي بسيط للتأكد من تحرير المنفذ

  // 2. قراءة حساس الـ pH الموزون على القيمة المستقرة (3070)
  int phRaw = analogRead(32); 
  float phValue = (phRaw / 3070.0) * 6.86; 
  if(phValue > 14.0) phValue = 14.0;
  if(phValue < 0.0) phValue = 0.0;
  
  // 3. قراءة حساس الأملاح (TDS) من منفذ 33
  int tdsRaw = analogRead(33);
  float tdsValue = (tdsRaw / 4095.0) * 1000.0; 

  // 4. قراءة حساس العكارة (Turbidity) من منفذ 34
  int turbRaw = analogRead(34);
  float turbidityValue = (turbRaw / 4095.0) * 100.0; 

  // 5. القراءة الرقمية الصحيحة لحساس الحرارة الذكي (DS18B20)
  sensors.requestTemperatures(); 
  float tempValue = sensors.getTempCByIndex(0);
  if(tempValue == DEVICE_DISCONNECTED_C) {
    tempValue = 0.0;
  }

  // 6. إعادة الاتصال بالواي فاي وبدء تشغيل Blynk وإرسال البيانات
  WiFi.begin(ssid, pass);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(50);
    retry++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    // الاتصال المباشر بالسيرفر لإجبار البيانات على الرفع فوراً
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 
    Blynk.run();
    Blynk.virtualWrite(V1, phValue);
    Blynk.virtualWrite(V2, tdsValue);
    Blynk.virtualWrite(V3, turbidityValue);
    Blynk.virtualWrite(V4, tempValue);
  }

  // الطباعة في الـ Serial Monitor للتأكد
  Serial.print("pH: "); Serial.print(phValue); Serial.print(" (Raw: "); Serial.print(phRaw); Serial.print(")");
  Serial.print(" | TDS: "); Serial.print(tdsValue);
  Serial.print(" | Turb: "); Serial.print(turbidityValue);
  Serial.print(" | Temp: "); Serial.println(tempValue);
}

void setup() {
  Serial.begin(115200);
  delay(3000); 
  
  Serial.println("---بدء التشغيل الحقيقي لمشروع يقين---");
  
  sensors.begin();
  analogReadResolution(12);
  
  // تشغيل ببلانك بالطريقة الرسمية المستقرة من البداية
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // تعديل الوقت إلى 8 ثوانٍ (8000L) لمنح السيرفر واللوحة وقت كافي للتبادل بدون تعليق
  timer.setInterval(8000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
