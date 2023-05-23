#ifndef __FAT_SYSTEM_H__
#define __FAT_SYSTEM_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>


/******************************************************************/ 

/************* Shift To Offset **************/
#define Shift_Offset(x)          fseek(g_fp, x, 0)
#define offset_Root(SF, NF, BF)  (((SF*NF)+BF)*0x200)

#define MAX_CLUSTER 4084
#define EOC 0xFF8
   
/******************** Offset Boot Sector *************************/
/* File System Indentifies ------------ 8 BYTE */
#define BOOT_FAT_TYPE                   (0x36U)
#define BOOT_FAT_TYPE_BYTE              (0x8U)

/* Number of FAT ----------------------- 1 BYTE*/
#define BOOT_NUMBER_FAT                 (0x10U)
#define BOOT_NUMBER_FAT_BYTE            (0x1U)

/*Number of Sector of FAT--------------- 2 BYTE*/
#define BOOT_SECTOR_PER_FAT             (0x16U)
#define BOOT_SECTOR_PER_FAT_BYTE         (0x2U)

/* Number of reserved sector (Bef FAT).- 2 BYTE*/
#define BOOT_RESERVED_SECTOR_COUNT      (0x0eU)
#define BOOT_RESERVED_SECTOR_COUNT_BYTE (0x2U)

/*Number of Root Directory-------------- 2 BYTE*/
#define BOOT_ROOT_ENTRY_COUNT            (0x11U)
#define BOOT_ROOT_ENTRY_COUNT_BYTE        (0x2U)

/*Number of Bytes per sector ----------- 2 BYTE*/
#define BOOT_BYTE_PER_SECTOR             (0x0bU)
#define BOOT_BYTE_PER_SECTOR_BYTE        (0x2U)

/* Number of blocks Per Cluster -------- 1 BYTE*/
#define BOOT_SECTOR_PER_CLUSTER          (0x0dU)
#define BOOT_SECTOR_PER_CLUSTER_BYTE     (0x1U)

/*Boot Sector Mark End Boot -------------2 BYTE*/
#define BOOT_END_BOOT_SECTOR             (0x1FEU)
#define BOOT_END_BOOT_SECTOR_BYTE        (0x2U)

#define ATTR_LONG_NAME                    0x0F
#define MAX_LONG_NAME_LENGTH 255 
#define LONG_NAME_ENTRY_CHARS 13 
#define LAST_LONG_ENTRY                   0x40
/****************************** Boot Sector *******************************/

/************************ Offset Short Name Entry ***********************/
/*File name */
#define    ENTRY_FILE_NAME                    (0X00U)
#define    ENTRY_FILE_NAME_BYTE               (0x08U)

/* File Extension */
#define    ENTRY_FILE_NAME_EXTENTION          (0X08U)
#define    ENTRY_FILE_NAME_EXTENTION_BYTE     (0x03U)

/* File Attribute */
#define    ENTRY_FILE_ATTRIBUTE               (0X0bU)
#define    ENTRY_FILE_ATTRIBUTE_BYTE          (0x01U)

/* First Cluster of A File (High) */
#define    ENTRY_START_CLUSTER_HIGH           (0x14U)
#define    ENTRY_START_CLUSTER_HIGH_BYTE      (0x02U)

/* File Lastest Update Time */
#define    ENTRY_TIME_CREATE_UPDATE           (0x16U)
#define    ENTRY_TIME_CREATE_UPDATE_BYTE      (0x02U)

/* File Lastest Update Date*/
#define    ENTRY_DATE_CREATE_UPDATE           (0x18U)
#define    ENTRY_DATE_CREATE_UPDATE_BYTE      (0x02U)

/* First Cluster of A File (Low) */
#define    ENTRY_START_CLUSTER_LOW            (0x1aU)
#define    ENTRY_START_CLUSTER_LOW_BYTE       (0x02U)

/* File Size */
#define    ENTRY_SIZE_FILE                    (0x1cU)
#define    ENTRY_SIZE_FILE_BYTE               (0x04U)

#define ENTRY_FLAG                            (0x0bU)
#define ENTRY_FLAG_BYTE                       (0x01U)
/*********************** Offset Short Name Entry ***********************/



/*************************** Entry In FAT ****************************/
/*FAT 12*/
#define FAT12_FREE                 (0x000U)
#define FAT12_ERROR                (0xff7U)
#define FAT12_EOF                  (0xfffU)
/*FAT 16*/
#define FAT16_FREE                 (0x0000U)
#define FAT16_ERROR                (0xfff7U)
#define FAT16_EOF                  (0xffffU)
/*FAT 32*/
#define FAT32_FREE                 (0x00000000U)
#define FAT32_ERROR                (0x0ffffff7U)
#define FAT32_EOF                  (0x0fffffffU)                    
/*************************** Entry In FAT **************************/


/************* STRUCT *************/
typedef enum bool
{
    true=1, false=0
}bool;

typedef struct Boot_Sector
{
    char FAT_type[8];
    char number_FAT;
    int sector_per_FAT;
    int reserved_sector_count;
    int root_entry_count;
    int byte_per_sector;
    char sector_per_cluster;
    char first_data_sector;
}Boot_Sector;

typedef enum FAT
{
    FAT12 = 1,
    FAT16 = 2,
    FAT32 = 3    
}FAT;

/* Entry main in Root Directory*/
typedef struct Entry_Main_Root
{
    char name[8];
    char extention[3];
    char attribute;
    int first_cluster_high;
    int timeUpdate;
    int dateUpdate;
    int first_cluster_low;
    int file_size[1024];
} Entry_Short;

/******************************* Declared Functions *******************************/
uint16_t getFat(uint32_t offset, uint16_t previous_cluster, uint8_t fat_type);
uint16_t getFatValue12(uint16_t previous_cluster);
uint16_t getFatValue16(uint16_t previous_cluster);

void getBootSector(Boot_Sector *boot);
Boot_Sector *readBootSector();
void displayBoot(Boot_Sector boot);

void getShortEntry(Entry_Short *entry_short, uint32_t offset);
void countEntryShort(uint32_t offset, uint32_t *count_entry_short);
Entry_Short *readEntryShort(uint32_t offset, uint32_t *count_entry_short);
void countEntryShortCluster(uint32_t offset, uint32_t *count_entry_short, uint16_t location_cluster);
Entry_Short *readEntryInCluster(uint32_t offset, uint32_t *count_entry_short, uint16_t location_cluster);
void displayEntryShort(Entry_Short *entry_short, uint32_t count_entry_short, uint16_t start_index);

void print_Str(uint8_t str[],uint32_t size );
void printByteFile(int n);
void readDate(uint16_t date);
void readTime(uint16_t time);
int offset_cluster(uint16_t first_cluster_low);
void main_offset();
/********************************** End Declared ********************************/

/********************************* LINKED LIST **********************************/
typedef struct Node
{
    Entry_Short entry_short;
    struct Node *pNext;
} Node;

/************** Declared Node **************/
void initList();
bool isEmpty();
void addNodeToList(Node *node);
void removeNode();
uint32_t createNode(Entry_Short *entry_short);
void printLinkedList(Node *pHead);
/********************************* LINKED LIST **********************************/

/***************global_variable***************/ 
FILE *g_fp; //FILE pointer
Boot_Sector *g_boot;
Node *g_pHead; 

uint32_t g_Root_offset;
uint32_t g_before_data_sector;
/***************global_variable***************/ 
#endif
