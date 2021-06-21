 /*
 MS5540C Miniature Barometer Module
 This program will read your MS5440C or compatible pressure sensor every 5 seconds and show you the calibration words, the calibration factors, 
 the raw values and the compensated values of temperature and pressure.
 Once you read out the calibration factors you can define them in the header of any sketch you write for the sensor.
 
Pins:
 MS5540 sensor attached to pins 10 - 13:
 MOSI: pin 11
 MISO: pin 12
 SCK: pin 13
 MCLK: pin 9 (or use external clock generator on 32kHz)
 CS for MS5540 isn't needed as 3 high pulses is START and 3 low pulses is STOP 
 CS for microSD card is pin 8
  
 
*/

/* Also added : 25/7/19
 the LCD display. Uses power from a battery and displays it on a LCD.     
*/

 // include libraries:
#include <SPI.h>
#include <LiquidCrystal.h>  //not yet used
#include <SD.h>
#include <RTClib.h>

 // generate a MCKL signal pin
 const int clock = 9;
 const int miso = 12;
 
 RTC_DS3231 rtc;
 File Filopn;

 char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

 
 // with the arduino pin number it is connected to
 // const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
 // LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void resetsensor() //this function keeps the sketch a little shorter
{
  SPI.setDataMode(SPI_MODE0); 
  SPI.transfer(0x15);
  SPI.transfer(0x55);
  SPI.transfer(0x40);
}

void setup() {
  // set up the LCD's number of columns and rows:
  //lcd.begin(16, 2);
  delay(5000);
  pinMode(10, OUTPUT);    
  Serial.begin(9600);
  // digitalWrite(8, LOW);
  SD.begin(8);    // Begin SPI communication for SD card with CS pin 8
  SD.mkdir("log1.txt");
  if (SD.exists("log1.txt")==true)
    Serial.println("yo");
  Filopn = SD.open("log.txt", FILE_WRITE);

  // setup for RTC 
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);

  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  SPI.begin();      //see SPI library details on arduino.cc for details
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV32);    //divide 16 MHz to communicate on 500 kHz
  pinMode(clock, OUTPUT);
  delay(100);
  }


void loop() 
{
  DateTime now = rtc.now();
  Serial.print("Temperature: ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");

  //so that CS pin for SD card module remains high ()  
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
    
  TCCR1B = (TCCR1B & 0xF8) | 1 ; //generates the MCKL signal
  analogWrite (clock, 128);
   
  resetsensor(); //resets the sensor - caution: afterwards mode = SPI_MODE0!

  //Calibration word 1
  unsigned int result1 = 0;
  unsigned int inbyte1 = 0;
  SPI.transfer(0x1D); //send first byte of command to get calibration word 1-29 in decimal
  SPI.transfer(0x50); //send second byte of command to get calibration word 1-80 in decimal
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  result1 = SPI.transfer(0x00); //send dummy byte to read first byte of word
  result1 = result1 << 8; //shift returned byte 
  inbyte1 = SPI.transfer(0x00); //send dummy byte to read second byte of word
  result1 = result1 | inbyte1; //combine first and second byte of word
  Serial.print("Calibration word 1 = ");
  Serial.print(result1, HEX);
  Serial.print(" ");  
  Serial.println(result1);  

  resetsensor();          //resets the sensor

  //Calibration word 2; see comments on calibration word 1
  unsigned int result2 = 0;
  byte inbyte2 = 0; 
  SPI.transfer(0x1D);
  SPI.transfer(0x60);
  SPI.setDataMode(SPI_MODE1); 
  result2 = SPI.transfer(0x00);
  result2 = result2 <<8;
  inbyte2 = SPI.transfer(0x00);
  result2 = result2 | inbyte2;
  Serial.print("Calibration word 2 = ");
  Serial.print(result2, HEX);  
  Serial.print(" ");  
  Serial.println(result2);  

  resetsensor(); //resets the sensor

  //Calibration word 3; 
  //see comments on calibration word 1
  unsigned int result3 = 0;
  byte inbyte3 = 0;
  SPI.transfer(0x1D);
  SPI.transfer(0x90); 
  SPI.setDataMode(SPI_MODE1); 
  result3 = SPI.transfer(0x00);
  result3 = result3 <<8;
  inbyte3 = SPI.transfer(0x00);
  result3 = result3 | inbyte3;
  Serial.print("Calibration word 3 = ");
  Serial.print(result3, HEX);  
  Serial.print(" ");  
  Serial.println(result3);  

  resetsensor(); //resets the sensor

  //Calibration word 4; see comments on calibration word 1
  unsigned int result4 = 0;
  byte inbyte4 = 0;
  SPI.transfer(0x1D);
  SPI.transfer(0xA0);
  SPI.setDataMode(SPI_MODE1); 
  result4 = SPI.transfer(0x00);
  result4 = result4 <<8;
  inbyte4 = SPI.transfer(0x00);
  result4 = result4 | inbyte4;
  Serial.print("Calibration word 4 = ");
  Serial.print(result4,HEX);
  Serial.print(" ");  
  Serial.println(result4);  
  
  //now we do some bitshifting to extract the calibration factors 
 //out of the calibration words; read datasheet AN510 for better understanding
 long c1 = result1 >> 1;
 long c2 = ((result3 & 0x3F) << 6) | (result4 & 0x3F);
 long c3 = (result4 >> 6);
 long c4 = (result3 >> 6);
 long c5 = (result2 >> 6) | ((result1 << 10)& 0x401);
 long c6 = result2 & 0x3F;

  Serial.print("c1 = ");
  Serial.println(c1);
  Serial.print("c2 = ");
  Serial.println(c2);
  Serial.print("c3 = ");
  Serial.println(c3);
  Serial.print("c4 = ");
  Serial.println(c4);
  Serial.print("c5 = ");
  Serial.println(c5);
  Serial.print("c6 = ");
  Serial.println(c6);

  resetsensor(); //resets the sensor

  //Pressure:
  int i=1;
  int Dsum=0;
  for (i=1; i<=8; ++i);
    unsigned int presMSB = 0; //first byte of value
    unsigned int presLSB = 0; //last byte of value
    unsigned int D1 = 0;
    SPI.transfer(0x0F); //send first byte of command to get pressure value
    SPI.transfer(0x40); //send second byte of command to get pressure value
    delay(35); //wait for conversion end
    SPI.setDataMode(SPI_MODE1); //change mode in order to listen
    presMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
    presMSB = presMSB << 8; //shift first byte
    presLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
    D1 = presMSB | presLSB; //combine first and second byte of value
    Dsum = Dsum + D1;
    
  Serial.print("D1 - Pressure raw = ");
  Serial.println(D1);
  

  resetsensor(); //resets the sensor  

  //Temperature:
  unsigned int tempMSB = 0; //first byte of value
  unsigned int tempLSB = 0; //last byte of value
  unsigned int D2 = 0;
  SPI.transfer(0x0F); //send first byte of command to get temperature value
  SPI.transfer(0x20); //send second byte of command to get temperature value
  delay(35); //wait for conversion end
  SPI.setDataMode(SPI_MODE1); //change mode in order to listen
  tempMSB = SPI.transfer(0x00); //send dummy byte to read first byte of value
  tempMSB = tempMSB << 8; //shift first byte
  tempLSB = SPI.transfer(0x00); //send dummy byte to read second byte of value
  D2 = tempMSB | tempLSB; //combine first and second byte of value
  Serial.print("D2 - Temperature raw = ");
  Serial.println(D2); //voila!

  //calculation of the real values by means of the calibration factors and the maths
  //in the datasheet. const MUST be long
  const long UT1 = (c5*8) + 20224;
  const long dT = D2 - UT1;
  const long TEMP = 200 + ((dT * (c6 + 50)) >> 10);
  const long OFF  = (c2 * 4) + (((c4 - 512) * dT) >> 12);
  const long SENS = c1 + ((c3 * dT) >> 10) + 24576;
  const long X = (SENS * (D1 - 7168) >> 14) - OFF;
  long PCOMP = ((X *10) >> 5) + 2500;
  float TEMPREAL = TEMP/10;
  //float PCOMPHG = PCOMP * 750.06 / 10000; // mbar*10 -> mmHg === ((mbar/10)/1000)*750/06
  float PCOMPH2o = (PCOMP *100)/98.1;
  
  /*
  Serial.print("UT1 = ");
  Serial.println(UT1);
  Serial.print("dT = ");
  Serial.println(dT);
  Serial.print("TEMP = ");
  Serial.println(TEMP);
  Serial.print("OFFP = ");
  Serial.println(OFF);
  Serial.print("SENS = ");
  Serial.println(SENS);
  Serial.print("X = ");
  Serial.println(X);
  */

  Serial.print("Real Temperature in C = ");
  Serial.println(TEMPREAL);
//
//  Serial.print("Compensated pressure in mbar = ");
//  Serial.println(PCOMP);
//  Serial.print("Compensated pressure in mmHg = ");
//  Serial.println(PCOMPHG);
//  Serial.print("Compensated pressure in mmH20 (Pure water)= ");
  Serial.println(PCOMPH2o);

  //2-nd order compensation only for T < 20°C or T > 45°C
   
  long T2 = 0;
  float P2 = 0;

  if (TEMP < 200)
    {
      T2 = (11 * (c6 + 24) * (200 - TEMP) * (200 - TEMP) ) >> 20;
      P2 = (3 * T2 * (PCOMP - 3500) ) >> 14;
    }
  else if (TEMP > 450)
    {
      T2 = (3 * (c6 + 24) * (450 - TEMP) * (450 - TEMP) ) >> 20;
      P2 = (T2 * (PCOMP - 10000) ) >> 13;    
    }

  if ((TEMP < 200) || (TEMP > 450))
  {
    const float TEMP2 = TEMP - T2;
    const float PCOMP2 = PCOMP - P2;

    float TEMPREAL2 = TEMP2/10;
    float PCOMPHG2 = PCOMP2 * 750.06 / 10000; // mbar*10 -> mmHg === ((mbar/10)/1000)*750/06
    float PCOMPH2o2 = (PCOMP2 *1000)/98.1;

    
//    Serial.print("2-nd Real Temperature in C = ");
//    Serial.println(TEMPREAL2);
//
//    Serial.print("2-nd Compensated pressure in mbar = ");
//    Serial.println(PCOMP2);
//    Serial.print("2-nd Compensated pressure in mmHg = ");
//    Serial.println(PCOMPHG2);
//    Serial.print("2-nd Compensated pressure in mmH20 = ");
    Serial.println(PCOMPH2o2);
  }  

  digitalWrite(8, LOW);

  digitalWrite(10, HIGH);

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  Filopn.print(PCOMPH2o);
  Filopn.print(' ');
  
  Filopn.print(now.month(), DEC);
  Filopn.print('/');
  Filopn.print(now.day(), DEC);
  Filopn.print('/');
  Filopn.print(now.year(), DEC);
  
  Filopn.print(" (");
  Filopn.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Filopn.print(") ");
  Filopn.print(now.hour(), DEC);
  Filopn.print(':');
  Filopn.print(now.minute(), DEC);
  Filopn.print(':');
  if (now.second()<10){
    Filopn.print("0");
    Filopn.print(now.second(), DEC);
  }
  else { 
    Filopn.print(now.second(), DEC);
  }
  Filopn.println();
  
  Filopn.flush();

  digitalWrite(10, LOW);

  //Every reading appears in delay seconds
  
  delay(5000);
  // when characters arrive over the serial port...

  /*
  if (Serial.available()) {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) {
      // display each character to the LCD
      lcd.write(Serial.read());
  delay(5000);
    }
  }
  */
}
