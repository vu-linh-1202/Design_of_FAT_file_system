#include "Read_FAT.h"

void callBootSector()
{
	g_boot = readBootSector();
	displayBoot(*g_boot);
}

FAT checkFAT()
{
    FAT fat;
    switch (g_boot->FAT_type[4])
    {
        case '2':
            fat = FAT12;
            break;
        case '6':
            fat = FAT16;
            break;
        default:
            fat = FAT32;
            break;
    }
    return fat;
}

uint16_t getFatValue(uint16_t previous_cluster) {
    uint16_t addressFAT;
    if (checkFAT() == FAT12) {
        addressFAT = getFatValue12(previous_cluster);
    } else if (checkFAT() == FAT16) {
        addressFAT = getFatValue16(previous_cluster);
    } else {
        addressFAT = FAT12_EOF;
    }
    return addressFAT;
}

/*
 * This function displays the contents of a file.
 */
uint32_t getDataFile(Entry_Short *entry_short, uint16_t index) {
    char full_name[13];
    uint16_t location_cluster = entry_short[index].first_cluster_low;
    uint16_t size = entry_short[index].file_size;
    void *buffer = malloc(g_boot->byte_per_sector);

    if ((entry_short[index].attribute & 0x10) == 0x10) {
        printf("\nThis is a directory\n");
        free(buffer);
        return 0;
    } else if ((entry_short[index].attribute & 0x08) == 0x08) {
        int i, j;
        for (i = 0; i < ENTRY_FILE_NAME_BYTE; i += 2) {
            if (entry_short[index].name[i] == 0x00) {
                break;
            }
            full_name[i] = entry_short[index].name[i];
        }
        full_name[i] = '.';
        for (j = 0; j < ENTRY_FILE_NAME_EXTENTION_BYTE; j += 2) {
            if (entry_short[index].extention[j] == 0x00) {
                break;
            }
            full_name[i + j + 1] = entry_short[index].extention[j];
        }
        full_name[i + j + 1] = '\0';
        printf("\nFile Opening: %s\n", full_name);
    } else {
        printf("\nFile Opening: ");
        print_Str(&entry_short[index].name[0], ENTRY_FILE_NAME_BYTE);
        print_Str(&entry_short[index].extention[0], ENTRY_FILE_NAME_EXTENTION_BYTE);
        printf("\n");
    }

    int offset;
    do {
        offset = ((location_cluster - 2) + g_before_data_sector) * g_boot->byte_per_sector;
        Shift_Offset(offset);
        readSector(offset / g_boot->byte_per_sector, buffer);
        if (size >= g_boot->byte_per_sector * g_boot->sector_per_cluster) {
            printByteFile( g_boot->byte_per_sector * g_boot->sector_per_cluster);
            size -= g_boot->byte_per_sector * g_boot->sector_per_cluster;
        } else {
            printByteFile(size);
            size = 0;
        }
        location_cluster = getFatValue(location_cluster);
    } while (location_cluster >= 0x002 && location_cluster <= 0xFEF && location_cluster != FAT12_EOF);

    free(buffer);
    return 1;
}

void readSector(uint16_t sectorNumber, void *buffer) {
    uint16_t offset = sectorNumber * g_boot->byte_per_sector;
    fseek(g_fp, offset, SEEK_SET);
    fread(buffer, 1, g_boot->byte_per_sector, g_fp);
}

void readDataNode() {
    uint32_t count_entry_short;
    Entry_Short *entry_short = readEntryShort(g_Root_offset, &count_entry_short);
    displayEntryShort(&entry_short[0], count_entry_short, 0);
    void *buffer = malloc(g_boot->byte_per_sector);
    readSector(g_Root_offset, buffer);
    checkFile(&entry_short[0], count_entry_short);
    free(buffer);
}

void checkFile(Entry_Short *entry_short, uint32_t count_entry_short)
{
    uint16_t check;
    uint16_t temp;
    uint16_t input;
    uint16_t offset_cluster;
    uint16_t status = 0;
    uint16_t flag = 0;

    Entry_Short *entry_short_subsidiary = entry_short;
    uint32_t count_entry_short_subsidiary = count_entry_short;
    uint32_t count_entry_copy = count_entry_short_subsidiary;

    while (true) {
        if (status == 1) {
            printf("\nSelect the function by entering directory number:\n");
            printf("\tx. Out program\n");
            printf("\tb. Back to previous folder\n");
            printf("\tn. Open file with with enter number.\n ");
            do {
                printf("Enter your selection: ");
                scanf(" %c", &input);
                fflush(stdin);
            } while ((input != 'x') && (input != 'b'));
            if (input == 'x') {
                return;
            }
            else if (input == 'b')
            {
                status = 0;
                displayEntryShort(&entry_short_subsidiary[0], count_entry_copy, 0);
            }
        }
        else 
        {
            printf("\nSelect the function or open the file by entering directory number:\n");
            printf("\tx. Out program\n");
            (isEmpty() != 1)?printf("\tb. Back to previous folder\n"):printf("");
            printf("\tn. Open file with with enter number.\n ");
            do {
                printf("Enter your selection: ");
                scanf(" %c", &input);
                fflush(stdin);
                ((isEmpty() == 1) && (input == 98))?(input = 47):printf("");
            } while(!isdigit(input) || ((input > (count_entry_copy + 47)) && (input != 'x' && input != 'b')));

            if (input == 'x')
			{
                return;
            }
            else if (input == 'b')
            {
                removeNode();

                if (isEmpty())
				{
                    count_entry_short_subsidiary = count_entry_short;
                    
                    entry_short_subsidiary = entry_short;
                    displayEntryShort(&entry_short_subsidiary[0], count_entry_short_subsidiary, 0);
                    count_entry_copy = count_entry_short_subsidiary;
                }
				else
				{
                    offset_cluster = ((g_pHead->entry_short.first_cluster_low - 2) + g_before_data_sector) * g_boot->byte_per_sector;
                    
                    entry_short_subsidiary = readEntryInCluster(offset_cluster, &count_entry_short_subsidiary, g_pHead->entry_short.first_cluster_low);
                    displayEntryShort(&entry_short_subsidiary[0], count_entry_copy, 0);
                    count_entry_copy = count_entry_short_subsidiary;
                }
            }
            else
            {
                temp = (uint16_t)input - '0';
                check = (entry_short_subsidiary[temp].attribute != 0x10);
                if (check)
                {
                    status = getDataFile(&entry_short_subsidiary[0], temp);
                }
                else
                {
                	
                	// this element have issue---------------------------------------------------------------------------
                    createNode(&entry_short_subsidiary[temp]);
                    offset_cluster = ((g_pHead->entry_short.first_cluster_low - 2) + g_before_data_sector) * g_boot->byte_per_sector; 
    
                    entry_short_subsidiary = readEntryInCluster(offset_cluster, &count_entry_short_subsidiary, g_pHead->entry_short.first_cluster_low);
                    displayEntryShort(&entry_short_subsidiary[0], count_entry_short_subsidiary, 0);
                    count_entry_copy = count_entry_short_subsidiary;
                    
                    printf("\nEnter the number of the file to open, or enter 'b' to go back: ");
                    do {
                        scanf(" %c", &input);
                        fflush(stdin);
                    } while (!isdigit(input) || (input > (count_entry_copy + 47)) && (input != 'b'));
                    if (input == 'b')
					{
                        removeNode();
                        status = 0;
                    } 
					else
					{
                        temp = (uint16_t)input - '0';
                        if (entry_short_subsidiary[temp].attribute == 0x10)
						{
                            createNode(&entry_short_subsidiary[temp]);
                            offset_cluster = ((g_pHead->entry_short.first_cluster_low - 2) + g_before_data_sector) * g_boot->byte_per_sector;
                            entry_short_subsidiary = readEntryInCluster(offset_cluster, &count_entry_short_subsidiary, g_pHead->entry_short.first_cluster_low);
                            displayEntryShort(&entry_short_subsidiary[0], count_entry_short_subsidiary, 1);
                            count_entry_copy = count_entry_short_subsidiary;
                        }
						else
						{
                            status = getDataFile(&entry_short_subsidiary[0], temp);
                            removeNode();
                        }
                    }
                }
            }
        }
    }
}

void callData()
{
	callBootSector();
	initList();
	if(checkFAT() != FAT32)
	{
		main_offset();
		readDataNode();
	}
	else
	{
		printf("Can't call data!'");
	}
}

