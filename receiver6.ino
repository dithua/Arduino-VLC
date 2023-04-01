#define MAX 5                                        // define max queue size to 5
String queue_start[MAX] = {"3", "3", "3", "3", "3"}; // initialize queue so it is always full
int qFront = 0;                                      // poin front to the beginning of the queue
int qRear = 4;                                       // poin rear to the end of the queue

const String msgPrefix = "10101";
const int dl = 3;  // reception delay per bit
int lightPin = A0; // define reception LED pin
int ambient = 0;   // ambient input/noise level
int reading;       // input reading

enum state
{
    Detect = 0,
    ReadLength = 1,
    ReadMessage = 2
};

state currentState = Detect; // indicator for 3 states{0: detect message, 1: read message length, 2: read message}

enum syncState
{
    InSync = 0,
    TransmitterFirst = 1,
    ResceiverFirst = 2
};

syncState currentSyncState; // indicator for 3 states{0: transmitter and receiver are in sync, 1: transmitter is ahead of time, 2: receiver is ahead of time}

int countZero = 0; // number of "0"s
int countOne = 0;  // number of "1"s

bool firstReadingZero; // indicates if that at the first reading(of the 3)the input value was0

String decodedBit; // decoded bit as string ex: "1" (max of n0, n1)

String temp_string; // current queue elements in one string
int msgLength = 0;  // message length

int currentCharAsInt; // current 8 bits of message to integer
char currentChar;     // character produced by integer x

int lengthBitPosition, charBitPosition; // counters
String finalMessage;                    // message

void setup()
{
    Serial.begin(9600);                   // set baud rate to 9600
    lengthBitPosition = 5;                //
    charBitPosition = 7;                  //
    firstReadingZero = false;             //
    currentSyncState = syncState::InSync; //
    for (int i = 0; i < 5; i++)
    {                                          //
        ambient += analogRead(lightPin) + 100; // read the ambient input/noise
    }                                          // level 5 times
    ambient /= 5;                              // calulate average ambient
    Serial.println(ambient);                   //
} //

//========== Q U E U E - F U N C T I O N S ==========
// queue_start[] is a circular queue

void dequeue()
{                                //---------- d e q u e u e ----------
    qFront = (qFront + 1) % MAX; // increase front by 1 and then take the modulo with MAX(in this case: 5)ex: front = 4-> front = (4 + 1)%5 = 5 % 5 = 0
} //---------- d e q u e u e ----------

void enqueue(String data)
{                              //---------- e n q u e u e ----------
    dequeue();                 // remove first item of the queue(because queue is full)
    qRear = (qRear + 1) % MAX; // increase rear by 1 and then take the modulo with MAX(in this case: 5) ex: rear = 4 -> rear = (4 + 1)%5 = 5 % 5 = 0
    queue_start[qRear] = data; // store value in queue at the new rear position
} //---------- e n q u e u e ----------

void resetQ()
{                         //---------- r e s e t Q ----------
    queue_start[4] = "3"; // set the last cell of the queue with an invalid character to break the sequence of the previous header bits(header:10101)
    qFront = 0;           // reset front position
    qRear = 4;            // reset rear position
} //---------- r e s e t Q ----------

//========== Q U E U E - F U N C T I O N S ==========

void handleInput(int position)
{

    reading = analogRead(lightPin); // read input
    if (reading > ambient)
    {                  // if input value is greater than ambient by a 100
        countOne += 1; // a "1" was found so increase n1 by 1
        if (position == 0)
        {                             // if the first reading
            firstReadingZero = false; // is 1 then f1 = 1
        }                             //
    }
    else
    {                   // else
        countZero += 1; // a "0" was found so increase n0 by 1
        if (position == 0)
        {                            // if the first reading
            firstReadingZero = true; // is 0 then f0 = 1
        }                            //
    }
    delay(dl);
}

//---------- A D A P T I V E - S Y N C ----------
void syncReceiver()
{
    for (int i = 0; i < 3; i++)
    {

        switch (currentSyncState)
        {

        case syncState::InSync:
            handleInput(i);
            break;
        case syncState::TransmitterFirst:
            if (i != 2)
            {
                break; // skip 3rd reading
            }
            handleInput(i);
            break;
        case syncState::ResceiverFirst:
            if (i == 0)
            {
                delay(dl); // if it is the first reading wait one reading period and then read 3 time
            }
            handleInput(i);
            break;
        }
    }

    // reset sync state
    currentSyncState = syncState::InSync;
}

void decodeBit()
{

    if (countOne >= countZero)
    {                     // if more "1"s were found
        decodedBit = "1"; //
        if (countZero == 1)
        { // if a 0 was found once
            currentSyncState = !firstReadingZero ? syncState::TransmitterFirst : syncState::ResceiverFirst;
            firstReadingZero = false;
        }
    }
    else
    { // else(more "0"s were found)
        decodedBit = "0";

        if (countOne == 1)
        { // if a 1 was found once
            currentSyncState = firstReadingZero ? syncState::TransmitterFirst : syncState::ResceiverFirst;
            firstReadingZero = false;
        }
    }

    // reset counters
    countOne = 0;
    countZero = 0;
}

void detectMsgStart()
{                                                                                                                                                                                            // if message has not been detected (state: 0)
    enqueue(decodedBit);                                                                                                                                                                     // add the decoded bit to the queue
    temp_string = String(queue_start[qFront % MAX] + queue_start[(qFront + 1) % MAX] + queue_start[(qFront + 2) % MAX] + queue_start[(qFront + 3) % MAX] + queue_start[(qFront + 4) % MAX]); // turn queue into string
    currentState = temp_string.compareTo(msgPrefix) == 0 ? state::ReadLength : currentState;
}

void decodeMsgLength()
{
    // for every bit of the 6 starting from the left-most(MSB: most significant bit)
    if (decodedBit.compareTo("1") == 0)
    {                                         // if decoded bit has a value of "1"
        bitSet(msgLength, lengthBitPosition); // set the "j"bit (of msg_length
    }                                         // variable) value as "1"
    if (lengthBitPosition > 0)
    {                        // ex: j=4 (msg_length is initialized as 0 )
        lengthBitPosition--; // 000000 --> 001000 binary or 16 decimal(msg_length is integer so it is auto-converted)
    }
    else
    {                                      // when the 6th bit is decoded before moving on with the next bit
        currentState = state::ReadMessage; // change the state to 2(state: read message)
    }
}

void decodeMsg()
{
    // for every bit of the current character starting from the MSB

    if (decodedBit.compareTo("1") == 0)
    {                                              // if decoded bit has a value of "1"
        bitSet(currentCharAsInt, charBitPosition); // set the "j"bit (of x variable)value as "1"
    }

    if (charBitPosition > 0)
    {
        charBitPosition--;
    }
    else
    {                                              // when the 8th bit is decoded before moving on with the next bit
        currentChar = currentCharAsInt;            // convert integer "x" to character "temp_char" according to ASCII ex:x = 97-->temp_char= a
        finalMessage = finalMessage + currentChar; // add character "temp_char" at the end of the string "final_message"ex: final_messag =kj, temp_char = a-->final_message=kja
        charBitPosition = 7;                       // reset counter k
        currentCharAsInt = 0;                      // reset x

        if (msgLength > 1)
        {
            msgLength--;
        }
        else
        {                                 // when the last character is added to the string "final_message"
            Serial.println(finalMessage); // print message
            currentState = state::Detect; // reset state to 0
            lengthBitPosition = 5;        // reset counter j
            msgLength = 0;                // reset message length(msg_length)to 0
            finalMessage = String('\0');  // reset string
            resetQ();                     //"final_message" to null and reset queue
        }
    }
}

void readMsg()
{
    switch (currentState)
    {

    case state::Detect:
        detectMsgStart();
        break;
    case state::ReadLength:
        decodeMsgLength();
        break;
    case state::ReadMessage:
        if (msgLength == 0)
        {
            return;
        }
        decodeMsg();
        break;
    }
}

void loop()
{
    syncReceiver();
    decodeBit();
    readMsg();
}
