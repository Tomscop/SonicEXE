#ifndef PSXF_GUARD_SAVE_H
#define PSXF_GUARD_SAVE_H

#include "psx.h"

typedef struct {
  	u16 id; // must be 0x4353
  	u8 iconDisplayFlag;
  	u8 iconBlockNum; // always 1
  	u8 title[64]; // 16 bit shift-jis format
  	u8 reserved[28];
  	u8 iconPalette[32];
  	u8 iconImage[128];
  	u8 saveData[7936];
} SaveFile;


void defaultSettings();
boolean readSaveFile();
void writeSaveFile();

#endif
