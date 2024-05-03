//----------------------------------------Including the libraries.
#include "WiFi.h"
#include <HTTPClient.h>
#include "DHT.h"
//----------------------------------------

// Defines the DHT11 PIN and DHT sensor type.
#define DHTPIN  4
#define DHTTYPE DHT22

// Defines the PIN for the Switches.
#define PUMP   13

//---------------SSID and PASSWORD of your WiFi network.-------------------
const char* ssid = "Gg";  //--> Your wifi name
const char* password = "ggmedi123"; //--> Your wifi password
//----------------------------------------

// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbybcjeRwHDzWfACYvwbb68cy5lIr7_5fMqCiXU1gatsSsmPbAWhQfIQL_miHHe6Sh72/exec";

float Temp;
int Humd;
String Pump = "OFF";

// Initialize DHT as dht11.
DHT dht(DHTPIN, DHTTYPE);

//________________________________________________________________________________Getting_DHT11_Sensor_Data()
// Subroutine for getting temperature and humidity data from the DHT11 sensor.
void Getting_DHT11_Sensor_Data() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  Humd = dht.readHumidity();
  // Read temperature as Celsius (the default)
  Temp = dht.readTemperature();

  // Humd = 70;
  // Temp = 100;
  // Check if any reads failed and exit early (to try again).
  if (isnan(Humd) || isnan(Temp)) {
    Serial.println();
    Serial.println(F("Failed to read from DHT sensor!"));
    Serial.println();

    Temp = 0.00;
    Humd = 0;
  }

  Serial.println();
  Serial.println("-------------");
  Serial.print(F(" | Humidity : "));
  Serial.print(Humd);
  Serial.print(F("% | Temperature : "));
  Serial.print(Temp);
  Serial.println(F("°C"));
  Serial.println("-------------");
}


//_____________________VOID SETUP()__________________
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);
  //----------------------------------------Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  // connecting_process_timed_out = connecting_process_timed_out * 2;
  // while (WiFi.status() != WL_CONNECTED) {
  //   if (connecting_process_timed_out > 0) connecting_process_timed_out--;
  //   if (connecting_process_timed_out == 0) {
  //     delay(1000);
  //     ESP.restart();
  //   }
  // }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("------------");

  delay(100);

  Serial.println();
  Serial.println("DHT11 Begin");
  Serial.println();
  delay(1000);
  
  dht.begin();

  delay(2000);
}
//________________________________________________________________________________

//________________________________________________________________________________VOID LOOP()
void loop() {

  Getting_DHT11_Sensor_Data();

  if (WiFi.status() == WL_CONNECTED) {

    // Create a URL for sending or writing data to Google Sheets.
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&temp=" + String(Temp);
    Send_Data_URL += "&humd=" + String(Humd);


    Send_Data_URL += "&pump=" + Pump;
    write_to_google_sheet(Send_Data_URL);

    delay(3000);

    String Read_Data_URL = Web_App_URL + "?sts=read";
    read_from_google_sheet(Read_Data_URL);
  }

  if(Pump == "ON") digitalWrite(PUMP, HIGH);
  else digitalWrite(PUMP, LOW);

  delay(1000);
}

void write_to_google_sheet(String Send_Data_URL){
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);
//::::::::::::::::::The process of sending or writing data to Google Sheets.
  // Initialize HTTPClient as "http".
  HTTPClient http;

  // HTTP GET Request.
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Gets the HTTP status code.
  int httpCode = http.GET(); 
  Serial.print("HTTP Status Code : ");
  Serial.println(httpCode);

  // Getting response from google sheets.
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload : " + payload);    
  }
  
  http.end();

  Serial.println("-------------");
}


String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
  }

void read_from_google_sheet(String Read_Data_URL) {

    Serial.println();
    Serial.println("-------------");
    Serial.println("Read data from Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Read_Data_URL);

  //::::::::::::::::::The process of reading or getting data from Google Sheets.
    // Initialize HTTPClient as "http".
    HTTPClient http;

    // HTTP GET Request.
    http.begin(Read_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Gets the HTTP status code.
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    // Getting response from google sheet.
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload : " + payload);  
    }

    http.end();

  if (httpCode == 200) {
    // The process of separating data that is read or obtained from Google Sheets.
    Temp = getValue(payload, ',', 0).toFloat();
    Humd = getValue(payload, ',', 1).toInt();
    Pump = getValue(payload, ',', 2);
  } 
  Serial.println();
  Serial.println("-------------");
  Serial.print(F(" | Humidity : "));
  Serial.print(Humd);
  Serial.print(F("% | Temperature : "));
  Serial.print(Temp);
  Serial.println(F("°C"));
  Serial.print(F("% | Pump : "));
  Serial.print(PUMP);
  Serial.println("-------------");
}


//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<