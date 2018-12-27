#include <SoftwareSerial.h>

SoftwareSerial mySerial(0, 5); // RX, TX
#define NUM_CHANS 4 // number of speaker outputs
bool x[NUM_CHANS];
#define CPU_MHZ F_CPU/1000000   // ATtiny CKSEL 0100: 8 Mhz, 1x prescale (lfuse=E4)


volatile unsigned int scorewait_interrupt_count;
volatile unsigned int delaywait_interrupt_count;

// variables for music-playing



volatile long accumulator [NUM_CHANS];
volatile long decrement [NUM_CHANS];
volatile boolean playing [NUM_CHANS];

#define POLLTIME_USEC 50   // polling interval in microseconds
#define ACCUM_RESTART 4194304L  // 2^22 allows 1-byte addition on 3- or 4-byte numbers
#define MAX_NOTE 123

const long decrement_PGM[MAX_NOTE+1] PROGMEM = {
    3429L, 3633L, 3849L, 4078L, 4320L, 4577L, 4850L, 5138L, 5443L, 5767L, 6110L, 6473L, 
    6858L, 7266L, 7698L, 8156L, 8641L, 9155L, 9699L, 10276L, 10887L, 11534L, 12220L, 
    12947L, 13717L, 14532L, 15396L, 16312L, 17282L, 18310L, 19398L, 20552L, 21774L, 
    23069L, 24440L, 25894L, 27433L, 29065L, 30793L, 32624L, 34564L, 36619L, 38797L, 
    41104L, 43548L, 46137L, 48881L, 51787L, 54867L, 58129L, 61586L, 65248L, 69128L, 
    73238L, 77593L, 82207L, 87096L, 92275L, 97762L, 103575L, 109734L, 116259L, 123172L, 
    130496L, 138256L, 146477L, 155187L, 164415L, 174191L, 184549L, 195523L, 207150L, 
    219467L, 232518L, 246344L, 260992L, 276512L, 292954L, 310374L, 328830L, 348383L, 
    369099L, 391047L, 414299L, 438935L, 465035L, 492688L, 521984L, 553023L, 585908L, 
    620748L, 657659L, 696766L, 738198L, 782093L, 828599L, 877870L, 930071L, 985375L, 
    1043969L, 1106047L, 1171815L, 1241495L, 1315318L, 1393531L, 1476395L, 1564186L, 
    1657197L, 1755739L, 1860141L, 1970751L, 2087938L, 2212093L, 2343631L, 2482991L, 
    2630637L, 2787063L, 2952790L, 3128372L, 3314395L, 3511479L, 3720282L, 3941502L, 
    4175876L
};


void tune_playnote (byte chan, byte note) {

 
        if (note>MAX_NOTE) note=MAX_NOTE;
        decrement[chan] = pgm_read_dword(decrement_PGM + note);
        accumulator[chan] = ACCUM_RESTART;
        playing[chan]=true;
    
}



void tune_stopnote (byte chan) {
    playing[chan]= false;
    x[chan]=0;

}

void tune_stopscore (void) {
  tune_stopnote(0);
  tune_stopnote(1);
  tune_stopnote(2);
  tune_stopnote(3);
  // depends on NUM_CHANS==4
//  tune_playing = false;
}


ISR(TIMER0_COMPA_vect) { //******* 8-bit timer: 50 microsecond interrupts
    
// We unroll code with a macro to avoid loop overhead.
// For even greater efficiency, we could write this in assembly code
// and do 3-byte instead of 4-byte arithmetic.

  #define dospeaker(spkr) if (playing[spkr]) {   \
      accumulator[spkr] -= decrement[spkr];    \
  if (accumulator[spkr]<0) {           \
       x[spkr] =! x[spkr]; \
      accumulator[spkr] += ACCUM_RESTART;    \
    }                      \
  } 
  
  dospeaker(0);
  dospeaker(1);
  dospeaker(2);
  dospeaker(3);
      OCR0B=(x[0]+x[1]+x[2]+x[3])*12;
 
   // Depends on NUM_CHANS==4
   
  
   
}


void setup () {
  
    
    // Clear watchdog timer-- this can prevent several things from going wrong.
  //Disable sleep mode

  DDRB = (1<<PB1);
  TCCR0A =0;
  TCCR0B =0;
  TCCR0A =  _BV(COM0B1) | _BV(WGM00);
  TCCR0B = _BV(CS01)| _BV(WGM02);
  OCR0A = POLLTIME_USEC;



  TIMSK =(1<<OCIE0A); // turn on match A interrupts for timer0



  mySerial.begin(31250);
  

}
byte aa;
byte midibyte[3];
byte a;
byte b;
byte c;
byte d;


void startchan2(byte notaa){
 
if(playing[1]==0&&b!=notaa){        
tune_playnote(1,notaa);
b=notaa;
}
}
void startchan3(byte notaa){
 
if(playing[2]==0&&c!=notaa){        
tune_playnote(2,notaa);
c=notaa;
}
}
void startchan4(byte notaa){
 
if(playing[3]==0&&d!=notaa){        
tune_playnote(3,notaa);
d=notaa;
}
}
byte prevnote;

void midinote(){
byte opcode = midibyte[0] & 0xf0;
byte midichan = midibyte[0] & 0x0f;
  if(opcode==0xB0&&midibyte[1]==0x40){
  void loop();
}
if(opcode==0xB0&&midibyte[1]==120){
  tune_stopscore();
}
if(midichan!=9){

  if(opcode==0x90&&midibyte[1]!=prevnote){
    prevnote=midibyte[1];
    
    if(playing[0]+playing[1]+playing[2]==3){
    startchan4(midibyte[1]);
    void loop();
    }
    if(playing[0]+playing[1]==2){
    startchan3(midibyte[1]);
    void loop();
    }
    if(playing[0]==1){
  startchan2(midibyte[1]);
  void loop();
}
if(playing[0]==0&&a!=midibyte[1]){        
tune_playnote(0,midibyte[1]);
a=midibyte[1];
void loop();
}
}
  if(opcode==0x80||midibyte[2]<0x01){
    if(prevnote==midibyte[1])prevnote=255;
    if(playing[0]==1&&a==midibyte[1]){ 
    tune_stopnote(0); 
    a=0;
  }
    if(playing[1]==1&&b==midibyte[1]){ 
    tune_stopnote(1); 
    b=0;
  }
      if(playing[2]==1&&c==midibyte[1]){ 
    tune_stopnote(2); 
    c=0;
  }
        if(playing[3]==1&&d==midibyte[1]){ 
    tune_stopnote(3); 
    d=0;
  }
  }   
  }
}
void loop(){

  while(mySerial.available()>0){
     midibyte[aa]=mySerial.read();
     if(midibyte[aa]>0x7F&&aa!=0){
   midibyte[0]=midibyte[aa];
   aa=0;
     }
  aa++;
  if(aa==3){
  aa=0;
  midinote();
 }
  }
}
