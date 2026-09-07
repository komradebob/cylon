
//////////////////////////////////////////////////////////////////////////////
//																			                                                       //
// Copyright 2026 R.M. Bownes												                                 		//
//																			                                                       //
//////////////////////////////////////////////////////////////////////////////

#define S

#ifdef UNO
#define CLOCK_PIN 12
#define DATA_OUT 11
#define PL_PIN 10
#define DATA_IN 9
#else                // digispark
#define CLOCK_PIN 0
#define DATA_OUT 1
#define PL_PIN 2
#define DATA_IN 5
#define PWM_PIN 4
#endif

#define DEBUG 0

unsigned int ginterval;

//
// get_keys
//
//  Get the status of the keys on the display.
//
// c is an unsigned char.
// bit 7 - sw17
// bit 6 - sw18
// bit 5 - sw19
// bit 4 - Data Ready  - a key 0-15 is pressed
// bit 3-0 - Which key, 0-15 is currently pressed if Data Ready is high, otherwise last key 0-15 pressed.

unsigned char get_keys()
{
 unsigned char c,i;
 
  digitalWrite(PL_PIN,LOW);    // toggle the Parallel Load pin
  digitalWrite(PL_PIN,HIGH);
  c=digitalRead(DATA_IN);      // read the first data bit
  for(i=0;i<7;i++) {           // clock the register 7 more times to read the rest
    c = c<<1;                  // shift c
    sendZero();                // send the clock
    if(digitalRead(DATA_IN))   // get the next bit and tack it onto the end of c
      c = c | 0x1;
  }
  return(c);                   // return the results
}

//                        Send the single start bit. Include toggling the PL line on the 74hc165
void start_bit()
{

  digitalWrite(PL_PIN,LOW);
  digitalWrite(PL_PIN,HIGH);
  sendOne();
}
  
//                        Send a one down the line. No read.
void sendOne()
{
    digitalWrite(DATA_OUT, HIGH);
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CLOCK_PIN, LOW);
}

//                       Send a zero down the line. No read.
void sendZero()
{
    digitalWrite(DATA_OUT, LOW);
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CLOCK_PIN, LOW);
}

void clear_display()    // Write all 0's to the display
{

  int i;
  sendOne();
  for (i=0;i<36;i++)
    sendZero();
}

void fill_display()    // Write all 1's to the display
{

  int i;
  for (i=0;i<36;i++)
    sendOne();
}


//  update display
//
// send display_word and 3buttons to the display and read c back from the keyboard
//
// display_word is 32 bit long with lsb being displayed in the red LED in position 16, bit 16 is the Green LED in position 16
// 3button is an insigned char with the 3 lsb being the LEDs on sw19,18,17, bit0 is sw17, bit1 is sw18, bit2 is sw19.
//
void update_display(unsigned long display_word, unsigned char keyleds)
{

unsigned long mask = 0x80000000;  
unsigned char c,k;  
int i;
  // send the start bit
 start_bit(); 
  
  //for i = 0 to 31  clock out a bit, read a bit
  for(i=0;i<32;i++) {
                        
   if(mask & display_word)  // If MSB is high, send a one, otherwise send a zero
       sendOne();
   else
       sendZero();
   display_word = display_word << 1;    // and pop the bit off the end. 
  }              // for 
  
 // send out 3button
  for(i=0;i<3;i++) {
   if(0x04 & keyleds)
     sendOne();
   else
     sendZero();
   keyleds = keyleds << 1;
 }
}







//                            Flash the display repeat times at interval ms
void flash_display(int repeat, int interval)
{
int i;
  for (i=0;i<=repeat;i++) {
     clear_display();
     delay(interval);
     
     fill_display();
     delay(interval);
  }
}
//
// setup()
//
// Run once at startup
//  
//  Initialize the panel, io pins, etc.
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_OUT, OUTPUT);
  pinMode(PL_PIN,OUTPUT);
  pinMode(DATA_IN,INPUT);
//  Serial.begin(9600);      // open the serial port at 9600 bps:    
//  trip = 0;
  ginterval = 100;
  flash_display(10,200);  // tell us its started

}


#define BUMP 10

//
// Main loop of the program
//
void loop() {

  
// loop forever. read the keys, put the results in the green row
// put some cylon lights on the second row
//

unsigned char c,dir,threekeys;
unsigned long disp,leds;
//unsigned int trip,interval;

disp = 0x01;
//if( trip >10)
//  trip = 0;
  
while(disp < 0x8000) {
  c = get_keys();
  leds = c;
  threekeys = c;
  threekeys = threekeys >>5;
  
  if((c & 0x40) == 0)   // sw18 pressed
  {
//    if (trip == 0) // never been here before
//      { 
//        trip++;
        ginterval = ginterval + BUMP;
//      }
  }
  if(( c & 0x20) == 0  )  //sw19 pressed
  {
//    if (trip ==0 )
//      {
//        trip++;
        if(ginterval >=20) 
          ginterval = ginterval - BUMP;
//      }
  }    
 
  if( (c & 0x80) == 0 )
    ginterval = 100;  
  
  leds = leds <<16;
  leds = leds | disp;
  update_display(leds,threekeys);
  disp = disp << 1;
  delay(ginterval);
//  trip = 0;
 }
 while(disp >1) {
   c = get_keys();
   leds = c;
   threekeys = c;
   threekeys = threekeys >>5;
  if((c & 0x40) == 0)   // sw18 pressed
  {
    //if (trip == 0) // never been here before
//      { 
//        trip++;
        ginterval = ginterval+BUMP;
//      }
  }
  if(( c & 0x20) == 0  )  //sw19 pressed
  {
 //   if (trip ==0 )
  //    {
//        trip++;
        if(ginterval > 20) 
          ginterval=ginterval-BUMP;
//        else
//          ginterval=30;
  //    }
  }    
 
  if( (c & 0x80) == 0 )
    ginterval = 100;  
    
   leds = leds <<16;
   leds = leds | disp;
   update_display(leds,threekeys);
   disp = disp >> 1;
   delay(ginterval);
 }  
}
