//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 04_Test_LCD_16x2_I2C_and_Button
//----------------------------------------Including the libraries.
#include <LiquidCrystal_I2C.h>  // https://github.com/marcoschwartz/LiquidCrystal_I2C
//----------------------------------------

#define BTN_PIN 0  // Defines the Button PIN.

// Initialize the LiquidCrystal_I2C lib as "lcd" and set the LCD I2C address to 0x27 and set the LCD configuration to 16 x 2.
// In general, the address of a 16x2 I2C LCD is "0x27".
// However, if the address "0x27" doesn't work, you can find out the address with "i2c_scanner". Look here : https://playground.arduino.cc/Main/I2cScanner/
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  pinMode(BTN_PIN, INPUT_PULLUP);
  // Initialize lcd.
  lcd.init();
  // Turn on the LED backlight on the LCD.
  lcd.backlight();
  // Clean the LCD display.
  delay(500);

}

void loop() {
  // put your main code here, to run repeatedly:

  static uint8_t bStATE = 3;
  bStATE = buttonEvent(BTN_PIN);
  // lcd.clear();

  if (bStATE == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("BTN_PIN State :");
    Serial.println("BTN_PIN State :" + String(bStATE));

    lcd.setCursor(0, 1);
    lcd.print(bStATE);
  }
  lcd.print("BTN_PIN State :");
  lcd.clear();
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

uint8_t buttonEvent(uint8_t bt) {
  static uint8_t prevStATE = HIGH;
  static uint32_t lastT = millis();

  if (digitalRead(bt) != prevStATE) {
    if (millis() - prevStATE >= 50) {
      prevStATE = !prevStATE;
      lastT = millis();
      return prevStATE;
    }
  }

  return 5;
}
