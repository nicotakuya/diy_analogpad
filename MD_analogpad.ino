// DIY AnalogPad - Arduino to MD convertor
// by takuya matsubara
// for arduino pro/pro mini(ATmega328 , 5V , 16MHz)

// XE-1AP MegaDrive mode only!

// +-------------------+  MD connector
// |(5) (4) (3) (2) (1)|
//  |                 |
//   |(9) (8) (7) (6)|
//   +---------------+

// (1)DATA0:port D2 
// (2)DATA1:port D3
// (3)DATA2:port D4
// (4)DATA3:port D5
// (5)VCC+5V
// (6)LH   :port D6
// (7)REQ  :port B0
// (8)GND  
// (9)ACK  :port D7

// A0:ch0 stick Y position
// A1:ch1 stick X position  
// A2:ch2 throttle

// REQ
// ---+   +--------------------------------------------------------------------
//    |   |  
//    +---+

// DATA   +0    +1    +2    +3    +4    +5    +6    +7    +8    +9    +10
// D3-D0   
// -----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
//      |E*FG |ABCD |ch1H |ch0H |     |ch2H |ch1L |ch0L |     |ch2L | ?   |   
//      +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//      <16us><34us>
//      <  50usec  >

// ACK
// -----+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +-------  
//      |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |       
//      +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+  +--+       

// LH
// ---+       +-----+     +-----+     +-----+     +-----+     +-----+     +----
//    |       |     |     |     |     |     |     |     |     |     |     |    
//    +-------+     +-----+     +-----+     +-----+     +-----+     +-----+    


// stick/throttleの向き設定
#define REVERSE_CH0 1 //  ch.0 stick Y( 0=そのまま/1=向きを逆転 ) 
#define REVERSE_CH1 0 //  ch.1 stick X( 0=そのまま/1=向きを逆転 )
#define REVERSE_CH2 0 //  ch.2 throttle ( 0=そのまま/1=向きを逆転 )

#define MD_PORT PORTD // data/LH/ACK用
#define MD_DDR DDRD   // data/LH/ACK用 向き
#define MD_DAT_SHIFT 2  // dataビットシフト数
#define MD_DAT_MASK 0x0f // 4bit通信用マスク
#define MD_BITLH (1<<6)  // LH用マスク
#define MD_BITACK (1<<7) // ACK用マスク

#define REQ_PORT PORTB  // REQ用
#define REQ_PIN PINB    // REQ用 入力
#define REQ_DDR DDRB    // REQ用 向き
#define REQ_BIT (1<<0)  // REQ用マスク

#define BTN_DDR  DDRB   // ボタン用　向き
#define BTN_PORT PORTB  // ボタン用　ポート
#define BTN_PIN PINB    // ボタン用　入力
#define BTN_BITA  (1<<1) // A button
#define BTN_BITB  (1<<2) // B button
#define BTN_BITC  (1<<3) // C button
#define BTN_BITD  (1<<4) // D button
#define BTN_BITE1 (1<<5) // E1 button

#define BTN2_DDR  DDRC   // ボタン(グループ2)用
#define BTN2_PORT PORTC  // ボタン(グループ2)用
#define BTN2_PIN PINC    // ボタン(グループ2)用
#define BTN2_BITE2 (1<<3) // E2 button
#define BTN2_BITF  (1<<4) // F(START)
#define BTN2_BITG  (1<<5) // G(SELECT)

//------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  REQ_DDR &= ~REQ_BIT;  // input
  REQ_PORT |= REQ_BIT;  // pullup

  MD_DDR |= (MD_DAT_MASK << MD_DAT_SHIFT) + (MD_BITLH + MD_BITACK);  // output
  MD_PORT |= (MD_DAT_MASK << MD_DAT_SHIFT) + (MD_BITLH + MD_BITACK);  // high

  BTN_DDR &= ~(BTN_BITA + BTN_BITB + BTN_BITC + BTN_BITD + BTN_BITE1);  // input
  BTN_PORT |= (BTN_BITA + BTN_BITB + BTN_BITC + BTN_BITD + BTN_BITE1);  // pullup

  BTN2_DDR &= ~(BTN2_BITE2 + BTN2_BITF + BTN2_BITG);  // input
  BTN2_PORT |= (BTN2_BITE2 + BTN2_BITF + BTN2_BITG);  // pullup

// AD init
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  
  TCCR1A = 0;
  TCCR1B = 1;
// 0 No clock source (Timer/Counter stopped).
// 1 clk /1 (No prescaling)
// 2 clk /8 (From prescaler)
// 3 clk /64 (From prescaler)
// 4 clk /256 (From prescaler)
// 5 clk /1024 (From prescaler)
}
//----- debug
void btntest(void)
{
  unsigned char mask;
  unsigned char btndata;

  while(1){
    btndata = 0;
    if((BTN_PIN & BTN_BITA)==0) btndata |= (1<<7);  // A
    if((BTN_PIN & BTN_BITB)==0) btndata |= (1<<6);  // B
    if((BTN_PIN & BTN_BITC)==0) btndata |= (1<<5);  // C
    if((BTN_PIN & BTN_BITD)==0) btndata |= (1<<4);  // D
    if((BTN_PIN & BTN_BITE1)==0) btndata |= (1<<3); // E1
    if((BTN2_PIN & BTN2_BITE2)==0) btndata |= (1<<2); // E2
    if((BTN2_PIN & BTN2_BITF)==0) btndata |= (1<<1);  // F START
    if((BTN2_PIN & BTN2_BITG)==0) btndata |= (1<<0);  // G SELECT
    mask = 0x80;
    while(mask){
      if(mask & btndata){
        Serial.write('1');
      }else{
        Serial.write('0');
      }
      mask >>= 1;
    }
    Serial.write('\n');
    delay(1000);
  }
}
//----- debug
void adctest(void)
{
  int ch0,ch1,ch2;

  while(1){
    ch0 = analogRead(A1);
    ch1 = analogRead(A0);
    ch2 = analogRead(A2);
    Serial.print(ch0,HEX);
    Serial.write(' ');
    Serial.print(ch1,HEX);
    Serial.write(' ');
    Serial.print(ch2,HEX);
    Serial.write('\n');
    delay(1000);
  }
}

//----- debug
void reqtest(void)
{
  char count;

  count=0;
  while(1){
    while(1){
      if((REQ_PIN & REQ_BIT)==0)break;   // REQ == L検出
    }
    while(1){
      if((REQ_PIN & REQ_BIT)!=0)break;   // REQ == H検出
    }
    count++;
    if(count>=60){
      count=0;
      Serial.write('.');            // debug
    }
  }
}

#define TIMER_1USEC (unsigned int)(16000000/1000000)  //16000000/(1000000/1)
#define TIMER_4USEC (unsigned int)(16000000/250000)   //16000000/(1000000/4)
#define TIMER_12USEC (unsigned int)(16000000/83333)   //16000000/(1000000/12)
#define TIMER_22USEC (unsigned int)(16000000/45454)   //16000000/(1000000/22)

void timerwait(unsigned int limitcnt)
{
  TCNT1 = 0;                    
  while(TCNT1 < limitcnt){      
  }                             
}

//----- debug
void timertest(){
  long i;
  while(1){
    for(i=0;i<1000000;i++){
      TCNT1 = 0;                        // wait
      while(TCNT1 <= TIMER_1USEC){      // wait
      }                                 // wait
    }
    Serial.write('.');            // debug
    Serial.write('\n');           // debug
  }
}

//------    
void loop() {
  unsigned char sendbuf[11];
  char datanum;
  unsigned char ch0,ch1,ch2;
  unsigned char temp;

//  adctest();  // debug
//  timertest();  // debug
//  reqtest();  // debug
//  btntest();  // debug

  while(1){
    temp = 0x0f;
    if((BTN_PIN & BTN_BITE1)==0)   temp &= ~(1<<3);   // E1 button
    if((BTN2_PIN & BTN2_BITE2)==0) temp &= ~(1<<2);   // E2 button
    if((BTN2_PIN & BTN2_BITF)==0)  temp &= ~(1<<1);   // F(START) button
    if((BTN2_PIN & BTN2_BITG)==0)  temp &= ~(1<<0);   // G(SELECT) button
    sendbuf[0] = temp;

    temp = 0x0f;
    if((BTN_PIN & BTN_BITA)==0) temp &= ~(1<<3);  // A button
    if((BTN_PIN & BTN_BITB)==0) temp &= ~(1<<2);  // B button
    if((BTN_PIN & BTN_BITC)==0) temp &= ~(1<<1);  // C button
    if((BTN_PIN & BTN_BITD)==0) temp &= ~(1<<0);  // D button
    sendbuf[1] = temp;

#if REVERSE_CH0
    ch0 = 255-(unsigned char)(analogRead(A1) >> 2);     //反転
#else
    ch0 = (unsigned char)(analogRead(A1) >> 2);  //そのまま
#endif

#if REVERSE_CH1
    ch1 = 255-(unsigned char)(analogRead(A0) >> 2);     //反転
#else
    ch1 = (unsigned char)(analogRead(A0) >> 2);  //そのまま
#endif
   
#if REVERSE_CH2
    ch2 = 255-(unsigned char)(analogRead(A2) >> 2);     //反転
#else
    ch2 = (unsigned char)(analogRead(A2) >> 2);  //そのまま
#endif

    sendbuf[2] = ch1 >> 4;  // CH1 H
    sendbuf[3] = ch0 >> 4;  // CH0 H
    sendbuf[4] = 0;
    sendbuf[5] = ch2 >> 4;  // CH2 H
    sendbuf[6] = ch1 & 0x0f;  // CH1 L
    sendbuf[7] = ch0 & 0x0f;  // CH0 L
    sendbuf[8] = 0;
    sendbuf[9] = ch2 & 0x0f;  // CH2 L
    sendbuf[10] = 0xf; //未調査

    while(1){
      if((REQ_PIN & REQ_BIT)!=0)break;   // REQ == H検出
    }
    while(1){
      if((REQ_PIN & REQ_BIT)==0)break;   // REQ == L検出
    }

    timerwait(TIMER_4USEC); //適切なウエイト時間が不明

    for(datanum=0 ;datanum<11; datanum++){
      if((datanum & 1)==0){
        MD_PORT &= ~MD_BITLH;   // LH = L
      }else{
        MD_PORT |= MD_BITLH;    // LH = H
      }

      temp = MD_PORT;
      temp &= ~(MD_DAT_MASK << MD_DAT_SHIFT);
      temp |= sendbuf[datanum] << MD_DAT_SHIFT;   // D0,D1,D2,D3
      MD_PORT = temp;

      MD_PORT &=  ~MD_BITACK; // ACK = L
      timerwait(TIMER_12USEC);
      MD_PORT |=  MD_BITACK;  // ACK = H
      // ループ回数の奇数／偶数で周期が違う
      if((datanum & 1)==0){
        timerwait(TIMER_4USEC);
      }else{
        timerwait(TIMER_22USEC);
      }
    }
    MD_PORT |= (MD_BITLH + (MD_DAT_MASK << MD_DAT_SHIFT));    // LH = H , D3-D0=H
  }
}
