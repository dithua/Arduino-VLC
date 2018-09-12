const int dl = 400; //transmission delay per bit
int led = 9;        //define transmission LED pin
String input;       //user input message
int message[300];   //table message, ready for transmission
int pos;            //next table position to be written

void setup() {
    Serial.begin(9600);         //set baud rate to 9600
    pinMode(led, OUTPUT);       //set pin 9 as output pin
    for(int i=0; i<=2; i += 2){ //---------- H E A D E R ----------
        message[i] = 255;       //set first 5 table cells with the 5 bits start header
        message[i+1] = 0;       //255 === max brightness, 0 === LED is off
    }                           //255 === 1 (bit value), 0 === 0 
    message[4] = 255;           //
    pos = 5;                    //
}                               //---------- H E A D E R ----------

void loop() {
    if (Serial.available() > 0) {                 //check if there are any data to send and if so,
        input = Serial.readString();              //read the incoming String
        Serial.println("I received: " + input);   //print the incoming String
        for(int i=7; i>=0; i--){                  
                                                  //---------- M E S S A G E - L E N G T H ----------
            if(bitRead(input.length(), i) == 1){  //convert message length to binary (8 bit)
                message[pos] = 255;               //starting from the
            }else{                                //most significant bit (MSB or left-most)
                message[pos] = 0;                 //ex: if message length = 3 (characters)
            }                                     //it will be converted to:
            pos++;                                //00000011
        }                                         //---------- M E S S A G E - L E N G T H ----------
                                                 
        for(int i=0; i<input.length(); i++){            //---------- M E S S A G E ----------
            for(int j=7; j>=0; j--){                    //for every character
                byte bytes = bitRead(input.charAt(i),j);//for every bit of the current character
                if(bytes == 1){                         //starting from the MSB
                    message[pos] = 255;                 //
                }else if(bytes == 0){                   //store the bit to the next cell of 
                    message[pos] = 0;                   //table "message[]"
                }                                       //the characters are stored with ASCII encoding
                pos++;                                  //
            }                                           //
        }                                               //---------- M E S S A G E ---------- 
                                                     
        for(int k=0; k<pos; k++ ){        //---------- T R A N S M I T - M E S S A G E ----------
            analogWrite(led, message[k]); //turn on or off the LED according to the current bit
            delay(dl);                    //wait for "dl" msec
        }                                 //---------- T R A N S M I T - M E S S A G E ----------
        
        pos = 5;//reset position to 5(the first 5{0,1,2,3,4} cells of the table are preset with the header)
    }
    analogWrite(led, 0);//reset LED
}
