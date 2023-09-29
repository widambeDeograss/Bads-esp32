#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Define the pins for HX711 load cell module
const int loadCellDoutPin = D2;
const int loadCellSckPin = D3;

// Define the pins for LCD display
const int lcdAddress = 0x27;
const int lcdColumns = 16;
const int lcdRows = 2;

HX711 scale;

LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

// Calibration factor (adjust this value as needed)
const float calibrationFactor = 400.0; // Adjust this based on your load cell and known weight

void setup() {
  // Initialize the serial communication for debugging
  Serial.begin(9600);

  // Initialize the HX711 load cell
  scale.begin(loadCellDoutPin, loadCellSckPin);
  scale.set_scale(calibrationFactor); // Apply the calibration factor
  scale.tare(); // Reset the scale to zero

  // Initialize the LCD display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scale Calibration");
}

void loop() {
  // Read the weight in kilograms from the load cell
  float weightKg = scale.get_units(10); // Read the weight with a 10ms stabilization time

  // Display the weight on the LCD
  lcd.setCursor(0, 1);
  lcd.print("Weight (kg): ");
  lcd.print(weightKg, 2); // Display weight with 2 decimal places

  // Wait for a moment before taking the next reading
  delay(1000);
}


// Certainly! Let's say you have a scale with a maximum measurement capacity of 5 kilograms, and you want to calibrate it using a known weight of 2.5 kilograms. Here's how you can calculate the calibration factor:

// Calibration Factor = Known Weight (in grams) / Raw Value

// 1. Measure the known weight (2.5 kilograms) using the scale.

// 2. Note the raw value displayed on the LCD when the known weight is placed on the scale.

// 3. Calculate the calibration factor:

//    Calibration Factor = 2500 grams (2.5 kg) / Raw Value

// For example, if the raw value displayed when you place the 2.5-kilogram weight on the scale is 1250, then the calibration factor would be:

// Calibration Factor = 2500 grams / 1250 = 2 grams per raw unit

// Now, you can use this calibration factor (2 grams per raw unit) in your Arduino code:

// ```cpp
// const float calibrationFactor = 2.0; // Calibration factor for 2 grams per raw unit
// ```

// This calibration factor will convert the raw sensor readings into kilograms correctly for your specific setup, where 1 unit of the raw value represents 2 grams.