#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "types.h"

/* 
 * This is a stubbed version of db.c that does not require MySQL.
 * It allows the project to run in CSV-only mode.
 */

int db_connect(const char *host, unsigned int port, const char *user, const char *password, const char *dbname) {
    printf("[DB] Running in CSV-only mode (MySQL support disabled for this run).\n");
    return 0; /* Return 0 to trigger CSV-only fallback in main.c */
}

void db_close(void) {}

int db_upsert_ai_systems(AISystem *systems, int count) { return 0; }
int db_insert_behavior(int ai_system_id, const char *decision_type, const char *affected_group, const char *outcome, const char *transparency, const char *source) { return 0; }
int db_insert_intermediate_scoring(int ai_system_id, const char *system_name, const char *ethical_principle, double alignment_score) { return 0; }
int db_insert_cultural_compatibility(int ai_system_id, const char *system_name, const char *region, double compatibility_score) { return 0; }
int db_insert_risk_detection(int ai_system_id, const char *system_name, const char *risk_category, const char *risk_level, double risk_score) { return 0; }
int db_insert_final_index(const char *system_name, double ethical_alignment, double cultural_compatibility, const char *risk_level, double overall_score) { return 0; }
int db_insert_dashboard_summary(const char *system_name, double ethical_alignment, double cultural_compatibility, const char *risk_level, int global_ethics_rank, double projected_ethical_score) { return 0; }
int db_store_intermediate_scores(PrincipleNode *root, AISystem *systems, int system_count) { return 0; }
int db_store_cultural_compatibility(PrincipleNode *root, AISystem *systems, int system_count, Culture *cultures, int culture_count) { return 0; }
int db_store_risk_detection(PrincipleNode *root, AISystem *systems, int system_count) { return 0; }
int db_store_final_index(PrincipleNode *root, AISystem *systems, int system_count, Culture *cultures, int culture_count, double *overall_scores) { return 0; }
int db_store_dashboard_summary(PrincipleNode *root, AISystem *systems, int system_count, Culture *cultures, int culture_count, double *overall_scores) { return 0; }
