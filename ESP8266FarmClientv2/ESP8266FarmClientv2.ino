
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
//



#include <SimpleDHT.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;



//A0 is the only analog pin on the ESP8266
int sensorPin = A0;

//GPIO 0 / D3 is the Humidity sensor
int pinDHT11 = 0;
SimpleDHT11 dht11(pinDHT11);

const char* ssid = "GRIEFF2";
const char* password = "Archangel";
 WiFiClient client;


void setup() {
  
  Serial.begin(74880);
  Serial.println("Into Setup");
  Serial.println("Initialising pins..."); 
  
  //D0 is the unit bit of the multiplexer bitmask
  pinMode(D0, OUTPUT);

  //D1 is the 2 bit of the multiplexer bitmask
   pinMode(D1, OUTPUT);
   
  //D2 is the Solenoid (24v) pin
  pinMode(D2, OUTPUT);

  //D5 is the High Pressure pump (12v) pin
  pinMode(D5, OUTPUT);

  //D8 is the lights (12v) - no diode protection
  pinMode(D8, OUTPUT);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  
//  Serial.println("Initialising WIFI");
//  WiFi.mode(WIFI_STA);
// 
//  WiFiMulti.addAP("GRIEFF_2GEXT","Archangel");
}

void loop() {

  Serial.println("");
  Serial.println("");
  Serial.println("======================================================================================");
  Serial.println("");
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);

  Serial.println("Version Date: 2020-04-19");
    PostMoistureData();
    PostTemperatureAndHumidityData();
    PostLightData();
  //PostSensorPowerData();
  //PostFarmBatteryPowerData();
  
  delay(2000);
  int pollingInterval = GetPollingInterval() * 1000;
  if (pollingInterval ==0)
  {
    pollingInterval=30000;
  }

  Serial.println("Waiting 2 seconds before getting tasks...");
  delay(2000);
  GetTasks();

  
  Serial.println("Waiting " + String(pollingInterval/60000) + " minutes before calling again..");
  delay(pollingInterval);
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
          Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
          return;
        }
  
        Serial.print("Sample OK: ");
        Serial.print((int)temperature); Serial.print(" *C, "); 
        Serial.print((int)humidity); Serial.println(" H");
  
//        digitalWrite(D0,0 );
//        digitalWrite(D1,0);
//        delay(2000);
//        int currentTemperature = analogRead(sensorPin);
//        const int TEMP_OFFSET = 175;
        Serial.println("currentTemperature:" + String(temperature));
        PostData(TEMPERATURE, (int)temperature);
        Serial.println("currentHumidity:" + String(humidity));
        PostData(HUMIDITY, (int)humidity);
        
        Serial.println("---------------------------------------------------");
  Serial.println("");
}

void PostMoistureData()
{
  Serial.println("");
    Serial.println("---------------------------------------------------");
   Serial.println("Into PostMoistureData");
   String MOISTURE =  String("72CFAC9D-B72A-4683-99C2-6FABA4A8A650");
   delay(2000);
   digitalWrite(D0, 0);
   digitalWrite(D1,0);
   delay(2000);
   int currentMoisture = analogRead(sensorPin);
   Serial.println("currentMoisture:" + String(currentMoisture));
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
   const String LIGHT = String("6BDB843C-8015-4150-B69E-B0EE73479C3B");
   digitalWrite(D0, 1);
   digitalWrite(D1,0);
   delay(2000);
   int currentLight= analogRead(sensorPin);
   Serial.println("Light:" + String(currentLight));
   PostData(LIGHT, 1000-currentLight);
  Serial.println("---------------------------------------------------");
  Serial.println("");
}

int GetPollingInterval()
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into GetPollingInterval");
  const char* host="griefffarmmanager.azurewebsites.net";
  const char* url = "/home/getconfigvalue?configValueName=pollinginterval";
  WiFiClient client;

  Serial.printf("\n[Connecting to %s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("connected]");

    Serial.println("[Requesting Tasks from Farm Manager....]");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 30000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return 0;
          }
        }

    Serial.println("[Response:]");
    String line="";
    while (client.connected())
    {
      if (client.available())
      {
        line = client.readStringUntil('\n');
        //Serial.println("PollingIntervalData:" + line);
        
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
    int pollingInterval = ParseConfigValue(line).toInt();
    return pollingInterval;
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
}



void GetTasks()
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into GetTasks");
  const char* host="griefffarmmanager.azurewebsites.net";
  const char* url = "/home/gettasks";
  WiFiClient client;
  
  Serial.printf("\n[Connecting to %s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("connected]");
    client.flush();
    
    Serial.println("[Requesting Tasks from Farm Manager....]");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 60000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
          }
        }

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
          found=true;
        }
      }
    }

//    while (client.available()) {
//    char ch = static_cast<char>(client.read());
//    Serial.print(ch);
//    }

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

//void GetTasks()
//{
//  Serial.println("");
//  Serial.println("---------------------------------------------------");
//  Serial.println("Into GetTasks");
//  const char* host="griefffarmmanager.azurewebsites.net";
//  const char* url = "/home/gettasks";
//  WiFiClient client;
//
//  Serial.printf("\n[Connecting to %s ... ", host);
//  if (client.connect(host, 80))
//  {
//    Serial.println("connected]");
//
//    Serial.println("[Requesting Tasks from Farm Manager....]");
//    
//    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
//               "Host: " + host + "\r\n" +
//               "Connection: close\r\n\r\n");
//        unsigned long timeout = millis();
//        while (client.available() == 0) {
//          if (millis() - timeout > 120000) {
//            Serial.println(">>> Client Timeout !");
//            client.stop();
//            return;
//          }
//        }
//
//    Serial.println("[Response:]");
//    String line = "";
//    while (client.connected())
//    {
//      if (client.available())
//      {
//        line = client.readStringUntil('\n');
//        
//        
//      }
//    }
//    Serial.println("taskData:" + line);
//    ParseTasks(line);
//    client.stop();
//    Serial.println("\n[Disconnected]");
//  }
//  else
//  {
//    Serial.println("Failed to get tasks");
//    client.stop();
//  }
//}

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
  WiFiClient client;
  client.stop();
 char server[] = "griefffarmmanager.azurewebsites.net";
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    //"/home/trackinginfo?dataType=D92D1AD6-416D-4E12-9D62-E50F3FE176D7&dataValue=234"
    // send the HTTP GET request:
    String fullRequest =  String("POST http://griefffarmmanager.azurewebsites.net/home/trackinginfo?dataType=" + dataType + "&dataValue=" + String(dataValue) + " HTTP/1.1");
    Serial.println("fullRequest:" + fullRequest);
    client.println("POST http://griefffarmmanager.azurewebsites.net/home/trackinginfo?dataType=" + dataType + "&dataValue=" + String(dataValue) + " HTTP/1.1");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: Keep-Alive");
    client.println("Content-Length: 40");
    client.println("Host: griefffarmmanager.azurewebsites.net");
    client.println();
    client.println("D92D1AD6-416D-4E12-9D62-E50F3FE176D7=234");

    // note the time that the connection was made:
    int lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void MarkTaskComplete(String taskId)
{
  Serial.println("");
  Serial.println("---------------------------------------------------");
  Serial.println("Into MarkTaskComplete");
  Serial.println(taskId);

  Serial.println("About to connect");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("waiting 1 second...");
    delay(1000);
  }

  
    if(WiFi.status()== WL_CONNECTED){   
        
        Serial.println("Connected");
        
        HTTPClient http;   
        http.begin("http://griefffarmmanager.azurewebsites.net/home/MarkTaskcomplete");  
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");             
        
        int httpResponseCode = http.POST("taskId=" + taskId);   
        Serial.println("Posted");
        if(httpResponseCode>0){
        
        String response = http.getString();                       
        Serial.println(httpResponseCode);   //Print return code
        Serial.println(response);           //Print request answer
    }
    else
    {
    
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
   }
 
   http.end();  
 
 }else{
 
    Serial.println("Error in WiFi connection");   
 
 }
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

  const int SOLENOID = 0;
  const int HP_PUMP = 1;
  const int PUMP_ON = 2;
  const int PUMP_OFF = 3;
  const int FANS_ON = 4;
  const int FANS_OFF = 5;
 
  int durationSeconds = duration.toInt();
  Serial.println("Duration is: " + String(duration));
  Serial.println("Task type is:" + String(taskType));
  switch(taskType.toInt())
  {
    case SOLENOID:
      digitalWrite(D2, HIGH);
      break;
      
//    case HP_PUMP:
//       //If the lights are on then turn them off temporarily to allow the pump to run off the same source...
//      digitalWrite(D5, HIGH);
//      break;

    case PUMP_ON:
      digitalWrite(D8, HIGH);
      break;

    case PUMP_OFF: 
      digitalWrite(D8, LOW);
      break;

    case FANS_ON:
      Serial.println("Setting D8 high...");
      digitalWrite(D5, HIGH);
      break;

    case FANS_OFF:
      digitalWrite(D5, LOW);
      break;
  }
  if (durationSeconds != 0)
  {
    Serial.println("This task comes with a duration, so wait for the specified duration (" + String(durationSeconds) + " seconds) then set the pin low...");
    delay(durationSeconds * 1000);
    switch(taskType.toInt())
    {
      case SOLENOID:
        digitalWrite(D2, LOW);
        break;

      case PUMP_ON:
        digitalWrite(D8, LOW);
        break;

      case FANS_ON:
        digitalWrite(D5, LOW);
        break;
    }
  }
  
  MarkTaskComplete(id);
}
