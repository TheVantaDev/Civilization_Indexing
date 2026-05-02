#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

// Load cultures from CSV file into array
int load_cultures(char *filename, Culture *arr, int max) {
    FILE *fp=fopen(filename, "r");
    if (!fp) {
        perror("Failed to open culture.csv");
        return 0;
    }
    
    char line[512];
    int count=0;
    // skip header
    fgets(line, sizeof(line), fp);
    
    while (fgets(line, sizeof(line), fp) && count<max) {
        int culture_id;
        char region[64], priority[32], sensitivity[16];
        
        if (sscanf(line, "%d,%63[^,],%31[^,],%15s",
                   &culture_id, region, priority, sensitivity)==4) {
            arr[count].culture_id=culture_id;
            strcpy(arr[count].region, region);
            strcpy(arr[count].priority, priority);
            strcpy(arr[count].sensitivity, sensitivity);
            count++;
        }
    }
    fclose(fp);
    return count;
}
