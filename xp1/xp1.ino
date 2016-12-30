/* Experimental Prototype 1 - 12/30/2016
 * =========================================================================
 *  DHT22  
 *      DATA, VCC(5v or 3v), GND
 *    
 *  DS18B20
 *      DATA, VCC(5v or 3v), GND
 */

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT protocol library
#include <ArduinoJson.h> // Format messages into JSON
#include <DHT.h>
#include <OneWire.h> // used for DS18B20
#include <DallasTemperature.h> // DS18B20 library

#define ONE_WIRE_BUS D2 //DS18B20 (water) temperature sensor
#define DHTPIN D3
#define DHTTYPE DHT22

OneWire oneWire(ONE_WIRE_BUS); // setup OneWire isntance (potentially other O.W. devices as well)
DHT dht(DHTPIN, DHTTYPE); // Initaiakize DHT sensor
DallasTemperature ds18b20(&oneWire); // pass the 'oneWire' memory address reference

// set networking values
const char* ssid = "esp";
const char* password = "lemondrops";
const char* mqttServer = "10.32.196.124";
const char* username = "rick"; //mqtt user
const char* userpass = "reality"; //mqtt password
const char* nodename = "xp1"; //name send in JSON
const char* host = "goldenwingedship.com"; //the LAMP server

WiFiClient espClient;
PubSubClient client(espClient);
char pub[100]; // the message to be published

void setup() {
  Serial.begin(115200);
  ds18b20.begin(); // start OneWire library
  dht.begin();
  //wifiConnect(); //connect to wifi
  client.setServer(mqttServer, 1883); // (server ip, port)
  //brokerConnect(); //Connect to the MQTT server
}

void loop() {

  // Check for WiFi connection
  /*if (WiFi.status() != WL_CONNECTED) {
    wifiConnect();
  }
  
  // Check for Broker connection
  if (!client.connected()) {
    brokerConnect();
  }
  */
  // Read from DHT22 sensor
  float humidity = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float ambient = dht.readTemperature(true);
  
  // Read from DS20b18
  ds18b20.requestTemperatures();
  float temperature = ds18b20.getTempFByIndex(0); // index 0 is the first sensor

  // create JSON message
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["node"] = nodename;
  root["ambient"] = ambient;
  root["humidity"] = humidity;
  root["temperature"] = temperature;
  
  root.prettyPrintTo(Serial);
  Serial.print("\n\n\n");
  root.printTo(pub, sizeof(pub));  
  Serial.print("Publishing: ");
  Serial.println(pub);

  // publish topic
  client.publish("plantTopic", pub);  

  /******************************/
  /****HTTP GET to LAMP server***/
  // WiFiClient espClient -- as defined earlier
 /* if (!espClient.connect(host, 80)) {
    Serial.println("HTTP connection failed");
  }

  String url = "/farm/input.php?";
  url += "plantname=tomato&humid=50&ambient=50&temp=50";

  Serial.print("Requesting URL: ");
  Serial.print(url);

  /*espClient.print(String("GET ") + url + "HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
  */

//  HTTPClient http;

 // http.begin("http://goldenwingedship.com/farm/insert.php?plantname=tomato&humid=33&ambient=33&temp=33");
  
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

void printDHT(int ambient, int humidity) {
  Serial.print("\n\nDHT11: ");
  Serial.print((int)ambient); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println("%");
  Serial.print("\n");
  
}
