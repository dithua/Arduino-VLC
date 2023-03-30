#define MAX 5                                        // define max queue size to 5
String queue_start[MAX] = {"3", "3", "3", "3", "3"}; // initialize queue so it is always full
int front = 0;                                       // poin front to the beginning of the queue
int rear = 4;                                        // poin rear to the end of the queue

const int dl = 3;     // reception delay per bit
int lightPin = A0;    // define reception LED pin
int ambient = 0;      // ambient input/noise level
int reading;          // input reading
int n0 = 0;           // number of "0"s
int n1 = 0;           // number of "1"s
int f0;               // indicates if that at the first reading(of the 3)the input value was0
int f1;               // indicates if that at the first reading(of the 3)the input value was1
int f2;               // indicator for 3 states{0: transmitter and receiver are in sync, 1: transmitter is ahead of time, 2: receiver is ahead of time}
int n_final;          // decoded bit as integer ex: 1  (max of n0, n1)
String s_final;       // decoded bit as string ex: "1" (max of n0, n1)
int start_flag = 0;   // indicator for 3 states{0: detect message, 1: read message length, 2: read message}
String temp_string;   // current queue elements in one string
int msg_length = 0;   // message length
int x;                // current 8 bits of message to integer
int j, k;             // counters
char temp_char;       // character produced by integer x
String final_message; // message

void setup()
{
    Serial.begin(9600); // set baud rate to 9600
    j = 5;              //
    k = 7;              //
    f0 = 0;             // initialise values
    f1 = 0;             //
    f2 = 0;             //
    for (int i = 0; i < 5; i++)
    {                                    //
        ambient += analogRead(lightPin); // read the ambient input/noise
    }                                    // level 5 times
    ambient /= 5;                        // calulate average ambient
    Serial.println(ambient);             //
} //

//========== Q U E U E - F U N C T I O N S ==========
// queue_start[] is a circular queue

void dequeue()
{                              //---------- d e q u e u e ----------
    front = (front + 1) % MAX; // increase front by 1 and then take the modulo with MAX(in this case: 5)ex: front = 4-> front = (4 + 1)%5 = 5 % 5 = 0
} //---------- d e q u e u e ----------

void enqueue(String data)
{                             //---------- e n q u e u e ----------
    dequeue();                // remove first item of the queue(because queue is full)
    rear = (rear + 1) % MAX;  // increase rear by 1 and then take the modulo with MAX(in this case: 5) ex: rear = 4 -> rear = (4 + 1)%5 = 5 % 5 = 0
    queue_start[rear] = data; // store value in queue at the new rear position
} //---------- e n q u e u e ----------

void resetQ()
{                         //---------- r e s e t Q ----------
    queue_start[4] = "3"; // set the last cell of the queue with an invalid character to break the sequence of the previous header bits(header:10101)
    front = 0;            // reset front position
    rear = 4;             // reset rear position
} //---------- r e s e t Q ----------

//========== Q U E U E - F U N C T I O N S ==========

void loop()
{ //---------- A D A P T I V E - S Y N C ----------
    for (int i = 0; i < 3; i++)
    { // for 3 times
        if (f2 == 0)
        {                                   // if transmitter and receiver are in sync(state:0)
            reading = analogRead(lightPin); // read input
            if (reading > (ambient + 100))
            {            // if input value is greater than ambient by a 100
                n1 += 1; // a "1" was found so increase n1 by 1
                if (i == 0)
                {           // if the first reading
                    f1 = 1; // is 1 then f1 = 1
                }           //
            }
            else
            {            // else
                n0 += 1; // a "0" was found so increase n0 by 1
                if (i == 0)
                {           // if the first reading
                    f0 = 1; // is 0 then f0 = 1
                }           //
            }               //
            delay(dl);      // wait dl msec
        }
        else if (f2 == 1)
        { // if transmitter is ahead of time(state:1)
            if (i != 2)
            {                                   // for the first 2 times
                reading = analogRead(lightPin); // read input
                if (reading > (ambient + 100))
                {            // if input value is greater than ambient by a 100
                    n1 += 1; // a "1" was found so increase n1 by 1
                    if (i == 0)
                    {           // if the first reading
                        f1 = 1; // is 1 then f1 = 1
                    }           //
                }
                else
                {            // else
                    n0 += 1; // a "0" was found so increase n0 by 1
                    if (i == 0)
                    {           // if the first reading
                        f0 = 1; // is 0 then f0 = 1
                    }           //
                }               //
                delay(dl);      // wait dl msec
            }                   //(skips last reading)
        }
        else
        { //
            if (i == 0)
            {                               // if it is the first reading
                delay(dl);                  // wait dl msec
            }                               //(wait one reading period and then read 3 times)
            reading = analogRead(lightPin); // read input
            if (reading > (ambient + 100))
            {            // if input value is greater than ambient by a 100
                n1 += 1; // a "1" was found so increase n1 by 1
                if (i == 0)
                {           // if the first reading
                    f1 = 1; // is 1 then f1 = 1
                }           //
            }
            else
            {            // else
                n0 += 1; // a "0" was found so increase n0 by 1
                if (i == 0)
                {           // if the first reading
                    f0 = 1; // is 0 then f0 = 1
                }           //
            }               //
            delay(dl);      // wait dl msec
        }                   //
    }                       //---------- A D A P T I V E - S Y N C ----------
    f2 = 0;                 // reset f2's state

    //---------- D E C O D E - B I T ----------
    if (n1 >= n0)
    {                  // if more "1"s were found
        n_final = 1;   // then bit value is 1
        s_final = "1"; //
        if (n0 == 1)
        { // if a 0 was found once
            if (f1 == 1)
            {           // if 1 was found at the first reading(0 was found at the last reading)
                f2 = 1; // transmitter is ahead of time
            }
            else
            {           // if 1 was not found at the first reading(0 was found at the first reading)
                f2 = 2; // receiver is ahead of time
            }           //
            f1 = 0;     // reset f1
        }               //
    }
    else
    {                  // else(more "0"s were found)
        n_final = 0;   // then bit value is 0
        s_final = "0"; //
        if (n1 == 1)
        { // if a 1 was found once
            if (f0 == 1)
            {           // if 0 was found at the first reading(1 was found at the last reading)
                f2 = 1; // transmitter is ahead of time
            }
            else
            {           // if 0 was not found at the first reading(1 was found at the first reading)
                f2 = 2; // receiver is ahead of time
            }           //
            f0 = 0;     // reset f0
        }               //
    }                   //
    n1 = 0;             // reset counters
    n0 = 0;             //---------- D E C O D E - B I T ----------

    //---------- S Y N C H R O N I Z A T I O N ----------
    if (start_flag == 0)
    {                                                                                                                                                                                       // if message has not been detected (state: 0)
        enqueue(s_final);                                                                                                                                                                   // add the decoded bit to the queue
        temp_string = String(queue_start[front % MAX] + queue_start[(front + 1) % MAX] + queue_start[(front + 2) % MAX] + queue_start[(front + 3) % MAX] + queue_start[(front + 4) % MAX]); // turn queue into string
        if (temp_string.compareTo("10101") == 0)
        {                   // if the string is the header
            start_flag = 1; // message has been detected so set state to 1(state: read message length)
        }                   //---------- S Y N C H R O N I Z A T I O N ----------

        //---------- D E C O D E - M E S S A G E - L E N G T H ----------
    }
    else if (start_flag == 1)
    { // if message has been detected (state: 1)
        if (j >= 0)
        { // for every bit of the 6 starting from the left-most(MSB: most significant bit)
            if (n_final == 1)
            {                          // if decoded bit has a value of "1"
                bitSet(msg_length, j); // set the "j"bit (of msg_length
            }                          // variable) value as "1"
            if (j > 0)
            {        // ex: j=4 (msg_length is initialized as 0 )
                j--; // 000000 --> 001000 binary or 16 decimal(msg_length is integer so it is auto-converted)
            }
            else
            {                   // when the 6th bit is decoded before moving on with the next bit
                start_flag = 2; // change the state to 2(state: read message)
            }                   //
        }                       //---------- D E C O D E - M E S S A G E - L E N G T H ----------

        //---------- D E C O D E - M E S S A G E ----------
    }
    else
    { // if message length has been decoded (state: 2)
        if (msg_length > 0)
        { // for every character
            if (k >= 0)
            { // for every bit of the current character starting from the MSB
                if (n_final == 1)
                {                 // if decoded bit has a value of "1"
                    bitSet(x, k); // set the "j"bit (of x variable)value as "1"
                }                 //
                if (k > 0)
                {        //
                    k--; //
                }
                else
                {                                              // when the 8th bit is decoded before moving on with the next bit
                    temp_char = x;                             // convert integer "x" to character "temp_char" according to ASCII ex:x = 97-->temp_char= a
                    final_message = final_message + temp_char; // add character "temp_char" at the end of the string "final_message"ex: final_messag =kj, temp_char = a-->final_message=kja
                    j = 5;                                     // reset counter j
                    k = 7;                                     // reset counter k
                    x = 0;                                     // reset x
                    if (msg_length > 1)
                    {                 //
                        msg_length--; //
                    }
                    else
                    {                                  // when the last character is added to the string "final_message"
                        Serial.println(final_message); // print message
                        start_flag = 0;                // reset state to 0
                        msg_length = 0;                // reset message length(msg_length)to 0
                        final_message = String('\0');  // reset string
                        resetQ();                      //"final_message" to null and reset queue
                    }                                  //---------- D E C O D E - M E S S A G E ----------
                }
            }
        }
    }
}
