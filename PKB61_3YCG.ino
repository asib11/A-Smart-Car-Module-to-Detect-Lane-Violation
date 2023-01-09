#define BLYNK_TEMPLATE_ID           "TMPLQXLwMooj"
#define BLYNK_DEVICE_NAME           "Smart Car System"
#define BLYNK_AUTH_TOKEN            "7Km_bYIN0c1zqOZp6cnP-8yrpilGIeDj"

#define BLYNK_PRINT Serial

#include <WiFiManager.h>
#ifdef ESP8266
#include <BlynkSimpleEsp8266.h>
#else
#error "Board not found"
#endif
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>

char* esp_ssid = "Smart Car System";
char* esp_pass = "1234567890";


String readString;
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
WiFiClient client1;
String GAS_ID = "AKfycbyzukBAaNi2O8zMzspSFh5ZOubUKYFORYiq7bOJIWx8RR4ytEdt";

String apiKey = "Z01XOOJJIL6U0RLD";
const char* server = "api.thingspeak.com";

char auth_token[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;


void checkPhysicalButton();

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
}

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

const int s0 = D0;
const int s1 = D5;
const int s2 = D6;
const int s3 = D7;
const int out = D8;
const int buzzer = D3;

int red = 0, green = 0, blue = 0;
int color = 0;


const int Erasing_button = 0;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup()

{
  Serial.begin(9600);
  pinMode(Erasing_button, INPUT);

  if (digitalRead(Erasing_button) == LOW)
  {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.restart();
    delay(1000);
  }

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(esp_ssid, esp_pass)) {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }
  Serial.println("connected...yeey :)");
  digitalWrite(D4, HIGH);
  Blynk.config(auth_token);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("Color Finder");

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  pinMode(buzzer, OUTPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);
  tone(buzzer, 1000);
  delay(1000);       
  noTone(buzzer);     
  delay(200);      
}

void loop()

{
  Blynk.run();
  timer.run();
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);

  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);

  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  Serial.print("R Intensity:");
  Serial.print(red, DEC);
  Serial.print(" G Intensity: ");
  Serial.print(green, DEC);
  Serial.print(" B Intensity : ");
  Serial.print(blue, DEC);

  if (red<10 & red>5 & green<23 & green>19 & blue<13 & blue>9) {
    delay(250);
    Serial.print("PINK");
    lcd.setCursor(6, 1);
    lcd.print("Pink");
    delay(2000);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Color Finder");
  }

  if (red<12 & red>7 & green<13 & green>9 & blue<21 & blue>16) {
    delay(250);
    Serial.print("Yellow");
    lcd.setCursor(5, 1);
    lcd.print("Yellow");
    Blynk.virtualWrite(V0, "Yellow");
    writing();
    thingspeak();
    tone(buzzer, 1000); 
    delay(1000);        
    noTone(buzzer);     
    delay(1000);        

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Color Finder");
    Blynk.virtualWrite(V0, "-");
  }
  Serial.println();

}


void writing() {
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/macros/s/" + GAS_ID + "/exec? value1=Yellow";
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +"Host: " + host + "\r\n" +"User-Agent: BuildFailureDetectorESP8266\r\n" +"Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was: "+line);
  Serial.println("closing connection");
  client.stop();
}

void thingspeak()
{
  if (client1.connect(server, 80))
  {

    String postStr = apiKey;
    postStr += "&field1=1";
    postStr += "\r\n\r\n";

    client1.print("POST /update HTTP/1.1\n");
    client1.print("Host: api.thingspeak.com\n");
    client1.print("Connection: close\n");
    client1.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client1.print("Content-Type: application/x-www-form-urlencoded\n");
    client1.print("Content-Length: ");
    client1.print(postStr.length());
    client1.print("\n\n");
    client1.print(postStr);

  }
  client1.stop();

  Serial.println("done.");
}
