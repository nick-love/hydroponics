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
 *    entire system is self contained and portable; a VLAN of sorts (convienient for development) 
 */


#include <EEPROM.h>
#include <SimpleDHT.h> // humidity/temperature sensor 
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT library


// assign GPIO pins to sensors
int pinDHT11 = D4;
int pinAnalog = A0; // The one and only analog input
int enable1 = D2; // Activate sensor 1 to be read.
int enable2 = D1; // Activate sensor 2 to be read.
SimpleDHT11 dht11;

// set networking values
const char* ssid = "ReallyBadVirus";
const char* password = "lemongrab";
const char* mqttServer = "10.42.0.1";
const char* username = "rick"; //mqtt user
const char* userpass = "reality"; //mqtt password

WiFiClient espClient;
PubSubClient client(espClient);
char pub[50]; // the message to be published

void setup() {
  Serial.begin(115200);

  // prepare to send +V to the switch
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);
  
  setupWiFi();
  client.setServer(mqttServer, 1883);
}

//Start WiFi connection
void setupWiFi() {
  delay(15);
  Serial.print("Connecting...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.print(WiFi.localIP()); // the ip address of the WAP
}


void loop() {

  /*if (!client.connected()) {
    setupWiFi();
  }*/
  
  /*****DHT11*****/
  byte temperature = 0;
  byte humidity = 0;
  
  //parses raw data from sensor '&' gets the memory address... pass by reference) 
  dht11.read(pinDHT11, &temperature, &humidity, NULL);


  /*****PH and PPM*****/
  double ph = 0; //sensor1
  double ppm = 0; //sensor2

  digitalWrite(enable1, HIGH); // enable sensor1
  ph = analogRead(pinAnalog); // read sensor1
  ph = ph/100+3; // adjust the range of the 50k pot to roughly reflect a real meter
  digitalWrite(enable1, LOW); // disable sensor1

  // enable, read, disable sensor2
  digitalWrite(enable2, HIGH);
  ppm = analogRead(pinAnalog);
  digitalWrite(enable2, LOW);

  // call print functions
  printDHT(temperature, humidity);
  printAnalog(ph, ppm);
  
  
  snprintf (pub, 40, "pH: %f", ph);

  Serial.print(pub);

  client.publish("outTopic", "Hello Wall");  
  
  delay(5000);
}


void printDHT(int temperature, int humidity) {
  Serial.print("DHT11: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println("%");
  Serial.print("");
  
}

void printAnalog(double ph, double ppm) {
  Serial.print("pH: ");
  Serial.print((double)ph);
  Serial.print(", PPM: "); 
  Serial.print((double)ppm);
  Serial.print("\n\n");
}
