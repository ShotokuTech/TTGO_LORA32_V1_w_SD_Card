#include "SSD1306.h"     //  OLED
#include <LoRa.h>        //  LoRa
#include <mySD.h>
//#include <SPI.h>
//#include <Wire.h> 
//#include "SSD1306Wire.h" 

File root;

//       OLED       Pin
#define  SDA          4   //  Serial Data
#define  SCL         15   //  Serial Clock
#define  oRST        16   //  Reset
#define  OLED_ADDR 0x3C   //  OLED display TWI address

SSD1306         display(OLED_ADDR, SDA, SCL);
//  OLED screen text rows:
#define  row1     0     //  y=0 is top row of size 1 text
#define  row2    10
#define  row3    20
#define  row4    30
#define  row5    40
#define  row6    50     //  row7 at 70 is too low

//  SPI port #2:  SD Card Adapter
#define  SD_CLK     17
#define  SD_MISO    13
#define  SD_MOSI    12
#define  SD_CS      23

//  SPI port:  LoRa (SX1278) 
//       LoRa      Pin
#define  LoRa_SCK    5
#define  LoRa_MISO  19
#define  LoRa_MOSI  27
#define  LoRa_CS    18
#define  LoRa_RST   14    //  LoRa_Reset
#define  DI0        26    //  LoRa_IRQ
#define  BAND    915E6    //  other freq: 433E6  // or ? LoRa f(MHz)

#define  Select    LOW   //  Low CS means that SPI device Selected
#define  DeSelect  HIGH  //  High CS means that SPI device Deselected

File     sessionFile;   //  SD card filenames are restricted to 8 characters + extension

int counter = 0;        //  count sent LoRa messages
int count = 0;
String recv = "";
String temp = "";
/***********************************************************/
void setup(){  
  // set output pins
  pinMode(oRST,OUTPUT);
  pinMode(SD_CS,OUTPUT);
  pinMode(LoRa_CS,OUTPUT);
  digitalWrite(LoRa_CS, DeSelect);

  // set GPIO16 Low then High to Reset OLED
  digitalWrite(oRST, LOW);  
  delay(50);
  digitalWrite(oRST, HIGH);

  Serial.begin(115200);
  while(!Serial);                     //  wait to connect to computer

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.display();

  Serial.print("Initializing SD card...");
  digitalWrite(SD_CS, Select);    //  SELECT (Low) SD Card SPI
/**/
  if (!SD.begin( SD_CS, SD_MOSI, SD_MISO, SD_CLK )) {
    Serial.println("initialization failed!");
    //  now what?
  } else {
    Serial.println("initialization done.");
    display.drawString( 5,row1,"SD Card OK!" );
    display.display();
    delay(1000);
  }
  /* open "test.txt" for writing */
  root = SD.open("test.txt", FILE_WRITE);
  if (root) {
    root.println("Hello world!");
    root.flush();
    root.close();
  } else {    //  file open error
    Serial.println("error opening test.txt");
  }
  display.drawString( 5,row2,"Wrote in test.txt" );
  display.display();
  delay(100);
  /* after writing, then reopen the file and read it */
  root = SD.open("test.txt");
  if (root) {    /* read from the file to the end of it */
    while (root.available()) {  // read the file
      Serial.write(root.read());
    }
    root.close();
  } else {
    Serial.println("error opening test.txt");
  }
  display.drawString( 5,row3,"Read from test.txt" );
  display.display();
  delay(100);
  //  done testing the SD Card
  digitalWrite(SD_CS, DeSelect); 
//  SD.end();
  delay( 100 ); 

  //  now test the LoRa

  SPI.begin( LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_CS );
  LoRa.setPins( LoRa_CS, LoRa_RST, DI0 );
  digitalWrite(LoRa_CS, Select);   //  SELECT (low) LoRa SPI 
  Serial.println("LoRa Sender");
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    display.drawString( 5, row4, "LoRa Init Failed!");
    display.display();
    // now what?
  } else {
    Serial.println("LoRa Initial OK!");
    display.drawString( 5, row4, "LoRa Initialized OK!");
    display.display();
    delay(1000);
  }
  digitalWrite(LoRa_CS, DeSelect);  
  Serial.println("Setup done!");
}
       
void loop() {
    digitalWrite(LoRa_CS, Select);
    if (LoRa.parsePacket()) {
        recv = "";
        while (LoRa.available()) {
            recv += (char)LoRa.read();
        }
        count++;
        display.clear();
        display.drawString(5, row4, recv);
        String info = "[" + String(count) + "]" + "RSSI " + String(LoRa.packetRssi());
        display.drawString(5, row2, info);
        display.display();
    }
    if(temp != recv){
    temp = recv;  
    digitalWrite(LoRa_CS, DeSelect);
    delay(50);         
    digitalWrite(SD_CS, Select);    
    root = SD.open("test.txt", FILE_WRITE);
    Serial.println(root);
    if (root) {
      root.println(temp);
      root.flush();
      root.close();   
    }
  Serial.println(temp); 
  digitalWrite(SD_CS, DeSelect);  
  delay(50);
  }  
}
