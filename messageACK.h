boolean checkIfMessageIDIsAlreadyInUse(uint8_t ID);
int randomNumberGenerator();
void insertMessageIntoMessageACKArray(String message, int messageID);
void removeMessageFromMessageACKarray(int ID);
void resendPendingMessages();

