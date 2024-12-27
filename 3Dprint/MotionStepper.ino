//// X = (0,00625mm) = 160step/mm
//// Y = (0,00625mm) = 160step/mm
//// Y = (0,00125mm) = 800step/mm
//// E = 100step = 6mm / 20step/mm
//// Speed = mm/minutes (360000step/minute)

void RunToCoordinate(double pointX, double pointY, double pointZ, double pointE, unsigned int Speed)
{
  int stepMoveX = 0, stepMoveY = 0, stepMoveZ = 0, stepMoveE = 0;
  double xmove = 0.00, ymove = 0.00, zmove = 0.00, emove = 0.00;
  double pointZ_offside = interpolateZ(pointX, pointY);
  znow -= (pointZ_offside-z_offside);
  
  xmove = pointX - xnow;
  ymove = pointY - ynow;
  zmove = pointZ - znow;
  emove = pointE - enow;
  z_offside = pointZ_offside;

//  Serial.print("  ZO:");
//  Serial.print(z_offside);
//  Serial.print("  X:");
//  Serial.print(xmove);
//  Serial.print("  Y:");
//  Serial.print(ymove);
//  Serial.print("  Z:");
//  Serial.print(zmove);
//  Serial.print("  E:");
//  Serial.print(emove);
  
  if(abs(xmove) >= 0.00625){stepMoveX = int(xmove/0.00625);}
  if(abs(ymove) >= 0.00625){stepMoveY = int(ymove/0.00625);}
  if(abs(zmove) >= 0.00125){stepMoveZ = int(zmove/0.00125);}
  if(abs(emove) >= 0.005){stepMoveE = int(emove/0.005);}

  unsigned int DelayStep = 3600000/Speed;
  MovingStepper(stepMoveX, stepMoveY, stepMoveZ, stepMoveE, DelayStep);
//
//  Serial.print("  X:");
//  Serial.print(xnow);
//  Serial.print("  Y:");
//  Serial.print(ynow);
//  Serial.print("  Z:");
//  Serial.print(znow);
//  Serial.print("  E:");
//  Serial.print(enow);
//  Serial.print("  X:");
//  Serial.print(stepMoveX);
//  Serial.print("  Y:");
//  Serial.print(stepMoveY);
//  Serial.print("  Z:");
//  Serial.print(stepMoveZ);
//  Serial.print("  E:");
//  Serial.print(stepMoveE);
//  Serial.print("  Speed:");
//  Serial.print(Speed);
//  Serial.print("  Speed:");
//  Serial.print(DelayStep);
//  Serial.println();
  
}

// Fungsi untuk menjalankan motor secara bersamaan dengan sinkronisasi waktu
void MovingStepper(int cyclesX, int cyclesY, int cyclesZ, int cyclesE, int Delay) {
  int pulseSUM[4];
  
  int stepsX = abs(cyclesX);
  int stepsY = abs(cyclesY);
  int stepsZ = abs(cyclesZ);
  int stepsE = abs(cyclesE);

  // Menentukan langkah maksimum
  int maxSteps = max(stepsE, max(stepsX, max(stepsY, stepsZ)));

  // Pengaturan arah motor
  if(cyclesX > 0){digitalWrite(dirPin[0], LOW); pulseSUM[0] = 1;}
  else{digitalWrite(dirPin[0], HIGH); pulseSUM[0] = -1;}
  if(cyclesY > 0){digitalWrite(dirPin[1], HIGH); pulseSUM[1] = 1;}
  else{digitalWrite(dirPin[1], LOW); pulseSUM[1] = -1;}
  if(cyclesZ > 0){digitalWrite(dirPin[2], HIGH); pulseSUM[2] = 1;}
  else{digitalWrite(dirPin[2], LOW); pulseSUM[2] = -1;}
  if(cyclesE > 0){digitalWrite(dirPin[3], LOW); pulseSUM[3] = 1;}
  else{digitalWrite(dirPin[3], HIGH); pulseSUM[3] = -1;}

  // Variabel untuk menghitung kapan setiap motor harus bergerak 
  double stepIntervalX = (float)maxSteps / stepsX;
  double stepIntervalY = (float)maxSteps / stepsY;
  double stepIntervalZ = (float)maxSteps / stepsZ;
  double stepIntervalE = (float)maxSteps / stepsE;

  // Posisi langkah untuk setiap motor
  double stepCounterX = 0;
  double stepCounterY = 0;
  double stepCounterZ = 0;
  double stepCounterE = 0;

  // Loop utama
  for (int i = 0; i < maxSteps; i++) 
  {
    // Motor X
    if (stepCounterX < i and stepsX > 0) {
      digitalWrite(stepPin[0], HIGH);
      stepCounterX += stepIntervalX;
      xnow += (pulseSUM[0]*0.00625);
    }

    // Motor Y
    if (stepCounterY < i and stepsY > 0) {
      digitalWrite(stepPin[1], HIGH);
      stepCounterY += stepIntervalY;
      ynow += (pulseSUM[1]*0.00625);
    }

    // Motor Z
    if (stepCounterZ < i and stepsZ > 0) {
      digitalWrite(stepPin[2], HIGH);
      stepCounterZ += stepIntervalZ;
      znow += (pulseSUM[2]*0.00125);
    }
    
    // Motor E
    if (stepCounterE < i and stepsE > 0) {
      digitalWrite(stepPin[3], HIGH);
      stepCounterE += stepIntervalE;
      enow += (pulseSUM[3]*0.005);
    }
    delayMicroseconds(Delay);
    digitalWrite(stepPin[0], LOW);
    digitalWrite(stepPin[1], LOW);
    digitalWrite(stepPin[2], LOW);
    digitalWrite(stepPin[3], LOW);
    delayMicroseconds(Delay); 
  }
}

/* Calibration Section */
void CalibrationX()
{
  digitalWrite(dirPin[0], HIGH);
  while(stateCalX)
  {
    digitalWrite(stepPin[0], HIGH);
    delayMicroseconds(200);
    digitalWrite(stepPin[0], LOW);
    delayMicroseconds(200);
  }
  digitalWrite(dirPin[0], LOW);
  for(int i=0; i<(102.25*160); i++)
  {
    digitalWrite(stepPin[0], HIGH);
    delayMicroseconds(20);
    digitalWrite(stepPin[0], LOW);
    delayMicroseconds(20);
  }
  xnow = 100.00;
}
void Reset_X(){stateCalX = false; Serial.println("Calibration X axis Succes");}

void CalibrationY()
{
  digitalWrite(dirPin[1], LOW);
  while(stateCalY)
  {
    digitalWrite(stepPin[1], HIGH);
    delayMicroseconds(200);
    digitalWrite(stepPin[1], LOW);
    delayMicroseconds(200);
  }
  digitalWrite(dirPin[1], HIGH);
  for(int i=0; i<(101.80*160); i++)
  {
    digitalWrite(stepPin[1], HIGH);
    delayMicroseconds(20);
    digitalWrite(stepPin[1], LOW);
    delayMicroseconds(20);
  }
  ynow = 100.00;
}
void Reset_Y(){stateCalY = false; Serial.println("Calibration Y axis Succes");}

void CalibrationZ(int stateCalib)
{
  if(stateCalib == 0){RunToCoordinate(100.00, 110.00, 10.00, 0, 7500);}
  else if(stateCalib == 1){RunToCoordinate(10.00, 10.00, 10.00, 0, 7500);}
  else if(stateCalib == 2){RunToCoordinate(10.00, 190.00, 10.00, 0, 7500);}
  else if(stateCalib == 3){RunToCoordinate(190.00, 190.00, 10.00, 0, 7500);}
  else if(stateCalib == 4){RunToCoordinate(190.00, 10.00, 10.00, 0, 7500);}
}
void Reset_Z(){stateCalZ = false; Serial.print("Calibration Z axis Succes");}


void ExtruderRun(int Speed)
{
  if(Speed > 0){digitalWrite(dirPin[3], LOW);}
  else{digitalWrite(dirPin[3], HIGH);}

  digitalWrite(stepPin[3], HIGH);
  delayMicroseconds(abs(Speed));
  digitalWrite(stepPin[3], LOW);
  delayMicroseconds(abs(Speed));
}
