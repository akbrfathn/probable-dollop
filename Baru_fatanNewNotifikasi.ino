#define BLYNK_TEMPLATE_ID "TMPL61JH8yvMP"
#define BLYNK_TEMPLATE_NAME "Skripsi PDAM"
#define BLYNK_AUTH_TOKEN "D3si6tNSvjo6419veoFND6R5tF0427TE"

#include<EEPROM.h>
#include <WiFi.h>
#include "waktuNoww.h"
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SENSOR1  27
#define SENSOR2  14

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
String Status;

float calibrationFactor1 = 4.5, calibrationFactor2 = 4.5;
volatile byte pulseCount1, pulseCount2;
byte pulse1Sec1 = 0, pulse1Sec2 = 0;
float flowRate1, flowRate2 ;
byte notifTele;

unsigned int flowMilliLitres1, flowMilliLitres2;
unsigned long totalMilliLitres1, totalMilliLitres2, liter1, liter2;

char ssid[] = "Rumah Aura";
char pass[] = "raisajela22";

unsigned long satu; // Hapus minggu1..minggu4

int address = 0;
int tambah, minggu, bulanan, hari, modehari;
int kirim1, kirim2, kirim3, kirim4, kirimbulan;

#define BOT_TOKEN "8127536876:AAHL-BZDmYK-nyKTE1AdloKyGXu7wOnfnOI"
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
String id = "7563182309";

WidgetLCD lcd1(V6);

void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(1000);
  lcd.begin();
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);
  kirim1 = 1;
  kirim2 = 1;
  kirim3 = 1;
  kirim4 = 1;
  kirimbulan = 1;
  notifTele = 1;

  pulseCount1 = 0;
  pulseCount2 = 0;

  flowRate1 = 0.0;
  flowRate2 = 0.0;
  flowMilliLitres1 = 0;
  flowMilliLitres2 = 0;
  totalMilliLitres1 = 0;
  totalMilliLitres2 = 0;
  previousMillis = 0;

  Serial.println(EEPROM.readULong(address));
  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  timeClient.begin();
  configTime(TZ, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
}

void loop()
{
  unsigned long dua = millis();
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    Blynk.run();
    pulse1Sec1 = pulseCount1;
    pulse1Sec2 = pulseCount2;
    pulseCount1 = 0;
    pulseCount2 = 0;

    flowRate1 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec1) / calibrationFactor1;
    flowRate2 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec2) / calibrationFactor2;
    previousMillis = millis();

    flowMilliLitres1 = (flowRate1 / 60) * 1000;
    flowMilliLitres2 = (flowRate2 / 60) * 1000;

    totalMilliLitres1 += flowMilliLitres1;
    totalMilliLitres2 += flowMilliLitres2;
    liter1 = totalMilliLitres1 / 1000;
    liter2 = totalMilliLitres2 / 1000;

    Blynk.virtualWrite(V0, flowRate1);
    Blynk.virtualWrite(V1, flowRate2);
    Blynk.virtualWrite(V2,  liter1);
    Blynk.virtualWrite(V3,  liter2);

    if (flowRate1 - flowRate2 >= 1) {
      Blynk.logEvent("notifikasi", "Terjadi Kebocoran, Silahkan Cek Pipa!!!");
      Status = "Ada Kebocoran";
      if (notifTele == 1) {
        for (int i = 0; i <= 2; i++) {
          bot.sendMessage(id, "Terdapat Kebocoran Pada Pipa");
          delay(1000);
        }
        notifTele = 0;
      }
    }
    else {
      Status = "Pipa Aman";
      notifTele = 1;
    }

    tampil(0, 0, String() + "Flow: " + flowRate1 + " L/min", 0, 1, "Total:" + String(liter1) + " L");
  }

  if (dua - satu >= 1000) {
    satu = dua;
    modehari = EEPROM.read(352);
    hari = EEPROM.read(351);
    waktu();

    // Tambah pemakaian langsung ke variabel bulanan
    bulanan += liter1;
    Serial.print("Hitung liter bulanan= ");
    Serial.println(bulanan);

    if (waktuku >= "10:10:10" && waktuku <= "10:10:10" ) {
      if (EEPROM.read(352) == 1) {
        hari = hari + 1;
        EEPROM.write(351, hari);
        EEPROM.write(352, 0);
        EEPROM.commit();
      }
    }
    else {
      EEPROM.write(352, 1);
      EEPROM.commit();
    }
    Serial.println("Mode hari: " + String(EEPROM.read(352)));
    Serial.println(" hari: " + String(EEPROM.read(351)));

    // Kirim data bulanan tanggal 1
    if (tgl == 1 && kirimbulan == 1) {
      if (waktuku >= "07:10:10" && waktuku <= "07:59:10") {
        Serial.println("Laporan Bulanan: " + String(bulanan));
        Blynk.logEvent("notifikasi", "penggunaan bulanan liter:" + String(bulanan));
        delay(5000);
        EEPROM.write(351, 1);
        EEPROM.commit();
        bulanan = 0;  // reset setelah kirim
        kirimbulan = 0;
      }
    }
  }

  Blynk.virtualWrite(V4, bulanan);
}

void tampil(int x0, int y0, String s0, int x1, int y1, String s1) {
  lcd.clear();
  lcd.setCursor(x0, y0);
  lcd.print(s0);
  lcd.setCursor(x1, y1);
  lcd.print(s1);
}

void lcdBlynk(int x0, int y0, String s0, int x1, int y1, String s1) {
  lcd1.clear();
  lcd1.print(x0, y0, s0);
  lcd1.print(x1, y1, s1);
}
