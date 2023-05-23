#include <stdio.h>
#include <stdlib.h>
#include "Read_FAT.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main() {
//      g_fp = fopen("file3.IMA", "rb");
//      g_fp = fopen("file1.img", "rb");
//      g_fp = fopen("file2.IMA", "rb");
      g_fp = fopen("file4.IMA", "rb");
    callData();
    fclose(g_fp);     
    return 0;
}
