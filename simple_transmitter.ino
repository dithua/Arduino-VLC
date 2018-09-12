const int dl = 400; //transmission delay per bit
int led = 9;        //define transmission LED pin
String input;       //user input message

void setup() {
    Serial.begin(9600);   //set baud rate to 9600
    pinMode(led, OUTPUT); //set pin 9 as output pin
}

void loop() {
    if (Serial.available() > 0) {              // check if there are any data to send and if so,
       input = Serial.readString();            // read the incoming String
       Serial.println("I received: " + input); // print the incoming String
       
       for(int i=0; i<2; i++){    //---------- H E A D E R ----------
           analogWrite(led, 255); // transmit the header's bits 10101
           delay(dl);             //if bit=1 set the LED to illuminate (255) 
           analogWrite(led, 0);   //if bit=0 turn off the LED (0)
           delay(dl);             //between each bit wait for "dl" msec
       }                          //
       analogWrite(led, 255);     //
       delay(dl);                 //---------- H E A D E R ----------
       
       for(int i=7; i>=0; i--){               //---------- M E S S A G E - L E N G T H ----------
          if(bitRead(input.length(), i) == 1){//convert message length to binary (8 bit)
            analogWrite(led, 255);            //starting from the 
            delay(dl);                        //most significant bit (MSB or left-most)
          }else{                              //ex: if message length = 3 (characters)
            analogWrite(led, 0);              //it will be converted to: 00000011
            delay(dl);                        //send each converted bit
          }                                   //
       }                                      //---------- M E S S A G E - L E N G T H ----------   
                                           
       for(int i=0; i<input.length(); i++){           //---------- M E S S A G E ----------
          if(bitRead(input.charAt(i),7) == 1){        //for every character
              analogWrite(led, 255);                  //send the first bit
              delay(dl);                              //(MSB) twice
          }else if(bitRead(input.charAt(i),7) == 0){  //
              analogWrite(led, 0);                    //
              delay(dl);                              //
          }                                           //
          for(int j=7; j>=0; j--){                    //for every bit of the current character
              byte bytes = bitRead(input.charAt(i),j);//
              if(bytes == 1){                         //starting from the MSB
                  analogWrite(led, 255);              //
                  delay(dl);                          //send current bit of
              }else if(bytes == 0){                   //current character
                  analogWrite(led, 0);                //
                  delay(dl);                          //
              }                                       //
          }                                           //
       }                                              //---------- M E S S A G E ----------
    }
    analogWrite(led, 0); //reset LED
}
