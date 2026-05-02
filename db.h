/* db.h  –  MySQL database interface for AI Civilisation Indexer */
#ifndef DB_H
#define DB_H

#include "types.h"

/* ---------------------------------------------------------- *
 *  Connection lifecycle                                       *
 * ---------------------------------------------------------- */
int  db_connect(const char *host, unsigned int port,
                const char *user, const char *password,
                const char *dbname);
void db_close(void);

/* ---------------------------------------------------------- *
 *  Upsert master tables (called once per run)               *
 * ---------------------------------------------------------- */
int db_upsert_ai_systems(AISystem *systems, int count);

/* ---------------------------------------------------------- *
 *  Insert behavior rows                                      *
 * ---------------------------------------------------------- */
int db_insert_behavior(int ai_system_id,
                       const char *decision_type,
                       const char *affected_group,
                       const char *outcome,
                       const char *transparency,
                       const char *source);   /* "csv" or "ollama" */

/* ---------------------------------------------------------- *
 *  Insert computed result tables                             *
 * ---------------------------------------------------------- */
int db_insert_intermediate_scoring(int ai_system_id,
                                   const char *system_name,
                                   const char *ethical_principle,
                                   double alignment_score);

int db_insert_cultural_compatibility(int ai_system_id,
                                     const char *system_name,
                                     const char *region,
                                     double compatibility_score);

int db_insert_risk_detection(int ai_system_id,
                             const char *system_name,
                             const char *risk_category,
                             const char *risk_level,
                             double risk_score);

int db_insert_final_index(const char *system_name,
                          double ethical_alignment,
                          double cultural_compatibility,
                          const char *risk_level,
                          double overall_score);

int db_insert_dashboard_summary(const char *system_name,
                                double ethical_alignment,
                                double cultural_compatibility,
                                const char *risk_level,
                                int global_ethics_rank,
                                double projected_ethical_score);

/* ---------------------------------------------------------- *
 *  Bulk helpers (called from main after all computation)     *
 * ---------------------------------------------------------- */
int db_store_intermediate_scores(PrincipleNode *root,
                                 AISystem *systems, int system_count);

int db_store_cultural_compatibility(PrincipleNode *root,
                                    AISystem *systems, int system_count,
                                    Culture *cultures, int culture_count);

int db_store_risk_detection(PrincipleNode *root,
                            AISystem *systems, int system_count);

int db_store_final_index(PrincipleNode *root,
                         AISystem *systems, int system_count,
                         Culture *cultures, int culture_count,
                         double *overall_scores);

int db_store_dashboard_summary(PrincipleNode *root,
                               AISystem *systems, int system_count,
                               Culture *cultures, int culture_count,
                               double *overall_scores);

#endif /* DB_H */
