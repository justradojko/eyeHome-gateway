#include <SPI.h>
#include <Ethernet.h>
#include <Timer.h>

Timer t;

#include "connectionFunctions.h"
#include "communicationFunctions.h"
#include "messageACK.h"
#include "encryption.h"

extern uint8_t CSforJennic;
extern uint8_t bufferCurrentPos;
extern uint8_t bufferDataForJennic[100];
extern String stringFromJennicForServer;
extern String encStringFromJennicForServer;
extern String decStringFromServer;
extern String stringFromServer;
extern String userID;
extern EthernetClient client;
extern boolean connectionToServer = 0;

int periodForPollingServer = 30000; // 30secs   vreme na koliko ce arduino da polluje server za nova stanja.
                                   // radi odrzavanja veze sa serverom i proveravanja dal ona postoji
int timeSinceLastMessageFromServer = 0; //
unsigned long timeOfLastMessageFromServer = 0;
int timeToNewAttemptToRecconect = 0;

void setup(){ 
  // for printing debug messages to serial window
  Serial.begin(9600);
  while (!Serial){
    Serial.println("Opening port..");
  }
  Serial.println("Arduino software version 1.3");
  Serial.print("Program starting for userID: ");
  Serial.println(userID);

  
  // setup for SPI communicating with Jennic
  digitalWrite(CSforJennic,HIGH);   
  pinMode(CSforJennic,OUTPUT); 
  SPI.begin(); //start of SPI bus
  SPI.setBitOrder(MSBFIRST); //send MSB first
  SPI.setClockDivider(SPI_CLOCK_DIV128); 
  

  initEth();  //initialization of ethernet  
  Serial.println();  
  connectToServer();
  
  connectionToServer = 1;
  
//  while (bufferCurrentPos > 0){ 
//    bufferDataForJennic[bufferCurrentPos] = 0;
//    bufferCurrentPos--;
//  }
  bufferCurrentPos = 0;

  stringFromJennicForServer.reserve(150);
  encStringFromJennicForServer.reserve(1600);
  decStringFromServer.reserve(120); 
  
  t.every(periodForPollingServer, pollServerForNewValues);  
  t.every(40000, resendPendingMessages);    
  t.every(2000, pollJennicSPI); 
  timeOfLastMessageFromServer = millis();
  Serial.println("End of program setup. Looping is starting");
}


void loop(){
//  if (connectionToServer != 0) {
    t.update();
//  }
  if (client.available()){  //proverava dal postoji neka poruka u bufferu Eth.contr.
     readFromServer();  // u stringFromServer[200] se smešta enkriptovana poruka sa Servera
     decryptString();  // Decryptcs message from Server and stores it in String decStringFromServer     
     Serial.print("SERVER -> ARDUINO: ");
     Serial.println(decStringFromServer);
     if (decStringFromServer.substring(0,1) == "00"){  //message ACK/NACK from server
        ACKReceivedFromServer();   
     }
          
     if (decStringFromServer.substring(0,1) == "01"){  //pending sensor state from Server
       if (decStringFromServer.substring(2,5) == userID.substring(0,3) {  // discards sve poruke koje nemaju odogovarajuci userID
             newSensorStateReceivedFromServer(); //handling of new received data
             timeOfLastMessageFromServer = millis ();
       }      
     }
     
     if (decStringFromServer.substring(0,1) == "02"){  //new allowJoining state
       if (decStringFromServer.substring(2,5) == userID.substring(0,3) {
            newAllowJoiningStateFromServer();  
            timeOfLastMessageFromServer = millis ();            
       }       
     }
     
     if (decStringFromServer.substring(0,1) == "03"){  //time update from Server
       timeReceivedFromServer();
       timeOfLastMessageFromServer = millis ();
     }   
     
     if (decStringFromServer.substring(0,1) == "04"){  //scheduling update
       if (decStringFromServer.substring(2,5) == userID.substring(0,3) {
            newSchedulingFromServer();  
            timeOfLastMessageFromServer = millis ();            
       } 
     }     
     
     if (decStringFromServer.substring(0,1) == "05"){  //bind update from Server
       Serial.print("BIND FROM SERVER: ");
       Serial.println(decStringFromServer);
       hanldeIncomingBind();
       timeOfLastMessageFromServer = millis ();
     } 
     
     if (decStringFromServer.substring(0,1) == "06"){  //handle cycleTime for curtain
       if (decStringFromServer.substring(2,5) == userID.substring(0,3) {
            handleCurtainCycleTimeUpdate();  
            timeOfLastMessageFromServer = millis ();            
       }       
     }     
     
     if (decStringFromServer == "NONE"){   //nema promena stanja na serveru
       timeOfLastMessageFromServer = millis ();       
     }
  }
 if (millis() - timeOfLastMessageFromServer > 5 * periodForPollingServer){
   Serial.println("NO CONNECTION TO THE SERVER!!!");
   connectionToServer = 0;   
   restartSketch();
   timeOfLastMessageFromServer = millis();
 }
}


void restartSketch(){
  initEth();
  Serial.println("Ethernet initializes after connection lost");
  delay(1000);  
  connectToServer();
  Serial.println("Connection established again after connection lost");  
  connectionToServer = 1;
}

