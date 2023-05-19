#define led 9
const int maxSize = 62;
const int bitDelay = 9;
int message[507] = {255, 0, 255, 0, 255};  // message table, 62*8=496 bits characters +6 bits message length +5 bits header=507 bits

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop() {
  if (Serial.available() == 0) {
    return;
  }

  const String input = Serial.readString();
  const int inputSize = input.length();

  if (inputSize > maxSize) {
    Serial.println("I received: " + input + "\n" + inputSize + " characters, message too big (max: 31 characters)");
    return;
  }

  Serial.println("sending...\nI received: " + input);

  int position = 5; // reset position to 5 (the first 5{0,1,2,3,4} cells of the table are preset with the header)
  analogWrite(led, 0);  // reset LED

  //---------- M E S S A G E - L E N G T H ----------
  // convert message length to binary (6 bit)
  // ex: if message length = 3 (characters)
  // it will be converted to: 000011
  for (int i = 5; i >= 0; i--) {
    const int inputBit = bitRead(input.length(), i);
    message[position] = inputBit == 1 ? 255 : 0;
    position++;
  }

  //---------- M E S S A G E ----------
  // convert input chars to ASCII bits and store the to message array
  for (int i = 0; i < input.length(); i++) {
    for (int j = 7; j >= 0; j--) {
      const int currentBit = bitRead(input.charAt(i), j);
      message[position] = currentBit == 1 ? 255 : 0;
      position++;
    }
  }

  //---------- T R A N S M I T - M E S S A G E ----------
  for (int i = 0; i < position; i++) {
    analogWrite(led, message[i]);
    delay(bitDelay);
  }

  Serial.println("message sent");
}
