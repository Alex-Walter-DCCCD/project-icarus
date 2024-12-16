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
int pressure = 0; //Mapped pressure signal from sensor, sent to client

WebServer server(80);

//HTML and javascript data for webserver
void handleRoot() {
  String htmlPage = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Project ICARUS Launcher</title>";
  htmlPage += "<style>body.light-mode {background-color: white; color: black;} body.dark-mode {background-color: black; color: white;}";
  htmlPage += ".container {display: flex; flex-direction: column; align-items: center; margin-top: 50px;}";
  htmlPage += ".textbox {margin: 10px 0; font-size: 18px;}";
  htmlPage += ".button {padding: 10px 20px; font-size: 16px; cursor: pointer; margin: 10px 0;}";
  htmlPage += ".keypad {display: grid; grid-template-columns: repeat(3, 100px); grid-gap: 14px; justify-content: center; margin-top: 20px;}";
  htmlPage += ".key {width: 86px; height: 86px; background-color: #00ffff; border: 5px solid #008080; display: flex; align-items: center; justify-content: center; font-size: 24px; cursor: pointer;}";
  htmlPage += ".redkey {width: 86px; height: 86px; background-color: #ff0000; border: 5px solid #800000; display: flex; align-items: center; justify-content: center; font-size: 24px; cursor: pointer;}";
  htmlPage += ".greenkey {width: 86px; height: 86px; background-color: #00ff00; border: 5px solid #008000; display: flex; align-items: center; justify-content: center; font-size: 24px; cursor: pointer;}";
  htmlPage += ".cancelkey {width: 316px; height: 86px; background-color: #ff0000; border: 5px solid #800000; display: flex; align-items: center; justify-content: center; font-size: 24px; cursor: pointer; margin-top: 14px;}";
  htmlPage += ".key:hover {background-color: #008080; border: 5px solid #00ffff; color: #ffffff;}";
  htmlPage += ".redkey:hover {background-color: #800000; border: 5px solid #ff0000; color: #ffffff;}";
  htmlPage += ".greenkey:hover {background-color: #008000; border: 5px solid #00ff00; color: #ffffff;}";
  htmlPage += ".display {width: 320px; height: 50px; margin: 0 auto; text-align: center; font-size: 24px; border: 1px solid #ccc; margin-bottom: 20px;}";
  htmlPage += ".cancelkey:hover {background-color: #800000; border: 5px solid #ff0000; color: #ffffff;}";
  htmlPage += ".dark-mode .key {background-color: #000000; border: 5px solid #00ffff; color: #00ffff;}";
  htmlPage += ".dark-mode .redkey {background-color: #000000; border: 5px solid #ff0000; color: #ff0000;}";
  htmlPage += ".dark-mode .greenkey {background-color: #000000; border: 5px solid #00ff00; color: #00ff00;}";
  htmlPage += ".dark-mode .cancelkey {background-color: #000000; border: 5px solid #ff0000; color: #ff0000;}";
  htmlPage += ".dark-mode .key:hover {background-color: #008080; border: 5px solid #00ffff; color: #ffffff;}";
  htmlPage += ".dark-mode .redkey:hover {background-color: #800000; border: 5px solid #ff0000; color: #ffffff;}";
  htmlPage += ".dark-mode .greenkey:hover {background-color: #008000; border: 5px solid #00ff00; color: #ffffff;}";
  htmlPage += ".dark-mode .cancelkey:hover {background-color: #800000; border: 5px solid #ff0000; color: #ffffff;}";
  htmlPage += "</style>";
  htmlPage += "<script>let CurrentPSI = 0; let TargetPSI = 0;";
  htmlPage += "function updateTextBoxes() {document.getElementById('currentPSI').innerText = `Current PSI: ${CurrentPSI}`; document.getElementById('targetPSI').innerText = `Target PSI: ${TargetPSI}`;}";
  htmlPage += "function showSettings() {document.getElementById('mainMenu').style.display = 'none'; document.getElementById('settingsMenu').style.display = 'flex'; document.getElementById('keypadMenu').style.display = 'none';}";
  htmlPage += "function showMainMenu() {document.getElementById('settingsMenu').style.display = 'none'; document.getElementById('mainMenu').style.display = 'flex'; document.getElementById('keypadMenu').style.display = 'none';}";
  htmlPage += "function showKeypad() {document.getElementById('settingsMenu').style.display = 'none'; document.getElementById('mainMenu').style.display = 'none'; document.getElementById('keypadMenu').style.display = 'flex'; document.getElementById('display').value = 0;}";
  htmlPage += "function toggleDarkMode() {document.body.classList.toggle('dark-mode');}";
  htmlPage += "function addToDisplay(value) {let display = document.getElementById('display'); if (display.value == 0) {display.value = value;} else {display.value += value;}}";
  htmlPage += "function keypadCancel() {document.getElementById('display').value = 0; showMainMenu();}";
  htmlPage += "function keypadDelete() {let keypadDisplay = document.getElementById('display'); if (keypadDisplay.value != 0) {keypadDisplay.value = keypadDisplay.value.slice(0, -1);}}";
  htmlPage += "function keypadEnter() {let keypadReturnValue = document.getElementById('display').value; if (keypadReturnValue != 0) {TargetPSI = parseInt(keypadReturnValue); updateTextBoxes(); showMainMenu();}}";
  htmlPage += "function sendValue(action) {let xhr = new XMLHttpRequest(); xhr.open('POST', `/${action}`, true); xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); xhr.send('value=' + TargetPSI);}";
  htmlPage += "function pressurize() {sendValue('pressurize');}";
  htmlPage += "function launch() {sendValue('launch');}";
  htmlPage += "window.onload = function() { updateTextBoxes(); setInterval(() => { fetch('/data').then(response => response.text()).then(data => { CurrentPSI = parseFloat(data); updateTextBoxes(); }); }, 1000); };";
  htmlPage += "</script></head><body class='light-mode'>";
  htmlPage += "<div id='mainMenu' class='container'><div id='currentPSI' class='textbox'>Current PSI: 0</div>";
  htmlPage += "<div id='targetPSI' class='textbox'>Target PSI: 0</div><button class='button' onclick='showKeypad()'>Set PSI</button>";
  htmlPage += "<button class='button' onclick='pressurize()'>Pressurize</button><button class='button' onclick='launch()'>Launch</button>";
  htmlPage += "<button class='button' onclick='showSettings()'>Settings</button></div>";
  htmlPage += "<div id='settingsMenu' class='container' style='display: none;'><div id='currentPSI' class='textbox'>Current PSI: 0</div>";
  htmlPage += "<div id='targetPSI' class='textbox'>Target PSI: 0</div><div class='textbox'>SSID: </div>";
  htmlPage += "<button class='button' onclick='toggleDarkMode()'>Dark Mode</button><button class='button'>Countdown</button>";
  htmlPage += "<button class='button' onclick='showMainMenu()'>Return</button></div>";
  htmlPage += "<div id='keypadMenu' class='container' style='display: none;'><div id='currentPSI' class='textbox'>Current PSI: 0</div>";
  htmlPage += "<div id='targetPSI' class='textbox'>Target PSI: 0</div><input type='text' id='display' class='display' readonly>";
  htmlPage += "<div class='keypad'><div class='key' onclick='addToDisplay(1)'>1</div><div class='key' onclick='addToDisplay(2)'>2</div>";
  htmlPage += "<div class='key' onclick='addToDisplay(3)'>3</div><div class='key' onclick='addToDisplay(4)'>4</div>";
  htmlPage += "<div class='key' onclick='addToDisplay(5)'>5</div><div class='key' onclick='addToDisplay(6)'>6</div>";
  htmlPage += "<div class='key' onclick='addToDisplay(7)'>7</div><div class='key' onclick='addToDisplay(8)'>8</div>";
  htmlPage += "<div class='key' onclick='addToDisplay(9)'>9</div><div class='redkey' onclick='keypadDelete()'>&#9003;</div>";
  htmlPage += "<div class='key' onclick='addToDisplay(0)'>0</div><div class='greenkey' onclick='keypadEnter()'>&#8629;</div></div>";
  htmlPage += "<div class='cancelkey' onclick='keypadCancel()'>C A N C E L</div></div></body></html>";
  server.send(200, "text/html", htmlPage);
}

//Function that returns one pressure reading
//Note that this reading can vary by up to 3 PSI without sampling several readings over time
int Pressure_Read() {
  int Reading = 0;

  // Serial.println("Getting pressure reading");

  int Signal = analogRead(Pressure_Sensor);

  // Serial.print("Signal value is: ");
  Serial.println(Signal);

  // No single mapped analogRead was accurate for the entire range
  // If & Else statements used to find best map range for the signal value 
  if(Signal >= 4095) {
    // >90PSI always returns 4095. If value above this check voltages and circuit
    Serial.println("WARNING: VALUE MAY BE OUTSIDE RANGE!");
    Reading = 90;
  }
  else if(Signal >= 3360) {
    // Tested and works for 75 - 90 PSI range
    // Off by 1 PSI in the ~80-87 PSI range
    Serial.println("Signal between 75 and 90 PSI");
    Reading = map(Signal, 3360, 4095, 75, 90);
  }
  else if(Signal >= 2765) {
    // Tested and works for 50 - 80 PSI range
    // Off by 1 PSI in 60-70 PSI range
    Serial.println("Signal between 60 and 75 PSI");
    Reading = map(Signal, 2765, 3360, 60, 75);
  }
  else if(Signal >= 2225) {
    // Tested and works for 35 - 70 PSI range
    Serial.println("Signal between 45 and 60 PSI");
    Reading = map(Signal, 2225, 2765, 45, 60);
  }
  else if(Signal > 1700) {
    //                ^--Need ref val for 30 PSI

    Serial.println("Signal between 30 and 45 PSI");
    Reading = map(Signal, 620, 2225, 0, 45);
  }
  else if(Signal > 1500) {
    //                ^--Need ref val for 20 PSI
    Serial.println("Signal between 20 and 30 PSI");
    Reading = map(Signal, 600, 1700, 0, 30);
  }
  else if(Signal > 620) {
    // Hasn't been reliably tested
    Serial.println("Signal between 0 and 20 PSI");
    Reading = map(Signal, 620, 1145, 0, 15);
  }
  else {
    // 620 is used as reference for 0 PSI, small permutations in sig val occur even at 0
    // If Sig is regularly below 580 or even 600 check pressure transducer and make sure it's getting 5V & good GND
    // Serial.println("WARNING: VALUE MAY BE OUTSIDE RANGE!");
    Reading = 0;
  }
  // Serial.print("Pressure reading is: ");
  // Serial.println(Reading);
  return Reading;
}

// Function allows a timer to run while handling the server
// Note that handleClient is used to make sure webserver doesn't time out
// .5 used since that delay is used in loop without causing issues
// Since handleClient is called within timer expect timer to run slightly longer and not to be exact 
void Timer (float Sec) {
  float i = 0.000; // Handles amount of 0.001 sec delays required to reach allocated time 
  float d = 0.000; // Handles amount of 0.001 sec delays between each call of handleClient
  float Rate = 0.500; // Handles how often to run handleClient
  Serial.println("timer start");
  while (i < Sec) {
    Serial.println("timing");
    if (d >= Rate) {
      Serial.println("Handle client");
      server.handleClient();
      d = 0.000;
    }
    delay(10);
    i =+ 0.01;
    d =+ 0.01;
  }
  Serial.println("exited timer, handling client");
  server.handleClient();
  Serial.println("returning");
  return;
}


// Sample polling to account for random permutations
int Accurate_Pressure ( int Poll_Amt, float Poll_Delay, bool Send_Pressure) {
  int Readings[Poll_Amt];
  int i;
  int j;
  float Avg = 0.00;

  // Stores a reading in the first address and how often it shows up in the second address
  // ModeTest works in the same way and allows us to compare two readings
  Serial.println("Accurate pressure start");
  int Mode[2] = {-1, 0};
  int ModeTest[2];

  //Loop to get readings with delay
  for (i = 0; i < Poll_Amt; i++) {
    Serial.println("Starting first for loop");
    Readings[i] = Pressure_Read();
    //If Send_Pressure is invoked as true, then the readings will be used to update the displayed value
    //Useful for when polling takes enough time that the displayed psi isn't being updated at a satisfactory rate
    //Function doesn't return this value so ultimately the sample polled reading will be the most up to date value
    if (Send_Pressure) {
      pressure = Readings[i];
    }
    Serial.println("Before timer");
    Timer(Poll_Delay);
    Serial.println("After Timer");
  }

  //Beginning of data evaluation
  for (i = 0; i < Poll_Amt; i++) {
    ModeTest[0] = Readings[i];
    ModeTest[1] = 0;
    for(j = 0; j < Poll_Amt; j++) {
      if(ModeTest[0] == Readings[j]) {
        ModeTest[1]++;
      }
      if(ModeTest[1] > Mode[1]) {
        Mode[0] = ModeTest[0];
        Mode[1] = ModeTest[1];
      }
    }
  }
  //Average can be compared with mode to find out how much the readings deviate from each other
  for (i = 0; i < Poll_Amt; i++) {
    Avg =+ Readings[i];
  }
  Avg = Avg / Poll_Amt;
  Serial.print("Mode is: ");
  Serial.println(Mode[0]);
  Serial.print("Average is: ");
  Serial.println(Avg);
  return Mode[0];
}

//When pressurize signal is recieved
void Set_PSI(int Target) {
  float Index = 0;
  Serial.println("Start of pressurize");

  //pressure = Accurate_Pressure(10, 0.2, 1);

  //Loops until pressure reaches target
  Serial.println("Accurate");
  while(1) {
    if (Target == pressure) {
      Serial.println("Equal");
      digitalWrite(Sprinkler_Valve, LOW);
      digitalWrite(Brass_Valve, LOW);
      pressure = Pressure_Read();
      if (Target == pressure) {
        break;
      }
    } 
    else if (Target < pressure) {
      digitalWrite(Sprinkler_Valve, HIGH);
      digitalWrite(Brass_Valve, LOW);
      delay(100);
      Index =+ 0.1;
    }
    else if (Target > pressure) {
      digitalWrite(Sprinkler_Valve, LOW);
      digitalWrite(Brass_Valve, HIGH);
      delay(100);
      Index =+ 0.1;
    }
    //Stuff to do after pressure check, such as handling server every .5 sec and updating pressure
    if (Index >= 0.5) {
      server.handleClient();
      Index = 0;
    }
    pressure = Pressure_Read();
  }
  return;
}

//When launch signal is recieved
void Launch(int Reference) {
  /*pressure = Accurate_Pressure(5, 0.2, 1);
  if (Reference > pressure) {
    Set_PSI(Reference);
  }*/
  digitalWrite(Sprinkler_Valve, HIGH);
  delay(750);
  digitalWrite(Sprinkler_Valve, LOW);
  pressure = Pressure_Read();
  server.handleClient();
  return;
}

// Handler for sending pressure data
// Handler for sending pressure data
void handleData() {
  String data = String(pressure, 2);
  server.send(200, "text/plain", data);
}

// Handler for storing the user input
void handleStore() {
  if (server.hasArg("value")) {
    int targetPSI = server.arg("value").toInt();
    Serial.print("Stored value: ");
    Serial.println(targetPSI);
  }
  server.send(200, "text/plain", "Value stored");
}

// Handler for pressurize action
void handlePressurize() {
  Serial.println("handlestart");
  if (server.hasArg("value")) {
    int targetPSI = server.arg("value").toInt();
    Serial.print("before call");
    Set_PSI(targetPSI);
  }
  Serial.println("after if");
  server.send(200, "text/plain", "Pressurize action received");
}

// Handler for launch action
void handleLaunch() {
  if (server.hasArg("value")) {
    int targetPSI = server.arg("value").toInt();
    Launch(targetPSI);
  }
  server.send(200, "text/plain", "Launch action received");
}

void setup() {
  Serial.begin(115200);

  //Initialize pins used for input and output
  pinMode(Brass_Valve, OUTPUT);  
  pinMode(Sprinkler_Valve, OUTPUT); 
  pinMode(Pressure_Sensor, INPUT);


  //wifi setup
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
  server.on("/pressurize", HTTP_POST, handlePressurize);
  server.on("/launch", HTTP_POST, handleLaunch);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //Read pressure here
  pressure = Pressure_Read();

  int Pressure_Signal = analogRead(Pressure_Sensor);
  Serial.print("Signal value: ");
  Serial.println(Pressure_Signal);

  server.handleClient();
  delay(500);
}

