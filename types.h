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

typedef struct BehaviorRef {
    int ai_system_id;
    struct PrincipleNode *principle;
    BehaviorNode *behavior;
    struct BehaviorRef *next;
} BehaviorRef;

// BST for ethical principles
typedef struct PrincipleNode {
    int ethics_id;
    char civilization[64];
    char ethical_principle[128];
    char domain[64];
    double weight;
    BehaviorNode *behaviors;          // linked list of behaviors under this principle
    int height;
    int total_behavior_count;
    int high_bias_count;
    int medium_transparency_count;
    int low_transparency_count;
    int fairness_hits_count;
    double sum_alignment;
    struct PrincipleNode *thread_prev;
    struct PrincipleNode *thread_next;
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
PrincipleNode* find_principle(PrincipleNode *root, char *principle);
PrincipleNode* insert_principle(PrincipleNode *root, PrincipleNode *node);
PrincipleNode* create_principle(int id, char *civ, char *principle, char *domain, double weight);

PrincipleNode* load_ethics(char *filename);
int load_cultures(char *filename, Culture *arr, int max);
void load_behaviors(char *filename, PrincipleNode *ethics_root);
void free_ethics_tree(PrincipleNode *root);

void reset_behavior_index(void);
BehaviorRef* get_behavior_refs_for_ai(int ai_id);
void free_behavior_refs(BehaviorRef *head);
int append_live_behaviors_from_ollama(char *behavior_csv, char *ai_systems_csv, char *model, int max_rows);

double compute_principle_alignment_for_ai(PrincipleNode *root, int ai_id, char *principle_name);
double compute_alignment_for_ai(PrincipleNode *root, int ai_id);
double compute_culture_compatibility_for_ai(PrincipleNode *root, int ai_id, Culture *culture);
double compute_overall_cultural_compatibility(PrincipleNode *root, int ai_id, Culture *cultures, int culture_count);
int evaluate_risks_for_ai(PrincipleNode *root, int ai_id, RiskSummary *out, int max_out);

// Visualization functions
void print_ascii_chart(char *title, char *labels[], double values[], int count);
void print_comparison_chart(char *title, char *ai_names[], double ethical[], double cultural[], double overall[], int count);
void print_risk_chart(char *ai_name, RiskSummary risks[], int risk_count);
int generate_html_dashboard(char *filename, AISystem *systems, double ethical[], double cultural[], double overall[], int system_count);
int generate_gnuplot_script(char *filename, AISystem *systems, double scores[], int system_count);

// JSON export (feeds the HTML dashboard)
int export_all_to_json(const char *filename,
                       PrincipleNode *root,
                       AISystem      *systems,   int system_count,
                       Culture       *cultures,  int culture_count,
                       double        *overall_scores);

#endif
