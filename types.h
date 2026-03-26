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

typedef struct AISystem {
    int ai_system_id;
    char system_name[128];
    char application_domain[128];
    char developer[128];
    char deployment_region[64];
} AISystem;

typedef struct RiskSummary {
    char risk_category[64];
    char risk_level[16];
    double score;            // 0.0 to 1.0
} RiskSummary;

// Function declarations
PrincipleNode* find_principle(PrincipleNode *root, const char *principle);
PrincipleNode* insert_principle(PrincipleNode *root, PrincipleNode *node);
PrincipleNode* create_principle(int id, const char *civ, const char *principle, const char *domain, double weight);

PrincipleNode* load_ethics(const char *filename);
int load_cultures(const char *filename, Culture *arr, int max);
void load_behaviors(const char *filename, PrincipleNode *ethics_root);
void free_ethics_tree(PrincipleNode *root);

double compute_principle_alignment_for_ai(PrincipleNode *root, int ai_id, const char *principle_name);
double compute_alignment_for_ai(PrincipleNode *root, int ai_id);
double compute_culture_compatibility_for_ai(PrincipleNode *root, int ai_id, const Culture *culture);
double compute_overall_cultural_compatibility(PrincipleNode *root, int ai_id, const Culture *cultures, int culture_count);
int evaluate_risks_for_ai(PrincipleNode *root, int ai_id, RiskSummary *out, int max_out);

// Visualization functions
void print_ascii_chart(const char *title, const char *labels[], const double values[], int count);
void print_comparison_chart(const char *title, const char *ai_names[], const double ethical[], const double cultural[], const double overall[], int count);
void print_risk_chart(const char *ai_name, const RiskSummary risks[], int risk_count);
int generate_html_dashboard(const char *filename, const AISystem *systems, const double ethical[], const double cultural[], const double overall[], int system_count);
int generate_gnuplot_script(const char *filename, const AISystem *systems, const double scores[], int system_count);

#endif
