#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout_1 = 4; //mcu > HX711 no 1 dout pin
const int HX711_sck_1 = 5; //mcu > HX711 no 1 sck pin
const int HX711_dout_2 = 6; //mcu > HX711 no 2 dout pin
const int HX711_sck_2 = 7; //mcu > HX711 no 2 sck pin


//HX711 constructors:
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2);

const int calVal_eepromAdress1 = 0;
const int calVal_eepromAdress2 = 4; // Second EEPROM address

unsigned long t = 0;

void setup() {
    // Serial setup
    Serial.begin(57600); delay(10);
    Serial.println();
    Serial.println("Starting...");

    unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
    boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
    
    // Init LoadCell_1
    LoadCell_1.begin();
    LoadCell_1.start(stabilizingtime, _tare);
    if (LoadCell_1.getTareTimeoutFlag() || LoadCell_1.getSignalTimeoutFlag()) {
        Serial.println("LoadCell1 Timeout, check MCU>HX711 wiring and pin designations");
        while (1);
    } else {
        LoadCell_1.setCalFactor(1.0);
        Serial.println("Startup of LoadCell1 is complete");
    }

    // Init LoadCell_2
    LoadCell_2.begin();
    LoadCell_2.start(stabilizingtime, _tare);
    if (LoadCell_2.getTareTimeoutFlag() || LoadCell_2.getSignalTimeoutFlag()) {
        Serial.println("LoadCell2 Timeout, check MCU>HX711 wiring and pin designations");
        while (1);
    } else {
        LoadCell_2.setCalFactor(1.0);
        Serial.println("Startup of LoadCell2 is complete");
    }

    //wating update 
    while (!LoadCell_1.update());
    while (!LoadCell_2.update());
    
    calibrate(); //start calibration procedure
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update();

  //get smoothed value from data set
  if ((newDataReady)) {
    if (millis() > t + serialPrintInterval) {
      float a = LoadCell_1.getData();
      float b = LoadCell_2.getData();
      Serial.print("Load_cell 1 output val: ");
      Serial.print(a);
      Serial.print("    Load_cell 2 output val: ");
      Serial.println(b);
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
    else if (inByte == 'r') calibrate(); //calibrate
    //else if (inByte == 'c') changeSavedCalFactor(); //edit calibration value manually
  }

  //check if last tare operation is complete
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare load cell 2 complete");
  }
}

void calibrate() {
    // ... [similar code for LoadCell2]
    // You need to duplicate the calibration process for the second LoadCell. 
    // And remember to save/load calibration values to/from EEPROM at different addresses.
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the BOTH load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell_1.update();
    LoadCell_2.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') {
          LoadCell_1.tareNoDelay();
          LoadCell_2.tareNoDelay();
        }
      }
    }
    if (LoadCell_1.getTareStatus() == true && LoadCell_2.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the BOTH loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell_1.update();
    LoadCell_2.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell_1.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  LoadCell_2.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue_1 = LoadCell_1.getNewCalibration(known_mass); //get the new calibration value
  float newCalibrationValue_2 = LoadCell_2.getNewCalibration(known_mass); //get the new calibration value
  
  Serial.print("New calibration value 1 has been set to: ");
  Serial.println(newCalibrationValue_1);
  Serial.print("New calibration value 2 has been set to: ");
  Serial.println(newCalibrationValue_2);
  Serial.println("Use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress1);
  Serial.print(", and ");
  Serial.print(calVal_eepromAdress2);
  Serial.println(" ? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress1, newCalibrationValue_1);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress2, newCalibrationValue_2);
        Serial.print("Value ");
        Serial.print(newCalibrationValue_1, newCalibrationValue_2);
        Serial.print(" saved to EEPROM address: ");
        Serial.print(calVal_eepromAdress1);
        Serial.print(", and ");
        Serial.print(calVal_eepromAdress2);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  //Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");

}
