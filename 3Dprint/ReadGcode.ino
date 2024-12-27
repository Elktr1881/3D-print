
// Fungsi untuk cek ekstensi file (case-insensitive)
bool endsWithIgnoreCase(const char* str, const char* suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);
  if (suffix_len > str_len) return false;
  return strcasecmp(str + (str_len - suffix_len), suffix) == 0;
}

// Fungsi untuk mencari file G-code
int printGcodeFiles(SdFile &dir) {
  int count = 0;  // Variabel untuk menghitung file

  SdFile entry;
  while (entry.openNext(&dir, O_RDONLY)) {  // Buka file berikutnya
    if (entry.isDir()) {
      // Rekursif ke dalam subfolder
      count += printGcodeFiles(entry);
    } else {
      // Dapatkan nama file
      char fileName[50];
      entry.getName(fileName, sizeof(fileName));

      // Periksa apakah ekstensi file .gcode
      if (endsWithIgnoreCase(fileName, ".gcode")) {
        if (fileCount < MAX_FILES) {
          fileNames[fileCount++] = String(fileName);  // Simpan nama file
        }
        count++;
      }
    }
    entry.close();  // Tutup file setelah selesai
  }
  return count;  // Kembalikan jumlah file
}


// Fungsi untuk membaca dan menampilkan isi file yang dipilih
bool readAndPrintFile() {
  
  state = readData = true;
  for(int i=0; i<9; i++){stateCode[i]=false;}  
  
  if(file.available () > 0)
  {
    while (file.available () and readData) 
    {
      static int Orde;
      char sdData = file.read(); // Tampilkan isi file
      if(sdData == ';'){stateCode[8] = true; state = false;}
      else if(sdData == '\n'){readData = false;}  
      if(!state){continue;}

      for(int i=0; i<8; i++){if(sdData == code[i]){Orde = i; stateCode[i]=true;}}

      if(isDigit(sdData) or sdData == '.'){floatString += sdData;}        
      else if(sdData == ' ' or sdData == '\n')
      {
        valueCode[Orde] = floatString.toFloat(); 
        floatString = ""; Orde = 0;
      } 

//      Serial.print(sdData);
    }
    return true;
  }
  else
  {
    file.close();  // Tutup file
    Serial.println("\n=== Akhir File ===");
    return false;
  }
}


bool positioningMode = true;
void ExecuteGcode()
{
  static double destination[4];
  static int RecSpeed;

//  controlTemperature();

  if(positioningMode)
  {
      if(stateCode[0]){destination[0] = valueCode[0];} else {destination[0] = xnow;}
      if(stateCode[1]){destination[1] = valueCode[1];} else {destination[1] = ynow;}
      if(stateCode[2]){destination[2] = valueCode[2];} else {destination[2] = znow;}
      if(stateCode[3]){destination[3] = valueCode[3];} else {destination[3] = enow;}
      if(stateCode[4]){RecSpeed = (int)valueCode[4];}
  }
  else
  {
      if(stateCode[0]){destination[0] = xnow + valueCode[0];} else {destination[0] = xnow;}
      if(stateCode[1]){destination[1] = ynow + valueCode[1];} else {destination[1] = ynow;}
      if(stateCode[2]){destination[2] = znow + valueCode[2];} else {destination[2] = znow;}
      if(stateCode[3]){destination[3] = enow + valueCode[3];} else {destination[3] = enow;}
      if(stateCode[4]){RecSpeed = (int)valueCode[4];}
  }
  
  if(stateCode[6])
  {
//    Serial.print((int)valueCode[6]);
     switch((int)valueCode[6])
     {
       case 0:
          RunToCoordinate(destination[0],  destination[1],  destination[2], enow, RecSpeed);
       break;
       
       case 1:
          RunToCoordinate(destination[0],  destination[1],  destination[2], destination[3], RecSpeed);
       break;
  
       case 28:
          if(stateCode[0]){RunToCoordinate(0.00 ,  ynow,  10.00, enow, 5000);}
          if(stateCode[1]){RunToCoordinate(xnow ,  0.00,  10.00, enow, 5000);}
          if(stateCode[2]){RunToCoordinate(xnow ,  ynow,  0.00, enow, 5000);}
       break;
  
       case 90:
          positioningMode = true;
       break;

       case 91:
          positioningMode = false;
       break;
       
       case 92:
          enow = 0.00;
       break;
     }
  }

  if(stateCode[7])
  {
     switch((int)valueCode[7])
     {
       //Start
       case 99:
          RunToCoordinate(100.00 , 0.00, 10.00, enow, 50000);
          lcd.clear();
          while(abs(counter) < 4)
          {
            controlTemperature();
            lcd.setCursor(3,0);
            lcd.print("Prepare BED !!");
            lcd.setCursor(3,1);
            lcd.print("Scroll to printing !");
            lcd.setCursor(0,2);
            lcd.print("NZ:" + String(Therm[1],2) + " BD:" + String(Therm[0],2) + "    ");
            lcd.setCursor(0,3);
            lcd.print("T1:" + String(nozzleTemp,2) + " T2:" + String(bedTemp,2) + "    ");
          }
          lcd.clear();
          counter = 0;
          RunToCoordinate(100.00 , 100.00, 1.00, enow, 5000);
       break;

       //Finish
       case 88:
         RunToCoordinate(0.00 , 0.00, 10.00, enow, 5000);
         for(int i=0; i<4; i++){digitalWrite(enaPin[i],HIGH);}
         nozzleTemp = 0;
         bedTemp = 0;
       break;
     }
  }
   
}
