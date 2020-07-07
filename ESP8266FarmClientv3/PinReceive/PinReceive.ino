const int SOURCE_PIN =0;
const int DESTINATION_PIN = 1;
const int PIN_VALUE = 2;

int pinMap[6][3]={
    { 8, 12, 0 }, 
    { 7, 11, 0 },
    { 6, 10, 0 },
    { 5, 9, 0},
    { 4, 2, 0},
    { 3, 13, 0} 
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  for (int pinCounter=0;pinCounter<6;pinCounter++)
  {
    int sourcePin = pinMap[pinCounter][SOURCE_PIN];
    pinMode(sourcePin, INPUT);
    int destinationPin = pinMap[pinCounter][DESTINATION_PIN];
    pinMode(destinationPin, OUTPUT);
    digitalWrite(destinationPin, LOW);
  }
}

void loop() {
  for (int pinCounter =0; pinCounter<6;pinCounter++)
  {
    int sourcePin = pinMap[pinCounter][SOURCE_PIN];
    Serial.println("SourcePin:" + String(sourcePin));
    
    int sourcePinValue = digitalRead(sourcePin);
    Serial.println("SourcePinValue:" + String(sourcePinValue));
    
    int currentPinValue = pinMap[pinCounter][PIN_VALUE];
    Serial.println("CurrentOutPutPinValue:" + String(currentPinValue));
    
    if (sourcePinValue != currentPinValue)
    {
      int destinationPin = pinMap[pinCounter][DESTINATION_PIN];
      Serial.println("Destination pin:" + String (destinationPin));
      digitalWrite(destinationPin, sourcePinValue);
      pinMap[pinCounter][PIN_VALUE] = sourcePinValue;
    }
  }
}
