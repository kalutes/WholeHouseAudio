#include <Ethernet.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

//THINGS YOU NEED TO CHANGE
byte ip[] = { 192, 168, 1, 199 }; //THE IP ADDRESS OF YOUR ARDUINO: CHANGE THIS BASED ON WHAT IP RANGE YOUR ROUTER USES (MOST PEOPLE WILL BE FINE LEAVING IT AS THIS)
byte gateway[] = { 192, 168, 1, 1 }; //THE IP ADDRESS OF YOUR ROUTER: TO FIND THIS USING A WINDOWS COMPUTER OPEN A COMMAND PROMPT AND TYPE "ipconfig" PRESS ENTER AND IT WILL BE LISTED UNDER DEFAULT GATEWAY
byte subnet[] = { 255, 255, 255, 0 };//THIS CAN ALSO BE FOUND BY USING THE METHOD ABOVE, IT WILL BE LISTED UNDER SUBNET MASK
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //THIS SHOULD BE ON A STICKER ON THE BOTTOM OF YOUR ETHERNET SHIELD 
#define address 0x44 //THE ADDRESS OF THE FIRST PT2258 IT SHOULD BE THIS IF CODE2 IS PULLED TO GND
#define address2 0x46 //THE ADDRESS OF THE SECOND PT2258 IT SHOULD BE THIS IF CODE2 IS PULLED TO +5V
int relayPin = 8; //THE ARDUINO PIN TO WHICH THE RELAY IS CONNECTED
int relayPin2 = 7;
//END THINGS YOU NEED TO CHANGE


Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

#define error(s) error_P(PSTR(s))
#define BUFSIZ 100

boolean reading = false;
boolean SystemOn = false;

boolean muted = false;
boolean muted1 = false;
boolean muted2 = false;
boolean muted3 = false;
boolean muted4 = false;
boolean muted5 = false;
boolean muted6 = false;



EthernetServer server = EthernetServer(80);
EthernetClient client;

int volume1 = 78;
int volume2 = 78;
int volume3 = 78;
int volume4 = 78;
int volume5 = 78;
int volume6 = 78;

void setup()
{
  pinMode(relayPin, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  card.init(SPI_FULL_SPEED, 4);
  
  volume.init(&card);
  
  root.openRoot(&volume);
  
  Wire.begin();
  delay(500);
  
  Wire.beginTransmission(address);
  Wire.write(0b11000000);
  Wire.endTransmission();
  Wire.beginTransmission(address2);
  Wire.write(0b11000000);
  Wire.endTransmission();
  setVolume1();
  setVolume2();
  setVolume3();
  setVolume4();
  setVolume5();
  setVolume6();

  Ethernet.begin(mac, ip, gateway, subnet);

  server.begin();
}

void loop()
{
  checkForClient();
}

void checkForClient()
{
  client = server.available();

  if (client) 
  {
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    String message;

    while (client.connected()) 
    {
      if (client.available()) 
      {
        if(!sentHeader)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        }

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true;

        if(reading)
        {
          message = message + c;
        }
        
        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') 
        {
          currentLineIsBlank = true;
          break;
        }
        else if (c != '\r') 
        {
          currentLineIsBlank = false;
        }
        
      }
    }
    delay(1);
    interpretMessage(message);
    client.stop();
  } 
}

void interpretMessage(String s)
{
  if(s.equalsIgnoreCase("?app"))
  {
          file.open(&root, "KAMDORA.htm", O_READ);          
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          
          int16_t c;
          while ((c = file.read()) > 0) {
              client.print((char)c);
          }
          file.close();
  }
  if(s.substring(1,2).equals("m"))
  {
    String c = s.substring(2,3);
    mute(c);
  }
  if(s.substring(1,2).equals("v"))
  {
    String channel = s.substring(2,3);
    String volume = s.substring(3,5);
    int c = channel.toInt();
    int v = volume.toInt();
    switch(c)
    {
      case 0:
        volume1 = v;
        volume2 = v;
        volume3 = v;
        volume4 = v;
        volume5 = v;
        volume6 = v;
        setVolume1();
        setVolume2();
        setVolume3();
        setVolume4();
        setVolume5();
        setVolume6();
        break;
      case 1:
        volume1 = v;
        setVolume1();
        break;
      case 2:
         volume2 = v;
         setVolume2();
         break;
      case 3:
         volume3 = v;
         setVolume3();
         break;
      case 4:
         volume4 = v;
         setVolume4();
         break;
      case 5:
         volume5 = v;
         setVolume5();
         break;
      case 6:
         volume6 = v;
         setVolume6();
         break;
    }
  }
  if(s.equals("?on"))
  {
    digitalWrite(relayPin, HIGH);
    digitalWrite(relayPin2, HIGH);
    SystemOn = true;
  }
  if(s.equals("?off"))
  {
    digitalWrite(relayPin, LOW);
    digitalWrite(relayPin2, LOW);
    SystemOn = false;
  }
  
  if(s.equals("?getVars"))
  {
    client.print("Ok*");
    client.print(muted);
    client.print("*");
    client.print(muted1);
    client.print("*");
    client.print(volume1);
    client.print("*");
    client.print(muted2);
    client.print("*");
    client.print(volume2);
    client.print("*");
    client.print(muted3);
    client.print("*");
    client.print(volume3);
    client.print("*");
    client.print(muted4);
    client.print("*");
    client.print(volume4);
    client.print("*");
    client.print(muted5);
    client.print("*");
    client.print(volume5);
    client.print("*");
    client.print(muted6);
    client.print("*");
    client.print(volume6);
    client.print("*");
    client.println(SystemOn);
  }
  
}

void setVolume1() 
{
  uint8_t tens = (uint8_t)volume1 / 10;
  uint8_t ones = (uint8_t)volume1 % 10;
  Wire.beginTransmission(address);
  Wire.write(0b10000000 | tens);
  Wire.write(0b10010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b00100000 | tens);
  Wire.write(0b00110000 | ones);
  Wire.endTransmission();
  muted1 = false;
}

void setVolume2() 
{
  uint8_t tens = (uint8_t)volume2 / 10;
  uint8_t ones = (uint8_t)volume2 % 10;
  Wire.beginTransmission(address);
  Wire.write(0b01000000 | tens);
  Wire.write(0b01010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b01100000 | tens);
  Wire.write(0b01110000 | ones);
  Wire.endTransmission();
  muted2 = false;
}

void setVolume3() 
{
  uint8_t tens = (uint8_t)volume3 / 10;
  uint8_t ones = (uint8_t)volume3 % 10;
  Wire.beginTransmission(address);
  Wire.write(0b00000000 | tens);
  Wire.write(0b00010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b10100000 | tens);
  Wire.write(0b10110000 | ones);
  Wire.endTransmission();
  muted3 = false;
}

void setVolume4() 
{
  uint8_t tens = (uint8_t)volume4 / 10;
  uint8_t ones = (uint8_t)volume4 % 10;
  Wire.beginTransmission(address2);
  Wire.write(0b10000000 | tens);
  Wire.write(0b10010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address2);
  Wire.write(0b00100000 | tens);
  Wire.write(0b00110000 | ones);
  Wire.endTransmission();
  muted4 = false;
}

void setVolume5() 
{
  uint8_t tens = (uint8_t)volume5 / 10;
  uint8_t ones = (uint8_t)volume5 % 10;
  Wire.beginTransmission(address2);
  Wire.write(0b01000000 | tens);
  Wire.write(0b01010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address2);
  Wire.write(0b01100000 | tens);
  Wire.write(0b01110000 | ones);
  Wire.endTransmission();
  muted5 = false;
}

void setVolume6() 
{
  uint8_t tens = (uint8_t)volume6 / 10;
  uint8_t ones = (uint8_t)volume6 % 10;
  Wire.beginTransmission(address2);
  Wire.write(0b00000000 | tens);
  Wire.write(0b00010000 | ones);
  Wire.endTransmission();
  Wire.beginTransmission(address2);
  Wire.write(0b10100000 | tens);
  Wire.write(0b10110000 | ones);
  Wire.endTransmission();
  muted6 = false;
}
  
void mute(String channel)
{
  if(channel.equals("0"))
  {
    if(muted == false)
    {
      Wire.beginTransmission(address);
      Wire.write(0b11010111);
      Wire.write(0b11101000);
      Wire.endTransmission();
      Wire.beginTransmission(address2);
      Wire.write(0b11010111);
      Wire.write(0b11101000);
      Wire.endTransmission();
      muted = true;
      muted1 = true;
      muted2 = true;
      muted3 = true;
      muted4 = true;
      muted5 = true;
      muted6 = true;
    }
    else
    {
      setVolume1();
      setVolume2();
      setVolume3();
      setVolume4();
      setVolume5();
      setVolume6();
      muted = false;
      muted1 = false;
      muted2 = false;
      muted3 = false;
      muted4 = false;
      muted5 = false;
      muted6 = false;
    }
  }
  
  if(channel.equals("1"))
  {
    if(muted1 == false)
    {
      int temp = volume1;
      volume1 = 78;
      setVolume1();
      volume1 = temp;
      muted1 = true;
    }
    else
    {
      setVolume1();
      muted1 = false;
    }
  }
  
  if(channel.equals("2"))
  {
    if(muted2 == false)
    {
      int temp = volume2;
      volume2 = 78;
      setVolume2();
      volume2 = temp;
      muted2 = true;
    }
    else
    {
      setVolume2();
      muted2 = false;
    }
  }
  
  if(channel.equals("3"))
  {
    if(muted3 == false)
    {
      int temp = volume3;
      volume3 = 78;
      setVolume3();
      volume3 = temp;
      muted3 = true;
    }
    else
    {
      setVolume3();
      muted3 = false;
    }
  }
  
  if(channel.equals("4"))
  {
    if(muted4 == false)
    {
      int temp = volume4;
      volume4 = 78;
      setVolume4();
      volume4 = temp;
      muted4 = true;
    }
    else
    {
      setVolume4();
      muted4 = false;
    }
  }
  
  if(channel.equals("5"))
  {
    if(muted5 == false)
    {
      int temp = volume5;
      volume5 = 78;
      setVolume5();
      volume5 = temp;
      muted5 = true;
    }
    else
    {
      setVolume5();
      muted5 = false;
    }
  }
  
  if(channel.equals("6"))
  {
    if(muted6 == false)
    {
      int temp = volume6;
      volume6 = 78;
      setVolume6();
      volume6 = temp;
      muted6 = true;
    }
    else
    {
      setVolume6();
      muted6 = false;
    }
  }
}
