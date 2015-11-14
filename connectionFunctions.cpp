#include "connectionFunctions.h"
#include <Ethernet.h>

byte macAddr[] = {0x90, 0xA2, 0xDA, 0x0F, 0x6B, 0x1C};
IPAddress serverAddr(188,226,226,76);
const int portOnServer = 31337;
EthernetClient client;
int numberOfTriesToInitializeEthernet = 0;

//void maintainConnectionToServer(){
//  if (!client.connected()){
//    connectToServer();
//  } else {
//    Serial.println("Connection checked and working fine");
//  }cd ..
//}

int initEth(){  
  int i = 0;
  int validIPAddress = 0; //checks if ip address is different then 255.255.255.255
  Serial.println("Initializing ethernet..");
  printf("echo About to restart ethernet");
  system("/etc/init.d/networking restart");  
  while (Ethernet.begin(macAddr) == 0){
    delay(10000); //wait for 10sec and try again to get the address from DHCP
    if (i>0){
      Serial.println("Faild to configure Ethernet using DHCP");
    }
    i++;
    Ethernet.begin(macAddr);
  }
  delay(3000);  
  Serial.println("Ethernet Initialized successfully"); 
  
  //print retrieved IP address from DHCP
  Serial.print("Retreived IP addr: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++){
    Serial.print(Ethernet.localIP()[thisByte],DEC);
    Serial.print(".");
    if (Ethernet.localIP()[thisByte] != 255) {
      validIPAddress = 1;
    }
  }  

  Serial.println();
  
  if (validIPAddress == 1){
    Serial.println("Valid IP address");
  } else {
    Serial.println("NOT valid IP address");
    initEth();
  }
  numberOfTriesToInitializeEthernet++;
  if (numberOfTriesToInitializeEthernet > 5){
    system("reboot");
  }
  
  return 1;
}

int connectToServer(){
  uint8_t i = 0;
  uint8_t numberOfChecks = 0;
  
  client.stop();
  delay(2000);
  
  if (Ethernet.begin(macAddr)==0){
    initEth();
    delay(5000);
  }  
  
  int conn = 0;
  
  while(!conn){
    if (i>0){
    }
    i++;

    if (numberOfChecks>5) {
      delay(3000);      
      Serial.println("Renewing lease from the DHCP server");
      initEth();      
      numberOfChecks = 0; 
      system("reboot");     
    }
    
    delay(3000);    
    conn = client.connect(serverAddr,portOnServer);
    numberOfChecks++;
  }    
  Serial.println("Connection to the server successful");
  delay(1000);
  numberOfTriesToInitializeEthernet = 0;

  client.flush(); 
}
