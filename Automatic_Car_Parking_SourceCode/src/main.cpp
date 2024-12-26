#include <Arduino.h>
#include <Wire.h> 
#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert_emqx.h"
#include "secrets/mqtt.h"
#include <PubSubClient.h>
#include "MQTT.h"
#include "LCD_DISPLAY.h"
#include <ESP32Servo.h>
#include <DHT.h>
#include <Ticker.h>
#include <Adafruit_Sensor.h>

Servo servo1; //Control entry
Servo servo2; //Control exit

//Pin of HC-SR04
const int trigPin1 = 17; //Entry
const int echoPin1 = 16; 
const int trigPin2 = 33; //Exit
const int echoPin2 = 14;
const int trigPin3 = 26; //Pos1 detect
const int echoPin3 = 10;
const int trigPin4 = 19; //Pos2 detect
const int echoPin4 = 18;
const int trigPin5 = 23; //Pos3 detect
const int echoPin5 = 5;
const int trigPin6 = 32; //Pos4 detect
const int echoPin6 = 12; 

//Pin of Servo
const int servoPin1 = 4; //Entry
const int servoPin2 = 2; //Exit

// //Pin of DHT, MQ2, LDR
const int DHTPin = 13;
const int LDRPin = 39;
const int MQ2Pin = 35;

//Pin of buzzer, LED/relay
const int buzzerPin = 9;
const int ledPin1 = 25;
const int ledPin2 = 15;

volatile bool handle_parking_flag = false;
volatile bool ldr_flag = false;
volatile bool dht_flag = false;
bool control_led = false;
const float distance_detect = 6;
long duration; 
float distanceCm;
long current_time = 0;
long current_time_print = 0;
int vehicleCount = 0;
boolean check_open = false;
bool reserved_status[4] = {false, false, false, false};

namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    const char *client_id = (String("esp32-client") + WiFi.macAddress()).c_str();

    DHT dht(DHTPin, DHT11);
    WiFiClientSecure tlsClient;
    PubSubClient mqttClient(tlsClient);

    Ticker dhtTicker;
    Ticker ldrTicker;
    Ticker posTicker;
    const char *temperature_topic = "home/temperature";
    const char *humidity_topic = "home/humidity";
    const char *gas_emission_topic = "home/gas";
    const char *LDR_brighness_topic = "home/LDR";
    const char *light_topic1 = "home/light1";
    const char *light_topic2 = "home/light2";
    const char *vehicle_count_topic = "home/vehicles_count";
    const char *home_parking = "home/parking";
    const char *handle_entry_topic  = "home/entry";
    const char *handle_exit_topic   = "home/exit";
    const char *reserved_pos = "home/reserved_pos";
}

/*------------------------TEMPERATURE, HUMADITY, GAS EMISSION-----------------------*/
void WarningRead_Publish()
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float gas = analogRead(MQ2Pin);
    if (isnan(temperature) || isnan(humidity) || isnan(gas))
    {
        Serial.println("Failed to read from warning sensor!");
        return;
    }
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Gas Value: ");
    Serial.println(gas);
    if (temperature <= 5.0 || temperature >= 40.0 || humidity <= 30.0 || humidity >= 85.0 || gas >= 1800) {
      tone(buzzerPin, 1000);
    }
    else {
     noTone(buzzerPin);
    }
    mqttClient.publish(temperature_topic, String(temperature).c_str(), false);
    mqttClient.publish(humidity_topic, String(humidity).c_str(), false);
    mqttClient.publish(gas_emission_topic, String(gas).c_str(), false);
}

/*------------------------------Brightness LDR--------------------------------------------------*/
void brightness() {
  const int ldr_value = analogRead(LDRPin);
  mqttClient.publish(LDR_brighness_topic, String(ldr_value).c_str(), false);
  Serial.print("Brightness: ");
  Serial.print(ldr_value);
  if (!control_led) {
    if (ldr_value < 700) {
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, HIGH);
      Serial.println(". It's dim");
    }
    else {
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, LOW);
      Serial.println(". It's bright");
    }
  }
  else {
    Serial.println("Control led from dashboard");
  }
}


/*------------------------------HC-04 DETECT OBJECT---------------------------------------------*/
float Read_HC_SR04(const int trigPin, const int echoPin){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

/*-----------------------------DETECT OBJECT EACH POSITION-------------------------------------*/
void handle_parking() {
  const int trigPin[] = {trigPin3, trigPin4, trigPin5, trigPin6};
  const int echoPin[] = {echoPin3, echoPin4, echoPin5, echoPin6};
  const char* pos_name[] = {"Position 1", "Position 2", "Position 3", "Position 4"};
  const int number_position = sizeof(trigPin) / sizeof(trigPin[0]);
  bool pos_status[number_position];
  
  for (int i = 0; i < number_position; i++) {
    float distance = Read_HC_SR04(trigPin[i], echoPin[i]);
    pos_status[i] = (distance > 0 && distance <= distance_detect);

    if (reserved_status[i]) {
      Serial.print(pos_name[i]);
      if (pos_status[i]) {       
        Serial.println(": Occupied");
        LCD_display::update_positions(i, "Occup");
        String message = String(pos_name[i]) + ": " + "true";
        mqttClient.publish(home_parking, message.c_str(), false);
      } 
      else{
        Serial.println(": Reserved");
        LCD_display::update_positions(i, "Reser");
      }
    }
    else {
      Serial.print(pos_name[i]);
      Serial.println(pos_status[i] ? ": Occupied" : ": Empty");
      LCD_display::update_positions(i, pos_status[i] ? "Occup" : "Empty");
      String message = String(pos_name[i]) + ": " + (pos_status[i] ? "true" : "false");
      mqttClient.publish(home_parking, message.c_str(), false);
    }
  }
}

/*-------------------------------HANDLE DOOR--------------------------------------------------------*/
void handleDoor(Servo& servo, const int trigPin, const int echoPin, int directCount) {
  static int previousAngle1 = 0;
  static int previousAngle2 = 0;
  static unsigned long openTime1 = 0; 
  static unsigned long openTime2 = 0; 

  float distanceCm = Read_HC_SR04(trigPin, echoPin);

  int& previousAngle = (&servo == &servo1) ? previousAngle1 : previousAngle2;
  unsigned long& openTime = (&servo == &servo1) ? openTime1 : openTime2;
  const char* door = (&servo == &servo1) ? "First" : "Second";
  const char* handle_door_topic = (&servo == &servo1) ? handle_entry_topic : handle_exit_topic;

  if (distanceCm > 0 && distanceCm <= distance_detect) {
    if (previousAngle != 90) {
      servo.write(90);
      previousAngle = 90;
      vehicleCount += directCount;
      Serial.print("Vehicle Count: ");
      Serial.println(vehicleCount);
      check_open = true;
      openTime = millis();

      mqttClient.publish(vehicle_count_topic, String(vehicleCount).c_str(), false);
      mqttClient.publish(handle_door_topic, String(check_open).c_str(), false);

    } 
    else if (previousAngle == 90 && millis() - openTime >= 2000) {
      openTime = millis();
      vehicleCount += directCount; 
      Serial.print("New Vehicle Detected. Vehicle Count: ");
      Serial.println(vehicleCount);

      mqttClient.publish(vehicle_count_topic, String(vehicleCount).c_str(), false);
      mqttClient.publish(handle_door_topic, String(check_open).c_str(), false);

    }
    if (millis() - current_time_print >= 2000) {
      current_time_print = millis();
      Serial.print("---OPEN----the ");
      Serial.print(door);
      Serial.println("door"); 
    }
  }

  if (previousAngle == 90 && (millis() - openTime >= 5000)) {
    servo.write(180);
    previousAngle = 180;
    check_open = false;

    mqttClient.publish(handle_door_topic, String(check_open).c_str(), false);

    Serial.print("---CLOSE---the ");
    Serial.print(door);
    Serial.println("door");
  }
}

void statusDoor(Servo& servo1, const int trigPin1, const int echoPin1, int directCount1,
                Servo& servo2, const int trigPin2, const int echoPin2, int directCount2){

  handleDoor(servo1, trigPin1, echoPin1, 1);
  handleDoor(servo2, trigPin2, echoPin2, -1);
}

/*-----------------------CALL BACK-------------------------------------------*/
void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  char message[length+1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char) payload[i];
  }
  message[length] = '\0';
  /*-------------------CONTROL SERVO ENTRY FROM DASHBOARD----------------------*/
  if (strcmp(topic, handle_entry_topic) == 0) {
    if (String(message) == "true") {
      servo1.write(90);
      Serial.println("Open entry command received");
    }
    else if (String(message) == "false") {
      servo1.write(180);
      Serial.println("Close entry command received");
    }
  }
  /*--------------------CONTROL SERVO EXIT FROM DASHBOARD-----------------------*/
  else if (strcmp(topic, handle_exit_topic) == 0){
    if (String(message) == "true") {
      servo2.write(90);
      Serial.println("Open exit command received");
    }
    else if (String(message) == "false") {
      servo2.write(180);
      Serial.println("Close exit command received");
    }
  }
  /*--------------------CONTROL LIGHT 1 FROM DASHBOARD-------------------------------*/
  else if (strcmp(topic, light_topic1) == 0) {
    if (String(message) == "true") {
      digitalWrite(ledPin1, HIGH);
    }
    else if (String(message) == "false") {
      digitalWrite(ledPin1, LOW);
    }
  }
  /*-------------------CONTROL LIGHT 2 FROM DASHBOARD----------------------------------*/
  else if (strcmp(topic, light_topic2) == 0){
    control_led = true;
    if (String(message) == "true"){
      digitalWrite(ledPin2, HIGH);
    }
    else if (String(message) == "false") {
      digitalWrite(ledPin2, LOW);
    }
  }
  /*-------------------CHECK AND PRINT RESERVED POSITION--------------------------------*/
  else if (strcmp(topic, reserved_pos) == 0) {
    const char* reservedMessages[] = {"reserved 1: true", "reserved 2: true", "reserved 3: true", "reserved 4: true"};
    for (int i = 0; i < 4; i++) {
      if (strcmp(message, reservedMessages[i]) == 0) {
        reserved_status[i] = true;
      }
      else {
        reserved_status[i] = false;
      }
    }
  }
  
}


void setup() {
  Serial.begin(115200); // Starts the serial communication
  delay(10);
  //set up pin of servo
  pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  //Set up pin of buzzer, led, mq2, dht, ldr
  pinMode(buzzerPin, OUTPUT);
  pinMode(LDRPin, INPUT);
  pinMode(DHTPin, INPUT);
  pinMode(MQ2Pin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  //Set up pin of HC-SR04: trig:out, echo:in
  pinMode(trigPin1, OUTPUT); 
  pinMode(echoPin1, INPUT); 
  pinMode(trigPin2, OUTPUT); 
  pinMode(echoPin2, INPUT); 
  pinMode(trigPin3, OUTPUT); 
  pinMode(echoPin3, INPUT); 
  pinMode(trigPin4, OUTPUT); 
  pinMode(echoPin4, INPUT); 
  pinMode(trigPin5, OUTPUT); 
  pinMode(echoPin5, INPUT); 
  pinMode(trigPin6, OUTPUT); 
  pinMode(echoPin6, INPUT); 
  //wifi, MQTT
  setup_wifi(ssid, password); 
  tlsClient.setCACert(ca_cert); 
  mqttClient.setCallback(mqttCallback);
  mqttClient.setServer(EMQX::broker, EMQX::port);
  //lcd
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0);
  //attach servo
  servo1.attach(servoPin1, 500, 2400);
  servo2.attach(servoPin2, 500, 2400);
  //Ticker 
  ldrTicker.attach(1, []() {ldr_flag = true; });
  posTicker.attach(1, []() { handle_parking_flag = true; });
  dhtTicker.attach(1, []() {dht_flag = true; });
}

void loop() {
  MQTT::reconnect(mqttClient, client_id, EMQX::username, EMQX::password, handle_entry_topic, handle_exit_topic);
  mqttClient.loop();
  handleDoor(servo1, trigPin1, echoPin1, 1);
  handleDoor(servo2, trigPin2, echoPin2, -1);
  if (handle_parking_flag) {
    handle_parking_flag = false;
    handle_parking();          
  }
  if (ldr_flag) {
    ldr_flag = false;
    brightness();          
  }
  if (dht_flag) {
    dht_flag = false;
    WarningRead_Publish();
  }
             
  LCD_display::LCD_display();
}
