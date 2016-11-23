
/* Sensors track environment variables. This information is published to an MQTT broker. 
 * =========================================================================
 *  DHT11  
 *      DATA, VCC(5v or 3v), GND
 *      dht read.(connected pin, temperature, humidity, array of data)
 *      
 *  Sensor1 and sensor2 are 10K pots that are standing in place of pH and PPM analog sensors.
 *    Since the ESP8266 only has one analog input, the sensors will be read one at a time. 
 *    Digital output pins will apply +V to a circuit that will act like a switch.
 *    ex: When enable1 is activated, the analog input will read sensor1.
 *  
 *  The entire network is run from an Ubuntu-based laptop. The IoT devices connect to a USB wireless 
 *    adapter configured as a hotspot. The laptop also runs as the MQTT Broker. This means the 
 *    entire system is self contained and portable; a VLAN of sorts. (convienient for development) 
 */


#include <EEPROM.h>
#include <SimpleDHT.h> // humidity/temperature sensor 
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT protocol library
#include <ArduinoJson.h> // Format messages into JSON


// assign GPIO pins to sensors
int pinDHT11 = D4;
int pinAnalog = A0; // The one and only analog input
int enable1 = D2; // Activate sensor 1 to be read.
int enable2 = D1; // Activate sensor 2 to be read.
SimpleDHT11 dht11;

// set networking values
const char* ssid = "IdentityTheft";
const char* password = "lemongrab";
const char* mqttServer = "10.42.0.1";
const char* username = "rick"; //mqtt user
const char* userpass = "reality"; //mqtt password

WiFiClient espClient;
PubSubClient client(espClient);
char pub[100]; // the message to be published

void setup() {
  Serial.begin(115200);

  // prepare to send +V to the analog switch
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);

  //connect to wifi
  wifiConnect();
  client.setServer(mqttServer, 1883);

  //Connect to the MQTT server
  brokerConnect();
}

void loop() {

  // Check for WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnect();
  }

  // Check for Broker connection
  if (!client.connected()) {
    brokerConnect();
  }
  
  // Read from DHT11 sensors
  byte temperature = 0;
  byte humidity = 0;
  
  dht11.read(pinDHT11, &temperature, &humidity, NULL);

  //printDHT(temperature, humidity);

  // Read from PH and PPM sensors
  double ph = 0; //sensor1
  int ppm = 0; //sensor2

  // enable, read, disable sensor1
  digitalWrite(enable1, HIGH); // enable sensor1
  ph = analogRead(pinAnalog); // read sensor1
  ph = ph/100+3; // adjust the range of the 50k pot to roughly reflect a real meter
  digitalWrite(enable1, LOW); // disable sensor1

  // enable, read, disable sensor2
  digitalWrite(enable2, HIGH);
  ppm = analogRead(pinAnalog);
  digitalWrite(enable2, LOW);

  //printAnalog(ph, ppm);

  // create JSON message
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["node"] = "node-1";
  root["temp"] = temperature;
  root["humidity"] = humidity;
  root["ph"] = ph;
  root["ppm"] = ppm;

  root.prettyPrintTo(Serial);
  Serial.print("\n\n\n");

  root.printTo(pub, sizeof(pub));
  
  Serial.print("Publishing: ");
  Serial.println(pub);

  
  // publish topic
  client.publish("plantTopic", pub);  
  
  delay(1000);
}

void wifiConnect() {
  delay(15);
  Serial.print("WiFi settup...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.print(WiFi.localIP()); // the ip address of the WAP
}

void brokerConnect() {
  //loop until connected to the server
  while (!client.connected()) {
    Serial.print("MQTT setup...");
    if (client.connect("Plant-Node-1", username, userpass)) {
      Serial.println("MQTT connected");
    }
  }
}

void printDHT(int temperature, int humidity) {
  Serial.print("\n\nDHT11: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println("%");
  Serial.print("\n");
  
}

void printAnalog(double ph, double ppm) {
  Serial.print("pH: ");
  Serial.print((double)ph);
  Serial.print(", PPM: "); 
  Serial.print((double)ppm);
  Serial.print("\n\n");
}
