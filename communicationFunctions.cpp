#include <stdint.h>
#include <Event.h>
#include <Arduino.h> 
#include <SPI.h>
#include <Ethernet.h>
#include <Timer.h>

#include "messages.h"
#include "connectionFunctions.h"
#include "messageACK.h"
#include "encryption.h"

// ************************** FUNCTION *****************************************
void addDataToBufferForJennic(uint8_t Data);
uint8_t checkIfSensorValueExistsInBuffer(uint8_t deviceIDhigh, uint8_t deviceIDlow, uint8_t nodID);
uint8_t checkIfTimeValueExistsInBuffer();
void pollJennicSPI();
void pollServerForTimeUpdate();
void pollServerForNewValues();
String readFromServer();
void newSensorStateReceivedFromServer();
void newSchedulingFromServer();
void timeReceivedFromServer();
uint8_t  sendDataToJennic(uint8_t data);
void sendDataToServer();


// ************************* VARIABLES *****************************************
extern Timer t;
extern EthernetClient client;
extern String encStringFromJennicForServer; // encrypted String from Jennic
extern String decStringFromServer; // encrypted String from Server

String userID = "1888";

int delayAfterSendingMessage = 400;

uint8_t CSforJennic = 6;

char stringFromServer[150]; // string for incoming serial data
String stringFromJennicForServer;
uint16_t stringPos = 0; // string index counter

//buffer for messages
uint8_t bufferDataForJennic[100] ;
uint8_t bufferCurrentPos = 0;


//****************** FUNCTIONS FOR HANDLING BUFFER FOR JENNIC *******************************
uint8_t  sendDataToJennic(uint8_t data){ 
  uint8_t receivedData;
  digitalWrite(CSforJennic, LOW);
  receivedData = SPI.transfer(data);
  
  receivedData = receivedData ^ 255;
  
  digitalWrite(CSforJennic,HIGH); 
  return receivedData;
}

void sendDataToServer(int messageID){
  encryptString();
  client.print(encStringFromJennicForServer);
  insertMessageIntoMessageACKArray(stringFromJennicForServer, messageID);  
  delay(delayAfterSendingMessage);
}

void addDataToBufferForJennic(uint8_t Data){
  bufferDataForJennic[bufferCurrentPos] = Data;
  if (bufferCurrentPos>=50){
  } else {
    bufferCurrentPos++;
  }  
}

void hanldeIncomingBind(){
  uint8_t pos1, pos2, pos3, pos4, pos5, pos6, pos7;
  uint8_t nwkAddrhigh, nwkAddrlow;
  pos1 = decStringFromServer.indexOf('_',0); //pozicija drugog _
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija treceg _
  pos3 = decStringFromServer.indexOf('_',pos2+1); //pozicija cetvrtog _  
  pos4 = decStringFromServer.indexOf('_',pos3+1); //pozicija petog _  
  pos5 = decStringFromServer.indexOf('_',pos4+1); //pozicija sestog _
  pos6 = decStringFromServer.indexOf('_',pos5+1); //pozicija sedmog _
  pos7 = decStringFromServer.indexOf('_',pos6+1); //pozicija osmog _  


  addDataToBufferForJennic(decStringFromServer.substring(pos6+1,pos7).toInt()); //bind
  
  nwkAddrlow = (decStringFromServer.substring(pos5+1,pos6).toInt() << 8) >> 8;
  nwkAddrhigh = decStringFromServer.substring(pos5+1,pos6).toInt() >> 8;    
  
  addDataToBufferForJennic(nwkAddrlow); //clusterID  low
  addDataToBufferForJennic(nwkAddrhigh); //clusterID  high
  
  addDataToBufferForJennic(decStringFromServer.substring(pos4+1,pos5).toInt()); //destEndPoint  

  nwkAddrlow = (decStringFromServer.substring(pos3+1,pos4).toInt() << 8) >> 8;
  nwkAddrhigh = decStringFromServer.substring(pos3+1,pos4).toInt() >> 8;  
      
  addDataToBufferForJennic(nwkAddrlow); //destNwkAddr low    
  addDataToBufferForJennic(nwkAddrhigh); //destNwkAddr high    
  
  addDataToBufferForJennic(decStringFromServer.substring(pos2+1,pos3).toInt()); //srcEndPoint
  
  nwkAddrlow = (decStringFromServer.substring(pos1+1,pos2).toInt() << 8) >> 8;
  nwkAddrhigh = decStringFromServer.substring(pos1+1,pos2).toInt() >> 8;          
  
  addDataToBufferForJennic(nwkAddrlow); //srcNwkAddr low
  addDataToBufferForJennic(nwkAddrhigh); //srcNwkAddr hign  
  
  addDataToBufferForJennic(BIND_UNBIND); //message type
  addDataToBufferForJennic(BIND_UNBIND); //message type  
  pollJennicSPI();  
}


//handling scheduling from server
void newSchedulingFromServer(){
  uint8_t pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8;
  uint8_t nwkAddrhigh, nwkAddrlow;
  String startTime, endTime;
  
  pos1 = decStringFromServer.indexOf('_',0); //pozicija drugog _
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija treceg _
  pos3 = decStringFromServer.indexOf('_',pos2+1); //pozicija cetvrtog _  
  pos4 = decStringFromServer.indexOf('_',pos3+1); //pozicija petog _  
  pos5 = decStringFromServer.indexOf('_',pos4+1); //pozicija sestog _
  pos6 = decStringFromServer.indexOf('_',pos5+1); //pozicija sedmog _   
  pos7 = decStringFromServer.indexOf('_',pos6+1); //pozicija sedmog _     
  pos8 = decStringFromServer.indexOf('_',pos7+1); //pozicija osmog _     

  addDataToBufferForJennic(decStringFromServer.substring(pos7+1,pos8).toInt()); //addRemove
  
  addDataToBufferForJennic(decStringFromServer.substring(pos6+1,pos7).toInt()); //activateDeactivate
  
  startTime = decStringFromServer.substring(pos4+1, pos5); 
  endTime = decStringFromServer.substring(pos5+1, pos6);
  
  addDataToBufferForJennic(endTime.substring(3, 5).toInt()); //end time min    
  addDataToBufferForJennic(endTime.substring(0, 2).toInt()); //end time hour  

  addDataToBufferForJennic(startTime.substring(3, 5).toInt()); //start time min
  addDataToBufferForJennic(startTime.substring(0, 2).toInt()); //start time hour
  
  addDataToBufferForJennic(decStringFromServer.substring(pos3+1,pos4).toInt()); //dayPattern 
  addDataToBufferForJennic(decStringFromServer.substring(pos2+1,pos3).toInt()); //endPoint
  
  nwkAddrlow = (decStringFromServer.substring(pos1+1,pos2).toInt() << 8) >> 8;
  nwkAddrhigh = decStringFromServer.substring(pos1+1,pos2).toInt() >> 8;    
  addDataToBufferForJennic(nwkAddrlow); //destNwkAddr low    
  addDataToBufferForJennic(nwkAddrhigh); //destNwkAddr high    

  addDataToBufferForJennic(SCHEDULING); //message type
  addDataToBufferForJennic(SCHEDULING); //message type  
  pollJennicSPI();
}

//************************** /TEMP *********************************
uint8_t checkIfTimeValueExistsInBuffer(){ //return position of exsiting timeMinute   ---------------
  uint8_t i;   //brojac koji ide od kraja buffera ka pocetku (pos 0)

  if (bufferCurrentPos > 2){  //2 za svaki slucaj, trebalo bi da stoji 0
    i = bufferCurrentPos - 1;
  } else {
    return 111;
  }
  while ( (i<50) && (i>=0) ){  
      if ( (bufferDataForJennic[i]) == DEVICE_UPDATE_VALUE1) { 
        i = i - 6;
      } 
      if ( (bufferDataForJennic[i]) == CLOCK_UPDATE) {
//        Serial.print("Vec postoji vreme u bufferu na poziciji: ");
//        Serial.println(i - 6);        
        return(i - 6);        
      } 
   }
   return 111;
}


// Ukoliko se proverava dal postoji stanjeSenzora u bufferu, fja vraca poziciku na kojoj treba da se upise novi pendingValueLow, pa pendingValueHigh
uint8_t checkIfSensorValueExistsInBuffer(uint8_t deviceIDhigh, uint8_t deviceIDlow, uint8_t nodID){ //return position of exsiting value(low) in buffer
//nodID je endPoint
  uint8_t i;   //brojac koji ide od kraja buffera ka pocetku (pos 0)
//  Serial.println("Checking if data exists in the buffer");
  if (bufferCurrentPos > 2){  //2 za svaki slucaj, trebalo bi da stoji 0
    i = bufferCurrentPos - 1;
  } else {
    return 111;
  }
  while ( (i<50) && (i>0) ){  
//      Serial.print("While loop: ");    
//      Serial.println(i);
      if ( (bufferDataForJennic[i] ) == DEVICE_UPDATE_VALUE1) { 
        if (bufferDataForJennic[i-1] == deviceIDhigh){
          if (bufferDataForJennic[i-2] == deviceIDlow){
            if (bufferDataForJennic[i-3] == nodID){
//              Serial.print("Vec postoji u bufferu endPoint: ");
//              Serial.print(nodID);
//              Serial.print("  Vracena pozicijia:  ");
//              Serial.println(i-5);
              return (i-5);   // vraca poziciju na kojoj treba da se upise novi pendingValueLow
            } else { i = i - 6; }
          } else { i = i - 6; }
        } else { i = i - 6; }
      } else if ( (bufferDataForJennic[i]) == CLOCK_UPDATE) {
//        Serial.println(F("Pronajdeno vreme u bufferu"));
        i = i - 7;        
      } 
   }
   return 111;
}

//****************** END OF FUNCTIONS FOR HANDLING BUFFER FOR JENNIC *******************************

String readFromServer(){
  if (!client.connected()){
    connectToServer();
  }
  stringPos = 0;
  memset( &stringFromServer, 0, 150 ); //clear inString memory from 0 to 150 bytes
  while(client.available()){
    if (stringPos<150){
      char c = client.read();
      stringFromServer[stringPos] = c;
      stringPos++;      
      if ( c == '\n'){
//          Serial.println("READ FUNCTION: Quit due to 0 bit char");
//           Serial.print("Message received from server ENC: ");
//           Serial.println(stringFromServer);
          return stringFromServer;          
          break;
      }
    } else {
//      Serial.println("READ FUNCTION: Quit due to 150 size");
      return stringFromServer;      
      break;
    }
  }
}


void pollServerForNewValues(){
  Serial.print("ARDUINO -> SERVER: ");
  stringFromJennicForServer = "CHECK_" + userID + "_";
  Serial.println(stringFromJennicForServer);
  encryptString();
  client.print(encStringFromJennicForServer);  
  delay(delayAfterSendingMessage);             
}

void ACKReceivedFromServer(){
  uint8_t pos1, pos2, pos3, pos4;
  int messageID;
  
//  Serial.print("SERVER -> ARDUINO: ");
//  Serial.println(decStringFromServer);

  pos1 = decStringFromServer.indexOf('_'); //pozicija prvog _  
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija drugog _
  pos3 = decStringFromServer.indexOf('_',pos2+1); //pozicija treceg _    
  
  messageID = (decStringFromServer.substring(pos2+1,pos3).toInt());
  if (messageID != 0){
    removeMessageFromMessageACKarray(messageID);
  }
}


// MEESSAGE FORM 02_1234_1_
void newAllowJoiningStateFromServer(){
  boolean newJoiningState;
  uint8_t pos1, pos2, pos3;
  
  Serial.print("SERVER -> ARDUINO: ");
  Serial.println(decStringFromServer);  
  
  pos1 = decStringFromServer.indexOf('_'); //pozicija prvog _  
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija drugog _
  
  newJoiningState = decStringFromServer.substring(pos1+1,pos2).toInt();
  
  addDataToBufferForJennic(newJoiningState); //ALLOW/DISALLOW joining state  
  addDataToBufferForJennic(ALLOW_DISALLOW_NWK_JOIN); //command
  addDataToBufferForJennic(ALLOW_DISALLOW_NWK_JOIN); //command  
  
  pollJennicSPI();  
}

void handleCurtainCycleTimeUpdate(){
  uint8_t pos1, pos2, pos3, pos4;
  
  Serial.print("SERVER -> ARDUINO: ");
  Serial.println(decStringFromServer);

  pos1 = decStringFromServer.indexOf('_'); //pozicija prvog _  
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija drugog _
  pos3 = decStringFromServer.indexOf('_',pos2+1); //pozicija treceg _
  pos4 = decStringFromServer.indexOf('_',pos3+1); //pozicija cetvrtog _  
  
  addDataToBufferForJennic((decStringFromServer.substring(pos3+1,pos4).toInt() << 8) >> 8); //cycleTime low          
  addDataToBufferForJennic(decStringFromServer.substring(pos3+1,pos4).toInt() >> 8); //cycleTime high  
  Serial.print("Value:");
  Serial.println(decStringFromServer.substring(pos3+1,pos4));  
  
  addDataToBufferForJennic(decStringFromServer.substring(pos2+1,pos3).toInt()); //endPoint value
  Serial.print("endPoint:");
  Serial.println(decStringFromServer.substring(pos2+1,pos3));    

  addDataToBufferForJennic((decStringFromServer.substring(pos1+1,pos2).toInt() << 8) >> 8); //nwkAddr low          
  addDataToBufferForJennic(decStringFromServer.substring(pos1+1,pos2).toInt() >> 8); //nwkAddr high
  Serial.print("nwkAddr:");
  Serial.println(decStringFromServer.substring(pos1+1,pos2));      

  addDataToBufferForJennic(UPDATE_FULL_MOTION_TIME); //message type
  addDataToBufferForJennic(UPDATE_FULL_MOTION_TIME); //message type. doubling command   
  
  pollJennicSPI();  
}


void newSensorStateReceivedFromServer(){
  uint8_t pos1, pos2, pos3, pos4;
  uint8_t nwkAddrlow, nwkAddrhigh, endPoint, pos;

  Serial.print("SERVER -> ARDUINO: ");
  Serial.println(decStringFromServer);


  pos4 = decStringFromServer.indexOf('_'); //pozicija prvog _  
  pos1 = decStringFromServer.indexOf('_',pos4+1); //pozicija drugog _
  pos2 = decStringFromServer.indexOf('_',pos1+1); //pozicija treceg _
  pos3 = decStringFromServer.indexOf('_',pos2+1); //pozicija cetvrtog _
  
  if ((pos1 > 0) && (pos2 > 0)){
      // zakomentirano zato sto pendingValue moze da bude 0 i da je sve ok. Nisam siguran za endPoint.
  //        if ((newValues.substring(0,pos1).toInt()!= 0 ) && (newValues.substring(pos1+1,pos2).toInt()!=0)) //pendingValue i endPoint
      nwkAddrlow = (decStringFromServer.substring(pos2+1,pos3).toInt() << 8) >> 8;
      nwkAddrhigh = decStringFromServer.substring(pos2+1,pos3).toInt() >> 8;
      endPoint = decStringFromServer.substring(pos1+1,pos2).toInt();
      pos = checkIfSensorValueExistsInBuffer(nwkAddrhigh, nwkAddrlow, endPoint);  //proveravanje dal postoji deviceID i endPoind u baferu
      if ( pos < 111 ){  //ukoliko postoji u baferu samo update sa novom vrednoscu
        bufferDataForJennic[pos] = (decStringFromServer.substring(pos4+1,pos1).toInt() << 8) >> 8; //pendingValue low                                            
        bufferDataForJennic[pos + 1] = decStringFromServer.substring(pos4+1,pos1).toInt() >> 8; //pendingValue high         
      } else {   // ukoliko ne postoji dodati sve potrebne podatke u buffer     
        addDataToBufferForJennic((decStringFromServer.substring(pos4+1,pos1).toInt() << 8) >> 8); //pendingValue low          
        addDataToBufferForJennic(decStringFromServer.substring(pos4+1,pos1).toInt() >> 8); //pendingValue high                
                  
        addDataToBufferForJennic(endPoint); //endPoint      
        addDataToBufferForJennic(nwkAddrlow); //Network Address low         
        addDataToBufferForJennic(nwkAddrhigh); //Network Address high
        
        addDataToBufferForJennic(DEVICE_UPDATE_VALUE1); //message type
        addDataToBufferForJennic(DEVICE_UPDATE_VALUE1); //message type. doubling command         
      }
  }

  pollJennicSPI();
}

void pollServerForTimeUpdate(){
//  Serial.println("Sending message for polling server for TIME update");
  
  stringFromJennicForServer = "TIME_" + userID + "_";
  Serial.print("ARDUINO -> SERVER:");
  Serial.println(stringFromJennicForServer);
  encryptString();
  client.print(encStringFromJennicForServer);
  delay(delayAfterSendingMessage);  
  
}



void timeReceivedFromServer(){
   if (decStringFromServer.substring(2,4).toInt()!= 0){ 
      uint8_t pos;
      Serial.print("Time received from server: ");
      Serial.println(decStringFromServer); 
      
//      pos = checkIfTimeValueExistsInBuffer();    ----------
//      if (pos = 111) {      
        addDataToBufferForJennic(decStringFromServer.substring(22,23).toInt()); //day in week  
        addDataToBufferForJennic(decStringFromServer.substring(16,18).toInt()); //minute
        addDataToBufferForJennic(decStringFromServer.substring(13,15).toInt()); //hour
        addDataToBufferForJennic(decStringFromServer.substring(10,12).toInt()); //day    
        addDataToBufferForJennic(decStringFromServer.substring(7,9).toInt()); //month
        addDataToBufferForJennic((decStringFromServer.substring(2,6).toInt() << 8) >> 8); // year (low)
        addDataToBufferForJennic(decStringFromServer.substring(2,6).toInt() >> 8); // year (high)
        addDataToBufferForJennic(CLOCK_UPDATE); //message type
        addDataToBufferForJennic(CLOCK_UPDATE); //message type        
//      } else {
//        bufferDataForJennic[pos] = decStringFromServer.substring(16,18).toInt(); //minute      -------------
//        bufferDataForJennic[pos + 1] = decStringFromServer.substring(13,15).toInt(); //hour
//        bufferDataForJennic[pos + 2] = decStringFromServer.substring(10,12).toInt(); //day    
//        bufferDataForJennic[pos + 3] = decStringFromServer.substring(7,9).toInt(); //month
//        bufferDataForJennic[pos + 4] = (decStringFromServer.substring(2,6).toInt() << 8) >> 8; // year (low)
//        bufferDataForJennic[pos + 5] = decStringFromServer.substring(2,6).toInt() >> 8; // year (high)
//        bufferDataForJennic[pos + 6] = CLOCK_UPDATE; //message type  // u starom kodu je bilo ...+6?? 
//      }
   }
   pollJennicSPI();
//  t.after(1800000,updateTimeFromServer);
//  t.after(43000,updateTimeFromServer);
}


void pollJennicSPI(){
//  Serial.println("ARDUINO -> JENNIC: Polling Jennic "); 
  
  uint8_t pendingDataFromArduino = 0;
  uint8_t pendingDataFromJennic = 0;
  uint8_t dataFromJennicSPI;
  uint8_t temp8, command;
  uint8_t possibleCommand = 0;
  uint16_t temp16;
  int messageID;
  boolean oneMoreLoopNeeded = 1;  // za proveru dal Jennic ima neku poruku, kad Arduino nema nista da salje
  
  boolean SPIsynced = 0; 
  
  if (bufferCurrentPos > 0){
    pendingDataFromArduino = bufferCurrentPos-1;
  }  

  while ((pendingDataFromArduino>0) || (pendingDataFromJennic > 0) || oneMoreLoopNeeded){ 

    oneMoreLoopNeeded = 0;    
    if (pendingDataFromArduino>0){
      dataFromJennicSPI = sendDataToJennic(bufferDataForJennic[pendingDataFromArduino]);
      bufferDataForJennic[pendingDataFromArduino] = 0;
      bufferCurrentPos--; 
      pendingDataFromArduino--; 
//      Serial.print("Buffer pointer: ");
//      Serial.print(bufferCurrentPos);
//      Serial.print(", pending data from Jennic: ");
//      Serial.println(pendingDataFromJennic);
    } else {
      dataFromJennicSPI = sendDataToJennic(0);
//      Serial.print("Buffer pointer: ");      
//      Serial.print(bufferCurrentPos);      
//      Serial.print(", pending data from Jennic: ");
//      Serial.println(pendingDataFromJennic);      
    }
    
    
//    Serial.print("  Data from Jennic:");
//    Serial.println(dataFromJennicSPI);

    // For detecting double command byte
    if (pendingDataFromJennic == 0){ //ocekuje se komanda, kad je komanda prihvacena, nece se ulaziti ovde
      if (dataFromJennicSPI != 0){  // odbacuju se dummy podadtci
        if (dataFromJennicSPI != possibleCommand){
          possibleCommand = dataFromJennicSPI;
          oneMoreLoopNeeded = 1;
        } else {
          command = possibleCommand;
          possibleCommand = 0;          
          SPIsynced = 1;
        }                    
      }
    }  
    
    if (SPIsynced == 1){      
      switch (command){
        //  ******** UPDATE OF NODS TEMPERATURES *******************************
        case NODE_UPDATE_TEMP:
          if (pendingDataFromJennic == 0){
              messageID = randomNumberGenerator();
              stringFromJennicForServer = "TEMP_"+userID+"_"+messageID+"_";        
              pendingDataFromJennic = 5;      
          } else if (pendingDataFromJennic == 4){  //deviceID high
              temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 3){  //deviceID low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_"; 
          } else if (pendingDataFromJennic == 2){  //value high 
              temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 1){  //value low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_"; 
              
              Serial.println();
              Serial.println("JENNIC -> ARDUINO: Nod temperature update. Forwarding to server");
              Serial.print("Message: ");
              Serial.println(stringFromJennicForServer);                                    
              
              sendDataToServer(messageID);            
              
              oneMoreLoopNeeded = 1;
              command = 0;       
      
          }
          break;
        // *********************************************************************
        
        //  ******** SINC CLOCK BETWEEN JENNIC AND SERVER **********************      
        case CLOCK_UPDATE_REQUEST:
          pollServerForTimeUpdate();
          oneMoreLoopNeeded = 1; 
          command = 0;
          
          Serial.println();
          Serial.println("JENNIC -> ARDUINO: Time update request received. Forwarding to server");
          Serial.print("Message: ");
          Serial.println(stringFromJennicForServer);                        
                
          break;
        // ********************************************************************* 
 
        case SCHEDULING:
          if (pendingDataFromJennic == 0){
              messageID = randomNumberGenerator();
              stringFromJennicForServer = "SCHA_"+userID+"_"+messageID+"_";
              pendingDataFromJennic = 11;
          } else if (pendingDataFromJennic == 10){ //nwkAddress high
              temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 9){ //nwkAddress low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_";             
          } else if (pendingDataFromJennic == 8){ //endPoint
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";
          } else if (pendingDataFromJennic == 7){ //dayPatter
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";
          } else if (pendingDataFromJennic == 6){ //startingTime hour
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+":";          
          } else if (pendingDataFromJennic == 5){ //startingTime min
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";          
          } else if (pendingDataFromJennic == 4){ //endingTime hour
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+":";          
          } else if (pendingDataFromJennic == 3){ //endingTime min
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";          
          } else if (pendingDataFromJennic == 2){ //activateDeactivate
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";          
          } else if (pendingDataFromJennic == 1){ //addRemove
              temp8 = dataFromJennicSPI; 
              stringFromJennicForServer += String(temp8,DEC)+"_";   
              
              Serial.println();
              Serial.println("JENNIC -> ARDUINO: Scheduling ack received. Forwarding to server");              
              Serial.print("Message: ");
              Serial.println(stringFromJennicForServer);                        
              
              sendDataToServer(messageID);
              
              oneMoreLoopNeeded = 1;  
              command = 0;            
          }
          break;
          
  
        //  ******** UPDATE OF NODS BATTERY LEVEL ******************************
        case NODE_UPDATE_BAT:
          if (pendingDataFromJennic == 0){
              messageID = randomNumberGenerator();            
              stringFromJennicForServer = "BATT_"+userID+"_"+messageID+"_";            
              pendingDataFromJennic = 5;            
          } else if (pendingDataFromJennic == 4){  //deviceID high
              temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 3){  //deviceID low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_"; 
          } else if (pendingDataFromJennic == 2){  //value high 
              temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 1){  //value low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_"; 

              Serial.println();
              Serial.println("JENNIC -> ARDUINO: Nod battery update. Forwarding to server");
              Serial.print("Message: ");
              Serial.println(stringFromJennicForServer);                        
                
              sendDataToServer(messageID);
                
              oneMoreLoopNeeded = 1;  
              command = 0;   
          }
          break;
        // *********************************************************************
        
        //  ******** EXCHANGE NEW VALUES BETWEEN SERVER ARDUINO AND JENNIC *****
        case DEVICE_UPDATE_VALUE1:
          if (pendingDataFromJennic == 0){  // message type  
              messageID = randomNumberGenerator();          
                stringFromJennicForServer = "DATA_"+userID+"_"+messageID+"_";               
                pendingDataFromJennic = 6;
          } else if (pendingDataFromJennic == 5){  //Network Address high byte
                temp8 = dataFromJennicSPI;  
          } else if (pendingDataFromJennic == 4){  //Network Address low byte
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";          
          } else if (pendingDataFromJennic == 3){  //nodID
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,DEC)+"_"; 
          } else if (pendingDataFromJennic == 2){  //value high bytes
                temp8 = dataFromJennicSPI;
          } else if (pendingDataFromJennic == 1){  //value low byte
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";                 
                Serial.println();
                Serial.println("JENNIC -> ARDUINO: New sensor state update. Forwarding to server");              
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer);                        
                
                sendDataToServer(messageID);
                
                oneMoreLoopNeeded = 1;  
                command = 0;          
          }
          break;
        //  *********************************************************************
        
        //  ******** ADDING JUST JOINED NOD TO THE SERVER DATABASE **************
        case NEW_NODE:
          if (pendingDataFromJennic == 0){
              messageID = randomNumberGenerator();            
              stringFromJennicForServer = "NODE_"+userID+"_"+messageID+"_";                
              pendingDataFromJennic = 13;      
          } else if (pendingDataFromJennic == 12){  //MAC add 8th byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 11){  //MAC add 7th byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";     
          } else if (pendingDataFromJennic == 10){  //MAC add 6th byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 9){  //MAC add 5th byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 8){  //MAC add 4th byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 7){  //MAC add 3rd byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
        } else if (pendingDataFromJennic == 6){  //MAC add 2nd byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 5){  //MAC add 1st byte
              temp8 = dataFromJennicSPI;
              stringFromJennicForServer += String(temp8,HEX)+"._";
          } else if (pendingDataFromJennic == 4){  // Network address high
              temp8 = dataFromJennicSPI;  
          } else if (pendingDataFromJennic == 3){  // Network address low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";   
          } else if (pendingDataFromJennic == 2){  // Location high
              temp8 = dataFromJennicSPI;  
          } else if (pendingDataFromJennic == 1){  // Location low
              temp16 = temp8 << 8;
              temp8 = dataFromJennicSPI;
              temp16 += temp8;
              stringFromJennicForServer += String(temp16,DEC)+"_";

              Serial.println();
              Serial.println("JENNIC -> ARDUINO: New nod joined the network. Forwarding to server");
              Serial.print("Message: ");
              Serial.println(stringFromJennicForServer);                                      

              sendDataToServer(messageID);
              
              oneMoreLoopNeeded = 1; 
              command = 0;     
          
          }
          break;
        //  ***********************************************************************
        
        
        //  ************* ADD/REMOVE NETWORK FROM DATABASE ************************      
        
        case ADD_REMOVE_NETWORK:
         if (pendingDataFromJennic == 0){
                messageID = randomNumberGenerator();
                stringFromJennicForServer = "ARNWK_"+userID+"_"+messageID+"_";               
                pendingDataFromJennic = 10;       
          } else if (pendingDataFromJennic == 9){  //MAC add 8th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 8){  //MAC add 7th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";     
          } else if (pendingDataFromJennic == 7){  //MAC add 6th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 6){  //MAC add 5th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 5){  //MAC add 4th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 4){  //MAC add 3rd byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 3){  //MAC add 2nd byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 2){  //MAC add 1st byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+"._";
          } else if (pendingDataFromJennic == 1){  //ADD/REMOVE info
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,DEC)+"_";        

                Serial.println();
                Serial.println("JENNIC -> ARDUINO: Request for ADD/REMOVE network to/from db. Forwarding to server");
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer);                                      
                
                sendDataToServer(messageID);
                
                oneMoreLoopNeeded = 1;
                command = 0;              
          }
          break;        
        //  ***********************************************************************        
        
        
        //  ************* ALLOW/DISALLOW NETWORK JOINING **************************            
        case ALLOW_DISALLOW_NWK_JOIN:
          if (pendingDataFromJennic == 0){
                messageID = randomNumberGenerator();
                stringFromJennicForServer = "ADNJ_"+userID+"_"+messageID+"_";        
                pendingDataFromJennic = 2;       
          } else if (pendingDataFromJennic == 1){  //MAC add 8th byte
                temp8 = dataFromJennicSPI;      
                stringFromJennicForServer += String(temp8,DEC)+"_";   
                Serial.println();
                Serial.println("JENNIC -> ARDUINO: ALLOW/DISALOW network joining. Forwarding to server");
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer); 

                sendDataToServer(messageID);    

                oneMoreLoopNeeded = 1;
                command = 0;                
          }
          break;
        
        //  ***********************************************************************        
  
        
        //  ************* REMOVE EXISTING NODE FROM THE DB ************************      
        case REMOVE_FROM_LISTS:
          if (pendingDataFromJennic == 0){
                messageID = randomNumberGenerator();
                stringFromJennicForServer = "DELN_"+userID+"_"+messageID+"_";              
                pendingDataFromJennic = 9;       
          } else if (pendingDataFromJennic == 8){  //MAC add 8th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 7){  //MAC add 7th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";     
          } else if (pendingDataFromJennic == 6){  //MAC add 6th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 5){  //MAC add 5th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 4){  //MAC add 4th byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 3){  //MAC add 3rd byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 2){  //MAC add 2nd byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+".";
          } else if (pendingDataFromJennic == 1){  //MAC add 1st byte
                temp8 = dataFromJennicSPI;
                stringFromJennicForServer += String(temp8,HEX)+"._";

                Serial.println();
                Serial.println("JENNIC -> ARDUINO: Request for removing nod from db. Forwarding to server");
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer);                                      
                
                sendDataToServer(messageID);
                
                oneMoreLoopNeeded = 1;                
                command = 0;              
          }
          break;      
        //  ***********************************************************************        
        
        //  ******** ADDING JUST JOINED DEVICE TO THE SERVER DATABASE *************
        case NEW_DEVICE:
          if (pendingDataFromJennic == 0){
                messageID = randomNumberGenerator();            
                stringFromJennicForServer = "DEVI_"+userID+"_"+messageID+"_";                
                pendingDataFromJennic = 6;      
          } else if (pendingDataFromJennic == 5){  // Network address high
                temp8 = dataFromJennicSPI; 
          } else if (pendingDataFromJennic == 4){  // Network address low
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";         
          } else if (pendingDataFromJennic == 3){  // EndpointID
                temp8 = dataFromJennicSPI; 
                stringFromJennicForServer += String(temp8,DEC)+"_";              
          } else if (pendingDataFromJennic == 2){  // DeviceID high
                temp8 = dataFromJennicSPI; 
          } else if (pendingDataFromJennic == 1){  // DeviceID low
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";

                Serial.println();
                Serial.println("JENNIC -> ARDUINO: New device joined the network. Forwarding to server");
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer);                                      
                
                sendDataToServer(messageID);
                
                oneMoreLoopNeeded = 1;
                command = 0;
          }   
          break;
        //  ***********************************************************************
        
        //  ************ BINDING OF TWO DEVICES ***********************************
        case BIND_UNBIND:
          if (pendingDataFromJennic == 0){
                messageID = randomNumberGenerator();            
                stringFromJennicForServer = "BIND_"+userID+"_"+messageID+"_";                
                pendingDataFromJennic = 10; 
          } else if (pendingDataFromJennic == 9){  // 1.nods Network address high
                temp8 = dataFromJennicSPI; 
          } else if (pendingDataFromJennic == 8){  // 1.nods Network address low
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";  
          } else if (pendingDataFromJennic == 7){  // 1.nods EndpointID
                temp8 = dataFromJennicSPI; 
                stringFromJennicForServer += String(temp8,DEC)+"_";
          } else if (pendingDataFromJennic == 6){  // 2.nods Network address high
                temp8 = dataFromJennicSPI; 
          } else if (pendingDataFromJennic == 5){  // 2.nods Network address low
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";  
          } else if (pendingDataFromJennic == 4){  // 2.nods EndpointID
                temp8 = dataFromJennicSPI; 
                stringFromJennicForServer += String(temp8,DEC)+"_"; 
          } else if (pendingDataFromJennic == 3){  // ClusterID high
                temp8 = dataFromJennicSPI; 
          } else if (pendingDataFromJennic == 2){  // ClusterID low
                temp16 = temp8 << 8;
                temp8 = dataFromJennicSPI;
                temp16 += temp8;
                stringFromJennicForServer += String(temp16,DEC)+"_";                
          } else if (pendingDataFromJennic == 1){  // bind/unbind?
                temp8 = dataFromJennicSPI; 
                stringFromJennicForServer += String(temp8,DEC)+"_"; 

                Serial.println();
                Serial.println("JENNIC -> ARDUINO: Bond/unbind ACK received. Forwarding to server"); 
                Serial.print("Message: ");
                Serial.println(stringFromJennicForServer);                                      
                
                sendDataToServer(messageID);
                
                oneMoreLoopNeeded = 1;
                command = 0;
          }
          break;
        //  ***********************************************************************      
        
  //      case DUMMY:
  //              pendingDataFromJennic = 0;
  //              command = 0;
  //              break;
  //      default: 
  //              pendingDataFromJennic = 0;
  ////              Serial.print("UNKNOWN COMMAND FROM JENNIC ");
  ////              Serial.println(dataFromJennicSPI & CONTROL_BITS_MASK,BIN); 
  //            command = 0;
  //              break;
      }
    }      
    if (pendingDataFromJennic>0){
      pendingDataFromJennic--;    
    }
  }
}






