#ifndef    __FAT_SYSTEM_C__
#define    __FAT_SYSTEM_C__
#include "FAT_system.h"

void initList()
{
    g_pHead = NULL;
}

bool isEmpty()
{
    if(g_pHead == NULL)
    {
        return true;
    }
    return false;
}

uint32_t createNode(Entry_Short *entry_short)
{
    uint16_t i;
    Node *node = (Node*)calloc(1, sizeof(Node));
    addNodeToList(node);
    for(i = 0; i < ENTRY_FILE_NAME_BYTE; i++)
    {
        (node->entry_short).name[i] = entry_short[0].name[i];
    }
    (node->entry_short).first_cluster_low = entry_short[0].first_cluster_low;
    printLinkedList(g_pHead);
}

void addNodeToList(Node *node)
{
    if(isEmpty())
    {
        g_pHead = node;
        g_pHead->pNext = NULL;
    }
    else
    {
        node->pNext = g_pHead;
        g_pHead = node;
    }
}

void removeNode()
{
    Node *last_node = g_pHead;
    if(isEmpty())
    {
        printf("Can't delete, because list is empty.\n'");
    }
    else if(g_pHead->pNext == NULL)
    {
        free(last_node);
        g_pHead = NULL;
    }
    else
    {
        g_pHead = g_pHead->pNext;
    }
    printLinkedList(g_pHead);
}


void printLinkedList(Node *g_pHead)
{
    if(g_pHead != NULL)
    {
        printf("\nFile opening: %s", (g_pHead->entry_short).name);
        printf("\n");
    }
}

/*************************** Boot Sector - Start ***********************************/

/*
This function used to reads information from the boot sector and 
stores it into a predefined boot sector structure. 
This boot sector structure contains important information about the FAT file system
*/
void getBootSector(Boot_Sector *g_boot)
{
    uint8_t buffer[32];
    Shift_Offset(BOOT_FAT_TYPE);
    fread(buffer, 1, BOOT_FAT_TYPE_BYTE, g_fp);
    memcpy(g_boot->FAT_type, buffer, BOOT_FAT_TYPE_BYTE);
    
    Shift_Offset(BOOT_NUMBER_FAT);
    g_boot->number_FAT = fgetc(g_fp);
    
    Shift_Offset(BOOT_SECTOR_PER_FAT);
    fread(buffer, 1, BOOT_SECTOR_PER_FAT_BYTE, g_fp);
    g_boot->sector_per_FAT = buffer[0] + (buffer[1] << 8);

    Shift_Offset(BOOT_RESERVED_SECTOR_COUNT);
    fread(buffer, 1, BOOT_RESERVED_SECTOR_COUNT_BYTE, g_fp);
    g_boot->reserved_sector_count = buffer[0] + (buffer[1] << 8);

    Shift_Offset(BOOT_ROOT_ENTRY_COUNT);
    fread(buffer, 1, BOOT_ROOT_ENTRY_COUNT_BYTE, g_fp);
    g_boot->root_entry_count = buffer[0] + (buffer[1] << 8);

    Shift_Offset(BOOT_BYTE_PER_SECTOR);
    fread(buffer, 1, BOOT_BYTE_PER_SECTOR_BYTE, g_fp);
    g_boot->byte_per_sector = buffer[0] + (buffer[1] << 8);

    Shift_Offset(BOOT_SECTOR_PER_CLUSTER);
    g_boot->sector_per_cluster = fgetc(g_fp);
}

void displayBoot(Boot_Sector g_boot)
{
    printf("------------------ CALL BOOT SECTOR ------------------\n");
    print_Str(g_boot.FAT_type,BOOT_FAT_TYPE_BYTE);
    printf("\n");
    printf("Byte_per_sector:\t%d\n",g_boot.byte_per_sector);
    printf("Sector_per_cluster:\t%d\n",g_boot.sector_per_cluster);
    printf("Reserved_sector_count:\t%d\n",g_boot.reserved_sector_count);
    printf("Number_FAT:\t%d\n",g_boot.number_FAT);
    printf("Root_entry_count:\t%d\n",g_boot.root_entry_count);
    printf("Sector_per_FAT:\t%d\n",g_boot.sector_per_FAT);
}

Boot_Sector *readBootSector()
{
    Boot_Sector *g_boot = (Boot_Sector*)calloc(1, sizeof(Boot_Sector));
    getBootSector(g_boot);
    return g_boot;
}
/*********************** BOOT SECTOR END ****************************/

/************************ FAT- START ****************************************/

/* This function used to get value of FAT entry 
The getFat() function accepts offset (starting position of FAT table), previous_cluster (previous cluster) 
and FAT type (12 or 16). The function uses data read functions such as fgetc() 
to read data from the FAT table and calculate the new value of the previous_cluster's FAT entry 
based on the FAT type. The end result is the new FAT entry value of previous_cluster.

the function reads the two bytes of data starting from that offset using the fgetc function, and stores them in the dataOfIndex array
*/


uint16_t getFat(uint32_t offset, uint16_t previous_cluster, uint8_t fat_type)
{
    uint16_t newValue;
    uint16_t high;
    uint16_t low;

    if (fat_type == 12) {
        Shift_Offset(512 + (previous_cluster * 3) / 2);

        unsigned char dataOfIndex[2];
        dataOfIndex[0] = fgetc(g_fp);
        dataOfIndex[1] = fgetc(g_fp);
        high = dataOfIndex[1];
        low = dataOfIndex[0];

        if (previous_cluster % 2 == 0) {
            newValue = (low | ((high & 0xF) << 8));
        }
        else
        {
            newValue = ((high << 4) | (low >> 4));
        }
    }
    else if(fat_type == 16)
    {
        Shift_Offset(512 + (previous_cluster * 2));

        unsigned char dataOfIndex[2];
        dataOfIndex[0] = fgetc(g_fp);
        dataOfIndex[1] = fgetc(g_fp);
        high = dataOfIndex[1];
        low = dataOfIndex[0];

        newValue = (high << 8) | low;
    }

    return newValue;
}

uint16_t getFatValue12(uint16_t previous_cluster)
{
    return getFat(512, previous_cluster, 12);
}

uint16_t getFatValue16(uint16_t previous_cluster)
{
    return getFat(0, previous_cluster, 16);
}
/*********************** FAT END  **************************/

/*********************** ROOT DIRECTORY/ENTRY***************/
/* Get entry attribute*/

/*
The getShortEntry() function is used to read information about a short entry in the directory.
 This function takes a pointer to a variable of type Entry_Short and offset to determine the starting position of the entry in the directory. 
 The function reads the fields of the entry (filename, extension, attribute, time, date, first cluster) 
 and stores it in the Entry_Short variable.
*/
void getShortEntry(Entry_Short *entry_short, uint32_t offset)
{
    uint8_t buffer[32];
    Shift_Offset(ENTRY_FILE_NAME + offset);
    fread(buffer, 1, ENTRY_FILE_NAME_BYTE, g_fp);
    memcpy(entry_short->name, buffer, ENTRY_FILE_NAME_BYTE);

    Shift_Offset(offset + ENTRY_FILE_NAME_EXTENTION);
    fread(buffer, 1, ENTRY_FILE_NAME_EXTENTION_BYTE, g_fp);
    memcpy(entry_short->extention, buffer, ENTRY_FILE_NAME_EXTENTION_BYTE);

    Shift_Offset(offset + ENTRY_FILE_ATTRIBUTE);
    entry_short->attribute = fgetc(g_fp);

    Shift_Offset(offset + ENTRY_START_CLUSTER_HIGH);
    fread(buffer, 1, ENTRY_START_CLUSTER_HIGH_BYTE, g_fp);
    entry_short->first_cluster_high = buffer[0] + (buffer[1] << 8);

    Shift_Offset(offset + ENTRY_TIME_CREATE_UPDATE);
    fread(buffer, 1, ENTRY_TIME_CREATE_UPDATE_BYTE, g_fp);
    entry_short->timeUpdate = buffer[0] + (buffer[1] << 8);

    Shift_Offset(offset + ENTRY_DATE_CREATE_UPDATE);
    fread(buffer, 1, ENTRY_DATE_CREATE_UPDATE_BYTE, g_fp);
    entry_short->dateUpdate = buffer[0] + (buffer[1] << 8);

    Shift_Offset(offset + ENTRY_START_CLUSTER_LOW);
    fread(buffer, 1, ENTRY_START_CLUSTER_LOW_BYTE, g_fp);
    entry_short->first_cluster_low = buffer[0] + (buffer[1] << 8);
}

/*
The countEntryShort() function is used to count the number of short entries in a directory. 
This function takes an offset to determine the starting position of the directory and 
a pointer to the counter variable count_entry_short
*/
void countEntryShort(uint32_t offset, uint32_t *count_entry_short)
{
    *count_entry_short = 0;
    uint32_t entry_location = 0;
    uint32_t temp = 1;

    while ((entry_location < g_boot->root_entry_count) && (temp != 0))
    {
        Shift_Offset(offset + 0x20 * entry_location);
        temp = fgetc(g_fp);

        if ((temp != 0) && (fgetc(g_fp) != 0x00))
        {
            (*count_entry_short)++;
        }

        entry_location++;
    }
}

/*
The readEntryShort() function is used to read all short entries in the directory. 
This function takes an offset to determine the starting position of the directory and 
a pointer to the counter variable count_entry_short. 
The function uses the countEntryShort() function to count the number of short entries in the directory 
and then dynamically allocates memory and stores information about the short entries into variables of type Entry_Short. 
The function returns a pointer to the array of Entry_Short
*/
Entry_Short *readEntryShort(uint32_t offset, uint32_t *count_entry_short)
{
    uint32_t count_cluster_root = g_boot->root_entry_count;
    uint32_t entry_location = 0;
    uint16_t i; 
    Entry_Short *entry_short;
    *count_entry_short = 0;

    countEntryShort(offset, count_entry_short);

    if (*count_entry_short > 0)
    {
        entry_short = (Entry_Short*)malloc(*count_entry_short * sizeof(Entry_Short));

        for ( i = 0; i < count_cluster_root; i++)
        {
            Shift_Offset(offset + 0x20 * entry_location);
            unsigned char temp = fgetc(g_fp);

            while ((temp != 0) && (fgetc(g_fp) == 0x0f))
            {
                entry_location++;
                Shift_Offset(offset + 0x20 * entry_location);
                temp = fgetc(g_fp);
            }

            if (temp != 0)
            {
                getShortEntry(&entry_short[i], offset + 0x20 * entry_location);
                entry_location++;
            }
        }
    }
    else
    {
        printf("File is empty!\n");
        entry_short = NULL;
    }
    return entry_short;
}

void countEntryShortCluster(uint32_t offset, uint32_t *count_entry_short, uint16_t location_cluster)
{
    *count_entry_short = 0;
    uint32_t entry_location = 0;
    uint16_t temp = 1;
    uint16_t i; 

     do{
        Shift_Offset(offset + (location_cluster * 3) / 2);
        for ( i = 0; i < 16; i++)
        {
            Shift_Offset(offset + 0x20 * entry_location);
            temp = fgetc(g_fp);
            while ((temp != 0) && (fgetc(g_fp) == 0x0F))
            {
                entry_location++;
                Shift_Offset(offset + 0x20 * entry_location);
                temp = fgetc(g_fp);
            }
            if (temp != 0)
            {
                (*count_entry_short)++;
            }
            entry_location++;
        }
        location_cluster = getFatValue12(location_cluster);
    }while (location_cluster != FAT12_EOF);
}

Entry_Short *readEntryInCluster(uint32_t offset, uint32_t *count_entry_short, uint16_t location_cluster)
{
    uint16_t index_entry = 0;
    uint32_t count_cluster_root = g_boot->root_entry_count;
    uint32_t entry_location = 0; 
    uint16_t flag;
    uint16_t temp = 1;
    Entry_Short *entry_short;
    countEntryShortCluster(offset, count_entry_short, location_cluster);
    
    if(*count_entry_short > 0)
    {
        entry_short = (Entry_Short*)malloc((*count_entry_short)*sizeof(Entry_Short));
        do
        {
            while((entry_location < (16)) && (temp != 0))
            {
                entry_location++;
                Shift_Offset(offset + 0x20 * entry_location);
                temp = fgetc(g_fp);
                if(temp != 0)
                {
                    getShortEntry(&entry_short[index_entry], offset + 0x20 * entry_location);
                    index_entry++;
                }
            }
            location_cluster = getFatValue12(location_cluster);
        }while(location_cluster != FAT12_EOF);
    }
    else
    {
        printf("File is empty!\n");
        entry_short = NULL;
    }
    return entry_short;
}

void displayEntryShort(Entry_Short *entry_short, uint32_t count_entry_short, uint16_t start_index)
{
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-13s | %-30s | %-10s | %-10s | %-15s | %-15s |\n", "File Directory", "Name", "Type", "Short name", "Date Modified", "Time Modified");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    uint16_t i = 1;
    do {
        if (entry_short[i].attribute != 0x0F && entry_short[i].name[0] != 0xE5 )
        {
            printf("| %-13d | %-30s | %-10s |", i, entry_short[i].name, (entry_short[i].attribute == 0x10 ? "Folder" : "File"));
            print_Str(&entry_short[i].name[0], ENTRY_FILE_NAME_BYTE);
            printf(".");
            print_Str(&entry_short[i].extention[0], ENTRY_FILE_NAME_EXTENTION_BYTE);
            printf("  ");
            readDate(entry_short[i].dateUpdate);
            printf("  ");
            readTime(entry_short[i].timeUpdate);
            printf("|\n");
        }
        i++;
    } while (i < count_entry_short);

    if (count_entry_short && i == 0) {
        printf("| %-13s | %-30s | %-10s | %-10s | %-15s | %-15s |\n", "", "No files or folders found", "", "", "", "");
        printf("---------------------------------------------------------------------------------------------------------------------\n");
        return;
    }

    printf("---------------------------------------------------------------------------------------------------------------------\n");
}

void print_Str(uint8_t str[], uint32_t size)
{
    uint16_t i;
    for( i = 0; i < size; i++)
    {
        printf("%c", str[i]);
    }
}

void readDate(uint16_t date)
{
    uint16_t day_binary = 0x001f;
    uint16_t month_binary = 0x01e0;
    uint16_t year_binary = 0xfe00;
    
    uint16_t day = (date & day_binary);
    uint16_t month = (date & month_binary) >> 5;
    uint16_t year = ((date & year_binary) >> 9) + 1980;
    
    printf("Date : %d/%d/%d   ", year, month, day);
}

void readTime(uint16_t time)
{
    uint16_t seconds_binary = 0x001f;
    uint16_t minutes_binary = 0x07e0;
    uint16_t hours_binary = 0xf800;
    
    uint16_t seconds = (time & seconds_binary);
    uint16_t minutes = (time & minutes_binary) >> 5;
    uint16_t hours = ((time & hours_binary) > 11) * 2;
    
    printf("Time : %d:%d:%d", hours, minutes, seconds);
}

void printByteFile(int n)
{
    int i;
    for( i = 0; i < n; i++)
    {
        printf("%c", fgetc(g_fp));
    }
}

int offset_cluster(uint16_t first_cluster_low)
{
    return ((first_cluster_low - 2) + 0x21)* 0x200 + 32 * 2;
}

void main_offset()
{
    g_Root_offset = ((g_boot->sector_per_FAT * g_boot->number_FAT) + g_boot->reserved_sector_count) * g_boot->byte_per_sector;
    g_before_data_sector = ((g_boot->sector_per_FAT * g_boot->number_FAT) + g_boot->reserved_sector_count) + (g_boot->root_entry_count * 32/g_boot->byte_per_sector);
}

#endif
