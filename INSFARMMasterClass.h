#include <WiFi.h>
#include <WiFiManager.h> 

//#include <Firebase_ESP_Client.h>
#include <FirebaseESP32.h>

//========== SETUP INSFARM FIREBASE ==========//
//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Insert Firebase project API Key
#define API_KEY "AIzaSyAc-wZiN5l3jEJRTkGV2lPNmQ6Cx6KOlv0"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://insfarm-iot-default-rtdb.firebaseio.com/"

// Define the user Email and password that alreadey registerd or added in your project
#define USER_EMAIL "insfarmhidroponik@gmail.com"
#define USER_PASSWORD "INSFARM12345"

//Define the WiFi credentials */
//#define WIFI_SSID "Maulana"
//#define WIFI_PASSWORD "123123123"
#define WIFI_SSID "OPPO A95"
#define WIFI_PASSWORD "12345678"

FirebaseData fbdo;
FirebaseData fbStream;
FirebaseAuth auth;
FirebaseConfig config;

class INSFARMMaster {
  private:
    // utilities variables:
    String _deviceId      = "282022";

  public:
    void  initSetup();
    bool  firebaseReady();
    int   sendDataInt(String path , int data);
    int   sendDataFloat(String path , float data);
    int   getDataInt(String path);
    float getDataFloat(String path);
    void  streamData();
};

//bool wifiManagerSetup() {
//  WiFi.mode(WIFI_STA);
//  WiFiManager wm;
//  wm.resetSettings();
//  wm.setConfigPortalBlocking(true);
//
//  bool res;
//  res = wm.autoConnect("Maulana", "123123123"); // password protected ap
//  return res;
//}
//
//void wifiManagerConnection() {
//  if (!wifiManagerSetup() || WiFi.status() != WL_CONNECTED) {
//    Serial.println("Failed to connect");
//    Serial2.print("104\n"); //wifi off
//  }
//  else {
//    //if you get here you have connected to the WiFi
//    Serial.println();
//    Serial.print("Connected with IP: ");
//    Serial.println(WiFi.localIP());
//    Serial.println(WiFi.SSID());
//  }
//}

void wifiConnection() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

///////////////////STREAM DATA CONFIG/////////////////////
  void streamCallback(StreamData data) {
  //Print out all information

  Serial.print("Stream Data INIT => ");
  Serial.println(data.streamPath());

  //Print out the value
  //Stream data can be many types which can be determined from function dataType

  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
    String path = "/insfarmiot";
    String devId = "282022";
    String concatedPath =  devId + path;
    
    if (data.to<int>() == 1) {
      Serial2.print("700\n"); //sprayer on
      //if (Firebase.RTDB.setIntAsync(&fbdo, concatedPath, 2)) {
//        Serial.print("sukses update sprayer state => 2");
//      }
    } else {
      Serial2.print("704\n"); //sprayer off
      //if (Firebase.RTDB.setIntAsync(&fbdo, concatedPath, 0)) {
//        Serial.print("sukses update sprayer state => 0");
//      }
    }
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    //Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }
}

///////////////////STREAM DATA CONFIG/////////////////////

void INSFARMMaster::initSetup() {
  Serial2.print("103\n");  //wifi waiting for connection...
  //  wifiConnection();
  wifiConnection();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
//  auth.user.email = USER_EMAIL;
//  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  config.database_url = DATABASE_URL;
  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino
  //  config.signer.test_mode = true;
  config.signer.tokens.legacy_token = "PUNdZWYtUGFCkRTARRxfEuUwsMLxeYNBkOt0VCkp";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(fbdo, 1000 * 60);

  ///////////////////STREAM DATA CONFIG/////////////////////
  Firebase.setStreamCallback(fbStream, streamCallback, streamTimeoutCallback);
  if (!Firebase.beginStream(fbStream, "/insfarmiot/282022"))
  {
    //Could not begin stream connection, then print out the error detail
    Serial.print("Error begin stream : => ");
    Serial.println(fbStream.errorReason());
  }
  ///////////////////STREAM DATA CONFIG/////////////////////
};

void INSFARMMaster::streamData() {
  //In loop()
  if (!Firebase.readStream(fbStream)) {
    Serial.print("Error read stream : => ");
    Serial.println(fbStream.errorReason());
  }

  if (fbStream.streamTimeout()) {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (fbStream.streamAvailable()) {
    if (fbStream.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      Serial.print("DATA STREAM BOSKUU => ");
      Serial.println(fbStream.to<int>());
    }
  }
}

int INSFARMMaster::sendDataInt(String path , int data) {
  String concatedPath = _deviceId + path;
  if (Firebase.RTDB.setIntAsync(&fbdo, concatedPath, data)) {
    return 1;
  }
  else {
    Serial.print("error int => ");
    Serial.println(fbdo.errorReason());
    return 0;
  }
};

int INSFARMMaster::sendDataFloat(String path, float data) {
  String concatedPath = _deviceId + path;
  if (Firebase.RTDB.setFloatAsync(&fbdo, concatedPath, data)) {
    return 1;
  }
  else {
    Serial.print("error set float => ");
    Serial.println(fbdo.errorReason());
    return 0;
  }
};

int INSFARMMaster::getDataInt(String path) {
  String concatedPath = _deviceId + path;
  if (Firebase.getInt(fbdo, concatedPath)) {
    if (fbdo.dataType() == "int") {
      //      Serial.print("get data int => ");
      //      Serial.println(fbdo.intData());
      return fbdo.intData();
    }

  } else {
    Serial.print("error get int => ");
    Serial.println(fbdo.errorReason());
  }
}

float INSFARMMaster::getDataFloat(String path) {
  String concatedPath = _deviceId + path;
  if (Firebase.getFloat(fbdo, concatedPath)) {

    if (fbdo.dataType() == "float") {
      //      Serial.print("get data float => ");
      //      Serial.println(fbdo.floatData());
      return fbdo.floatData();
    }

  } else {
    Serial.print("error get float => ");
    Serial.println(fbdo.errorReason());
  }
}

bool INSFARMMaster::firebaseReady() {
  return Firebase.ready();
};
