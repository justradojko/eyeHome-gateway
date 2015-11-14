#include <stdint.h>
#include <Event.h>
#include <WString.h>
#include <Arduino.h>
#include <Ethernet.h>

#include "messageACK.h"
#include "connectionFunctions.h"
#include "communicationFunctions.h"
#include "encryption.h"


extern EthernetClient client;
extern String encStringFromJennicForServer; // encrypted String from Jennic
extern String decStringFromServer; // encrypted String from Server
extern String stringFromJennicForServer;

//arrays for message ACK in server-arduino communication
uint8_t sizeOfArrayForMessageACK = 50;
int messageIDForACK[50]; //Array for messageIDs that are sent to Server;
String messageTypeForACK[50]; //Array for messages that are sent to Server;
unsigned long timeOfMessage[50]; // time in millis when the message was sent to server
uint8_t indexForMessageACK; 



boolean checkIfMessageIDIsAlreadyInUse(uint8_t ID){
  boolean inUse = 0;
  int i;
  for (i = 0; i < sizeOfArrayForMessageACK; i++){
    if (ID == messageIDForACK[i]){
      inUse = 1;
      break;
    }
  }
  return inUse;
}

int randomNumberGenerator() {
  int randomNumber = random(1,10000);
  while (checkIfMessageIDIsAlreadyInUse(randomNumber)){
    randomNumber = random(1,10000);
  }
  return randomNumber;
}

void insertMessageIntoMessageACKArray(String message, int messageID){
  messageTypeForACK[indexForMessageACK] = message;
  messageIDForACK[indexForMessageACK] = messageID;
  timeOfMessage[indexForMessageACK] = millis();
//  Serial.print("Message inserted into array with ID: ");
//  Serial.print(messageID);
//  Serial.print(" on index: ");
//  Serial.println(indexForMessageACK);  
  if (indexForMessageACK == sizeOfArrayForMessageACK - 1){
    indexForMessageACK = 0;
  } else {
    indexForMessageACK++;
  } 
}

void removeMessageFromMessageACKarray(int messageID){
  uint8_t i;
  for (i = 0; i < sizeOfArrayForMessageACK; i++){
    if (messageID == messageIDForACK[i]){
      messageIDForACK[i] = 0;
      messageTypeForACK[i] = "";      
//      Serial.print("Message with ID: ");
//      Serial.print(messageID);  
//      Serial.println(" removed from array");        
    }
  }
}

void resendPendingMessages(){
  uint8_t i;
  
  Serial.println("Resending pending messages");
  for (i = 0; i < sizeOfArrayForMessageACK; i++){
    if ((timeOfMessage[i] != 0) && (timeOfMessage[i]  - millis() > 20000) && (messageIDForACK[i] != 0)){ //if messageACK is not received within 20secs
      stringFromJennicForServer = messageTypeForACK[i];
      encryptString();
      client.print(encStringFromJennicForServer);
      delay(700);
//      Serial.print("Message: ");
//      Serial.print(messageTypeForACK[i]);
//      Serial.println(" is resent");
    }
  }  
  
}
