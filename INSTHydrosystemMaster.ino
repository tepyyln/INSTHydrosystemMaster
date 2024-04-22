#include <WiFi.h>
#include "INSFARMMasterClass.h"
#include "structData.h"

#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//========== SETUP INSFARM TELEGRAM BOT ==========//
// Telegram BOT Token (Get from Botfather)

//MAUL
//#define BOT_TOKEN "6839169589:AAGDEswkAXh4UgNF0im8kBwo4LzGN36Zd2Q"
//#define CHAT_ID "6672892296"
//TEPY
#define BOT_TOKEN "7071416830:AAHKVkiE21gI9sQpskTAjBEoHmml4eKoJNM"
#define CHAT_ID "1104672431"

const unsigned long BOT_MTBS = 1000;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long bot_lasttime;          // last time messages scan has been done
bool Start = false;

//Wi-Fi Config:
//const char* ssid     = "Maulana";
//const char* password = "123123123";
const char* ssid     = "OPPO A95";
const char* password = "12345678";
WiFiServer  server(80);

//Parsing Data Variables:
bool      sendDataStatus = false;
int       bufLength = 0;
const int BUFFER_SIZE = sizeof(struct insfarmData);
char      buf[BUFFER_SIZE];

//Millis Config:
unsigned long sendDataPrevMillis = 0;
int           intervalSendData = 50;
unsigned long statusWifiMillis = 0;
unsigned long statusErrorWifiMillis = 0;
int           intervalStatusWifi = 8000;

//Class Declaration:
INSFARMMaster instm;

//Struct Declaration:
insfarmData instd;

struct insfarmData * tinstd;

void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++) {
    
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID ) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
    }
    else {
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";
       
      if (text == "/readTemp") {
          String msg = "Menampilkan Data Temperatur Ruangan Terkini: ";
          msg += (tinstd->roomtemp);
          msg += "C\n";
          bot.sendMessage(chat_id,msg, ""); 
      }
      
      if (text == "/readWatertemp") {
          String msg = "Menampilkan Data Temperatur Air Terkini ";
          msg += (tinstd->watertemp);
          msg += "C"; 
          bot.sendMessage(chat_id,msg, ""); 
      }

      if (text == "/readLux") {  
          String msg = "Menampilkan Data Intensitas Cahaya Terkini : ";
          msg += (tinstd->lux);
          msg += "lx"; 
          bot.sendMessage(chat_id,msg, ""); 
      }

      if (text == "/readTDS") {  
          String msg = "Menampilkan Data Zat Terlarut Air Terkini : ";
          msg += (tinstd->tds);
          msg += "ppm"; 
          bot.sendMessage(chat_id,msg, ""); 
      }

      if (text == "/readPH") {  
          String msg = "Menampilkan Data PH Air Terkini : ";
          msg =+(tinstd->ph);
          msg =+ "pH"; 
          bot.sendMessage(chat_id,msg, ""); 
      }

      if (text == "/actON") {
        tinstd->sprayerState == 1;
        String msg = "Pump Telah Diaktifkan";
        bot.sendMessage(chat_id,msg, "");
      }

      if (text == "/actOFF") {
        tinstd->sprayerState == 0;
        String msg = "Pump Telah Dinonaktifkan";
        bot.sendMessage(chat_id,msg, "");
      }

       if (text == "/start") {
       String welcome = "Selamat Datang INSFARM, Gunakan Command dibawah untuk Menampilkan Data dan Mengontrol Dari INSFARM, \n";
       welcome += "/readTemp : Menampilkan Data Temperatur Ruangan Terkini\n";
       welcome += "/readHumd : Menampilkan Data Kelembaban Ruangan Terkini\n";
       welcome += "/readWatertemp : Menampilkan Data Temperatur Air Terkini\n";
       welcome += "/readLux  : Menampilkan Data Intensitas Cahaya Terkini\n";
       welcome += "/readTDS : Menampilkan Data Zat Terlarut Air Terkini\n";
       welcome += "/readPH : Menampilkan Data PH Air Terkini\n";
       welcome += "/readSensor : Menampilkan Data Pembacaan Terkini Semua Sensor\n";
       welcome += "/actON : Mengaktifkan Cooler dan Sprayer\n";
       welcome += "/axtOFF : Menonaktifkan Cooler dan Sprayer\n";
       bot.sendMessage(chat_id, welcome, "Markdown");
      }   
    }  
  }
}


//========== END INSFARM TELEGRAM BOT ==========//
void setup() {
  Serial.begin(112500);
  Serial2.begin(112500);
  instm.initSetup();

  bot.sendMessage(CHAT_ID, "Bot started up", "");
}

void loop() {
 if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
 }

  
 if (WiFi.status() == WL_CONNECTED) {
    while (Serial2.available() >= BUFFER_SIZE) {
      bufLength = Serial2.readBytesUntil('\n', buf, BUFFER_SIZE);
      sendDataStatus = true;
    }
    if (instm.firebaseReady() && (millis() - sendDataPrevMillis > intervalSendData || sendDataPrevMillis == 0)) {
      if (millis() - statusWifiMillis >= intervalStatusWifi) {
        Serial2.print("100\n"); //wifi on
        statusWifiMillis = millis();
      }
      if (sendDataStatus == true) {
        //copy buffer to struct using memcpy:
        memcpy(&instd, buf, sizeof(instd));
        //read updated data sprayer status from Firebase:
        instm.streamData();
        //send data to Firebase:
        sendDataToFirebase(&instd);

        //NULL-ing condition
        buf[0] = '\0';
        bufLength = 0;
        sendDataStatus = false;
      }
      sendDataPrevMillis = millis();
    }
  } 
  else {
    if ((millis() - statusErrorWifiMillis) >= intervalStatusWifi) {
      Serial2.print("104\n"); //wifi off
      statusErrorWifiMillis = millis();
    }
  }
}

void sendDataToFirebase(struct insfarmData *sinstd) {
  instm.sendDataFloat("/TemperaturAir", sinstd->watertemp);
  instm.sendDataFloat("/TemperaturRuangan", sinstd->roomtemp);
  instm.sendDataFloat("/Radiasi", sinstd->lux);
  instm.sendDataFloat("/TDS", sinstd->tds);
  instm.sendDataFloat("/PH", sinstd->ph);

  if(sinstd->setPointUpper != 0) {
    instm.sendDataFloat("/setPointUpper", sinstd->setPointUpper);  
  }
  if(sinstd->setPointBottom != 0) {
    instm.sendDataFloat("/setPointBottom", sinstd->setPointBottom); 
  }
  if (sinstd->sprayerState == 2 || sinstd->sprayerState == 1 || sinstd->sprayerState == 0) {
    instm.sendDataInt("/sprayerState", sinstd->sprayerState);
  }

  Serial.print(" room => ");
  Serial.println(sinstd->roomtemp);
  Serial.print(" water => ");
  Serial.println(sinstd->watertemp);
  Serial.print(" lux => ");
  Serial.println(sinstd->lux);
  Serial.print(" water => ");
  Serial.println(sinstd->tds);
  Serial.print(" lux => ");
  Serial.println(sinstd->ph);
  Serial.print(" setPointBottom => ");
  Serial.println(sinstd->setPointBottom);
  Serial.print(" setPointUpper => ");
  Serial.println(sinstd->setPointUpper);
  Serial.print(" sprayerState => ");
  Serial.println(sinstd->sprayerState);
}
