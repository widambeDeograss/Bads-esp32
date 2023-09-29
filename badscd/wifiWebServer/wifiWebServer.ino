
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 05_ESP32_Digital_Scale_Web_Server_STA_Mode
//----------------------------------------Including the libraries.

#include <LiquidCrystal_I2C.h>  // https://github.com/marcoschwartz/LiquidCrystal_I2C
#include "HX711.h"
#include <Preferences.h>
#include <AsyncTCP.h>           // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebSrv.h>  // https://github.com/me-no-dev/ESPAsyncWebServer
#include <Arduino_JSON.h>
#include "PageIndex.h"
//----------------------------------------

//----------------------------------------Defines the connected PIN between HX711 and ESP32.
#define LOADCELL_DOUT_PIN 5
#define LOADCELL_SCK_PIN  18
//----------------------------------------

// Defines the Button PIN.
#define BTN_MODE_PIN 15

/////////////////// ENTER THE WEIGHT OF OBJECTS WHICH WEIGHT IS ALREADY KNOWN FOR THE CALIBRATION PROCESS ///////////////////
// Defines the weight of an object whose weight is known for the calibration process.
// Before the calibration process, prepare an object with a known weight in grams.
// Then enter the weight value into the "weight_of_object_for_calibration" variable below.
// Use objects weighing 100 grams or more for calibration.
// In this project I used an object weighing 181 grams for calibration.

#define weight_of_object_for_calibration 900
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------SSID and PASSWORD of your WiFi network.
const char* ssid = "eGANET";  //--> Your wifi name
const char* password = "eG@_2022"; //--> Your wifi password
//----------------------------------------

// Timer variables to send weighing results to the server.
unsigned long lastTime = 0;
unsigned long timerDelay = 1500;

// Float variable to load the calibration values stored in the flash memory.
float LOAD_CALIBRATION_FACTOR;

int weight_In_g;        // Int variable to hold the value of the weighing results in units of grams (g).
int last_weight_In_g;   // Int variable to hold the value of the last weighing result in units of grams (g).
float weight_In_oz;     // Float variable to hold the value of the weighing results in units of ounce (oz).

// The variables used to check the parameters passed in the URL.
// Look in the "PageIndex.h" file.
// xhr.open("GET", "BTN_Comd?BTN_Tare=T", true);
// For example :
// BTN_Comd?BTN_Tare=T
// PARAM_INPUT_1 = T
const char* PARAM_INPUT_1 = "BTN_Tare";

// Variable declaration to hold data from the web server.
String BTN_Tare = "";

// Initialize the LiquidCrystal_I2C lib as "lcd" and set the LCD I2C address to 0x27 and set the LCD configuration to 16 x 2.
// In general, the address of a 16x2 I2C LCD is "0x27".
// However, if the address "0x27" doesn't work, you can find out the address with "i2c_scanner". Look here : https://playground.arduino.cc/Main/I2cScanner/
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize the HX711 library as LOADCELL_HX711.
HX711 LOADCELL_HX711;

// Initialize the Preferences library as preferences.
Preferences preferences;

// Initialize JSONVar
JSONVar JSON_All_Data;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

//________________________________________________________________________________scale_Tare()
void scale_Tare() {
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Tare...");
  delay(1000);

  if (LOADCELL_HX711.is_ready()) {
    // Reset the scale to 0 so that the weight reading starts at 0.
    LOADCELL_HX711.tare(); // reset the scale to 0.
    delay(500);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    while (1) {
      delay(1000);
    }
  }
}
//________________________________________________________________________________

//________________________________________________________________________________loadcell_Calibration()
void loadcell_Calibration() {
  lcd.clear();
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CALIBRATION MODE");
  delay(1500);

  for (byte i = 0; i < 2; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Do not place");
    lcd.setCursor(0, 1);
    lcd.print("any object");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("on the scale.");
    delay(1500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please wait...");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start");
  lcd.setCursor(0, 1);
  lcd.print("calibration...");
  delay(1000);

  if (LOADCELL_HX711.is_ready()) {
    // Set the scale.
    LOADCELL_HX711.set_scale();
    delay(100);

    // Reset the scale to 0.
    LOADCELL_HX711.tare();
    delay(100);

    for (byte i = 0; i < 2; i++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Place an object");
      lcd.setCursor(0, 1);
      lcd.print("on the scale");
      delay(1500);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("weighing 150 g");
      lcd.setCursor(9, 0);
      lcd.print(weight_of_object_for_calibration);
      lcd.print(" g");
      delay(1500);
    }

    lcd.clear();
    delay(1000);

    long sensor_Reading_Results = 0;

    for (byte i = 0; i < 5; i++) {
      sensor_Reading_Results = LOADCELL_HX711.get_units(10);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor reading ");
      lcd.print(i + 1);
      lcd.setCursor(0, 1);
      lcd.print(sensor_Reading_Results);
      delay(1000);
    }

    lcd.clear();
    delay(1000);

    // Formula to get CALIBRATION_FACTOR value.
    float CALIBRATION_FACTOR = sensor_Reading_Results / weight_of_object_for_calibration;

    // Saves calibration values to flash memory.
    preferences.putFloat("CFVal", CALIBRATION_FACTOR);
    delay(100);

    // Load calibration values stored in flash memory.
    LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0);
    delay(100);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibration Val:");
    lcd.setCursor(0, 1);
    lcd.print(LOAD_CALIBRATION_FACTOR);
    delay(1500);

    // Set the scale with the CALIBRATION_FACTOR value.
    LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
    delay(100);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibration");
    lcd.setCursor(0, 1);
    lcd.print("complete.");
    delay(1500);

    lcd.clear();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    while (1) {
      delay(1000);
    }
  }
}
//________________________________________________________________________________

//________________________________________________________________________________VOID SETUP()
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  Serial.println();
  Serial.println("Start Setup...");
  delay(100);

  pinMode(BTN_MODE_PIN, INPUT_PULLUP);
  delay(100);

  Serial.println();
  Serial.println("Open Preferences with namespace \"CF\" on flash memory.");
  preferences.begin("CF", false);
  delay(100);

  Serial.println();
  Serial.println("Load calibration values stored in flash memory.");
  // Loads the calibration value stored in the flash memory after the calibration process is complete.
  LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0);
  delay(500);

  Serial.println();
  Serial.print("CALIBRATION_FACTOR : ");
  Serial.println(LOAD_CALIBRATION_FACTOR);
  delay(500);

  Serial.println();
  Serial.println("Setting up the LCD.");
  delay(500);
  // Initialize lcd.
  lcd.init();
  // Turn on the LED backlight on the LCD.
  lcd.backlight();

  Serial.println();
  Serial.println("Do not place any object on the scale until setup is complete.");
  delay(500);

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Do not place");
  lcd.setCursor(0, 1);
  lcd.print("any object");
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("on the scale");
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("until setup");
  lcd.setCursor(0, 1);
  lcd.print("is complete");
  delay(1500);

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Start setup...");
  delay(1500);

  lcd.clear();

  LOADCELL_HX711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(100);

  // Set the scale with the CALIBRATION_FACTOR value.
  LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
  delay(100);

  // Reset the scale to 0 so that the weight reading starts at 0.
  LOADCELL_HX711.tare();
  delay(100);

  //---------------------------------------- Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  //---------------------------------------- 

  delay(100);

  //---------------------------------------- Connect to Wi-Fi (STA).
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  delay(1500);
  
  Serial.println("------------");
  Serial.println("WIFI STA");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.

  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("------------");

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Connected");
  lcd.setCursor(0, 1);
  lcd.print("successfully.");
  delay(1500);
  lcd.clear();
  //::::::::::::::::::
  //----------------------------------------

  delay(500);

  //---------------------------------------- Handle Web Server
  Serial.println();
  Serial.println("Setting Up the Main Page on the Server.");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", MAIN_page);
  });
  //----------------------------------------

  //---------------------------------------- Handle Web Server Events
  Serial.println();
  Serial.println("Setting up event sources on the Server.");
  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 10 second
    client->send("hello!", NULL, millis(), 10000);
  });
  //----------------------------------------

  //---------------------------------------- Send a GET request to <ESP_IP>/BTN_Comd?BTN_Tare=<inputMessage1>
  server.on("/BTN_Comd", HTTP_GET, [] (AsyncWebServerRequest * request) {
    //::::::::::::::::::
    // GET input value on <ESP_IP>/BTN_Comd?BTN_Tare=<inputMessage1>
    // PARAM_INPUT_1 = inputMessage1
    // BTN_Tare = PARAM_INPUT_1
    //::::::::::::::::::

    if (request->hasParam(PARAM_INPUT_1)) {
      BTN_Tare = request->getParam(PARAM_INPUT_1)->value();

      Serial.println();
      Serial.print("BTN_Tare : ");
      Serial.println(BTN_Tare);
    }
    else {
      BTN_Tare = "No message";
      Serial.println();
      Serial.print("BTN_Tare : ");
      Serial.println(BTN_Tare);
    }
    request->send(200, "text/plain", "OK");
  });
  //----------------------------------------

  //---------------------------------------- Adding event sources on the Server.
  Serial.println();
  Serial.println("Adding event sources on the Server.");
  server.addHandler(&events);
  //----------------------------------------

  //---------------------------------------- Starting the Server.
  Serial.println();
  Serial.println("Starting the Server.");
  server.begin();
  //---------------------------------------- 

  Serial.println();
  Serial.println("------------");
  Serial.print("ESP32 IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Visit the IP Address above in your browser to open the main page.");
  Serial.println("------------");
  Serial.println();

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("IP Address :");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(4000);

  Serial.println();
  Serial.print("Setup complete.");
  delay(500);

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Setup complete.");
  delay(1500);

  Serial.println();
  Serial.println("The scale is ready to use.");
  delay(500);

  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("The scale");
  lcd.setCursor(0, 1);
  lcd.print("is ready to use.");
  delay(1500);

  lcd.clear();
  delay(500);
}
//________________________________________________________________________________

//________________________________________________________________________________VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  //----------------------------------------The process of reading the weight and displaying it on the LCD.
  // Acquire reading without blocking.
  // It is also possible to define a maximum timeout to wait for the hardware to be initialized.
  // This won't send the program into a spinlock when the scale is disconnected and will probably also account for hardware failures.
  // Source : https://github.com/bogde/HX711#non-blocking-mode
  if (LOADCELL_HX711.wait_ready_timeout(100)) {
    // From the experiments I did, if you want a stable weighing value, use get_units(5), or you can replace the value 5 with a value of 6 to 10.
    // But the effect is to get the sensor reading results to be slower.

    // Get the reading of the object's weight in grams (g).
    weight_In_g = LOADCELL_HX711.get_units(2);

    // Get the reading of the object's weight in ounce (oz).
    // The ounce is a unit of mass that has several definitions depending on the system of units.
    // As an example :
    // - If you use "International avoirdupois ounce", then 1 ounce = 28.349523125 grams.
    // - If you use "French ounce (once)", then 1 ounce = 30.59 grams.
    // - For more details, you can see here: https://en.wikipedia.org/wiki/Ounce.
    // - In this project, I use "International avoirdupois ounce".
    weight_In_oz = float(weight_In_g) / 28.34952;

    //::::::::::::::::::The condition for refreshing the LCD display is only in the weight value section.
    if (last_weight_In_g != weight_In_g) {
      lcd.setCursor(7, 0);
      lcd.print("       ");
      lcd.setCursor(7, 1);
      lcd.print("       ");
    }
    //::::::::::::::::::

    lcd.setCursor(0, 0);
    lcd.print("Weight:");

    String check_isNegative;

    //::::::::::::::::::Condition for displaying the weight value (weighing result) in gram (g) units on the LCD with right alignment.
    byte x_pos_g;
    check_isNegative = String(weight_In_g);

    if (abs(weight_In_g) >= 0 && abs(weight_In_g) <= 9) {
      x_pos_g = 12;
    } else if (abs(weight_In_g) >= 10 && abs(weight_In_g) <= 99) {
      x_pos_g = 11;
    } else if (abs(weight_In_g) >= 100 && abs(weight_In_g) <= 999) {
      x_pos_g = 10;
    } else if (abs(weight_In_g) >= 1000 && abs(weight_In_g) <= 5000) {  // I'm using a loadcell sensor with a maximum load weight of 5000 g (5 kg).
      x_pos_g = 9;
    }

    if (check_isNegative[0] == '-') x_pos_g = x_pos_g - 1;

    lcd.setCursor(x_pos_g, 0);
    lcd.print(weight_In_g);

    lcd.setCursor(15, 0);
    lcd.print("g");
    //::::::::::::::::::

    check_isNegative = "";

    //::::::::::::::::::Condition for displaying the weight value (weighing result) in ounces (oz) units on the LCD with right alignment.
    byte x_pos_oz;
    check_isNegative = String(weight_In_oz);

    if (abs(weight_In_oz) >= 0 && abs(weight_In_oz) <= 9.99) {
      x_pos_oz = 9;
    } else if (abs(weight_In_oz) >= 10 && abs(weight_In_oz) <= 99.99) {
      x_pos_oz = 8;
    } else if (abs(weight_In_oz) >= 100 && abs(weight_In_oz) <= 177.00) {  // I'm using a loadcell sensor with a maximum load weight of 5000 g (5 kg) = approximately 176.37 ounces (oz).
      x_pos_oz = 7;
    }

    if (check_isNegative[0] == '-') x_pos_oz = x_pos_oz - 1;

    lcd.setCursor(x_pos_oz, 1);
    lcd.print(weight_In_oz, 2);

    lcd.setCursor(14, 1);
    lcd.print("oz");
    //::::::::::::::::::

    // Stores the last weight value (weighing value).
    last_weight_In_g = weight_In_g;

    //Serial.print("Weight : ");
    //Serial.print(weight_In_g);
    //Serial.print(" g");
    //Serial.print(" | ");
    //Serial.print(weight_In_oz, 2);
    //Serial.println(" oz");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error...");
    lcd.setCursor(0, 1);
    lcd.print("HX711 not found.");
    while (1) {
      delay(1000);
    }
  }
  //----------------------------------------

  //----------------------------------------Timer to send weighing results to the server.
  // The weighing result is sent to the server every 1500 milliseconds (see "timerDelay" variable).
  if ((millis() - lastTime) > timerDelay) {
    // Enter the data into JSONVar(JSON_All_Data).
    JSON_All_Data["weight_In_g"] = weight_In_g;

    // Create a JSON String to hold all data.
    String JSON_All_Data_Send = JSON.stringify(JSON_All_Data);

    // Send Event to client with weighing result value in gram units Every 1500 milliseconds.
    events.send("ping", NULL, millis());
    events.send(JSON_All_Data_Send.c_str(), "allDataJSON" , millis());
    lastTime = millis();
  }
  //----------------------------------------

  //----------------------------------------Calling the "scale_Tare()" function via the web page.
  if (BTN_Tare == "T") {
    BTN_Tare = "";
    Serial.println();
    Serial.println("Tare...");
    scale_Tare();
  }
  //----------------------------------------

  //----------------------------------------The condition that is executed when the "MODE" button is pressed.
  // - If the "MODE" button is pressed once, it will call the "TARE" function. The "TARE" function is useful for ignoring the weight of certain objects in the weighing process.
  //   For example, if you want to weigh the water in a cup, you must ignore the weight of the cup to get the net weight of the water.
  //   To do that, you place the glass first on the scale, then press the "MODE" button once, then the weight reading will be 0 (the cup weight is ignored).
  //   Then you can pour water into a cup to get the weight value of the water.
  //
  // - If the "MODE" button is long pressed until the text "CALIBRATION MODE" appears on the LCD, then release the button, then it will go to calibration mode.
  //   Calibration mode is useful for calibrating loadcell sensors.
  if (digitalRead(BTN_MODE_PIN) == LOW) {
    byte btn_mode_cnt = 0;

    while (digitalRead(BTN_MODE_PIN) == LOW) {
      btn_mode_cnt++;
      if (btn_mode_cnt > 3) {
        loadcell_Calibration();
        break;
      }
      delay(1000);
    }

    while (digitalRead(BTN_MODE_PIN) == HIGH) {
      if (btn_mode_cnt < 3) scale_Tare();
      break;
    }
  }
  //----------------------------------------
}
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



