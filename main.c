#include <stdio.h>
#include "types.h"

// function declarations
PrincipleNode* load_ethics(const char *filename);
int load_cultures(const char *filename, Culture *arr, int max);
void load_behaviors(const char *filename, PrincipleNode *ethics_root);
double compute_alignment_for_ai(PrincipleNode *root, int ai_id);

int main(void) {
    PrincipleNode *ethics_root=load_ethics("ethics.csv");
    if (!ethics_root) {
        printf("Failed to load ethics.csv\n");
        return 1;
    }

    Culture cultures[10];
    int culture_count=load_cultures("culture.csv", cultures, 10);

    printf("Loaded %d cultures.\n", culture_count);

    load_behaviors("ai_behavior.csv", ethics_root);

    int ai_ids[]={101, 102, 103};
    int n=sizeof(ai_ids)/sizeof(ai_ids[0]);

    for (int i=0; i<n; i++) {
        double align=compute_alignment_for_ai(ethics_root, ai_ids[i]);
        printf("AI System %d: Ethical Alignment Score = %.2f\n", ai_ids[i], align);
    }

    return 0;
}
