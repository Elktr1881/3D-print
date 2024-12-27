const int Rsensity = 2;
int menuState = 0, subMenuState = 0, posMenu = 0;
int shiftCh = 0, setTemp = 0, posString = 0;
float precision = 0.00;
   
void menuStart() 
{
  lcd.setCursor(0,0);
  lcd.print("3D PRINTER By:EFORTH");
  lcd.setCursor(3,1);
  lcd.print("By:EFORTH TECH");
  lcd.setCursor(4,2);
  lcd.print("PREPARATION");
  lcd.setCursor(4,3);
  lcd.print("EXECUTION");
  
  if(abs(counter) > Rsensity)
  {menuState++; counter=0; lcd.clear(); if(menuState > 1){menuState=0;}}
  lcd.setCursor(3,2+menuState);
  lcd.write(0);

  if(buttonPressed){GlobalMenu = menuState+1; setTemp = menuState = 0; lcd.clear(); delay(1000);}
}

void menuPreparation()
{
  precision = ((1023-potentioVal)/1023.00)*100.00;
  const int lcdPos[16] = {1,1,2,2,2,3,3,3,6,12,0,6,12,0,6,12};
  String ModeExtd[3] = {"PUSH  ","PULL  ","MANUAL"};
  String StepperState[3] = {"ALL ON", "ALL OFF", "Z AXIS OFF"};

  switch(subMenuState)
  {
    case 0:
      lcd.setCursor(2,0);
      lcd.print("HEATING NOZLE");
      lcd.setCursor(2,1);
      lcd.print("HEATING BED");
      lcd.setCursor(2,2);
      lcd.print("EXTRUDER");
      lcd.setCursor(2,3);
      lcd.print("POSITIONING");
    
      if(counter > Rsensity)
      {menuState++; counter=0; lcd.clear(); if(menuState > 3){menuState=3;}}
      if(counter < -Rsensity)
      {menuState--; counter=0; lcd.clear(); if(menuState < 1){menuState=0;}}
      lcd.setCursor(1,menuState);
      lcd.write(0);
      posString = shiftCh = 0;

      if(buttonPressed){subMenuState = menuState+1; setTemp = menuState = 0; lcd.clear(); delay(1000);}
    break;  

    case 1:
      if(counter > Rsensity and menuState == 0){nozzleTemp+=10.00; resetPID(pidNozzle); counter=0;}
      if(counter < -Rsensity and menuState == 0){nozzleTemp-=10.00; resetPID(pidNozzle); counter=0;}
      nozzleTemp = constrain(nozzleTemp,0,300);
      
      lcd.setCursor(1,0); lcd.print("Set Point:");
      lcd.setCursor(13,0); lcd.print(nozzleTemp);  
      lcd.setCursor(16,0); lcd.print((char)223);
      lcd.setCursor(17,0); lcd.print("C     ");

      lcd.setCursor(1,1); lcd.print("Temp Nozzle:");
      lcd.setCursor(14,1); lcd.print(Therm[1]);  
      lcd.setCursor(17,1); lcd.print((char)223);
      lcd.setCursor(18,1); lcd.print("C    ");

      lcd.setCursor(2,2); lcd.print("Setting Set Point");
      lcd.setCursor(2,3); lcd.print("Save Set Point ");
      
      lcd.setCursor(1,menuState+2);
      lcd.write(0);

      if(buttonPressed){menuState++; lcd.clear(); if(menuState > 1){menuState = subMenuState=0;}}
    break;

    case 2:
      if(counter > Rsensity and menuState == 0){bedTemp+=10.00; resetPID(pidBed); counter=0;}
      if(counter < -Rsensity and menuState == 0){bedTemp-=10.00; resetPID(pidBed); counter=0;}
      bedTemp = constrain(bedTemp,0,100);
      
      lcd.setCursor(1,0); lcd.print("Set Point:");
      lcd.setCursor(13,0); lcd.print(bedTemp);  
      lcd.setCursor(16,0); lcd.print((char)223);
      lcd.setCursor(17,0); lcd.print("C   ");

      lcd.setCursor(1,1); lcd.print("Temp Bed:");
      lcd.setCursor(14,1); lcd.print(Therm[0]);  
      lcd.setCursor(17,1); lcd.print((char)223);
      lcd.setCursor(18,1); lcd.print("C    ");

      lcd.setCursor(2,2); lcd.print("Setting Set Point");
      lcd.setCursor(2,3); lcd.print("Save Set Point ");

      lcd.setCursor(1,menuState+2);
      lcd.write(0);

      if(buttonPressed){menuState++; lcd.clear(); if(menuState > 1){menuState = subMenuState=0;}}
    break;

    case 3:
      lcd.setCursor(2,0);
      lcd.print("MODE: " + ModeExtd[posString]);
      lcd.setCursor(2,1);
      lcd.print("SPEED: " + String((int)precision/100) + String((int)precision%100/10) + String((int)precision%100%10) + "%");
      lcd.setCursor(2,2);
      lcd.print("START     STOP");
      lcd.setCursor(2,3);
      lcd.print("EXIT");

      if(counter > Rsensity)
      {menuState++; posMenu=counter=0; lcd.clear();if(menuState == 1){menuState++;} if(menuState > 3){menuState=3;}}
      else if(counter < -Rsensity)
      {menuState--; posMenu=counter=0; lcd.clear();if(menuState == 1){menuState--;} if(menuState < 1){menuState=0;}}

      if(menuState == 2 and posMenu == 1){lcd.setCursor(10,menuState);}
      else{lcd.setCursor(1,menuState);} 
      lcd.write(0); 

      while(!buttonPressed and menuState == 2 and posMenu == 1)
      {
        controlTemperature();
        if(posString == 0){speedStepper = ((100-(int)precision)*1000) + 50; ExtruderRun(speedStepper);}
        else if(posString == 1){speedStepper = ((100-(int)precision)*1000) + 50; ExtruderRun(-speedStepper);}
        else if(posString == 2)
        {
          speedStepper = ((100-(int)precision)*1000) + 50;
          if(counter > Rsensity){counter=0; for(int i=0; i<100; i++){ExtruderRun(speedStepper);}}
          else if(counter < -Rsensity){counter=0; for(int i=0; i<100; i++){ExtruderRun(-speedStepper);}}
        }
      }
 
      if(buttonPressed)
      {
        lcd.clear();
        if(menuState == 0){posString++; if(posString>2){posString=0;}}
        else if(menuState == 2){posMenu++; if(posMenu>1){posMenu=0;}}
        else if(menuState == 3){posMenu = menuState = subMenuState=0;}
        buttonPressed = false;
      }   
    break;

    /*Calibration Position*/
    case 4:
      lcd.setCursor(2,0);
      lcd.print("RESET XY AXIS");
      lcd.setCursor(2,1);
      lcd.print("POWER: " + StepperState[posString]);
      lcd.setCursor(2,2);
      lcd.print("Z POINT CALIB");
      lcd.setCursor(2,3);
      lcd.print("SAVE & EXIT");

      lcd.setCursor(1,menuState);
      lcd.write(0);

      if(buttonPressed)
      {
        if(menuState == 0){CalibrationX(); CalibrationY(); stateCalX = true; stateCalY = true;}
        else if(menuState == 2){subMenuState=41; menuState=0;}
        else if(menuState == 3){GlobalMenu=menuState=subMenuState=0;}
        else if(menuState == 1)
        {
          posMenu++; if(posMenu>1){posMenu=0;}
          if(posString == 0){for(int i=0; i<4; i++){digitalWrite(enaPin[i],LOW);}}
          else if(posString == 1){digitalWrite(enaPin[3],HIGH);}
          else if(posString == 2){for(int i=0; i<4; i++){digitalWrite(enaPin[i],HIGH);}}
        }
        buttonPressed = false; lcd.clear();
      } 
      
      if(counter > Rsensity and posMenu == 1 and menuState == 1)
      {posString++; counter=0; lcd.clear(); if(posString > 2){posString=2;}}
      else if(counter < -Rsensity and posMenu == 1 and menuState == 1)
      {posString--; counter=0; lcd.clear(); if(posString < 1){posString=0;}}
      else if(counter > Rsensity){menuState++; counter=0; lcd.clear(); if(menuState > 3){menuState=3;}}
      else if(counter < -Rsensity){menuState--; counter=0; lcd.clear(); if(menuState < 1){menuState=0;}}
    break;

    case 41:
      lcd.setCursor(2,0);
      lcd.print("LEV:" + String((int)precision/100) + String((int)precision%100/10) + String((int)precision%100%10) + "%");
      lcd.setCursor(12,0);
      lcd.print("FINISH");
      lcd.setCursor(2,1);
      lcd.print("START");

      for(int i=1; i<9; i++)
      {
        lcd.setCursor(2+lcdPos[i+7],lcdPos[i-1]);
        if(Z_point[i]>=0){lcd.print("P");}
        else{lcd.print("N");}
        lcd.setCursor(3+lcdPos[i+7],lcdPos[i-1]);
        lcd.print(String(abs(Z_point[i])/1000)+
                  String(abs(Z_point[i])%1000/100)+
                  String(abs(Z_point[i])%1000%100/10)+
                  String(abs(Z_point[i])%1000%100%10));
      }

      if(menuState == 0){lcd.setCursor(1,0);}
      else if(menuState == 1){lcd.setCursor(1,1);}
      else if(menuState == 10){lcd.setCursor(11,0);}
      else{lcd.setCursor(1+lcdPos[menuState+6],lcdPos[menuState-2]);}
      lcd.write(0);

      if(counter > Rsensity){menuState++; counter=0; lcd.clear(); if(menuState > 10){menuState=10;}}
//      else if(counter > Rsensity and !stateCalZ){menuState++; counter=0; lcd.clear(); if(menuState > 1){menuState=1;}}
      else if(counter < -Rsensity){menuState--; counter=0; lcd.clear(); if(menuState < 0){menuState=0;}}

      while(!buttonPressed and stateCalZ)
      {
        precision = ((1023-analogRead(POT))/1023.00)*100.00;
        lcd.setCursor(2,0);
        lcd.print("LEV:" + String((int)precision/100) + String((int)precision%100/10) + String((int)precision%100%10) + "%");

        if(counter > Rsensity){counter=0; MovingStepper(0, 0, (int)precision*8, 0, 50); Z_point[shiftCh]+=(int)precision*8;}
        else if(counter < -Rsensity){counter=0; MovingStepper(0, 0, (int)precision*-8, 0, 50); Z_point[shiftCh]-=(int)precision*8;}
        
        if(shiftCh > 0)
        {
          lcd.setCursor(2+lcdPos[shiftCh+7],lcdPos[shiftCh-1]);
          if(Z_point[shiftCh]>=0){lcd.print("P");}
          else{lcd.print("N");}
          lcd.setCursor(3+lcdPos[shiftCh+7],lcdPos[shiftCh-1]);
          lcd.print(String(abs(Z_point[shiftCh])/1000)+
                    String(abs(Z_point[shiftCh])%1000/100)+
                    String(abs(Z_point[shiftCh])%1000%100/10)+
                    String(abs(Z_point[shiftCh])%1000%100%10));
                    
          znow = 0.00125*Z_point[shiftCh];
        }
        else{znow = 0.00; Z_point[shiftCh] = 0;}

        menuState = 0; 
      }
      
      stateCalZ = false;

      if(buttonPressed)
      {
        if(menuState == 0){stateCalZ = false;}
        else if(menuState == 2){RunToCoordinate(10.00,  10.00,  5.00, 0, 25000); shiftCh=1; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 3){RunToCoordinate(10.00,  100.00, 5.00, 0, 25000); shiftCh=2; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 4){RunToCoordinate(10.00,  190.00, 5.00, 0, 25000); shiftCh=3; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 5){RunToCoordinate(100.00, 190.00, 5.00, 0, 25000); shiftCh=4; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 6){RunToCoordinate(190.00, 190.00, 5.00, 0, 25000); shiftCh=5; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 7){RunToCoordinate(190.00, 100.00, 5.00, 0, 25000); shiftCh=6; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 8){RunToCoordinate(190.00, 10.00,  5.00, 0, 25000); shiftCh=7; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 9){RunToCoordinate(100.00, 10.00,  5.00, 0, 25000); shiftCh=8; Z_point[shiftCh] = 4000; stateCalZ = true;}
        else if(menuState == 1){RunToCoordinate(100.00, 105.00, 5.00, 0, 25000); shiftCh=0; Z_point[shiftCh] = 4000; stateCalZ = true; 
        for(int i=0; i<8; i++){levelingCal[i]= 0.00;}}
        else if(menuState == 10 )
        {
          RunToCoordinate(100.00, 100.00, 0.30, 0, 50000);
          
          for(int i=0; i<9; i++){levelingCal[i] = (float)Z_point[i]*0.00125; Serial.print(levelingCal[i]*1000); Serial.print("um  ");} 
          subMenuState=4; menuState=0; z_offside = 0.00;
         
          RunToCoordinate(10.00, 10.00, 0.30, 0, 50000);
          RunToCoordinate(10.00, 100.00, 0.30, 0, 50000);
          RunToCoordinate(10.00, 190.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 100.00, 0.30, 0, 50000);
          
          RunToCoordinate(190.00, 10.00, 0.30, 0, 50000);
          RunToCoordinate(190.00, 100.00, 0.30, 0, 50000);
          RunToCoordinate(190.00, 190.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 100.00, 0.30, 0, 50000);
          
          RunToCoordinate(10.00, 10.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 10.00, 0.30, 0, 50000);
          RunToCoordinate(190.00, 10.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 100.00, 0.30, 0, 50000);
          
          RunToCoordinate(10.00, 190.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 190.00, 0.30, 0, 50000);
          RunToCoordinate(190.00, 190.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 100.00, 0.30, 0, 50000);
          RunToCoordinate(100.00, 100.00, 0.30, 0, 7200);
        }
        Serial.println(shiftCh);
        buttonPressed = false; lcd.clear();
      } 
    break;
  }
}


void menuExeution()
{
  switch(subMenuState)
  {
    case 0:
      lcd.setCursor(2,0);
      lcd.print("SELECT FILE");
      lcd.setCursor(2,1);
      lcd.print("START PRINTING");
      lcd.setCursor(2,2);
      lcd.print("HOME PAGE");
      lcd.setCursor(0,3);
      lcd.print(selectedFile);

      if(counter > Rsensity)
      {menuState++; counter=0; lcd.clear(); if(menuState > 2){menuState = 2;}}
      if(counter < -Rsensity)
      {menuState--; counter=0; lcd.clear(); if(menuState < 0){menuState = 0;}}
      
      lcd.setCursor(1,menuState);
      lcd.write(0);

      if(buttonPressed){subMenuState = menuState+1; menuState = 0; lcd.clear();}
     break;
     
     case 1:
      if(counter > Rsensity)
      {menuState++; counter=0; lcd.clear(); if(menuState > 3){shiftCh++;}}
      if(counter < -Rsensity)
      {menuState--; counter=0; lcd.clear(); if(menuState < 1){shiftCh--;}}
      menuState = constrain(menuState,0,3);
      
      if(shiftCh > fileCount-5 and fileCount > 5){shiftCh = fileCount-5;}
      else if(fileCount < 5 or shiftCh < 0){shiftCh = 0;}
      
      lcd.setCursor(1,menuState);
      lcd.write(0);
      selectedFile = fileNames[menuState+shiftCh];
      
      for(int i=0; i<=3; i++)
      {
        lcd.setCursor(2,i);
        lcd.print(fileNames[i+shiftCh]);
      }
      
      if(buttonPressed){menuState = subMenuState = 0; lcd.clear();}
     break;

     case 2:
      subMenuState = 0; printing = readData = true;
      
      for(int i=0; i<=100; i++)
      {   
        controlTemperature();
        lcd.setCursor(0,0); lcd.print("PRINTING:");
        lcd.setCursor(16,0); lcd.print(i/100);
        lcd.setCursor(17,0); lcd.print(i%100/10);
        lcd.setCursor(18,0); lcd.print(i%100%10);
        lcd.setCursor(19,0); lcd.print("%");

        if(i==20){file = SD.open(selectedFile);}
        else if(i==30)
        {
          if(!file)
          {
            lcd.setCursor(0,4); lcd.print("FAILED READ FILE !!!");
            while(!buttonPressed){controlTemperature();}
          }
          else{lcd.setCursor(0,4); lcd.print("SUCCES READ :)");}
        }
      }   
      lcd.clear();
     break;

     case 3:
      GlobalMenu=menuState=subMenuState=0;
     break;
  }
}
