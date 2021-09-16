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
            SS1    --                      ㅌ -----------------   SS1 (D7)
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
String  STATUSINT = "";
String _MODE = "";
String _MODE2 = "";
String _MODE3 = "";
boolean stringComplete = false;  // whether the string is complete

typedef struct STR_STEP 
{
  //{"cmd":"front","countL":15,"countR":15}
  String cmd;
  int countL;
  int countR;
  int countBL;
  int countBR;
  int AngleZ;
  int lidar;
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

int digidata_1;
int digidata_2;
int Zangle;

int spdl;
float slp = 111;
int spdr;
float srp= 120;

int spbl;
float slb = 113;
int spbr;
float srb = 120;

boolean clean = false;
boolean RGR = false;

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
  Serial1.begin(115200);
  
  for(int i = 0 ; i < 16; i++)
  { 
    pinMode(pins[i], OUTPUT);
  }

  pinMode(18,OUTPUT);
  digitalWrite(18,LOW);

  pinMode(24,INPUT);
  pinMode(25,INPUT);
  
  spdl = slp;
  spbl = slb;
  spdr = srp;
  spbr = srb;
 
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
     encoder1count = readEncoder(1); 
     encoder2count = readEncoder(2);
    
     for(int a = 0; a < 8; a++)
     {
         adcdata[a] = analogRead(adcpin[a]);
     }

     digidata_1 = digitalRead(24);
     digidata_2 = digitalRead(25);
  
     if(Serial1.available()) 
     {
        Zangle = Serial1.parseInt();
     }

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

     if(_MODE == "FRONT")
     {
         sr_mot(33 ,32, 150);
         sl_mot(35 ,34, 150);
         vm_mot(13);

         if(3 < encoder2count - (encoder1count+4))//&&(Zangle <= -5))
         {
             spdl = slp + 0.2;
             spdr = srp - 0.2;
         }

         else if(3 < (encoder1count+4) - encoder2count)//&&(Zangle >= 5))
         {
             spdl = slp - 0.2;
             spdr = srp + 0.2;
         }

         else
         {
             spdl = spdl;
             spdr = spdr;
         }
         
         encoder("gl","gr",uStep.countL,uStep.countR);
         
         if(((encoder1count+4) >= uStep.countL)&&(encoder2count >= uStep.countR)) 
         _MODE = "STOP";
     }

     if(_MODE == "BACK")
     {
         sr_mot(33 ,32, 150);
         sl_mot(35 ,34, 150);
         vm_mot(13);

         if((-3 > encoder2count - encoder1count))//&&(Zangle >= 5))
         {
              spbl = slb + 0.2;
              spbr = srb - 0.2;
         }

         else if((-3 > encoder1count - encoder2count))//&&(Zangle <= -5))
         {
              spbl = slb - 0.2;
              spbr = srb + 0.2;
         }

         else
         {
              spbl = spbl;
              spbr = spbr;
         }
         
         encoder("bl","br",uStep.countBL,uStep.countBR);
         
         if((encoder1count <= uStep.countBL)||(encoder2count <= uStep.countBR))
         _MODE = "STOP";
     }

     if(_MODE == "LEFT")
     {
        sr_mot(33 ,32, 150);
        sl_mot(35 ,34, 150);
        vm_mot(13);
  
        encoder("bl","gr",uStep.countBL,uStep.countR);
        
        if(encoder2count >= uStep.countR)
        _MODE = "STOP";
     }
  
     if(_MODE  == "RIGHT")
     {
        sr_mot(33 ,32, 150);
        sl_mot(35 ,34, 150);
        vm_mot(13);
  
        encoder("gl","br",uStep.countL,uStep.countBR);
       
        if(encoder1count >= uStep.countL)
        _MODE = "STOP";
     }

     if(_MODE  == "START")
     {
        if(clean == false)
        {
            go(50, 50,"NT");
            //stopm();
            delay(100);
            if(_MODE2 == "NT")
            {
               clean = true;
               _MODE3 = "RT";
               _MODE2 = "";
            }
        }

        else if(clean == true)
        {
            sr_mot(33 ,32, 150);
            sl_mot(35 ,34, 150);
            vm_mot(13);
            
            if(_MODE3 == "RT")
            {
                if((adcdata[3]>=200)||(adcdata[6]>=200)|(digidata_1>=1)||(digidata_2>=1))
                {
                    _MODE3 = "RGR";
                    _MODE2 = "BACK";
                    clearEncoderCount();
                }
    
                else
                {
                    go2();
                }
            }

            if(_MODE3 == "RGR")
            {
                if(_MODE2 == "BACK")
                {
                    back(-7, -7, "STOP");
                }
                    
                if(_MODE2 == "STOP")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "RIGHT";
                }
                    
                if(_MODE2 == "RIGHT")
                {
                    right(21, -21, 90,"STOP1");
                }

                if(_MODE2 == "STOP1")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "GO";
                }

                if(_MODE2 == "GO")
                {
                    go(30, 30,"STOP2");
                }
                    
                if(_MODE2 == "STOP2")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "RIGHT2";
                }

                if(_MODE2 == "RIGHT2")
                {
                    right(21, -21, 90,"STOP3");
                }

                if(_MODE2 == "STOP3")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE3 = "LT";
                }
            }

            if(_MODE3 == "LT")
            {
                if((adcdata[3]>=200)||(adcdata[6]>=200)|(digidata_1>=1)||(digidata_2>=1))
                {
                    _MODE3 = "LGL";
                    _MODE2 = "BACK";
                    clearEncoderCount();
                }
    
                else
                {
                    go2();
                }
            }

            if(_MODE3 == "LGL")
            {
                if(_MODE2 == "BACK")
                {
                    back(-7, -7, "STOP");
                }
                    
                if(_MODE2 == "STOP")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "LEFT";
                }
                    
                if(_MODE2 == "LEFT")
                {
                    left(-21, 21, 90,"STOP1");
                }

                if(_MODE2 == "STOP1")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "GO";
                }

                if(_MODE2 == "GO")
                {
                    go(30, 30,"STOP2");
                }
                    
                if(_MODE2 == "STOP2")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE2 = "LEFT2";
                }

                if(_MODE2 == "LEFT2")
                {
                    right(-21, 21, 90,"STOP3");
                }

                if(_MODE2 == "STOP3")
                {
                    digitalWrite(28, LOW);
                    digitalWrite(29, LOW);
                    digitalWrite(30, LOW);
                    digitalWrite(31, LOW);
                    analogWrite(3, 0);
                    analogWrite(4, 0);
                        
                    clearEncoderCount();
                        
                    delay(100);
                    if((encoder1count == 0)||(encoder2count == 0))
                    _MODE3 = "RT";
                }
            }
        }
     }

     if(stringComplete == true)
     {
        clearEncoderCount();
        
        if(uStep.cmd == "front")
         {
              _MODE = "FRONT";
              Serial.println("FRONT");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(uStep.cmd == "back")
         {
              _MODE = "BACK";
              Serial.println("BACK");
              Serial.print("countL"); Serial.println(uStep.countBL);
              Serial.print("countR"); Serial.println(uStep.countBR);
         }
         else if(uStep.cmd == "left")
         {
              _MODE = "LEFT";
              Serial.println("LEFT");
              Serial.print("countL"); Serial.println(uStep.countBL);
              Serial.print("countR"); Serial.println(uStep.countR);
         }
         else if(uStep.cmd == "right")
         {
              _MODE = "RIGHT";
              Serial.println("RIGHT");
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countBR);
         }
         else if(uStep.cmd == "start")
         {
              _MODE = "START";
              Serial.println("START");
         }
         else if(uStep.cmd == "stop")
         {
              _MODE = "STOP";
              Serial.println("STOP");
         }

         else if(uStep.lidar == 1)
         {
              digitalWrite(18,HIGH);
              Serial.println("lidar on");
         }

         stringComplete = false;
     }
}

void isCommandJSON(String _comm)
{
      String jsonString = _comm;
      //bool ret = false;
  
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(jsonString);
  
      if (root.success())
      {
         String _cmd = root["cmd"];
         int _AngleZ = root["AngleZ"];
         int _lidar = root["lidar"];
  
         uStep.cmd = _cmd;
         //uStep.AngleZ = _AngleZ;
         uStep.lidar = _lidar;
         //Serial.print("AngleZ"); Serial.println(uStep.AngleZ);

         if(uStep.cmd == "num")
         {
              int _countL = root["countL"];
              int _countR = root["countR"];
              uStep.countL = _countL;
              uStep.countR = _countR;
              uStep.countBL = _countL * -1;
              uStep.countBR = _countR * -1;
              Serial.print("countL"); Serial.println(uStep.countL);
              Serial.print("countR"); Serial.println(uStep.countR);
              Serial.print("countBL"); Serial.println(uStep.countBL);
              Serial.print("countBR"); Serial.println(uStep.countBR);
         }
         clearEncoderCount();
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

void go(int len, int ren, String next)
{
    if(2 < encoder2count - (encoder1count+4))//&&(Zangle <= -5))
    {
        spdl = slp + 0.2;
        spdr = srp - 0.2;
    }

    else if(2 < (encoder1count+4) - encoder2count)//&&(Zangle >= 5))
    {
        spdl = slp - 0.2;
        spdr = srp + 0.2;
    }

    else
    {
        spdl = spdl;
        spdr = spdr;
    }
         
    encoder("gl","gr",len,ren);
         
    if(((encoder1count+4) >= len)||(encoder2count >= ren)) 
    _MODE2 = next;//st_bk();
}

void go2()//int len, int ren, String next)
{   
    if(2 < encoder2count - (encoder1count+4))//&&(Zangle <= -5))
    {
        spdl = slp + 0.2;
        spdr = srp - 0.2;
    }

    else if(2 < (encoder1count+4) - encoder2count)//&&(Zangle >= 5))
    {
        spdl = slp - 0.2;
        spdr = srp + 0.2;
    }

    else
    {
        spdl = spdl;
        spdr = spdr;
    }
         
    //encoder("gl","gr",len,ren);
    mr_mot(29 ,28 ,spdl);
    ml_mot(30 ,31 ,spdr);
         
    //if(((encoder1count+4) >= len)&&(encoder2count >= ren)) 
    //_MODE2 = next;
}

/*void stopm(String next)
{ 
    if(_MODE2 == "STOP")
    {
        clearEncoderCount();st_bk();
        delay(500);
        _MODE2 = next;
    }
}*/

void back(int len, int ren, String next)
{
    if((-3 > encoder2count - encoder1count))//&&(Zangle >= 5))
    {
         spbl = slb + 0.2;
         spbr = srb - 0.2;
    }

    else if((-3 > encoder1count - encoder2count))//&&(Zangle <= -5))
    {
         spbl = slb - 0.2;
         spbr = srb + 0.2;
    }

    else
    {
         spbl = spbl;
         spbr = spbr;
    }
         
    encoder("bl","br",len,ren);
         
    if((encoder1count <= len)||(encoder2count <= ren))
    _MODE2 = next;//st_bk();
}

void left(int len, int ren, int ang, String next)
{
    //if(_MODE2 == "LEFT")
    //{
      encoder("bl","gr",len,ren);
          
      if((encoder2count >= ren))//||(Zangle > ang-5))
      _MODE2 == next;//st_bk();
    //}  
}

void right(int len, int ren, int ang, String next)
{
    //if(_MODE2 == "RIGHT")
    //{
      encoder("gl","br",len,ren);
         
      if((encoder1count >= len))//||(Zangle > ang-5))
      _MODE2 = next;//st_bk();
    //}
}

void st_bk()
{
    digitalWrite(28, LOW);
    digitalWrite(29, LOW);
    digitalWrite(30, LOW);
    digitalWrite(31, LOW);
    analogWrite(3, 0);
    analogWrite(4, 0);
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

void encoder(String det, String det2, int ennumL, int ennumR)
{       
     if(det == "gl")
     {     
           if(encoder1count >= ennumL)
           {           
              //mr_mot(28 ,29 ,spbl);
              //delay(10);
              //bk_R(); 
              bk_L();
              /*digitalWrite(28, LOW);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);*/
              analogWrite(3, 0);
              //analogWrite(4, 0);
    
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              //_MODE = "STOP";
              //delay(200);
              //Serial.println(ennumL2);
              
              STATUSINT = "{\"cmd\":\"ENCO\"";
              STATUSINT += ",\"val\":\"GOEND\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
              
           }

           /*else if((adcdata[1] > 750)||(adcdata[3] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              analogWrite(3, 0);
              //analogWrite(4, 0);
              _MODE = "STOP";

              STATUSINT = "{\"cmd\":\"SENSOR\"";
              STATUSINT += ",\"val\":\"RIGHT\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
           }

           else if((adcdata[0] > 750)||(adcdata[2] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              analogWrite(3, 0);
              //analogWrite(4, 0);
              _MODE = "STOP";

              STATUSINT = "{\"cmd\":\"SENSOR\"";
              STATUSINT += ",\"val\":\"LEFT\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
           }*/
          
           else
           {
              mr_mot(29 ,28 ,spdl);
              Serial.print("{");
              Serial.print("\"LEN\":");
              Serial.print("\"");
              Serial.print(encoder1count);
              Serial.print("\"");
              Serial.println("}");
           }
     }

     else if(det == "bl")
     {
           if(encoder1count <= ennumL)
           {
              //mr_mot(29 ,28 ,spdl);
              //delay(10);
              //bk_R(); 
              bk_L();
              /*digitalWrite(28, HIGH);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);*/
              //analogWrite(4, 0);
              analogWrite(3, 0);
              
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              //_MODE = "STOP";
              //delay(200);
              
              STATUSINT = "{\"cmd\":\"ENCO\"";
              STATUSINT += ",\"val\":\"BACKEND\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
               //_MODE = "STOP";
           }

           /*else if((adcdata[1] > 750)||(adcdata[3] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              //analogWrite(4, 0);
              analogWrite(3, 0);
              _MODE = "STOP";
           }

           else if((adcdata[0] > 750)||(adcdata[2] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              //analogWrite(4, 0);
              analogWrite(3, 0);
              _MODE = "STOP";
           }*/
          
           else
           {
              //ml_mot(31 ,30 ,120);
              mr_mot(28 ,29 ,spbl);
              Serial.print("{");
              Serial.print("\"LEN\":");
              Serial.print("\"");
              Serial.print(encoder1count);
              Serial.print("\"");
              Serial.println("}");
           }
     }

     if(det2 == "gr")
     {
           if(encoder2count >= ennumR)
           {
              //ml_mot(31 ,30 ,spbr);
              //delay(10);
              bk_R(); //bk_L();
              /*digitalWrite(28, LOW);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);*/
              //analogWrite(3, 0);
              analogWrite(4, 0);
    
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              //_MODE = "STOP";
              //delay(200);
              //Serial.println(ennumL2);
              
              STATUSINT = "{\"cmd\":\"ENCO\"";
              STATUSINT += ",\"val\":\"GOEND\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
              
           }

           /*else if((adcdata[1] > 750)||(adcdata[3] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              //analogWrite(3, 0);
              analogWrite(4, 0);
              _MODE = "STOP";

              STATUSINT = "{\"cmd\":\"SENSOR\"";
              STATUSINT += ",\"val\":\"RIGHT\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
           }

           else if((adcdata[0] > 750)||(adcdata[2] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              //analogWrite(3, 0);
              analogWrite(4, 0);
              _MODE = "STOP";

              STATUSINT = "{\"cmd\":\"SENSOR\"";
              STATUSINT += ",\"val\":\"LEFT\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
           }*/
          
           else
           {
              ml_mot(30 ,31 ,spdr);
              //mr_mot(29 ,28 ,131);
              Serial.print("{");
              Serial.print("\"REN\":");
              Serial.print("\"");
              Serial.print(encoder2count);
              Serial.print("\"");
              Serial.println("}");
           }
  
           
     }

     

     else if(det2 == "br")
     {
           if(encoder2count <= ennumR)
           {
              //ml_mot(30 ,31 ,spdr);
              //delay(10);
              bk_R(); //bk_L();
              /*digitalWrite(28, HIGH);
              digitalWrite(29, HIGH);
              digitalWrite(30, HIGH);
              digitalWrite(31, HIGH);*/
              analogWrite(4, 0);
              //analogWrite(3, 0);
              
              digitalWrite(33, LOW);
              digitalWrite(32, LOW); 
              digitalWrite(35, LOW);
              digitalWrite(34, LOW); 
                   
              digitalWrite(13, LOW);
              digitalWrite(12, LOW);
              digitalWrite(14, LOW);
              digitalWrite(15, LOW);
              //_MODE = "STOP";
              //delay(200);
              
              STATUSINT = "{\"cmd\":\"ENCO\"";
              STATUSINT += ",\"val\":\"BACKEND\"";
              STATUSINT += "}";
              Serial.println(STATUSINT);
               //_MODE = "STOP";
           }

           /*else if((adcdata[1] > 750)||(adcdata[3] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              analogWrite(4, 0);
              //analogWrite(3, 0);
              _MODE = "STOP";
           }

           else if((adcdata[0] > 750)||(adcdata[2] < 150)||(adcdata[4] < 150))
           {
              back();
              delay(300);
              analogWrite(4, 0);
              //analogWrite(3, 0);
              _MODE = "STOP";
           }*/
          
           else
           {
              ml_mot(31 ,30 ,spbr);
              //mr_mot(28 ,29 ,128);
              Serial.print("{");
              Serial.print("\"REN\":");
              Serial.print("\"");
              Serial.print(encoder2count);
              Serial.print("\"");
              Serial.println("}");
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
      isCommandJSON(receivedData);
      inputString = "";
      stringComplete = true;
    }
  }
}
