

// Fungsi PID untuk multi-sistem
double calculatePID(double input, double setPoint, double Kp, double Ki, double Kd, PID &pid) {
  unsigned long now = millis(); // Waktu saat ini
  double elapsedTime = (now - pid.lastTime) / 1000.0; // Waktu dalam detik

  double error = setPoint - input;    // Hitung error saat ini
  if(error > 255.00){error = 255.00;} else if(error < -255.00){error = -255.00;}
  
  pid.integral += error * elapsedTime; // Hitung komponen integra
  
  if(pid.lastError > 255.00){pid.lastError = 255.00;} 
  else if(pid.lastError < -255.00){pid.lastError = -255.00;}
  double derivative = (error - pid.lastError) / elapsedTime; // Hitung komponen derivatif
  
  double output = (Kp * error) + (Ki * pid.integral) + (Kd * derivative); // Hitung PID
  if(output > 255.00){output = 255.00;} else if(output < 0.00){output = 0.00;}
  
  // Simpan nilai untuk iterasi berikutnya
  pid.lastError = error;
  pid.lastTime = now;

//  Serial.print(error);
//  Serial.print("  ");
//  Serial.print(pid.integral);
//  Serial.print("  ");
//  Serial.print(derivative);
//  Serial.print("  ");
//  Serial.print(Kp*error);
//  Serial.print("  ");
//  Serial.print(Ki*pid.integral);
//  Serial.print("  ");
//  Serial.print(Kd*derivative);
//  Serial.print("  ");
//  Serial.print(output);
//  Serial.println("  ");

  return output; // Kembalikan nilai output
}

void resetPID(PID &pid) {
  pid.lastError = 0.0;   // Reset error terakhir
  pid.integral = 0.0;    // Reset integral
  pid.lastTime = millis(); // Atur ulang waktu terakhir
}

void controlTemperature()
{
    for(int i=0; i<2; i++)
    {
        int adcValue = analogRead(TERM[i]);   
        float V_out = (adcValue / 1023.0) * 5.0; 
        float R_NTC = (V_out * R_SER) / (5.0 - V_out);
        float tempK = B / (log(R_NTC / R_25) + (B / T_25));
        Therm[i] = tempK - 273.15;
    }
  
    double pwmHeatNozzle = calculatePID(Therm[1], nozzleTemp, 2.00, 0.01, 20, pidNozzle);
    analogWrite(NOZZLE,pwmHeatNozzle);
    
    double pwmHeatBed = calculatePID(Therm[0], bedTemp, 3.00, 0.05, 50, pidBed);
    analogWrite(BEDHEAT,(int)pwmHeatBed);
}


double bicubicKernel(double t) {
    if (t < 0) t = -t; // ambil nilai absolut
    if (t < 1) {
        return (1.5 * t * t * t - 2.5 * t * t + 1);
    } else if (t < 2) {
        return (-0.5 * t * t * t + 2.5 * t * t - 4 * t + 2);
    } else {
        return 0;
    }
}

double interpolateZ(double x, double y) {
    double result = 0.0;

    const double x_coords[3] = {10, 100, 190};
    const double y_coords[3] = {10, 100, 190};

    // Nilai Z pada titik sampel (diukur secara manual)
    double z[3][3] = {
        {levelingCal[1], levelingCal[8], levelingCal[7]}, // Z00, Z10, Z20
        {levelingCal[2], levelingCal[0], levelingCal[6]}, // Z01, Z11, Z21
        {levelingCal[3], levelingCal[4], levelingCal[5]}  // Z02, Z12, Z22
    };

    // Loop melalui semua titik sampel
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // Hitung kernel untuk x dan y
            double wx = bicubicKernel((x - x_coords[j]) / (x_coords[1] - x_coords[0]));
            double wy = bicubicKernel((y - y_coords[i]) / (y_coords[1] - y_coords[0]));
            // Tambahkan kontribusi titik ke hasil akhir
            result += z[i][j] * wx * wy;
        }
    }
    return result;
}
