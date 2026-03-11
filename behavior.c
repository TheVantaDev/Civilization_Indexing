#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

// attach to a principle by domain or by some rule
void attach_behavior_to_principle(PrincipleNode *root, BehaviorNode *b) {
    // For demo, attach all recruitment-like decisions to "Dharma (Duty & fairness)"
    // In real code, you can map using domain or priority.
    PrincipleNode *p=find_principle(root, "Dharma (Duty & fairness)");
    if (!p) return;

    b->next=p->behaviors;
    p->behaviors=b;
}

BehaviorNode* create_behavior(int record_id, int ai_id,
                              const char *decision,
                              const char *group,
                              const char *outcome,
                              const char *trans) {
    BehaviorNode *b=malloc(sizeof(BehaviorNode));
    b->record_id=record_id;
    b->ai_system_id=ai_id;
    strcpy(b->decision_type, decision);
    strcpy(b->affected_group, group);
    strcpy(b->outcome, outcome);
    strcpy(b->transparency, trans);
    b->next=NULL;
    return b;
}

void load_behaviors(const char *filename, PrincipleNode *ethics_root) {
    FILE *fp=fopen(filename, "r");
    if (!fp) {
        perror("Failed to open ai_behavior.csv");
        return;
    }
    char line[512];
    fgets(line, sizeof(line), fp); // header

    while (fgets(line, sizeof(line), fp)) {
        int rec_id, ai_id;
        char decision[128], group[128], outcome[128], trans[16];
        if (sscanf(line, "%d,%d,%127[^,],%127[^,],%127[^,],%15s",
                   &rec_id, &ai_id, decision, group, outcome, trans)==6) {
            BehaviorNode *b=create_behavior(rec_id, ai_id, decision, group, outcome, trans);
            attach_behavior_to_principle(ethics_root, b);
        }
    }
    fclose(fp);
}
