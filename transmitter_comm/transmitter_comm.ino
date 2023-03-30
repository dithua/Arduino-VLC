const int dl = 9; // transmission delay per bit
int led = 9;      // define transmission LED pin
String input;     // user input message
int message[507]; // message table, 62*8=496 bits characters +6 bits message length +5 bits header=507 bits
int pos;          // next table position to be written
byte bytes;       // current bit value
int newSize;      // message length

void setup()
{
    Serial.begin(9600);   // set baud rate to 9600
    pinMode(led, OUTPUT); // set pin 9 as output pin
    for (int i = 0; i <= 2; i += 2)
    {                       //---------- H E A D E R ----------
        message[i] = 255;   // set first 5 table cells with the 5 bits
        message[i + 1] = 0; // start header
    }                       // 255 === max brightness, 0 === LED is off
    message[4] = 255;       // 255 === 1 (bit value), 0 === 0
} //---------- H E A D E R ----------

void loop()
{
    if (Serial.available() == 0)
    {
        return;
    }

    // check if there are any data to send and if so,
    input = Serial.readString(); // read the incoming String
    newSize = input.length();    // store message length

    // if message is too long
    if (newSize > 62)
    {
        Serial.println("I received: " + input);
        Serial.print(newSize);
        Serial.println(" characters, message too big (max: 31 characters)");
        return;
    }

    Serial.println("sending...");           // informe user that transmission has started
    Serial.println("I received: " + input); // print the received input

    pos = 5;             // reset position to 5 (the first 5{0,1,2,3,4} cells of the table are preset with the header)
    analogWrite(led, 0); // reset LED

    //---------- M E S S A G E - L E N G T H ----------
    // convert message length to binary (6 bit)
    // and store it to the next 6 cells of table "message[]"
    // starting from the most significant bit(MSB orleft-most)
    // ex: if message length = 3 (characters)
    // it will be converted to: 000011
    for (int i = 5; i >= 0; i--)
    {

        int inputBit = bitRead(input.length(), i);
        message[pos] = inputBit == 1 ? 255 : 0;
        pos++;
    }

    //---------- M E S S A G E ----------
    // for every bit of
    // for every character
    // the current character
    // starting from the MSB
    // store the bit to the next cell
    // of table "message[]"
    // the characters are
    // stored with ASCII encoding
    for (int i = 0; i < input.length(); i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            bytes = bitRead(input.charAt(i), j);
            message[pos] = bytes == 1 ? 255 : 0;
            pos++;
        }
    }

    //---------- T R A N S M I T - M E S S A G E ----------
    // turn on or off the LED according to the current bit
    // wait for "dl" msec
    for (int k = 0; k < pos; k++)
    {
        analogWrite(led, message[k]);
        delay(dl);
    }

    Serial.println("message sent"); // inform user that message has been sent
}
