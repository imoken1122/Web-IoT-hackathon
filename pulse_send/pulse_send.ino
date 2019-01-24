#include<WiFi.h>
#include <WebSocketClient.h>

// myakuhaku
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
//

const char* ssid     = "";
const char* password = "";
char path[] = "/";
char host[] = "";
WebSocketClient webSocketClient;

// nyakuhaku
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
float old_value=65.0;
int i = 0;
int num = 0;
//

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(5000);
  

  // Connect to the websocket server
  if (client.connect("", 3000)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
    delay(3000);
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }  
  }
  //myakuhaku
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  //

}

void loop() {
  //myakuhaku

  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      //1つ前の値と現在の値の差が15以上ならば前回の値+1を現在の値とする。
      //if(abs(beatsPerMinute-old_value)>=10) beatsPerMinute = old_value+1.0; 
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  old_value=beatsPerMinute;
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  if (irValue < 50000)
    Serial.print(" No finger?");
  Serial.println();
  //


  if(i == 60){
    String data = "0 ";
    num /= 60;
    if (client.connected()) {    
      // capture the value of analog 1, send it along
      //pinMode(1, INPUT);
      data.concat(String(num));
      data.concat(" ");
      data.concat(beatAvg);
      webSocketClient.sendData(data);
      
      
    } else {
      Serial.println("Client disconnected.");
      while (1) {
        // Hang on disconnect.
      }
    }
    i=0;
    num=0;
  }
  else{
    num += beatsPerMinute;
    i += 1;
  }
  // wait to fully let the client disconnect  
}
