#ifndef ___READ_FAT_H___
#define ___READ_FAT_H___

#include "FAT_system.h"

void callBootSector();
FAT checkFAT();
uint16_t getFatValue( uint16_t previous_cluster);
uint32_t getDataFile(Entry_Short *entry_short, uint16_t index);
void readSector(uint16_t sectorNumber, void *buffer);
void readDataNode();
Entry_Short* findEntryByName(Entry_Short *entry_short, uint32_t count_entry_short, uint16_t index);
void checkFile(Entry_Short *entry_short, uint32_t count_entry_short);
void callData();

#endif
