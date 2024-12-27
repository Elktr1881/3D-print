#include <LiquidCrystal_I2C.h>
#include <SdFat.h>
#include "PinChangeInterrupt.h"
LiquidCrystal_I2C lcd(0x3F,20,4);
byte selectorChar[] = {0x18,0x1C,0x1E,0x1F,0x1F,0x1E,0x1C,0x18};

#define SW  A10
#define CLK A12
#define DT  44
#define POT A3
#define NOZZLE 9
#define BEDHEAT 8

// Struktur data untuk setiap sistem PID
struct PID {
  double lastError;
  double integral;
  unsigned long lastTime;
};

// Struktur PID untuk dua sistem
PID pidNozzle = {0, 0, 0}; // PID untuk sistem 1 (nozzle PID)
PID pidBed = {0, 0, 0}; // PID untuk sistem 2 (bed PID)


// Parameter thermistor
const int TERM[2] = {A13,A14};
const float B = 3950;           // B-value dari datasheet
const float R_SER = 4700;       // Resistor seri (4.7kΩ)
const float R_25 = 100000;      // Resistansi thermistor pada 25°C
const float T_25 = 298.15;      // Suhu 25°C dalam Kelvin

volatile float potentioVal = 0;
volatile int counter = 0;     // Menyimpan posisi rotary encoder
volatile int lastStateCLK;    // Menyimpan status terakhir CLK
volatile bool buttonPressed = false;  // Menyimpan status tombol

// Konstanta dan variabel
#define MAX_FILES 50          // Maksimum jumlah file yang disimpan
const int chipSelect = 53;    // Pin CS untuk modul SD Card

// Objek SdFat
SdFat SD;
File file;
String fileNames[MAX_FILES];  // Array untuk menyimpan nama file
int fileCount = 0;            // Jumlah file yang ditemukan
bool fileSelected = false;    // Status apakah file dipilih
String selectedFile = "file not chage!!!";          // Nama file yang dipilih

char code[8] = {'X','Y','Z','E','F','S','G','M'};
double valueCode[8] = {0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00};
bool stateCode[9] = {false,false,false,false,false,false,false,false,false};
bool state = false;
bool readData = false;
String floatString = "";

volatile bool stateCalX = true;
volatile bool stateCalY = true;
volatile bool stateCalZ = true;
double xnow = 0.00 , ynow = 0.00, znow = 0.00, enow = 0.00, z_offside = 0.00;
double levelingCal[9] = {0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00};

// Define pin connections & motor's steps per revolution
const int dirPin[4]       = {A1, A7, 48, 28};
const int stepPin[4]      = {A0, A6, 46, 26};
const int enaPin[4]       = {38, A2, A8, 24};
const int safetySwitch[3] = {2,18,3};

//int state = 0;
long pos[3] = {0,0,0};
String int_String = "";
int Z_point[9] = {0,104,16,624,144,416,-160,56,-40};

const int stepsPerRevolution = 100;

volatile float Therm[2]; //0=bed, 1=nozle
float nozzleTemp = 0;
float bedTemp = 0;
int speedStepper = 0;

bool printing = false;

void setup()
{
  Serial.begin(115200);

  for(int i=0; i<4; i++)
  {
    pinMode(stepPin[i], OUTPUT);
    pinMode(dirPin[i],  OUTPUT);
    pinMode(enaPin[i],  OUTPUT);
    digitalWrite(enaPin[i],HIGH);
  }
  pinMode(NOZZLE,  OUTPUT);
  pinMode(BEDHEAT,  OUTPUT);
  
  pinMode(safetySwitch[0],INPUT);
  pinMode(safetySwitch[1],INPUT);
  pinMode(safetySwitch[2],INPUT);
  
  pinMode(SW,  INPUT_PULLUP);
  pinMode(CLK, INPUT);
  pinMode(DT,  INPUT);

  pinMode(POT, INPUT);
  pinMode(TERM[0], INPUT);
  pinMode(TERM[1], INPUT);

  // Inisialisasi status awal CLK
  lastStateCLK = digitalRead(CLK);

  // Menambahkan interrupt untuk CLK dan SW
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(CLK), readEncoder, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SW), readButton, FALLING);
    
  lcd.init(); 
  lcd.createChar(0, selectorChar);
  lcd.home();
  lcd.backlight();
  
  lcd.setCursor(5,0);
  lcd.print("3D PRINTER");
  lcd.setCursor(3,1);
  lcd.print("By:EFORTH TECH");
  delay(500);
  
  lcd.setCursor(0,2);
  lcd.print("Initialization");
  Serial.println("123");
  
  for(int i=0; i<=100; i++)
  {   
    lcd.setCursor(16,2); lcd.print(i/100);
    lcd.setCursor(17,2); lcd.print(i%100/10);
    lcd.setCursor(18,2); lcd.print(i%100%10);
    lcd.setCursor(19,2); lcd.print("%");
    delay(20);

    if(i==20){while(!digitalRead(safetySwitch[0]))
    {lcd.setCursor(0,3); lcd.print("X AXIS NOT RELEASE !");}}
    else if(i==40){while(!digitalRead(safetySwitch[1]))
    {lcd.setCursor(0,3); lcd.print("Y AXIS NOT RELEASE !");}}
    else if(i==60){while(!digitalRead(safetySwitch[2]))
    {lcd.setCursor(0,3); lcd.print("Z AXIS NOT RELEASE !");}}
    else if(i==80){while(!SD.begin(chipSelect))
    {lcd.setCursor(0,3); lcd.print("  INSERT SD CARD !  ");}}
    else if(i==99)
    {
      // Buka root directory
      SdFile root;
      root.open("/");
      fileCount = printGcodeFiles(root);
      root.close();
      while(fileCount<1){lcd.setCursor(0,3); lcd.print("NO FILES DETECTED !!");}
    }
    else if(i==100){delay(500);}
  }
  lcd.setCursor(0,3);
  lcd.print("  PRES TO CONTINUE  ");
   
  while(!buttonPressed);
  lcd.clear(); delay(1000);
  buttonPressed = false; 
  
  for(int i=0; i<4; i++){digitalWrite(enaPin[i],LOW);}
  
  attachInterrupt(digitalPinToInterrupt(safetySwitch[0]), Reset_X, FALLING);
  attachInterrupt(digitalPinToInterrupt(safetySwitch[1]), Reset_Y, FALLING);
  attachInterrupt(digitalPinToInterrupt(safetySwitch[2]), Reset_Z, FALLING);
  Serial.println("123");
}


int GlobalMenu = 0;
void loop()
{ 
    potentioVal = 1023.00-analogRead(POT);
    controlTemperature();

    switch(GlobalMenu)
    {
      case 0: menuStart(); break;
      case 1: menuPreparation(); break;
      case 2: menuExeution(); break;
    }
     
    while(printing)
    {   
      controlTemperature();
      printing = readAndPrintFile();
      ExecuteGcode();

      if(stateCode[8])
      {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("X:" + String(xnow,2) + " Y:" + String(ynow,2) + "    ");
          lcd.setCursor(0,1);
          lcd.print("Z:" + String(znow,2) + " E:" + String(enow,2) + "    ");
          lcd.setCursor(0,2);
          lcd.print("NZ:" + String(Therm[1],2) + " BD:" + String(Therm[0],2) + "    ");
          lcd.setCursor(0,3);
          lcd.print("T1:" + String(nozzleTemp,2) + " T2:" + String(bedTemp,2) + "    ");
      }
      
      if(buttonPressed){printing = false; lcd.clear();}
    }

    buttonPressed = false;
}

//////////////////////////////////////////////  INTERUPT FUNCTION  //////////////////////////////////////////
void readEncoder() 
{
  int currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK) {  // Hanya jika status CLK berubah
    if (digitalRead(DT) != currentStateCLK) {counter++;} 
    else {counter--;}
  }
  lastStateCLK = currentStateCLK;  // Simpan status CLK saat ini
}

void readButton() 
{
  delay(2000);
  buttonPressed = true;  // Tandai bahwa tombol ditekan
}
