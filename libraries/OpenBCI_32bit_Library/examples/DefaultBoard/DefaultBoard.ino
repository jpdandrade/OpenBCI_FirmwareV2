#include <DSPI.h>
#include <OBCI32_SD.h>
#include <EEPROM.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Booleans Required for SD_Card_Stuff.ino
boolean addAccelToSD = false; // On writeDataToSDcard() call adds Accel data to SD card write
boolean addAuxToSD = false; // On writeDataToSDCard() call adds Aux data to SD card write
boolean SDfileOpen = false; // Set true by SD_Card_Stuff.ino on successful file open

void setup() {
  // Bring up the OpenBCI Board
  board.begin();

  // Notify the board we want to use accel data
  board.useAccel = true;
  
  byte N_setting = board.RREG(0x10,8);

  board.leadOffSettings[0][1] = ON;
  board.leadOffSettings[1][1] = ON;
  board.leadOffSettings[2][1] = ON;
  board.leadOffSettings[3][1] = ON;
  board.leadOffSettings[4][1] = ON;
  board.leadOffSettings[5][1] = ON;
  board.leadOffSettings[6][1] = ON;
  board.leadOffSettings[7][1] = ON;

  bitSet(N_setting,0x00);
  bitSet(N_setting,0x01);
  bitSet(N_setting,0x02);
  bitSet(N_setting,0x03);
  bitSet(N_setting,0x04);
  bitSet(N_setting,0x05);
  bitSet(N_setting,0x06);
  bitSet(N_setting,0x07);

  board.WREG(0x10,N_setting,8);
}

void loop() {
  if (board.streaming) {
    if (board.channelDataAvailable) {
      // Read from the ADS(s), store data, set channelDataAvailable flag to false
      board.updateChannelData();

      // Check to see if accel has new data
      if(board.accelHasNewData()) {
        // Get new accel data
        board.accelUpdateAxisData();

        // Tell the SD_Card_Stuff.ino to add accel data in the next write to SD
        addAccelToSD = true; // Set false after writeDataToSDcard()
      }

      // Verify the SD file is open
      if(SDfileOpen) {
        // Write to the SD card, writes aux data
        writeDataToSDcard(board.sampleCounter);
      }
      if (board.timeSynced) {
        // Send time synced packet with channel data, current board time, and an accel reading
        //  X axis is sent on sampleCounter % 10 == 7
        //  Y axis is sent on sampleCounter % 10 == 8
        //  Z axis is sent on sampleCounter % 10 == 9
        board.sendChannelDataWithTimeAndAccel();
      } else {
        // Send standard packet with channel data
        board.sendChannelDataWithAccel();
      }
    }
  }
  // Check serial 0 for new data
  if (board.hasDataSerial0()) {
    // Read one char from the serial 0 port
    char newChar = board.getCharSerial0();

    // Send to the sd library for processing
    sdProcessChar(newChar);

    // Send to the board library
    board.processChar(newChar);
  }
}
