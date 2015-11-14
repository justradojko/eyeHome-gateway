#include <stdint.h>
#include <Event.h>
#include <WString.h>

void pollJennicSPI();
uint8_t  sendDataToJennic(uint8_t data);
void addDataToBufferForJennic(uint8_t Data);
uint8_t checkIfSensorValueExistsInBuffer(uint8_t deviceIDhigh, uint8_t deviceIDlow, uint8_t nodID);
uint8_t checkIfTimeValueExistsInBuffer();
String readFromServer();
void pollServerForNewValues();
void newSensorStateReceivedFromServer();
void pollServerForTimeUpdate();
void timeReceivedFromServer();
void ACKReceivedFromServer();
void sendDataToServer();
void newAllowJoiningStateFromServer();
void newSchedulingFromServer();
void handleCurtainCycleTimeUpdate();


//temp
void hanldeIncomingBind();
void bind();





