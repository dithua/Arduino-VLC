const int dl = 9; //transmission delay per bit
int led = 9;      //define transmission LED pin
String input;     //user input message
int message[507]; //message table, 62*8=496 bits characters +6 bits message length +5 bits header=507 bits
int pos;          //next table position to be written
byte bytes;       //current bit value
int newSize;      //message length

void setup() {
    Serial.begin(9600);         //set baud rate to 9600
    pinMode(led, OUTPUT);       //set pin 9 as output pin
    for(int i=0; i<=2; i += 2){ //---------- H E A D E R ----------
        message[i] = 255;       //set first 5 table cells with the 5 bits
        message[i+1] = 0;       //start header 
    }                           //255 === max brightness, 0 === LED is off
    message[4] = 255;           //255 === 1 (bit value), 0 === 0
    pos = 5;                    //
}                               //---------- H E A D E R ----------

void loop() {
    if(Serial.available()>0){                                                 // check if there are any data to send and if so, 
        input = Serial.readString();                                          //read the incoming String
        newSize = input.length();                                             //store message length
        if(newSize > 62){                                                     //if message is too long
          Serial.println("I received: " + input);                             //inform user
          Serial.print(newSize);                                              //
          Serial.println(" characters, message too big (max: 31 characters)");//
        }else if((newSize <= 62)){                                            //else
            Serial.println("sending...");                                     //informe user that transmission has started 
            Serial.println("I received: " + input);                           //print the received input 
            
            for(int i=5; i>=0; i--){                  //---------- M E S S A G E - L E N G T H ----------
                if(bitRead(input.length(), i) == 1){  //convert message length to binary (6 bit) 
                    message[pos] = 255;               //and store it to the next 6 cells of table "message[]"
                }else{                                //starting from the most significant bit(MSB orleft-most)
                    message[pos] = 0;                 //ex: if message length = 3 (characters)                
                }                                     //it will be converted to: 000011              
                pos++;                                //
            }                                         //---------- M E S S A G E - L E N G T H ----------       
                                             
            for(int i=0; i<input.length(); i++){       //---------- M E S S A G E ----------
                for(int j=7; j>=0; j--){               // for every character
                    bytes = bitRead(input.charAt(i),j);//for every bit of
                    if(bytes == 1){                    //the current character
                        message[pos] = 255;            //starting from the MSB
                    }else if(bytes == 0){              //store the bit to the next cell 
                        message[pos] = 0;              //of table "message[]"
                    }                                  //the characters are
                     pos++;                            //stored with ASCII encoding
                }                                      //
            }                                          //---------- M E S S A G E ----------         
                                            
            for(int k=0; k<pos; k++ ){         //---------- T R A N S M I T - M E S S A G E ---------- 
                analogWrite(led, message[k]);  //turn on or off the LED according to the current bit
                delay(dl);                     //wait for "dl" msec
            }                                  //---------- T R A N S M I T - M E S S A G E ----------     
                                         
            pos = 5;                       //reset position to 5 (the first 5{0,1,2,3,4} cells of the table are preset with the header)
            Serial.println("message sent");//inform user that message has been sent  
        }
    }
    analogWrite(led, 0);//reset LED
}
