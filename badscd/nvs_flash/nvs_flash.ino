
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 02_Erase_and_Reformat_The_NVS_Memory
#include <nvs_flash.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  Serial.println("erase the NVS partition.");
  nvs_flash_erase();  // erase the NVS partition and...
  Serial.println("initialize the NVS partition.");
  nvs_flash_init();   // initialize the NVS partition.
  Serial.println("Finished.");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

