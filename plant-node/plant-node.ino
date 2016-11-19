#include <EEPROM.h>
#include <SimpleDHT.h> // humidity/temperature sensor 

/* This is the device that monitors the plants
 * ===================================================== 
 *  DHT11  
 *      DATA, VCC(5v or 3v), GND
 *      dht read.(connected pin, temperature, humidity, array of data)
 *      
 *  Sensor1 and sensor2 are 10K pots that are standing in place of pH and PPM analog sensors.
 *    Snce the 8266 only has one analog input, digital pins will apply +V to a series of 
 *    diodes/resistors which will enable the sensors to be read one at a time.
 *    ex: When enable1 is activated, the analog input will read sensor1.
 */


// declare the gpio pins
int pinDHT11 = 2; // Pin D4 on the ESP8266 node mcu
int pinAnalog = A0; // The one and only analog input
int enable1 = 5; // Activate sensor 1 to be read. Pin D2 on the ESP8266.
int enable2 = 4; // Activate sensor 2 to be read. Pin D1 on the ESP8266.

// initialize the sensors
SimpleDHT11 dht11;

void setup() {
  Serial.begin(115200);

  // prepare to send +V to the switch
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);
}

void loop() {
  /*****DHT11*****/
  byte temperature = 0;
  byte humidity = 0;
  
  //parses raw data from sensor '&' gets the memory address... pass by reference) 
  dht11.read(pinDHT11, &temperature, &humidity, NULL);

  // call print function
  printDHT(temperature, humidity);

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

  // call print function
  printAnalog(ph, ppm);
  
  delay(5000);
}


void printDHT(int temperature, int humidity) {
  Serial.print("DHT11: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println("%");
  Serial.print("\n");
  
}

void printAnalog(double ph, double ppm) {
  Serial.print("pH: ");
  Serial.print((double)ph);
  Serial.print(", PPM: "); 
  Serial.print((double)ppm);
  Serial.print("\n");
}
