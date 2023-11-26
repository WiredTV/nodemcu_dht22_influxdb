// I'm using a cheap knockoff NODEMCU clone ESP8266 with an ESP12F chip
// Due to this fact, after uploading the code to the board I need to 
// take the power of the board after uploading and connect it back to a 
// powersource. Then it will run normally.

#define ledPin 2 /**/

// Influx Stuff
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Temp Sensor Stuff
#include <DHT.h>
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor

//Variables
int chk;
float humidity;  //Stores humidity value
float temperature; //Stores temperature value

#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
#endif
  
// WiFi
#define WIFI_SSID "MYNETWORK"
#define WIFI_PASSWORD "MYWIFIPASSWORD"

// Define Influx details  
#define INFLUXDB_URL "http://192.168.1.200:8086" // where you installed Influx
#define INFLUXDB_TOKEN "MYSECRETTOKEN" // Your influx token, see the admin section of influx UI
#define INFLUXDB_ORG "af0125d82d90db64" // The influx org defined
#define INFLUXDB_BUCKET "Temperature" // The bucket that you would like to use to store the measurements
  
// Time zone info
#define TZ_INFO "GMT1"
  
// Declare InfluxDB client instance 
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
// Declare Data point
Point sensor("Woonkamer"); // I've used 'Woonkamer' to identify the sensor (its Dutch for Living Room)

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  
  // Setup DHT
  Serial.begin(115200);
  dht.begin();
  
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  
  // Set Time server
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
    } 
  else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    }

}
void loop() {

  // Read data and store it to variables hum and temp
  humidity = dht.readHumidity();
  temperature= dht.readTemperature();

  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();
  sensor.addField("Humidity", humidity);
  sensor.addField("Temperature", temperature);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());
  
  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }
  
  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  Serial.println("Waiting 1 second");
  
  digitalWrite(ledPin, LOW);
  delay(5000);

  // Put the controller in Deep Sleep (this is a timed deep sleep)
  // Timer is set at 15 minutes (microseconds used)
  ESP.deepSleep(900000000); 

}

   