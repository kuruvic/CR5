//=========================HEADER=============================================================
/*
   Dual LS7366 Quadrature Counter Test Code
   AUTHOR: Jason Traud
   DATE: June 22, 2013
   
   This is a simple test program to read encoder counts
   collected by the LS7366 breakout board. The counts are
   then displayed in the Arduino's serial monitor at a 
   baud rate of 9600
   
   Hardware: Arduino Uno R3
   Powered 
   
   LS7366 Breakout    -------------   Arduino
   -----------------                    -------
            MOSI   -------------------   SDO (D11)
            MISO   -------------------   SDI (D12)
            SCK    -------------------   SCK (D13)
            SS1    -------------------   SS1 (D7)
            SS2    -------------------   SS2 (D8)
            GND    -------------------   GND
            VDD    -------------------   VCC (5.0V)
			
   License: CCAv3.0 Attribution-ShareAlike (http://creativecommons.org/licenses/by-sa/3.0/)
   You're free to use this code for any venture. Attribution is greatly appreciated. 

//============================================================================================
*/

// Inclde the standard Arduino SPI Library, please ensure the SPI pins are
// connected properly for your Arduino version
#include <ArduinoJson.h>
#include <SPI.h>

String inputString = "";         // a string to hold incoming data
String receivedData = "";
String _MODE = "";
String TECT = "";
boolean stringComplete = false;  // whether the string is complete

typedef struct STR_STEP 
{
  //{"cmd":"front","countL":15,"countR":15}
  int countL;
  int countR;
};

typedef struct STR_STEP STEP_MEM;
STEP_MEM uStep;

double currentTime = 0;
double lastTime = 0;
double getLastTime()
{
      return currentTime = millis()/1000.0 - lastTime;
}

// Slave Select pins for encoders 1 and 2
// Feel free to reallocate these pins to best suit your circuit
const int slaveSelectEnc1 = 38;
const int slaveSelectEnc2 = 40;

// These hold the current encoder count.
signed long encoder1count = 0;
signed long encoder2count = 0;
int number = 0;

int pins[] = {3, 4, 12, 13, 14, 28, 29, 30, 31, 32 ,33 ,34, 35, 41, 42, 43, 44};
int adcpin[] = {A0, A1, A2, A3, A4, A5, A6, A7};
int adcdata[] = {0, 1, 2, 3, 4, 5, 6, 7}; 
int j = 0;
int y = 0;
int k = 0;
char index;
char index1;
char Position;
int lootin;
int lootin10;
int num;
int num1 = 0;
int num2 = 0;
int ennumL2 = 0;
int ennumR2 = 0;
int start = 0;

void initEncoders() {
  
  // Set slave selects as outputs
  pinMode(slaveSelectEnc1, OUTPUT);
  pinMode(slaveSelectEnc2, OUTPUT);
  
  // Raise select pins
  // Communication begins when you drop the individual select signsl
  digitalWrite(slaveSelectEnc1,HIGH);
  digitalWrite(slaveSelectEnc2,HIGH);
  
  SPI.begin();
  
  // Initialize encoder 1
  //    Clock division factor: 0
  //    Negative index input
  //    free-running count mode
  //    x4 quatrature count mode (four counts per quadrature cycle)
  // NOTE: For more information on commands, see datasheet
  digitalWrite(slaveSelectEnc1,LOW);        // Begin SPI conversation
  SPI.transfer(0x88);                       // Write to MDR0
  SPI.transfer(0x03);                       // Configure to 4 byte mode
  digitalWrite(slaveSelectEnc1,HIGH);       // Terminate SPI conversation 

  // Initialize encoder 2
  //    Clock division factor: 0
  //    Negative index input
  //    free-running count mode
  //    x4 quatrature count mode (four counts per quadrature cycle)
  // NOTE: For more information on commands, see datasheet
  digitalWrite(slaveSelectEnc2,LOW);        // Begin SPI conversation
  SPI.transfer(0x88);                       // Write to MDR0
  SPI.transfer(0x03);                       // Configure to 4 byte mode
  digitalWrite(slaveSelectEnc2,HIGH);       // Terminate SPI conversation 
}

long readEncoder(int encoder) {
  
  // Initialize temporary variables for SPI read
  unsigned int count_1, count_2, count_3, count_4;
  long count_value;  
  
  // Read encoder 1
  if (encoder == 1) {
    digitalWrite(slaveSelectEnc1,LOW);      // Begin SPI conversation
    SPI.transfer(0x60);                     // Request count
    count_1 = SPI.transfer(0x00);           // Read highest order byte
    count_2 = SPI.transfer(0x00);           
    count_3 = SPI.transfer(0x00);           
    count_4 = SPI.transfer(0x00);           // Read lowest order byte
    digitalWrite(slaveSelectEnc1,HIGH);     // Terminate SPI conversation 
  }
  
  // Read encoder 2
  else if (encoder == 2) {
    digitalWrite(slaveSelectEnc2,LOW);      // Begin SPI conversation
    SPI.transfer(0x60);                      // Request count
    count_1 = SPI.transfer(0x00);           // Read highest order byte
    count_2 = SPI.transfer(0x00);           
    count_3 = SPI.transfer(0x00);           
    count_4 = SPI.transfer(0x00);           // Read lowest order byte
    digitalWrite(slaveSelectEnc2,HIGH);     // Terminate SPI conversation 
  }
  
  // Calculate encoder count
  count_value = (count_1 << 8) + count_2;
  count_value = (count_value << 8) + count_3;
  count_value = (count_value << 8) + count_4;
  
  return count_value;
}

void clearEncoderCount() {
    
  // Set encoder1's data register to 0
  digitalWrite(slaveSelectEnc1,LOW);      // Begin SPI conversation  
  // Write to DTR
  SPI.transfer(0x98);    
  // Load data
  SPI.transfer(0x00);  // Highest order byte
  SPI.transfer(0x00);           
  SPI.transfer(0x00);           
  SPI.transfer(0x00);  // lowest order byte
  digitalWrite(slaveSelectEnc1,HIGH);     // Terminate SPI conversation 
  
  delayMicroseconds(100);  // provides some breathing room between SPI conversations
  
  // Set encoder1's current data register to center
  digitalWrite(slaveSelectEnc1,LOW);      // Begin SPI conversation  
  SPI.transfer(0xE0);    
  digitalWrite(slaveSelectEnc1,HIGH);     // Terminate SPI conversation   
  
  // Set encoder2's data register to 0
  digitalWrite(slaveSelectEnc2,LOW);      // Begin SPI conversation  
  // Write to DTR
  SPI.transfer(0x98);    
  // Load data
  SPI.transfer(0x00);  // Highest order byte
  SPI.transfer(0x00);           
  SPI.transfer(0x00);           
  SPI.transfer(0x00);  // lowest order byte
  digitalWrite(slaveSelectEnc2,HIGH);     // Terminate SPI conversation 
  
  delayMicroseconds(100);  // provides some breathing room between SPI conversations
  
  // Set encoder2's current data register to center
  digitalWrite(slaveSelectEnc2,LOW);      // Begin SPI conversation  
  SPI.transfer(0xE0);    
  digitalWrite(slaveSelectEnc2,HIGH);     // Terminate SPI conversation 
}


void setup() {
 Serial.begin(115200);      // Serial com for data output
  
  for(int i = 0 ; i < 16; i++)
  { 
    pinMode(pins[i], OUTPUT);
  }
 
 initEncoders();       Serial.println("Encoders Initialized...");  
 clearEncoderCount();  Serial.println("Encoders Cleared...");
   encoder1count = readEncoder(1); 
   encoder2count = readEncoder(2);
   Serial.println(encoder1count);
   Serial.println(encoder2count);

   //delay(500);

   lastTime = millis()/1000.0;
}

void loop() 
{
    //delay(500);
    //number = number + uStep.count;
    encoder1count = readEncoder(1); 
    encoder2count = readEncoder(2);

     if(_MODE  == "STOP")
     {
          clearEncoderCount();
          st_bk(); 
          digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);
          delay(500);
          _MODE = "NORMAL";
     }

     if(j > 50)
     {
          k++;
          j = 0;
     }

     if(_MODE  == "CREANING")
     {
         TECT = "go";
         _MODE = "START";
         j = 0;
     }

     if(_MODE == "START")
     {
         for(int a = 0; a < 8; a++)
         {
              adcdata[a] = analogRead(adcpin[a]);
         }
      
         if(y>0)
         {
              _MODE = "BYUK";
         }

         else turn();
     }

     if(_MODE == "BYUK")
     {
             for(int a = 0; a < 8; a++)
             {
                  adcdata[a] = analogRead(adcpin[a]);
             }
             
              sr_mot(33 ,32, 70);
              sl_mot(35 ,34, 70);
              vm_mot(13);
              //mb_mot(44 , 43, 120);

              byuk();
              
              if(uStep.countL < k)
              {
                  _MODE = "RANDOM";
              }
     }

     if(_MODE == "RANDOM")
     {
         for(int a = 0; a < 8; a++)
         {
              adcdata[a] = analogRead(adcpin[a]);
         }
         
          k = 0; 
          j = 0;
          y = 0;
          //_MODE  = "STOP";
          Random();
     }
     /*engo(5, 5, "right");
     //enback(-5, -5, "0");
     //enleft(-6, 6, "0");
     enright(6, -6, "go2");
     engo2(5, 5, "right2");
     enright2(8, -8, "0");*/

     if(_MODE == "FRONT")
     {
         sr_mot(33 ,32, 70);
         sl_mot(35 ,34, 70);
         vm_mot(13);
         //mb_mot(44 , 43, 120);
         
         //engo(uStep.countL,uStep.countR);
         encoder('g',uStep.countL,uStep.countR);
     }

     if(_MODE == "BACK")
     {
         sr_mot(33 ,32, 70);
         sl_mot(35 ,34, 70);
         vm_mot(13);
         //mb_mot(44 , 43, 120);
         
         //enback(uStep.countL,uStep.countR);
         encoder('b',uStep.countL,uStep.countR);
     }

     if(_MODE == "LEFT")
     {
        //delay(200);
        //clearEncoderCount();  Serial.println("Encoders Cleared...");
        sr_mot(33 ,32, 70);
        sl_mot(35 ,34, 70);
        vm_mot(13);
        //mb_mot(44 , 43, 120);
        //vm_mot(13); 
  
        encoder('l',uStep.countL,uStep.countR);
        //_MODE = "NORMAL";
     }
  
     if(_MODE  == "RIGHT")
     {
        //delay(200);
        //clearEncoderCount();  Serial.println("Encoders Cleared...");
        sr_mot(33 ,32, 70);
        sl_mot(35 ,34, 70);
        vm_mot(13);
        //mb_mot(44 , 43, 120);
  
        encoder('r',uStep.countL,uStep.countR);
       // _MODE = "NORMAL";
     }

  if(stringComplete == true)
  {
     StaticJsonBuffer<200> jsonBuffer;
     JsonObject& root = jsonBuffer.parseObject(receivedData);
     
     String _cmd = root["cmd"];
     uStep.countL = root["countL"];
     uStep.countR = root["countR"];
     
     clearEncoderCount();
     
    if (root.success()) 
    {
        stringComplete = false;    

        if(_cmd == "cleaning")
         {            
             _MODE = "CREANING";
             Serial.println("CREANING");    
         }
         else if(_cmd == "front")
         {
              _MODE = "FRONT";
              Serial.println("FRONT");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(_cmd == "back")
         {
              _MODE = "BACK";
              Serial.println("BACK");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(_cmd == "left")
         {
              _MODE = "LEFT";
              Serial.println("LEFT");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(_cmd == "right")
         {
              _MODE = "RIGHT";
              Serial.println("RIGHT");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(_cmd == "stop")
         {
              _MODE = "STOP";
              Serial.println("STOP");
         }
     }
  }
}

void turn()
{
  if((adcdata[0] > 750) || (adcdata[1] > 750) || (adcdata[2] < 150) || (adcdata[3] < 150) || (adcdata[4] < 150)) //|| (adcdata[5] > 200) || (adcdata[6] > 200)) 
  {
    y++;
    TECT  = "back";
    clearEncoderCount(); 
    Serial.println(y);    
  }
  
  else
  {  
      engo(10, 10, "right");
      //enback(-5, -5, "0");
      //enleft(-5, 5, "0");
      enright(5, -5, "go2");
      engo2(5, 5, "right2");
      enright2(5, -5, "go3");
      //j = 0;
      
      if(TECT == "go3")
      {
          go();
      }
      /*if(start == 0)
      {
          go();
          delay(500);
          right();
          delay(450);
          go();
          delay(500);
           right();
          delay(450);
          start = 1;
      }

      else if(start == 1)
      {
          go();
      }*/
  }
}

void byuk()
{
  int f;
  if((adcdata[0] > 750) || (adcdata[1] > 750) || (adcdata[2] < 150) || (adcdata[3] < 150) || (adcdata[4] < 150)) //|| (adcdata[5] > 200) || (adcdata[6] > 200)) 
  {
    //Serial.println(j); 
    f = 1;
    clearEncoderCount();
    TECT = "back";
    
    if(f == 1)
    {
        j = j+1;
        f = 0;
    }
    //delay(100);
    /*back();
    delay(300);
    st_bk();
    delay(200);
    left();
    delay(300);
    st_bk();
    delay(200);*/  
  }
      
  else
  {
    engo(10, 10, "right");
    enright(5, -5, "go2");
    engo2(10, 10, "right2");
    enright2(5, -5, "go3");
     if(TECT == "go3")
     {
          go();
     }
  } 

   enback(-3, -3, "left");
   enleft(-5, 5, "go");
}

void Random()
{
  if((adcdata[0] > 1000)||(adcdata[2]<150)&&(adcdata[3]>150) || (adcdata[6] > 250)) 
  {
    back();
    delay(400);
    st_bk();
    delay(200);
    left();
    delay(400);
    st_bk();
    delay(200);
  }
  
  else if((adcdata[1] > 1000)||(adcdata[3]<150)&&(adcdata[2]>150) || (adcdata[5] > 250)) 
  {
    back();
    delay(400);
    st_bk();
    delay(200);
    right();
    delay(400);
    st_bk();
    delay(200);
  }
  
  else if((adcdata[0] > 1000) && (adcdata[1] > 1000)||(adcdata[3]<150)&&(adcdata[2]<150)||(adcdata[4] < 150) || (adcdata[5] > 250) || (adcdata[6] > 250)) 
  {
    back();
    delay(400);
    st_bk();
    delay(200);
    right();
    delay(400);
    st_bk();
    delay(200);
  }

  else
  {
    go();
  } 
}

void mr_mot(int mrl , int mrr ,int mpr)
{
  analogWrite(3, mpr);
  digitalWrite(mrl, LOW);
  digitalWrite(mrr, HIGH);
}

void ml_mot(int mll , int mlr ,int mpl)
{
  analogWrite(4, mpl);
  digitalWrite(mll, LOW);
  digitalWrite(mlr, HIGH);
}

void sr_mot(int srl , int srr, int spr)
{
  analogWrite(12, spr);
  digitalWrite(srl, LOW);
  digitalWrite(srr, HIGH);
}

void sl_mot(int sll , int slr, int spl)
{
  analogWrite(14, spl);
  digitalWrite(sll, LOW);
  digitalWrite(slr, HIGH);
}

void mb_mot(int mbl , int mbr, int spl)
{
  analogWrite(13, spl);
  digitalWrite(mbl, LOW);
  digitalWrite(mbr, HIGH);
}

void vm_mot(int vml)
{
  //analogWrite(15, spl);
  //digitalWrite(vml, LOW);
  digitalWrite(vml, HIGH);
}

void go()
{
    mr_mot(29 ,28 ,120);
    ml_mot(30 ,31 ,120);
}

void back()
{
    mr_mot(28 ,29 ,100);
    ml_mot(31 ,30 ,120);
}

void left()
{
    mr_mot(28 ,29 ,120);
    ml_mot(30 ,31 ,120);
}

void right()
{
    mr_mot(29 ,28 ,120);
    ml_mot(31 ,30 ,120);
}

void st_bk()
{
    digitalWrite(28, HIGH);
    digitalWrite(29, HIGH);
    digitalWrite(30, HIGH);
    digitalWrite(31, HIGH);
    analogWrite(3, 120);
    analogWrite(4, 120);
    //_MODE = "0";
}

void bk_L()
{
    digitalWrite(28, HIGH);
    digitalWrite(29, HIGH);
    analogWrite(3, 120);
}

void bk_R()
{
    digitalWrite(30, HIGH);
    digitalWrite(31, HIGH);
    analogWrite(4, 120);
}

void encoder(char det,int ennumL, int ennumR)
{
     if(det == 'g')
     {
           if((encoder1count >= ennumL) || (encoder2count >= ennumR))
           {
              
              //back();
              //delay(80);
              //bk_R(); bk_L();
              digitalWrite(28, HIGH);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);
              analogWrite(3, 120);
              analogWrite(4, 120);
    
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              delay(200);
              //Serial.println(ennumL2);
              //Serial.println(encoder1count);
              //Serial.println(encoder2count);
           }
          
           else
           {
              ml_mot(30 ,31 ,120);
              mr_mot(29 ,28 ,120);
           }
     }

     else if(det == 'b')
     {
           if((encoder1count <= ennumL)||(encoder2count <= ennumR))
           {
              //bk_L();bk_R(); 
              digitalWrite(28, HIGH);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);
              analogWrite(3, 120);
              analogWrite(4, 120);
              
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              delay(200);
              
              //Serial.println(encoder1count);
              //Serial.println(encoder2count);
               //_MODE = "STOP";
           }
          
           else
           {
              ml_mot(31 ,30 ,120);
              mr_mot(28 ,29 ,100);
           }
     }

     else if(det == 'l')
     {
             if((encoder1count <= ennumL)||(encoder2count >= ennumR))
             {
                
               // go();
                //delay(80);
                digitalWrite(28, HIGH);
                digitalWrite(29, HIGH);
                digitalWrite(30, HIGH);
                digitalWrite(31, HIGH);
                analogWrite(3, 120);
                analogWrite(4, 120);
                
                digitalWrite(33, LOW);
                digitalWrite(32, LOW); 
                digitalWrite(35, LOW);
                digitalWrite(34, LOW); 
                     
                digitalWrite(13, LOW);
                digitalWrite(12, LOW);
                digitalWrite(14, LOW);
                digitalWrite(15, LOW);
                delay(200);
                
                //Serial.println(encoder1count);
                //Serial.println(encoder2count);
                 //_MODE = "STOP";
             }
            
             else
             {
                ml_mot(30 ,31 ,120);
                mr_mot(28 ,29 ,120);
             }
     }

     else if(det == 'r')
     {
             if((encoder1count >= ennumL)||(encoder2count <= ennumR))
             {
                digitalWrite(28, HIGH);
                digitalWrite(29, HIGH);
                digitalWrite(30, HIGH);
                digitalWrite(31, HIGH);
                analogWrite(3, 120);
                analogWrite(4, 120);
                
                digitalWrite(33, LOW);
                digitalWrite(32, LOW); 
                digitalWrite(35, LOW);
                digitalWrite(34, LOW); 
                     
                digitalWrite(13, LOW);
                digitalWrite(12, LOW);
                digitalWrite(14, LOW);
                digitalWrite(15, LOW);
                delay(200);
                
                //Serial.println(encoder1count);
                //Serial.println(encoder2count);
                 //_MODE = "STOP";
             }
            
             else
             {
                ml_mot(31 ,30 ,120);
                mr_mot(29 ,28 ,120);
             }
     }
}

void engo(int ennumL, int ennumR, String next)
{
    if(TECT == "go")
    {  
       if((encoder1count >= ennumL) && (encoder2count >= ennumR))
       {
          
          //back();
          //delay(80);
          //bk_R(); bk_L();
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          /*digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);*/
          Serial.println(encoder1count);
          Serial.println(encoder2count);
          clearEncoderCount(); 
          delay(500);
          TECT = next;
          //Serial.println(ennumL2);
       }
      
       else
       {
          ml_mot(30 ,31 ,120);
          mr_mot(29 ,28 ,120);
       }
    }    
}

void enback(int ennumL, int ennumR, String next)
{      
    if(TECT == "back")
    {
       if((encoder1count <= ennumL)&&(encoder2count <= ennumR))
       {
          //bk_L();bk_R(); 
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          /*digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);*/
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT  = next;
       }
      
       else
       {
          ml_mot(31 ,30 ,120);
          mr_mot(28 ,29 ,100);
       }
   }
}

void enleft(int ennumL, int ennumR, String next)
{
    if(TECT == "left")
    {
       if((encoder1count <= ennumL)&&(encoder2count >= ennumR))
       {
          
         // go();
          //delay(80);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          /*digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);*/
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT  = next;
       }
      
       else
       {
          ml_mot(30 ,31 ,120);
          mr_mot(28 ,29 ,120);
       }
   }
}

void enright(int ennumL, int ennumR, String next)
{
    if(TECT == "right")
    {
       if((encoder1count >= ennumL)&&(encoder2count <= ennumR))
       {
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          /*digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);*/
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT = next;
       }
      
       else
       {
          ml_mot(31 ,30 ,120);
          mr_mot(29 ,28 ,120);
       }
   }
}

void engo2(int ennumL, int ennumR, String next)
{
    if(TECT == "go2")
    {  
       if((encoder1count >= ennumL) && (encoder2count >= ennumR))
       {
          
          //back();
          //delay(80);
          //bk_R(); bk_L();
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);
          Serial.println(encoder1count);
          Serial.println(encoder2count);
          clearEncoderCount(); 
          delay(500);
          TECT = next;
          //Serial.println(ennumL2);
       }
      
       else
       {
          ml_mot(30 ,31 ,120);
          mr_mot(29 ,28 ,120);
       }
    }    
}

void enback2(int ennumL, int ennumR, String next)
{      
    if(TECT == "back2")
    {
       if((encoder1count <= ennumL)&&(encoder2count <= ennumR))
       {
          //bk_L();bk_R(); 
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT  = next;
       }
      
       else
       {
          ml_mot(31 ,30 ,120);
          mr_mot(28 ,29 ,100);
       }
   }
}

void enleft2(int ennumL, int ennumR, String next)
{
    if(TECT == "left2")
    {
       if((encoder1count <= ennumL)&&(encoder2count >= ennumR))
       {
          
         // go();
          //delay(80);
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT  = next;
       }
      
       else
       {
          ml_mot(30 ,31 ,120);
          mr_mot(28 ,29 ,120);
       }
   }
}

void enright2(int ennumL, int ennumR, String next)
{
    if(TECT == "right2")
    {
       if((encoder1count >= ennumL)&&(encoder2count <= ennumR))
       {
          digitalWrite(28, HIGH);
          digitalWrite(29, HIGH);
          digitalWrite(30, HIGH);
          digitalWrite(31, HIGH);
          analogWrite(3, 120);
          analogWrite(4, 120);
          
          digitalWrite(33, LOW);
          digitalWrite(32, LOW); 
          digitalWrite(35, LOW);
          digitalWrite(34, LOW); 
               
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          digitalWrite(14, LOW);
          digitalWrite(15, LOW);
          Serial.println(encoder1count);
          Serial.println(encoder2count);
           clearEncoderCount(); 
          delay(500);
          TECT = next;
       }
      
       else
       {
          ml_mot(31 ,30 ,120);
          mr_mot(29 ,28 ,120);
       }
   }
}

void serialEvent() 
{
  while (Serial.available()) 
  {
    
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;

    if (inChar == '}') 
    {
      receivedData = inputString;
      inputString = "";
      stringComplete = true;
    }
  }
}
