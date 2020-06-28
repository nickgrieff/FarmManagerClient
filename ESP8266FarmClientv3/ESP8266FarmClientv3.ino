
//  Controller config
//  Left to Right - with the USB on the RHS
//  1 - 
//  2 - 
//  3 - Solenoid -ve
//  4 - Solenoid +ve
//  5 - Fans -ve
//  6 - Fans +ve
//  7 - Pump  -ve
//  8 - Pump  +ve
//
//  Boards reference: http://arduino.esp8266.com/stable/package_esp8266com_index.json,http://resource.heltec.cn/download/package_heltec_esp32_index.json
//

//Sensor wiring config: 
//Brown - Earth
//Stripe Brown - Power
//Stripe Green - Signal Ouput (Analog)
//Stripe Blue - Multiplexer low bit 
//Orange - Multiplexer high bit
//Green - Humidity/Temp Sensor


#include <SimpleDHT.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
 
#define SOLENOID_1 0 //D3 Socket 6
#define SOLENOID_2 2 //D4 Socket 5 
#define SOLENOID_3 14 //D5 Socket 4
#define HP_PUMP 13 //D7 Socket 2
//#define LP_PUMP 14 //GPIO5
//Right-most power connection
#define FANS 15 //D8 Socket 1

#define MULTIPLEXER_LOW_BIT 9
#define MULTIPLEXER_HIGH_BIT 10
#define COMMON_ADC A0
#define TEMP_HUMID_SENSOR 16

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    0 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
String panelText[3];

const int MULTIPLEXER_MOISTURE =0;
const int MULTIPLEXER_LIGHT = 1;

//#define DHTTYPE DHT11 
//DHT dht(TEMP_HUMID_SENSOR, DHTTYPE);

//Standard retry intervals..
int retry=1;
int maxRetry = 10;

//A0 is the only analog pin on the ESP8266
int sensorPin = A0;

//GPIO 0 / D3 is the Humidity sensor
//int pinDHT11 = 0;
SimpleDHT11 dht11(TEMP_HUMID_SENSOR);

const char* ssid = "GRIEFF";
const char* password = "Archangel";
WiFiClient client;
String macAddress="";
unsigned long startTime = millis();

void setup() {
  
  Serial.begin(74880);
  Serial.println("Into Setup");
  Serial.println("Initialising pins..."); 

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  // Clear the buffer
  display.clearDisplay();


  
  
 
  pinMode(MULTIPLEXER_LOW_BIT, OUTPUT);
  pinMode(MULTIPLEXER_HIGH_BIT, OUTPUT);
  pinMode(SOLENOID_1, OUTPUT);
  pinMode(SOLENOID_2, OUTPUT);
  pinMode(SOLENOID_3, OUTPUT);
  pinMode(FANS, OUTPUT);
  pinMode(HP_PUMP, OUTPUT);
  pinMode(TEMP_HUMID_SENSOR,INPUT);
   
  
  log("ESP8266 Farm Client 3");
  delay(2000);
  Serial.printf("Connecting to %s ", ssid);
  log("Connecting to " + String(ssid));
  //WiFi.begin(ssid, password);
  EnsureWifi();
  
  WriteLog("ESP8266", "ESP8266 is in startup");
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);

  macAddress=WiFi.macAddress();
  log("Mac:" + macAddress);
  delay(1000);
}



void loop() {

  Serial.println("");
  Serial.println("");
  Serial.println("======================================================================================");
  Serial.println("");
  Serial.println("Version Date: 2020-05-03");
  //EnsureWifi();
  
  //PostSensorData();    

  GetTasks();

  //Wait();
  
}

void Wait()
{
  Serial.println("Waiting 2 seconds before getting polling interval...");
  delay(2000);
  int pollingInterval = GetPollingInterval() * 1000;
  if (pollingInterval ==0)
  {
    pollingInterval=10000;
  }
  WriteLog("ESP8266", "Waiting " + String(pollingInterval/60000) + " minutes before calling again..");
  log ("Waiting " + String(pollingInterval/60000) + " mins..");
  delay(pollingInterval);
  unsigned long upTime = millis()-startTime;
  WriteLog("UPTIME",  String(upTime/60000));
}

void PostSensorData()
{
    log("Post sensor data...");
    PostMoistureData();
    PostLightData();
    //PostTemperatureAndHumidityData();
  //PostSensorPowerData();
  //PostFarmBatteryPowerData();
}

void EnsureWifi()
{

//  WiFi.disconnect();
//  WiFi.begin(ssid, password);
//  retry = 1;
//  log("About to try for wifi");
//  log("Connecting to " + String(ssid));
//  while (WiFi.status() != WL_CONNECTED && retry<maxRetry)
//  {
//    WiFi.begin(ssid, password);
//    delay(1000);
//    //Serial.println("Connection attempt " + String(retry) + " of " + String(maxRetry) );
//    log("Attempt " + String(retry) + " of " + String(maxRetry));
//    retry++;
//  }
//  if (WiFi.status()==WL_CONNECTED)
//  {
//    log("Connected at try " + String(retry));
//  }
//  else
//  {
//    log("No Connection");
//  }
  
  //
  Serial.println("Checking wifi is connected....");
  log("Checking connected");
  //Keep trying to get a connection until we hit the max retry limit
  retry = 1;
  while (WiFi.status() != WL_CONNECTED && retry<maxRetry)
  {
    WiFi.begin(ssid, password);
    delay(500);
    Serial.println("Connection attempt " + String(retry));
    log("Connection attempt " + String(retry));
    retry++;
  }

  //Check if we are connected or we reached the max retries....
  if (WiFi.status() == WL_CONNECTED)
  {
    log("Connected !!");
  }
  else
  {
    log("Failed to connect");
    ESP.restart();
  }  
}


void PostTemperatureAndHumidityData()
{
  Serial.println("");
    Serial.println("---------------------------------------------------");
        Serial.println("Into PostTemperateAndHumidityData");
        String TEMPERATURE = String("D92D1AD6-416D-4E12-9D62-E50F3FE176D7");
        String HUMIDITY = String("CBD9A0A7-5347-4583-B5FA-054BA8E9D448");
        byte temperature = 0;
        byte humidity = 0;
        delay(2000);
        int err = SimpleDHTErrSuccess;
        if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
          log("Failed Temp/Humidity");
          Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
          return;
        }
  
        Serial.print("Sample OK: ");
        Serial.print((int)temperature); Serial.print(" *C, "); 
        Serial.print((int)humidity); Serial.println(" H");
        Serial.println("currentTemperature:" + String(temperature));
        log("Temp : "  + String(temperature));
        PostData(TEMPERATURE, (int)temperature);
        Serial.println("currentHumidity:" + String(humidity));
        log("Humidity : " + String(humidity));
        PostData(HUMIDITY, (int)humidity);
        
        Serial.println("---------------------------------------------------");
  Serial.println("");
}

//void readTempAndHumidity()
//{
//   // Wait a few seconds between measurements.
//  delay(2000);
//
//  // Reading temperature or humidity takes about 250 milliseconds!
//  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
//  float h = dht.readHumidity();
//  // Read temperature as Celsius (the default)
//  float t = dht.readTemperature();
//  // Read temperature as Fahrenheit (isFahrenheit = true)
//  float f = dht.readTemperature(true);
//
//  // Check if any reads failed and exit early (to try again).
//  if (isnan(h) || isnan(t) || isnan(f)) {
//    Serial.println(F("Failed to read from DHT sensor!"));
//    return;
//  }
//
//  // Compute heat index in Fahrenheit (the default)
//  float hif = dht.computeHeatIndex(f, h);
//  // Compute heat index in Celsius (isFahreheit = false)
//  float hic = dht.computeHeatIndex(t, h, false);
//
//  String TEMPERATURE = String("D92D1AD6-416D-4E12-9D62-E50F3FE176D7");
//  String HUMIDITY = String("CBD9A0A7-5347-4583-B5FA-054BA8E9D448");
//  log("Temperature: " + String(t));
//  PostData(TEMPERATURE, (int)t);
//  delay(2000);
//  log("Humidity: " + String(h));
//  PostData(HUMIDITY, (int)h);
//  delay(2000);
//  Serial.print(F("Humidity: "));
//  Serial.print(h);
//  Serial.print(F("%  Temperature: "));
//  Serial.print(t);
//  Serial.print(F("°C "));
//  Serial.print(f);
//  Serial.print(F("°F  Heat index: "));
//  Serial.print(hic);
//  Serial.print(F("°C "));
//  Serial.print(hif);
//  Serial.println(F("°F"));
//}

void PostMoistureData()
{
  Serial.println("");
    Serial.println("---------------------------------------------------");
   Serial.println("Into PostMoistureData");
   log("Checking moisture");
   String MOISTURE =  String("72CFAC9D-B72A-4683-99C2-6FABA4A8A650");
   delay(2000);
   digitalWrite(MULTIPLEXER_LOW_BIT, 0);
   digitalWrite(MULTIPLEXER_HIGH_BIT,0);
   delay(2000);
   int currentMoisture = analogRead(COMMON_ADC);
   Serial.println("Moisture : " + String(currentMoisture));
   log("Moisture : " + String(currentMoisture));
   PostData(MOISTURE, currentMoisture);
   Serial.println("---------------------------------------------------");
  Serial.println("");
   
}

void PostSensorPowerData()
{
  Serial.println("");
    Serial.println("---------------------------------------------------");
   Serial.println("Into PostSensorPowerData");
   String SENSOR_POWER =  String("3D0C85C5-E52C-4E20-96F4-D98958B57453");
   digitalWrite(D0, 0);
   digitalWrite(D1,1);
   delay(2000);
   int sensorPower = analogRead(sensorPin);
   Serial.println("sensorPower:" + String(sensorPower));
   PostData(SENSOR_POWER, sensorPower);
   Serial.println("---------------------------------------------------");
  Serial.println("");
   
}

void PostFarmBatteryPowerData()
{
  Serial.println("");
    Serial.println("---------------------------------------------------");
   Serial.println("Into PostFarmBatteryPowerData");
   String BATTERY_POWER =  String("0270E9A6-002F-4AC1-821A-7AB874655F90");
   digitalWrite(D0, 1);
   digitalWrite(D1,1);
   delay(2000);
   int batteryPower = analogRead(sensorPin);
   Serial.println("batteryPower:" + String(batteryPower));
   PostData(BATTERY_POWER, batteryPower);
   Serial.println("---------------------------------------------------");
  Serial.println("");
   
}

void PostLightData()
{
    Serial.println("");
    Serial.println("---------------------------------------------------");
   Serial.println("Into PostLightData");
   log("Checking light");
   const String LIGHT = String("6BDB843C-8015-4150-B69E-B0EE73479C3B");
   digitalWrite(MULTIPLEXER_LOW_BIT, 1);
   digitalWrite(MULTIPLEXER_HIGH_BIT,0);
   delay(2000);
   int currentLight= analogRead(COMMON_ADC);
   Serial.println("Light : " + String(currentLight));
   log("Light:" + String(currentLight));
   PostData(LIGHT, 1000-currentLight);
  Serial.println("---------------------------------------------------");
  Serial.println("");
}

int GetPollingInterval()
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into GetPollingInterval");
  log("Get polling interval");
  const char* host="griefffarmmanager.azurewebsites.net";
  const char* url = "/home/getconfigvalue?configValueName=pollinginterval";

  Serial.printf("\n[Connecting to %s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("Connected for polling interval");
    String fullRequest = String("GET ") + url + " HTTP/1.1\r\n" +  "Host: " + host + "\r\n" +  "Connection: close\r\n\r\n";
    Serial.println(fullRequest);
    client.print(fullRequest);

    WaitForResponse(6000);
  
    String line="";
    while (client.connected())
    {
      if (client.available())
      {
        line = client.readStringUntil('\n');
      }
    }
    client.stop();
    int pollingInterval = ParseConfigValue(line).toInt();
    return pollingInterval;
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
}


void WaitForResponse(int waitTime)
{
  Serial.println("Waiting " + String(waitTime) + " milliseconds for response...");
  unsigned long tickStart = millis();
  while (client.available() == 0) 
  {
      if (millis() - tickStart > waitTime) 
      {
        Serial.println("Request timed out..");
        client.stop();
        ESP.restart();
      }
   }
}

void GetTasks()
{
  Serial.println("Waiting 2 seconds before getting tasks...");
  log("Getting Tasks");
  delay(2000);
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into GetTasks");
  const char* host="griefffarmmanager.azurewebsites.net";
  const char* url = "/task/gettasks?MacAddress=";
 
  
  Serial.printf("\n[Connecting to %s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("connected]");
    client.flush();
    
    Serial.println("[Requesting Tasks from Farm Manager....]");
    client.print(String("GET ") + url + macAddress + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    WaitForResponse(30000);

    Serial.println("[Response:]");
    bool found = false;
    String line="";
    while ((client.connected() ||client.available()) && !found)
    {
      if (client.available())
      {
        line = client.readStringUntil('\n');
        //Serial.println("TaskData:" + line);
        //Serial.println(line.substring(2,6));
        if (line.substring(2,6) == "data") {
          Serial.println("Found the data line...");
          log("Found task data");
          found=true;
        }
      }
    }



    Serial.println("Data:" + line);
    client.stop();
    Serial.println("\n[Disconnected]");
    ParseTasks(line);
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
}





void ParseTasks(String taskJson)
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into ParseTasks");
  Serial.println("taskData:" + taskJson);
   DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(taskJson);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("No valid tasks found");
    return;
  }
  else
  {
    Serial.println("parsingConfirmed");
    String data = root["data"];
    Serial.println("Data" + data);
    DynamicJsonBuffer arrayBuffer;
    JsonArray& taskArray = arrayBuffer.parseArray(data);
    Serial.println("Parsed array");
    Serial.println(taskArray.size());
    log("Found " + String(taskArray.size()) + " tasks");
    String first = taskArray[0];
    DynamicJsonBuffer firstTask;
    
    JsonObject& task = firstTask.parseObject(first);
    String id = task["id"];
    Serial.println(id);

    Serial.println("About to iterate over array");
    DynamicJsonBuffer taskBuffer;
    for(int currentTask=0;currentTask<taskArray.size();currentTask++)
    {
      String taskJson = taskArray[currentTask];
      JsonObject& task = taskBuffer.parseObject(taskJson);
      ProcessTask(task);
    }
  }
}

String ParseConfigValue(String configJson)
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into ParseConfigValue");
  Serial.println("configData:" + configJson);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(configJson);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("No valid config found");
    return "";
  }
  else
  { 
    String value = root["data"]["value"];
    Serial.println(value);
    return value;
  }
}

void PostData(String dataType, int dataValue)
{
  Serial.println("");
  Serial.println("dataType:" + String(dataType));
  Serial.println("dataValue:" + String(dataValue));
  String params = "dataType=" + dataType + "&dataValue=" + String(dataValue) + "&controllerId=" + macAddress;
  PostToFarm("trackingdata/add", params);
}

void PostToFarm(String url, String params)
{
  Serial.println("");
  Serial.println("url:" + url);
  Serial.println("params:" + params);
  
  client.stop();
  char server[] = "griefffarmmanager.azurewebsites.net";
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    
    
    String fullRequest = "POST http://griefffarmmanager.azurewebsites.net/" + url + "?" + params + " HTTP/1.1";
    Serial.println("fullRequest:" + fullRequest);
    client.println(fullRequest);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: Keep-Alive");
    client.println("Content-Length: 40");
    client.println("Host: griefffarmmanager.azurewebsites.net");
    client.println();
    client.println("D92D1AD6-416D-4E12-9D62-E50F3FE176D7=234");

    String line = client.readStringUntil('\n');
    //Serial.println(line);
    log(line);
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void WriteLog(String entryType, String logMessage)
{
  PostToFarm("logging/add", "entryType=" + urlencode(entryType) + "&logText=" + urlencode(logMessage)+ "&controllerId=" + urlencode(macAddress));
  Serial.println(logMessage);
}

void MarkTaskComplete(String taskId)
{
  
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into MarkTaskComplete");
  Serial.println(taskId);
  PostToFarm("task/marktaskcomplete", "taskId=" + urlencode(taskId)+ "&controllerId=" + urlencode(macAddress));

}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}

void ProcessTask(JsonObject& task)
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into ProcessTask");  
  
  String id = task["id"];
  Serial.println("Task:"   + id);
  
  String taskType=task["taskType"];
  Serial.println("TaskType:" + taskType);
  String duration = task["taskParam"];
  Serial.println("Param:" + duration);
  String description = task["taskDescription"];
  Serial.println("Description:" + description);
  log("Task: "   + description);


  const int OpenMistingSolenoid = 0;
  const int HighPressurePump = 1;
  const int LightsOn = 2;
  const int LightsOff = 3;
  const int FansOn = 4;
  const int FansOff = 5;
  const int OpenSolenoid1 = 6;
  const int OpenSolenoid2 = 7;
  const int OpenSolenoid3 = 8;
  const int LowPressurePump = 9;
 
  int durationSeconds = duration.toInt();
  Serial.println("Duration is: " + String(duration));
  Serial.println("Task type is:" + String(taskType));
  log("Task type : " + String(taskType));
  switch(taskType.toInt())
  {

    case HighPressurePump:
      WriteLog("ESP32", "Turning pump on");
      pinHigh(HP_PUMP);
      break;

    case FansOn:
      WriteLog("ESP32", "Turning fans on");
      pinHigh(FANS);
      break;

    case FansOff:
      WriteLog("ESP32", "Turning fans off");
      pinLow(FANS);
      break;

    case OpenSolenoid1:
      WriteLog("ESP32", "Opening Solenoid 1");
      pinHigh(SOLENOID_1);
      break;
      
    case OpenSolenoid2:
      WriteLog("ESP32", "Opening Solenoid 2");
      pinHigh(SOLENOID_2);
      break;
      
     case OpenSolenoid3:
      WriteLog("ESP32", "Opening Solenoid 3");
      pinHigh(SOLENOID_3);
      break;
      
    
  }
  if (durationSeconds != 0)
  {
    Serial.println("This task comes with a duration, so wait for the specified duration (" + String(durationSeconds) + " seconds) then set the pin low...");
    delay(durationSeconds * 1000);
    log("Duration " + String(durationSeconds) + " seconds");
    switch(taskType.toInt())
    {
      case OpenSolenoid1:
        WriteLog("ESP32", "Closing Solenoid 1");
        log("Closing Solenoid 1");
        pinLow(SOLENOID_1);
        break;

      case OpenSolenoid2:
        WriteLog("ESP32", "Closing Solenoid 2");
        log("Closing Solenoid 2");
        pinLow(SOLENOID_2);
        break;

      case OpenSolenoid3:
        WriteLog("ESP32", "Closing Solenoid 3");
        log("Closing Solenoid 3");
        pinLow(SOLENOID_3);
        break;

      case HighPressurePump:
        WriteLog("ESP32", "Stopping HP Pump");
        log("Stopping HP Pump");
        pinLow(HP_PUMP);
        break;

      case FansOn:
        WriteLog("ESP32", "Stopping Fans");
        log("Stopping Fans");
        pinLow(FANS);
        break;

      
    }
  }
  
  MarkTaskComplete(id);
}


void log(String message)
{
  Serial.println(message);
  for(int slot=0;slot<2;slot++)
  {
    panelText[slot]=panelText[slot+1];
  }
  panelText[2]=message;
  drawDisplay();
}

void drawDisplay(void) {
  display.clearDisplay();
  delay(100);
  display.setCursor(0, 0);
  display.print(panelText[0]);
  display.setCursor(0,10);
  display.print(panelText[1]);
  display.setCursor(0,20);
  display.print(panelText[2]);
  display.display();      // Show initial text
  delay(100);
}

void pinHigh(int pin)
{
  log("Pin " + String(pin) + " high...");
  digitalWrite(pin, HIGH);
}

void pinLow(int pin)
{
  log("Pin " + String(pin) + " low...");
  digitalWrite(pin, LOW);
}
