#include <WiFi.h>
#include <WebServer.h>

// Replace these with your network credentials
const char* ssid = "dcccd-wireless";
const char* password = "";

// Definitions for pin connections
#define Brass_Valve 47
#define Sprinkler_Valve 11
#define Pressure_Sensor 1

// Values sent between client and server 

int storedValue = 620; // Value recieved by browser, changes min value of mappad anologRead
float pressure = 0.0; //Mapped pressure signal from sensor, sent to client

WebServer server(80);

//HTML and javascript data for webserver
void handleRoot() {
  String htmlPage = "<!DOCTYPE html><html><head><title>ESP32 Web Server</title>";
  htmlPage += "<script>function submitValue() { let xhr = new XMLHttpRequest(); xhr.open('POST', '/store', true); xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); xhr.send('value=' + document.getElementById('inputValue').value); }</script>";
  htmlPage += "<script>setInterval(() => { fetch('/data').then(response => response.text()).then(data => { document.getElementById('pressure').innerText = data; }); }, 1000);</script></head><body>";
  htmlPage += "<h1>Pressure Reading</h1><p>Current Pressure: <span id='pressure'>Loading...</span> PSI</p>";
  htmlPage += "<h2>Store a Value</h2><input type='number' id='inputValue'><button onclick='submitValue()'>Submit</button></body></html>";
  server.send(200, "text/html", htmlPage);
}

// Handler for sending pressure data
void handleData() {
  String data = String(pressure, 2);
  server.send(200, "text/plain", data);
}

// Handler for storing the user input
void handleStore() {
  if (server.hasArg("value")) {
    storedValue = server.arg("value").toInt();
    Serial.print("Stored value: ");
    Serial.println(storedValue);
  }
  server.send(200, "text/plain", "Value stored");
}

void setup() {
  Serial.begin(115200);

  //Initialize pins used for input and output
  pinMode(Brass_Valve, OUTPUT);  
  pinMode(Sprinkler_Valve, OUTPUT); 
  pinMode(Pressure_Sensor, INPUT);


  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/store", HTTP_POST, handleStore);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //Read pressure here
  pressure = map(analogRead(Pressure_Sensor), storedValue, 4095, 0, 150);

  server.handleClient();
  delay(500);
}

