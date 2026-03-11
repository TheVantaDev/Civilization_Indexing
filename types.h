// types.h
#ifndef TYPES_H
#define TYPES_H

typedef struct BehaviorNode {
    int record_id;
    int ai_system_id;
    char decision_type[128];
    char affected_group[128];
    char outcome[128];
    char transparency[16];   // "Low"/"Medium"/"High"
    struct BehaviorNode *next;
} BehaviorNode;

// BST for ethical principles
typedef struct PrincipleNode {
    int ethics_id;
    char civilization[64];
    char ethical_principle[128];
    char domain[64];
    double weight;
    BehaviorNode *behaviors;          // linked list of behaviors under this principle
    struct PrincipleNode *left;
    struct PrincipleNode *right;
} PrincipleNode;

typedef struct Culture {
    int culture_id;
    char region[64];
    char priority[32];       // "fairness", "privacy"...
    char sensitivity[16];    // "High"/"Medium"/"Low"
} Culture;

// Function declarations
PrincipleNode* find_principle(PrincipleNode *root, const char *principle);
PrincipleNode* insert_principle(PrincipleNode *root, PrincipleNode *node);
PrincipleNode* create_principle(int id, const char *civ, const char *principle, const char *domain, double weight);

#endif
