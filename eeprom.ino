#ifdef USE_EEPROM
#define EEPROM_I2C_ADDRESS 0x50

//========================================================================
//  readEEPROM: Read historicalData from NVM
//========================================================================
void readEEPROM(struct historicalData *rainfall)
{
  int structSize;
  byte buffer[200];
  byte c;
  bool nonZero = false;
  int address = 0x0000;

  structSize = sizeof(historicalData);
  memset(buffer, 0, sizeof(structSize));

  //Send dummy write to set addpress register of NVM
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((int)(address >> 8));
  Wire.write((int)(address & 0xFF));
  Wire.endTransmission();

  //Read rainfall data
  for (int location = 0; location < structSize; location++)
  {
    Wire.requestFrom(EEPROM_I2C_ADDRESS, 1);
    delay(1);
    while (Wire.available())
    {
      Serial.print(".");
      c = Wire.read();
      Serial.print(c, HEX);
    }
    if (c)
    {
      nonZero = true;
    }
    buffer[location] = c;
  }
  //If EEPROM contains non-zero values, restore data to historical structure
  if (nonZero)
  {
    MonPrintf("\nRestoring rainfall structure from EEPROM\n");
    memcpy(rainfall, buffer, structSize);
  }
}


//========================================================================
//  writeEEPROM: Conditional write of  historicalData from NVM
// if rainfall has non-zero values, or needs updated to zero
// boot must be >1 in order for a write to take place, array will never
// be written on boot = 1
//========================================================================
void writeEEPROM(struct historicalData *rainfall)
{
  byte buffer[200];
  int structSize;
  int writePosition = 0;
  int pageSize = 32;
  int pageAddr = 0;
  int page = 0;


  structSize = sizeof(historicalData);
  memcpy(buffer, rainfall, structSize);
  for (page = 0; (page * pageSize) < structSize; page++)
  {
    pageAddr = page * pageSize;
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((int)(pageAddr >> 8));
    Wire.write((int)(pageAddr & 0xFF));
    MonPrintf("\npageAddr: %04x\n", pageAddr);
    for (writePosition = pageAddr; writePosition < (pageAddr + pageSize); writePosition++)
    {
      if (writePosition < structSize)
      {
        MonPrintf("position: % 02x: % 02x\n", writePosition, buffer[writePosition]);
        Wire.write(buffer[writePosition]);
      }
    }
    Wire.endTransmission();
    delay(10);
    MonPrintf("page write\n");
  }
}

//========================================================================
//  initEEPROM: Clear NVM to 0x00 where structure is stored
//========================================================================
void initEEPROM(void)
{
  int structSize;
  int x;
  structSize = sizeof(historicalData);
  MonPrintf("sizeof int: %i\n", sizeof(int));
  for (x = 0; x < structSize; x++)
  {
    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((int)(x >> 8));
    Wire.write((int)(x & 0xFF));
    Wire.write(0x00);
    Wire.endTransmission();
    delay(10);
  }
}

//========================================================================
//  conditionalWriteEEPROM: Only write EEPROM if something has changed
//========================================================================
void conditionalWriteEEPROM(struct historicalData *rainfall)
{
  struct historicalData historyBuffer;
  bool match = true;
  int structSize;
  int x;

  structSize = sizeof(historicalData);

  readEEPROM(&historyBuffer);

  //compare EEPROM to RTC structure
  //look at hourly totals for rainfall mismatch
  for (int hour = 0; hour < 24; hour++)
  {
    if (historyBuffer.hourlyRainfall[hour] != rainfall->hourlyRainfall[hour])
    {
      MonPrintf("Hourly rainfall: %i\n", rainfall->hourlyRainfall[hour]);
      match = false;
    }
  }

  //look at minute totals for rainfall mismatch
  for (int hour = 0; hour < 12; hour++)
  {
    if (historyBuffer.current60MinRainfall[hour] != rainfall->current60MinRainfall[hour])
    {
      MonPrintf("This hour rainfall: %i\n", rainfall->current60MinRainfall[hour]);
      match = false;
    }
  }

  //if mismatch exists, write EEPROM
  if (!match)
  {
    writeEEPROM(rainfall);
  }
}

#endif
