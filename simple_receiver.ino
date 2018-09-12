#define MAX 5                                       //define max queue size to 5
String queue_start[MAX] = {"3", "3", "3", "3", "3"};//initialize queue so it is always full
int front = 0;                                      //poin front to the beginning of the queue
int rear = 4;                                       //poin rear to the end of the queue
int itemCount = 0;                                  //number of items in the queue

const int dl = 80;        //reception delay per bit
int lightPin = A0;        //define a pin for Photo resistor
int ambient = 0;          //ambient input/noise level
int reading;              //input reading
int n0 = 0;               //number of "0"s
int n1 = 0;               //number of "1"s
int n_final;              //decoded bit as integer ex: 1  (max of n0, n1)
String s_final;           //decoded bit as string ex: "1" (max of n0, n1)
int start_flag = 0;       //indicator for 3 states{0: detect message, 1: read message length, 2: read message}
String temp_string;       //current queue elements in one string
double msg_length = 0.00; //message length
double x;                 //current 8 bits of message to integer
int j;                    //counter
char temp_char;           //character produced by integer x
String final_message;     //message

void setup() {
    Serial.begin(9600);                 //set baud rate to 9600
    j = 7;                              //
    for(int i=0; i<5; i++){             //
        ambient += analogRead(lightPin);//read the ambient input/noise 
    }                                   //level 5 times
    ambient /= 5;                       //calulate average ambient
    Serial.println(ambient);            //
}

                                    //========== Q U E U E - F U N C T I O N S ==========
                                    //queue_start[] is a circular queue     
                                          
void enqueue(String data) {         //---------- e n q u e u e ----------
    if(QisEmpty()){                 //if queue is empty 
        front = 0;                  //
        rear = 0;                   //set front and rear to 0
    }else{                          //if queue is not empty 
        rear = (rear + 1)%MAX;      //increase rear by 1 and then take the modulo with MAX(in this case: 5) 
    }                               //ex: rear = 4 -> rear = (4 + 1)%5 = 5 % 5 = 0 
    queue_start[rear] = data;       //store value in queue at the new rear position
}                                   //---------- e n q u e u e ----------

void dequeue() {                    //---------- d e q u e u e ----------
    if(QisEmpty()){                 //if queue is empty 
        return;                     //do nothing
    }else if (front == rear){       //if there is only one element in the queue
        front = -1;                 //set front and rear to -1
        rear = -1;                  //
    }else{                          //if there are more than one elements in the queue
        front = (front + 1)%MAX;    //increase front by 1 and then take the
    }                               // modulo with MAX(in this case: 5)
}                                   //---------- d e q u e u e ----------

bool QisEmpty() {                   //---------- Q i s E m p t y ----------
    if((front == -1)&&(rear == -1)){//if queue is epmty
        return true;                //return false
    }else{                          //if queue is not empty
        return false;               //return false
    }                               //
}                                   //---------- Q i s E m p t y ----------
        
bool QisFull() {                    //---------- Q i s F u l l ----------
    if(((rear + 1)%MAX) == front){  //if queue is full(no extra space)
        return true;                //return true
    }else{                          //else
        return false;               //return false
    }                               //
}                                   //---------- Q i s F u l l ----------

                                    //========== Q U E U E - F U N C T I O N S ==========
                        
void loop() {                           //---------- D E C O D E - B I T ----------
    for(int i=0; i<5; i++){             //for 5 times
        reading = analogRead(lightPin); //read input 
        if (reading > (ambient + 100)){ //if input value is greater than ambient by a 100 
           n1 += 1;                     //a "1" was found so increase n1 by 1
        }else{                          //else
           n0 += 1;                     //a "0" was found so increase n0 by 1
        }                               //
        delay(dl);                      //wait dl msec
    }                                   //
    if (n1>n0){                         //if more "1"s were found
        n_final = 1;                    //then bit value is 1
        s_final = "1";                  //
    }else{                              //else(more "0"s were found)
        n_final = 0;                    //then bit value is 0
        s_final = "0";                  //
    }                                   //
    n1 = 0;                             //reset counters
    n0 = 0;                             //---------- D E C O D E - B I T ---------- 
             
                                                //---------- S Y N C H R O N I Z A T I O N ----------
    if (start_flag == 0){                       //if message has not been detected (state: 0)
        dequeue();                              //add the decoded bit to the queue
        enqueue(s_final);                       //turn queue into string(next line)
        temp_string = String(queue_start[front%MAX] + queue_start[(front + 1)%MAX] + queue_start[(front + 2)%MAX] + queue_start[(front + 3)%MAX] + queue_start[(front + 4)%MAX]);
        if(temp_string.compareTo("10101") == 0){//if the string is the header
            start_flag = 1;                     //message has been detected so set state to 1
        }                                       //---------- S Y N C H R O N I Z A T I O N ----------
        
                                                    //---------- D E C O D E - M E S S A G E - L E N G T H ----------
    }else if (start_flag == 1){                     //if message has been detected (state: 1)
        if (j>=0){                                  //for every bit of the 8 starting from the left-most (MSB: most significant bit)
            if(n_final == 1){                       //if decoded bit has a value of "1"
                msg_length = msg_length + pow(2,j); //calculate 2^j and add 
            }                                       //the result to msg_length variable
            j--;                                    //
        }else{                                      //when message length has been calculated
            start_flag = 2;                         //change the state to 2
            j = 7;                                  //reset counter j
        }                                           //
                                                    //---------- D E C O D E - M E S S A G E - L E N G T H ----------
                                                    
                                                            //---------- D E C O D E - M E S S A G E ----------
    }else{                                                  //if message length has been decoded (state: 2)
        if(msg_length>0.00){                                //for every character
            if (j>=0){                                      //for every bit of the current character starting from the MSB
                if(n_final == 1){                           //if decoded bit has a value of "1"
                    x = x + pow(2,j);                       //calculate 2^j and add the 
                }                                           //result to x variable
                j--;                                        //
            }else{                                          //if int value of character has been calculated
                temp_char = ++x;                            //convert integer "x" to character "temp_char" according to ASCII ex: x = 97 -->temp_char = a
                final_message = final_message + temp_char;  //add character "temp_char" at the end of the string "final_message" ex: final_message = kj, temp_char = a -->final_message=kja
                j = 7;                                      //reset counter j
                x = 0.00;                                   //reset x
                msg_length--;                               //
            }                                               //
        }else{                                              //when the last character is added to the string"final_message"
            Serial.println(final_message);                  //print message
            start_flag = 0;                                 //reset state to 0
            msg_length = 0.00;                              //reset message length(msg_length) to 0.00
            j = 7;                                          //reset counter j
            final_message = String('\0');                   //reset string"final_message"to null
        }                                                   //---------- D E C O D E - M E S S A G E ----------
    }                                                       
} 
