#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static int contains_any(const char *text, const char **keys, int n) {
    int i;
    for (i=0; i<n; i++) {
        if (strstr(text, keys[i])) return 1;
    }
    return 0;
}

static int should_attach(const PrincipleNode *p, const BehaviorNode *b) {
    const char *fairness_keys[] = {"hiring", "candidate", "screen", "rank", "recruit", "loan", "credit"};
    const char *compassion_keys[] = {"minority", "underrepresented", "patient", "student", "welfare"};
    const char *harmony_keys[] = {"recommend", "learning", "govern", "social", "community"};
    const char *justice_keys[] = {"justice", "law", "diagnosis", "screen", "equal"};
    const char *virtue_keys[] = {"behavior", "ranking", "decision", "review", "personal"};
    const char *environment_keys[] = {"environment", "sustain", "carbon", "resource", "waste"};

    if (strstr(p->ethical_principle, "Dharma") || strstr(p->ethical_principle, "fairness")) {
        if (contains_any(b->decision_type, fairness_keys, 7) || strstr(b->outcome, "Rejected") || strstr(b->outcome, "Biased")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Compassion")) {
        if (contains_any(b->affected_group, compassion_keys, 5) || strstr(b->outcome, "Rejected")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Social harmony")) {
        if (contains_any(b->decision_type, harmony_keys, 5) || strstr(b->affected_group, "All")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Justice") || strstr(p->ethical_principle, "Adl")) {
        if (contains_any(b->decision_type, justice_keys, 5) || strstr(b->affected_group, "applicant") || strstr(b->affected_group, "Patients")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Virtue")) {
        if (contains_any(b->decision_type, virtue_keys, 5)) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Environmental")) {
        if (contains_any(b->decision_type, environment_keys, 5) || contains_any(b->outcome, environment_keys, 5)) {
            return 1;
        }
    }

    return 0;
}

static BehaviorNode* clone_behavior(const BehaviorNode *src) {
    BehaviorNode *b=(BehaviorNode*)malloc(sizeof(BehaviorNode));
    if (!b) return NULL;
    *b=*src;
    b->next=NULL;
    return b;
}

static void attach_behavior_recursive(PrincipleNode *root, const BehaviorNode *b) {
    BehaviorNode *copy;
    if (!root) return;

    attach_behavior_recursive(root->left, b);

    if (should_attach(root, b)) {
        copy=clone_behavior(b);
        if (copy) {
            copy->next=root->behaviors;
            root->behaviors=copy;
        }
    }

    attach_behavior_recursive(root->right, b);
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

        // Ignore incompatible lines from external datasets accidentally appended.
        if (line[0]<'0' || line[0]>'9') {
            continue;
        }

        if (sscanf(line, "%d,%d,%127[^,],%127[^,],%127[^,],%15s",
                   &rec_id, &ai_id, decision, group, outcome, trans)==6) {
            BehaviorNode *b=create_behavior(rec_id, ai_id, decision, group, outcome, trans);
            if (!b) continue;
            attach_behavior_recursive(ethics_root, b);
            free(b);
        }
    }
    fclose(fp);
}
