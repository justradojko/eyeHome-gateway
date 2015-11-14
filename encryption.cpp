
#include <stdint.h>
#include <WString.h>
#include <SPI.h>

#include "encryption.h"

extern char stringFromServer[150]; // string for incoming serial data
extern String stringFromJennicForServer;

unsigned char S[256]; // S String for encryption;
unsigned char key[] = "1A7N4rwF3xWs";
String encStringFromJennicForServer; // encrypted String from Jennic
String decStringFromServer; // encrypted String from Server
uint16_t enci, encj;  // counters for encryption process

void swap(unsigned char *S, unsigned int enci, unsigned int encj) {    
        unsigned char temp = S[enci];
        S[enci] = S[encj];
        S[encj] = temp;
}

void rc4_init(unsigned char *key, unsigned int key_length) {
    for (enci = 0; enci < 256; enci++){
        S[enci] = enci;
    }
    for (enci = encj = 0; enci < 256; enci++) {
        encj = (encj + key[enci % key_length] + S[enci]) % 256;
        swap(S, enci, encj);
    }

    enci = encj = 0;
}    

unsigned char rc4Encrypt(){
    enci = (enci + 1) % 256;
    encj = (encj + S[enci]) % 256;

    swap(S, enci, encj);
    return S[(S[enci] + S[encj]) % 256];
}

void encryptString(){
  uint8_t p, pom;
  encStringFromJennicForServer = " "; //krece od 1 umesto od 0, jer pri prelasku na Galileo prvi char ne stize do servera.
  rc4_init(key,12);
  for (int p = 0; p < stringFromJennicForServer.length(); p++){ 
    pom = stringFromJennicForServer.charAt(p)^rc4Encrypt();
    if (pom >= 16){
      encStringFromJennicForServer += String(pom,HEX);
    } else if ( (pom < 16) && (pom > 0)){
      encStringFromJennicForServer += "0" + String(pom,HEX);
    } else if (pom == 0){
      encStringFromJennicForServer += "00";
    }    
  }      
}

//transformation of two hex characters into decimal number
uint8_t charToDec(char A, char B){
  if (A >= 65){
    if ( B >=65 ){
      return ( A - 87 ) * 16 + (B - 87);
    } else {
      return ( A - 87 ) * 16 + ( B - 48 );
    }    
  } else {
    if ( B >=65 ){
      return ( A - 48 ) * 16 + (B - 87);
    } else {
      return  ( A - 48 ) * 16 + ( B - 48 );      
    }   
  }    
}

void decryptString(){
  decStringFromServer = "";
  char c;
  rc4_init(key,12);
  for (uint8_t p = 0; p < strlen(stringFromServer)-1; p+=2){
      c = (charToDec(stringFromServer[p],stringFromServer[p+1])^rc4Encrypt());   
//      decStringFromServer += ASCIIcodeToChar(charToDec(stringFromServer[p],stringFromServer[p+1])^rc4Encrypt());
      decStringFromServer += c;

  }    
  decStringFromServer += "\0";
  Serial.println(" ");
}
//****************** END OF ENCRYPTION FUNCTIONS ********************************************
