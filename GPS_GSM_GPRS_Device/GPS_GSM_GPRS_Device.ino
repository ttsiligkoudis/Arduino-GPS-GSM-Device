/*Project: Ai Thinker A7 GSM GPRS GPS module
The module U_RXD pin is connected to Arduino pin 9
The module U_TXD pin is connected to Arduino pin 8
The module PWR_KEY pin is connected to Arduino pin 7
The module V_BAT pin is connected to Arduino 5V pin
The module GND pin is connected to Arduino GND pin*/
//*************************************************************
#include <TinyGPS++.h> //This library provides compact and easy-to-use methods for extracting position, date, time, altitude, speed, and course from consumer GPS devices.
#include <AltSoftSerial.h> //Include all relevant libraries, emulate an additional serial port, allowing you to communicate with another serial device
#include <avr/sleep.h>  //This AVR library contains the methods that controls the sleep modes
#include <PinChangeInt.h>
//*************************************************************
TinyGPSPlus GPS; // The TinyGPS++ object for interfacing with the GPS
AltSoftSerial GSM; // The serial connection object to the GPS device
int i, j, sec, min, hour, day, month, year; // Variables to store time and date
byte sensorRead = 0; // Variable for operating the sensor
float lat[50], lng[50]; // Variables to store the GPS coordinates
String text; // String to receive our SMS keyword (L1 and L2 are our keywords)
boolean multiplesignals = false, wakeUp = false, NumberFound = false, wait, powersave = false; // flag to check what action you want
unsigned long lastMillis = 0, lastMotion = 0; // Enable loops based on time
char number[14] = {}; //13 chars + null
char* pch;
//*************************************************************
void setup()
{
  pinMode(5, INPUT_PULLUP);
  pinMode(6, OUTPUT); // Mark Arduino pin 6 as output pin (Module power)
  pinMode(7, OUTPUT); // Mark Arduino pin 7 as output pin (PWR_KEY on module)
  Initialise();
}
//*************************************************************
void Initialise()
{
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH); // Press the PWR_KEY in order to activate the module automatically
  delay(2000); // Add a delay to perform the previous action
  digitalWrite(7, LOW); // Release the PWR_KEY
  delay(10000); // Add a delay to perform the previous action
  GSM.begin(115200); //Default baud-rate for the board
  delay(200);
  GSM.println(F("AT+IPR=9600")); // Change baud-rate for better communication
  delay(600);
  Serial.begin(9600); // Begin the new serial connection
  GSM.begin(9600); // Begin the new serial connection for the board
  delay(200);
  Serial.println(F("Welcome to my Arduino project")); // The (F("")) acts like a macro to store these strings to program storage space and free-up dynamic memory
  GSM.println(F("AT")); // Communication test command
  delay(500);
  Serial.println(F("Initialising"));
  GSM.println(F("AT+GPS=1")); // Enable GPS
  delay(500);
  Serial.println(F("."));
  delay(500);
  Serial.println(F("."));
  GSM.println(F("AT+CMGF=1")); // Enable Text-mode
  delay(500);
  Serial.println(F("."));
  delay(500);
  Serial.println(F("Operation started SUCCESSFULLY"));
  GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow (just in case)
  delay(1000);
}
//*************************************************************
void loop()
{
  if (GSM.available() > 0) //If there is stuff in the buffer
  {
    Serial.println(F("Waiting for Messages"));
    char textMessage[100] = {};
    byte numChars = GSM.readBytes(textMessage, 100);//.readBytes returns number read
    textMessage[numChars] = '\0';//null Terminator
    Serial.println(textMessage); // Print the new message
    pch = (strstr(textMessage, "+3069"));
    if (pch)
    {
      Serial.println("Number found");
      strncpy(number, pch, 13);//copy 13 characters after pointer into number
      Serial.println(number);
    }
    else
      Serial.println("Number not found");
    pch = (strstr(textMessage, "L1")); // If our keyword is "L1", get single coordinates and send SMS
    if (pch)
    {
      Serial.println(F("Received msg"));
      Serial.println(F("L1"));
      Serial.println(F("Valid code"));
      GSM.println(F("AT+GPS=1")); // Enable GPS
      delay(1000);
      GSM.println(F("AT+GPSRD=1")); // Enable the GPS Raw Data flow to get coordinates
      delay(1000);
      Serial.println(F("Extracting Coordinates"));
      lat[0]=lng[0]=0;
      do{
        while (GSM.available() > 0) //While there is stuff in the buffer
          if (GPS.encode(GSM.read())) // If it can successfully decode it, do it. Else try again when more charachters are in the buffer
            DisplayInfo(); // Our Function to get coordinates
      }while (lat[0] == 0 || lng[0] == 0 || day == 0 || month == 0 || year == 0 || hour == 0 || min == 0 || sec == 0); // Stop the loop only when all variables are valid
      GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow to send SMS
      lat[1] = lat[0]; // Store our Latitude in a table
      lng[1] = lng[0]; // Store our Longtitude in a table
      delay(1000);
      while (GSM.available())  // forward data from GSM to monitor
      Serial.write(GSM.read());
      multiplesignals = false;
      SendMsg(1, number); // Our Function to sent SMS
      delay(2000);
      while (GSM.available())  // forward data from GSM to monitor
      Serial.write(GSM.read());
      //SendEmail(0);
    }
    pch = (strstr(textMessage, "L2"));
    if (pch) // If our keyword is "L2", get several coordinates and send SMS
    {
      Serial.println(F("Received msg"));
      Serial.println(F("L2"));
      Serial.println(F("Valid code"));
      GSM.println(F("AT+GPS=1")); // Enable GPS
      delay(1000);
      GSM.println(F("AT+GPSRD=1")); // Enable the GPS Raw Data flow to get coordinates
      delay(1000);
      Serial.println(F("Extracting Several Coordinates"));
      Serial.println(F("Please wait approximately 6 minutes"));
      i=1;
      while(i<=3) // Loop to get 3 different coordinates with 2 minutes difference  with each other
      {
        if (millis() - lastMillis >= 2*60*1000UL) // This helps us to get coordinates only every 2 minutes
        {
          lastMillis = millis(); // Store our last time so we can continue the loop
          lat[0]=lng[0]=0;
          do{
            while (GSM.available() > 0) //While there is stuff in the buffer
              if (GPS.encode(GSM.read())) // If it can successfully decode it, do it. Else try again when more charachters are in the buffer
                DisplayInfo(); // Our Function to get coordinates
          }while (lat[0] == 0 || lng[0] == 0 || day == 0 || month == 0 || year == 0 || hour == 0 || min == 0 || sec == 0); // Stop the loop only when all variables are valid
          lat[i] = lat[0]; // Store our Latitude in a table
          lng[i] = lng[0]; // Store our Longtitude in a table
          i++;
        }
      }
      GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow to send SMS
      delay(1000);
      while (GSM.available())  // forward data from GSM to monitor
      Serial.write(GSM.read());
      multiplesignals=true; // Flag used to decide which action we choose based on our keywords
      SendMsg(i-1, number); // Our Function to sent SMS
      delay(2000);
      while (GSM.available())  // forward data from GSM to monitor
      Serial.write(GSM.read());
      //SendEmail(i-1);
    }
    pch = (strstr(textMessage, "L3"));
    if (pch) // If our keyword is "L3", Send Coordinated through SMS until the the device stops moving
    {
      Serial.println(F("Received msg"));
      Serial.println(F("L3"));
      Serial.println(F("Valid code"));
      GSM.println(F("AT+GPS=1")); // Enable GPS
      delay(1000);
      Serial.println(F("Extracting Several Coordinates"));
      i = 1;
      multiplesignals = false;
      do // Loop to get different coordinates per minute
      {
        if (millis() - lastMillis >= 60*1000UL) // This helps us to get coordinates only every minute
        {
          GSM.println(F("AT+GPSRD=1")); // Enable the GPS Raw Data flow to get coordinates
          delay(1000);
          lastMillis = millis(); // Store our last time so we can continue the loop
          lat[0]=lng[0]=0;
          do{
            while (GSM.available() > 0) //While there is stuff in the buffer
              if (GPS.encode(GSM.read())) // If it can successfully decode it, do it. Else try again when more charachters are in the buffer
                DisplayInfo(); // Our Function to get coordinates
          }while (lat[0] == 0 || lng[0] == 0 || day == 0 || month == 0 || year == 0 || hour == 0 || min == 0 || sec == 0); // Stop the loop only when all variables are valid
          lat[i] = lat[0]; // Store our Latitude in a table
          lng[i] = lng[0]; // Store our Longtitude in a table
          GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow to send SMS
          delay(1000);
          while (GSM.available())  // forward data from GSM to monitor
          Serial.write(GSM.read());
          if(i != 1)
            multiplesignals = true;
          SendMsg(i, number); // Our Function to sent SMS
          delay(2000);
          while (GSM.available())  // forward data from GSM to monitor
            Serial.write(GSM.read());
          //SendEmail(i);
          i++;
        }
        if(lat[i] == lat[i-1] && lng[i] == lng[i-1] && lat[i] == lat[i-2] && lng[i] == lng[i-2])
          break;
      }while(true);
    }
    pch = (strstr(textMessage, "L4"));
    if (pch) // If our keyword is "L4", Disable the device
    {
      Serial.println(F("Received msg"));
      Serial.println(F("L4"));
      Serial.println(F("Valid code"));
      digitalWrite(6, LOW);
      delay(2000);
      Serial.println(F("Turning device off"));
      powersave = true;
    }
    pch = (strstr(textMessage, "L5"));
    if (pch) // If our keyword is "L5", turn off only the GPS function
    {
      Serial.println(F("Received msg"));
      Serial.println(F("L5"));
      Serial.println(F("Valid code"));
      GSM.println(F("AT+GPS=0")); // Enable GPS
      delay(1000);
      powersave = true;
    }
    pch = (strstr(textMessage, "Reset"));
    if (pch) // If our keyword is "Reset", Reset the device
    {
      Serial.println(F("Received msg"));
      Serial.println(F("Reset"));
      Serial.println(F("Valid code"));
      digitalWrite(6, LOW);
      delay(2000);
      Serial.println(F("Reseting device"));
      Initialise();
    }
  }
  Sensor(); // Checks if the device is moved to wake up the module
  /*if (millis() - lastMotion >= 60*1000UL)
    Going_To_Sleep();
  if (wakeUp == true)
  {
    GSM.println(F("AT+GPSRD=1")); // Enable the GPS Raw Data flow to get coordinates
    delay(1000);
    Serial.println(F("Extracting Coordinates"));
    lat[0]=lng[0]=0;
    do{
      while (GSM.available() > 0) //While there is stuff in the buffer
        if (GPS.encode(GSM.read())) // If it can successfully decode it, do it. Else try again when more charachters are in the buffer
          DisplayInfo(); // Our Function to get coordinates
    }while (lat[0] == 0 || lng[0] == 0 || day == 0 || month == 0 || year == 0 || hour == 0 || min == 0 || sec == 0); // Stop the loop only when all variables are valid
    GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow to send SMS
    delay(1000);
    SendMsg(0, "+306943444650"); // Our Function to sent SMS
  }*/
}
//*************************************************************
void SendMsg(int x, char number[14]) // Function to send SMS
{
  GSM.print(F("AT+CMGS=")); // AT command used for sending SMS
  GSM.println(number); // Mobile number you want to text to
  delay(100);
  GSM.print(F("Date ")); // This is the whole syntax of the message we want to send (using variables stored from DisplayInfo function)
  if (day < 10) GSM.print(F("0"));
  GSM.print(day);
  GSM.print(F("/"));
  if (month < 10) GSM.print(F("0"));
  GSM.print(month);
  GSM.print(F("/"));
  GSM.println(year);
  GSM.print(F("Time "));
  if (hour < 10) GSM.print(F("0"));
  GSM.print(hour);
  GSM.print(F(":"));
  if (min < 10) GSM.print(F("0"));
  GSM.print(min);
  GSM.print(F(":"));
  if (sec < 10) GSM.print(F("0"));
  GSM.println(sec);
  GSM.println(F("Location"));
  if(multiplesignals == false) // Send single coordinates
  {
    GSM.print(F("https://www.google.com/maps/place/"));
    GSM.print(lat[1],6);
    GSM.print(F(","));
    GSM.print(lng[1],6);
  }
  else // Send multiple coordinates
  {
    GSM.print(F("https://www.google.com/maps/dir/"));
    if(x==2)
    {
        for(j=x-1;j<=x;j++)
      {
        GSM.print(lat[j],6);
        GSM.print(F(","));
        GSM.print(lng[j],6);
        GSM.print(F("/"));
      }
    }
    else
    {
      for(j=x-2;j<=x;j++)
      {
        GSM.print(lat[j],6);
        GSM.print(F(","));
        GSM.print(lng[j],6);
        GSM.print(F("/"));
      }
    }
  }
  GSM.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to the module
}
//*************************************************************
void SendEmail(int x)
{
  Serial.println(F("Sending email"));
  GSM.println(F("AT+CGATT=1"));
  delay(5000);
  GSM.println(F("AT+CGDCONT=1,\"IP\",\"internet.vodafone.gr\""));
  delay(1000);
  GSM.println(F("AT+CGACT=1,1"));
  delay(5000);
  while (GSM.available())  // forward data from GSM to monitor
    Serial.write(GSM.read());
  GSM.println(F("AT+CIFSR"));
  delay(1000);
  GSM.println(F("AT+CIPSTART=\"TCP\",\"smtp-relay.sendinblue.com\",587"));
  delay(4000);
  CIPSEND(30,"EHLO smtp-relay.sendinblue.com");
  CIPSEND(10,"AUTH LOGIN");
  CIPSEND(24,"dGhlbXRzaWxAZ21haWwuY29t");
  CIPSEND(24,"MkVrR0R5S1A2TTl6UUpMYQ==");
  CIPSEND(31,"MAIL FROM: <themtsil@gmail.com>");
  CIPSEND(33,"RCPT To: <maxfighter21@gmail.com>");
  CIPSEND(4,"DATA");
  CIPSEND(26,"To: maxfighter21@gmail.com");
  CIPSEND(24,"From: themtsil@gmail.com");
  CIPSEND(25,"Subject: Your Arduino\r\n");
  GSM.println(F("AT+CIPSEND=15"));
  delay(200);
  GSM.print(F("Date "));
  if (day < 10) GSM.print(F("0"));
  GSM.print(day);
  GSM.print(F("/"));
  if (month < 10) GSM.print(F("0"));
  GSM.print(month);
  GSM.print(F("/"));
  GSM.println(year);
  delay(1500);
  GSM.println(F("AT+CIPSEND=13"));
  delay(200);
  GSM.print(F("Time "));
  if (hour < 10) GSM.print(F("0"));
  GSM.print(hour);
  GSM.print(F(":"));
  if (min < 10) GSM.print(F("0"));
  GSM.print(min);
  GSM.print(F(":"));
  if (sec < 10) GSM.print(F("0"));
  GSM.println(sec);
  delay(1500);
  while (GSM.available())  // forward data from GSM to monitor
    Serial.write(GSM.read());
 if(multiplesignals == false) // Send single coordinates
  {
    GSM.println(F("AT+CIPSEND=52"));
    delay(200);
    GSM.print(F("https://www.google.com/maps/place/"));
    GSM.print(lat[0],6);
    GSM.print(F(","));
    GSM.println(lng[0],6);
  }
  else // Send multiple coordinates
  {
    GSM.println(F("AT+CIPSEND=87"));
    delay(200);
    GSM.print(F("www.google.com/maps/dir/"));
    if(x==1)
    {
        for(j=x-1;j<=x;j++)
      {
        GSM.print(lat[j],6);
        GSM.print(F(","));
        GSM.print(lng[j],6);
        GSM.print(F("/"));
      }
    }
    {
      for(j=x-2;j<=x;j++)
      {
        GSM.print(lat[j],6);
        GSM.print(F(","));
        GSM.print(lng[j],6);
        GSM.print(F("/"));
      }
    }
  }
  GSM.println();
  delay(1500);
  CIPSEND(1,".");
  CIPSEND(4,"QUIT");
  Serial.println("Done");
}
//*************************************************************
void CIPSEND(int x,String Command)
{
  GSM.print(F("AT+CIPSEND="));
  GSM.println(x);
  delay(200);
  GSM.println(Command);
  delay(1500);
  while (GSM.available())  // forward data from GSM to monitor
    Serial.write(GSM.read());
}
//*************************************************************
void DisplayInfo() // Function to display and store our coordinates
{
  Serial.print(F("Location: ")); // This is the whole syntax of the message we see in Serial monitor
  if (GPS.location.isValid()) 
  {
    Serial.print(GPS.location.lat(), 6); // Function inside the tinygps++ library to get the latitude
    lat[0] = (GPS.location.lat()); // Store latitude to the lat variable
    Serial.print(F(","));
    Serial.print(GPS.location.lng(), 6); // Function inside the tinygps++ library to get the longtitude
    lng[0] = (GPS.location.lng()); // Store longtitude to the lng variable
  }
  else
  {
    Serial.print(F("INVALID"));
    lat[0] = lng[0] = 0;
  }
  Serial.print(F("  Date/Time: "));
  if (GPS.date.isValid())
  {
    Serial.print(GPS.date.day()); // Function inside the tinygps++ library to get the day
    day = (GPS.date.day()); // Store day to the day variable
    Serial.print(F("/"));
    Serial.print(GPS.date.month()); // Function inside the tinygps++ library to get the month
    month = (GPS.date.month()); // Store month to the month variable
    Serial.print(F("/"));
    Serial.print(GPS.date.year()); // Function inside the tinygps++ library to get the year
    year = (GPS.date.year()); // Store year to the year variable
  }
  else
  {
    Serial.print(F("INVALID"));
    day = month = year = 0;
  }
  Serial.print(F(" "));
  if (GPS.time.isValid())
  {
    if (GPS.time.hour() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.hour()+3); // Function inside the tinygps++ library to get the hour
    hour = (GPS.time.hour()+3); // Store hour to the hour variable
    Serial.print(F(":"));
    if (GPS.time.minute() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.minute()); // Function inside the tinygps++ library to get the minute
    min = (GPS.time.minute()); // Store minute to the min variable
    Serial.print(F(":"));
    if (GPS.time.second() < 10) Serial.print(F("0"));
    Serial.print(GPS.time.second()); // Function inside the tinygps++ library to get the second
    sec = (GPS.time.second()); // Store second to the sec variable
  }
  else
  {
    Serial.print(F("INVALID"));
    hour = min = sec = 0;
  }
  Serial.println();
}
//*************************************************************
void Sensor()
{
  byte sensorRead = 0;
  sensorRead = digitalRead(5);
  if(sensorRead == 1)
  {
    Serial.println(F("Sensed Movement"));
    if (powersave == true)
    {
      Initialise();
      powersave = false;
    }
    /*GSM.println("ATD6943444650;");
    delay(3000);
    while (GSM.available())  // forward data from GSM to monitor
      Serial.write(GSM.read());
    delay(7000);
    GSM.println(F("ATH"));
    GSM.println(F("AT+GPSRD=1")); // Enable the GPS Raw Data flow to get coordinates
    delay(1000);
    Serial.println(F("Extracting Coordinates"));
    lat=lng=0;
    do{
      while (GSM.available() > 0) //While there is stuff in the buffer
        if (GPS.encode(GSM.read())) // If it can successfully decode it, do it. Else try again when more charachters are in the buffer
          DisplayInfo(); // Our Function to get coordinates
    }while (lat == 0 || lng == 0 || day == 0 || month == 0 || year == 0 || hour == 0 || min == 0 || sec == 0); // Stop the loop only when all variables are valid
    GSM.println(F("AT+GPSRD=0")); // Disable the GPS Raw Data flow to send SMS
    delay(1000);
    while (GSM.available())  // forward data from GSM to monitor
    Serial.write(GSM.read());
    multiplesignals = false;
    SendMsg(0, "+306943444650"); // Our Function to sent SMS
    delay(2000);
    while (GSM.available())  // forward data from GSM to monitor
    Serial.write(GSM.read());
    //SendEmail(0);*/
    sensorRead = 0;
    delay(2000);
  }
}
//*************************************************************
void Going_To_Sleep()
{
  sleep_enable();//Enabling sleep mode
  delay(2000);
  Serial.println("going to sleep");//Print message to serial monitor
  wakeUp = false;
  attachPinChangeInterrupt(digitalPinToInterrupt(5), wake_Up, CHANGE);//attaching a interrupt to pin d5
  set_sleep_mode(SLEEP_MODE_EXT_STANDBY);//Setting the sleep mode, in our case full sleep
  digitalWrite(LED_BUILTIN,LOW);//turning LED off
  delay(1000); //wait a second to allow the led to be turned off before going to sleep
  sleep_cpu();//activating sleep mode
  Serial.println("just woke up!");//next line of code executed after the interrupt 
  digitalWrite(LED_BUILTIN,HIGH);//turning LED on
}
//*************************************************************
void wake_Up()
{
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  lastMotion = millis();
  wakeUp = true;
  detachInterrupt(digitalPinToInterrupt(5)); //Removes the interrupt from pin 5 (IF you want to start counting after the 1st interupt)
}
//*************************************************************
