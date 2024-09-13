
#include <SoftwareSerial.h>              // include library for connecting and transmiting to the device(serial communication)
#include <LiquidCrystal.h>              // include the library for LCD display
#include "SparkFun_UHF_RFID_Reader.h"  // library for controlling the M6E Nano module

SoftwareSerial softSerial(2, 3); //RX, TX(defining RX and TX pins)
RFID nano; //create an instance to proceed specific functions

#define BUZZER1 10 //define pin 10 for the buzzer
#define BUZZER2 9 //define pin 9 for the buzzer

#define TAG_COUNT 4                                                                                       //number of RFID tags    
#define TAG_LENGTH 12                                                                                    //length of each RFID TAG
byte accessEPC[TAG_COUNT][TAG_LENGTH]={{0xE2,0x80,0x11,0x60,0x60,0x00,0x02,0x0D,0xBC,0x30,0x49,0x2A},
                                       {0xE2,0x80,0x11,0x60,0x60,0x00,0x02,0x0D,0xBC,0x30,0x1D,0xE9},     //adhesive tag 2
                                       {0xE2,0x80,0x69,0x95,0x00,0x00,0x40,0x01,0xBE,0x20,0x89,0xA6},    //tag de plastic
                                       {0xE2,0X00,0X00,0X1B,0X95,0X17,0X02,0X52,0X24,0X20,0X20,0X7E}};  //tag card CR-007

LiquidCrystal lcd(11,12,4,5,6,7); //LiquidCrystal(rs, enable, d4, d5, d6, d7)-syntax

//boolean tagDetected; //Keeps track of when we've beeped    

void setup()
{
  Serial.begin(115200);                      //Serial.begin(speed) set the data transfer speed to: 115200[bit/second]; we need to have this speed the same with the one selected from the serial monitor
  lcd.begin(16, 2);                         //lcd.begin(colums, rows)
  lcd.clear();                             //clears the screen and positions the cursor on position (0,0)
  lcd.print("Starting device");           // Print a message to the LCD.

  pinMode(BUZZER1, OUTPUT);                //sets the digital pin BUZZER1 as output
  pinMode(BUZZER2, OUTPUT);               //sets the digital pin BUZZER2 as output
  digitalWrite(BUZZER2, LOW);            //put half of the buzzer to ground and works with the other half(sets the digital pin BUZZER2 to off(0V))

  while (!Serial);                                                     //wait for the serial port to come online, waiting for the connexion to serial monitor
  Serial.println();                                                    //blank space left
  Serial.println("Waiting for the connexion...");
  lcd.clear();
  lcd.print("Waiting...");

  if (setupNano(38400) == false)                                           //configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Check Wiring!"));  //if we use this syntax with that F() it stands for the fact that the string is moved to the program memory instead of the RAM memory
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connexion failed");
    lcd.setCursor(0,1);
    lcd.print("Check wiring!");
    while (1);                                                             //freeze
  }
  nano.setRegion(REGION_EUROPE);   //Set to EUROPE
  nano.setReadPower(2700);        //Limited read power -> 27dBm 
}
void loop()
{
  lcd.clear();
  lcd.home();
  lcd.print("Searching tag...");
  
  Serial.println(F("Press a key to scan for a tag"));       // daca comentam astea 3 randuri nu ne mai pune sa dam caracter?
  while (!Serial.available()); //Wait for user to send a character
  Serial.read(); //Throw away the user's character
 

  byte myEPC[12];                           //the tags we use have EPCs of 12 bytes
  byte myEPClength;                         //variable to stock the length of each EPC
  byte responseType = 0;                    //variable to stock the type of answer

  while (responseType != RESPONSE_SUCCESS)                      //RESPONSE_IS_TAGFOUND)//this loop waits for the tag to be succesfull read.
    {
    myEPClength = sizeof(myEPC);                                //length of EPC is modified each time .readTagEPC is called; sets it to 12
    responseType = nano.readTagEPC(myEPC, myEPClength, 500);   //calls for the readTagEPC ;scan for a new tag up to 500ms
    Serial.println(F("Searching tag"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Searching tag");
    }

  tone(BUZZER1, 2093, 150);                                     //C //piano keys to frequencies: http://www.sengpielaudio.com/KeyboardAndFrequencies.gif 
  delay(150);
  tone(BUZZER1, 2349, 150);                                    //D//tone(pin, frequency, duration)
  delay(150);
  tone(BUZZER1, 2637, 150);                                   //E
  delay(150);

  Serial.print(F(" epc["));                            //print epc on serial monitor
  for (byte x = 0 ; x < myEPClength ; x++)
  {
    if (myEPC[x] < 0x10) Serial.print(F("0"));
    Serial.print(myEPC[x], HEX);
    Serial.print(F(" "));
  }
  Serial.println(F("]")); 

boolean accessGranted = false;                               // loop through each element in accessEPC array
for (int i = 0; i < 4; i++) {                               // reads the number of tags that are in the for(loop through each element in accessEPC array); we put 4 there because there are 4 elements in accessEPC array

  if (memcmp(myEPC, accessEPC[i], TAG_LENGTH) == 0) {     // compare the EPC of the current tag (myEPC) with the stored EPCs (accessEPC)
    accessGranted = true;                                // If a match is found, set accessGranted to true and exit the loop
    break;
  }
}
// check if access is granted or denied 
if (accessGranted) {
  highBeep();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access granted");
  Serial.println("Access granted");
} else {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access denied");
  Serial.println("Access denied");
}
delay(5000); // delay after the display of the message
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tells to the library to communicate over software serial port
  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate

  while (!softSerial); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200.
  while (softSerial.available()) softSerial.read();// ignores the data at the beggining

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a continuous read
    nano.stopReading();
    Serial.println(F("Module continuously reading. Asking it to stop..."));
    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200
    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg
    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate
  }
  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right
  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2
  nano.setAntennaPort(); //Set TX/RX antenna ports to 1
  return (true); //We are ready to rock
}

void lowBeep()
{
  tone(BUZZER1, 130, 200); //Low C//tone(pin,frequency of the tone,time duration of the tone)
  delay(350);
}

void highBeep()
{
  tone(BUZZER1, 2093, 200); //High C
  delay(350);
}