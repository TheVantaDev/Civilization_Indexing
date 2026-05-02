/* db.c  –  MySQL implementation using MySQL Connector/C (libmysqlclient)
 *
 * Compile (MinGW / Windows example):
 *   gcc ... db.c -I"C:\Program Files\MySQL\MySQL Server 8.0\include" \
 *               -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" \
 *               -lmysql
 *
 * The db_connect() call is made from main() using:
 *   host="127.0.0.1", port=3306, user="root",
 *   password="Sam@2006", dbname="ai_civilisation"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include "db.h"
#include "types.h"

/* ------------------------------------------------------------------ *
 *  Private state                                                       *
 * ------------------------------------------------------------------ */
static MYSQL *g_conn = NULL;

/* Helper: escape a string and wrap it in single-quotes for SQL        */
static void escape(MYSQL *conn, char *out, size_t out_size,
                   const char *in) {
    char tmp[1024];
    size_t in_len = strlen(in);
    if (in_len >= sizeof(tmp) / 2) in_len = sizeof(tmp) / 2 - 1;
    mysql_real_escape_string(conn, tmp, in, (unsigned long)in_len);
    snprintf(out, out_size, "'%s'", tmp);
}

/* Helper: execute a query and print the error on failure              */
static int exec_query(const char *sql) {
    if (!g_conn) return 0;
    if (mysql_query(g_conn, sql) != 0) {
        fprintf(stderr, "[DB] Query failed: %s\n  SQL: %.120s\n",
                mysql_error(g_conn), sql);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ *
 *  Connection lifecycle                                                *
 * ------------------------------------------------------------------ */
int db_connect(const char *host, unsigned int port,
               const char *user, const char *password,
               const char *dbname) {
    g_conn = mysql_init(NULL);
    if (!g_conn) {
        fprintf(stderr, "[DB] mysql_init() failed – out of memory\n");
        return 0;
    }

    /* Enable auto-reconnect */
    int reconnect = 1;
    mysql_options(g_conn, MYSQL_OPT_RECONNECT, &reconnect);

    if (!mysql_real_connect(g_conn, host, user, password,
                            dbname, port, NULL, 0)) {
        fprintf(stderr, "[DB] Connection failed: %s\n",
                mysql_error(g_conn));
        mysql_close(g_conn);
        g_conn = NULL;
        return 0;
    }

    /* Use UTF-8 */
    mysql_set_character_set(g_conn, "utf8mb4");
    printf("[DB] Connected to MySQL  db=%s  host=%s:%u\n",
           dbname, host, port);
    return 1;
}

void db_close(void) {
    if (g_conn) {
        mysql_close(g_conn);
        g_conn = NULL;
        printf("[DB] MySQL connection closed.\n");
    }
}

/* ------------------------------------------------------------------ *
 *  Upsert AI systems master table                                     *
 * ------------------------------------------------------------------ */
int db_upsert_ai_systems(AISystem *systems, int count) {
    int i, ok = 1;
    char sql[2048];
    char ename[256], edomain[256], edev[256], eregion[256];

    if (!g_conn || !systems || count <= 0) return 0;

    for (i = 0; i < count; i++) {
        escape(g_conn, ename,    sizeof(ename),    systems[i].system_name);
        escape(g_conn, edomain,  sizeof(edomain),  systems[i].application_domain);
        escape(g_conn, edev,     sizeof(edev),     systems[i].developer);
        escape(g_conn, eregion,  sizeof(eregion),  systems[i].deployment_region);

        snprintf(sql, sizeof(sql),
            "INSERT INTO ai_systems "
            "(ai_system_id, system_name, application_domain, developer, deployment_region) "
            "VALUES (%d, %s, %s, %s, %s) "
            "ON DUPLICATE KEY UPDATE "
            "system_name=%s, application_domain=%s, developer=%s, deployment_region=%s;",
            systems[i].ai_system_id,
            ename, edomain, edev, eregion,
            ename, edomain, edev, eregion);

        if (!exec_query(sql)) ok = 0;
    }
    return ok;
}

/* ------------------------------------------------------------------ *
 *  Insert one behavior row                                            *
 * ------------------------------------------------------------------ */
int db_insert_behavior(int ai_system_id,
                       const char *decision_type,
                       const char *affected_group,
                       const char *outcome,
                       const char *transparency,
                       const char *source) {
    char sql[2048];
    char edecision[256], egroup[256], eoutcome[256], etrans[64], esrc[32];

    if (!g_conn) return 0;

    escape(g_conn, edecision, sizeof(edecision), decision_type);
    escape(g_conn, egroup,    sizeof(egroup),    affected_group);
    escape(g_conn, eoutcome,  sizeof(eoutcome),  outcome);
    escape(g_conn, etrans,    sizeof(etrans),    transparency);
    escape(g_conn, esrc,      sizeof(esrc),      source);

    snprintf(sql, sizeof(sql),
        "INSERT INTO ai_behavior "
        "(ai_system_id, decision_type, affected_group, outcome, transparency, source) "
        "VALUES (%d, %s, %s, %s, %s, %s);",
        ai_system_id, edecision, egroup, eoutcome, etrans, esrc);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Insert intermediate scoring row                                    *
 * ------------------------------------------------------------------ */
int db_insert_intermediate_scoring(int ai_system_id,
                                   const char *system_name,
                                   const char *ethical_principle,
                                   double alignment_score) {
    char sql[1024];
    char ename[256], eprinciple[256];

    if (!g_conn) return 0;
    escape(g_conn, ename,      sizeof(ename),      system_name);
    escape(g_conn, eprinciple, sizeof(eprinciple), ethical_principle);

    snprintf(sql, sizeof(sql),
        "INSERT INTO intermediate_scoring "
        "(ai_system_id, system_name, ethical_principle, alignment_score) "
        "VALUES (%d, %s, %s, %.4f);",
        ai_system_id, ename, eprinciple, alignment_score);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Insert cultural compatibility row                                  *
 * ------------------------------------------------------------------ */
int db_insert_cultural_compatibility(int ai_system_id,
                                     const char *system_name,
                                     const char *region,
                                     double compatibility_score) {
    char sql[1024];
    char ename[256], eregion[128];

    if (!g_conn) return 0;
    escape(g_conn, ename,   sizeof(ename),   system_name);
    escape(g_conn, eregion, sizeof(eregion), region);

    snprintf(sql, sizeof(sql),
        "INSERT INTO cultural_compatibility "
        "(ai_system_id, system_name, region, compatibility_score) "
        "VALUES (%d, %s, %s, %.4f);",
        ai_system_id, ename, eregion, compatibility_score);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Insert risk detection row                                          *
 * ------------------------------------------------------------------ */
int db_insert_risk_detection(int ai_system_id,
                             const char *system_name,
                             const char *risk_category,
                             const char *risk_level,
                             double risk_score) {
    char sql[1024];
    char ename[256], ecategory[128], elevel[32];

    if (!g_conn) return 0;
    escape(g_conn, ename,     sizeof(ename),     system_name);
    escape(g_conn, ecategory, sizeof(ecategory), risk_category);
    escape(g_conn, elevel,    sizeof(elevel),    risk_level);

    snprintf(sql, sizeof(sql),
        "INSERT INTO risk_detection "
        "(ai_system_id, system_name, risk_category, risk_level, risk_score) "
        "VALUES (%d, %s, %s, %s, %.4f);",
        ai_system_id, ename, ecategory, elevel, risk_score);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Insert final index row                                             *
 * ------------------------------------------------------------------ */
int db_insert_final_index(const char *system_name,
                          double ethical_alignment,
                          double cultural_compatibility,
                          const char *risk_level,
                          double overall_score) {
    char sql[1024];
    char ename[256], elevel[32];

    if (!g_conn) return 0;
    escape(g_conn, ename,  sizeof(ename),  system_name);
    escape(g_conn, elevel, sizeof(elevel), risk_level);

    snprintf(sql, sizeof(sql),
        "INSERT INTO final_index "
        "(system_name, ethical_alignment, cultural_compatibility, risk_level, overall_score) "
        "VALUES (%s, %.4f, %.4f, %s, %.4f);",
        ename, ethical_alignment, cultural_compatibility, elevel, overall_score);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Insert dashboard summary row                                       *
 * ------------------------------------------------------------------ */
int db_insert_dashboard_summary(const char *system_name,
                                double ethical_alignment,
                                double cultural_compatibility,
                                const char *risk_level,
                                int global_ethics_rank,
                                double projected_ethical_score) {
    char sql[1024];
    char ename[256], elevel[32];

    if (!g_conn) return 0;
    escape(g_conn, ename,  sizeof(ename),  system_name);
    escape(g_conn, elevel, sizeof(elevel), risk_level);

    snprintf(sql, sizeof(sql),
        "INSERT INTO dashboard_summary "
        "(system_name, ethical_alignment, cultural_compatibility, risk_level, "
        " global_ethics_rank, projected_ethical_score) "
        "VALUES (%s, %.4f, %.4f, %s, %d, %.4f);",
        ename, ethical_alignment, cultural_compatibility,
        elevel, global_ethics_rank, projected_ethical_score);

    return exec_query(sql);
}

/* ------------------------------------------------------------------ *
 *  Bulk helpers – called from main.c after all scoring is done        *
 * ------------------------------------------------------------------ */

/* Helper shared with main.c scoring logic */
static double risk_penalty_for(PrincipleNode *root, int ai_id,
                               RiskSummary *risks, int *risk_count_out) {
    int j;
    double penalty = 0.0;
    int rc = evaluate_risks_for_ai(root, ai_id, risks, 3);
    *risk_count_out = rc;
    for (j = 0; j < rc; j++) penalty += (
        strcmp(risks[j].risk_level, "High")   == 0 ? 0.60 :
        strcmp(risks[j].risk_level, "Medium") == 0 ? 0.30 : 0.10);
    if (rc > 0) penalty /= rc;
    return penalty;
}

int db_store_intermediate_scores(PrincipleNode *root,
                                 AISystem *systems, int system_count) {
    int i, ok = 1;
    if (!g_conn || !root || !systems) return 0;

    for (i = 0; i < system_count; i++) {
        /* Walk BST in-order */
        PrincipleNode *stack[128];
        int top = 0;
        PrincipleNode *cur = root;

        while (cur || top > 0) {
            while (cur) { stack[top++] = cur; cur = cur->left; }
            cur = stack[--top];
            double score = compute_principle_alignment_for_ai(
                root, systems[i].ai_system_id, cur->ethical_principle);
            if (!db_insert_intermediate_scoring(systems[i].ai_system_id,
                    systems[i].system_name,
                    cur->ethical_principle, score)) ok = 0;
            cur = cur->right;
        }
    }
    return ok;
}

int db_store_cultural_compatibility(PrincipleNode *root,
                                    AISystem *systems, int system_count,
                                    Culture *cultures, int culture_count) {
    int i, j, ok = 1;
    if (!g_conn || !root || !systems || !cultures) return 0;

    for (i = 0; i < system_count; i++) {
        for (j = 0; j < culture_count; j++) {
            double compat = compute_culture_compatibility_for_ai(
                root, systems[i].ai_system_id, &cultures[j]);
            if (!db_insert_cultural_compatibility(systems[i].ai_system_id,
                    systems[i].system_name,
                    cultures[j].region, compat)) ok = 0;
        }
    }
    return ok;
}

int db_store_risk_detection(PrincipleNode *root,
                            AISystem *systems, int system_count) {
    int i, j, ok = 1;
    if (!g_conn || !root || !systems) return 0;

    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int rc = evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        for (j = 0; j < rc; j++) {
            if (!db_insert_risk_detection(systems[i].ai_system_id,
                    systems[i].system_name,
                    risks[j].risk_category,
                    risks[j].risk_level,
                    risks[j].score)) ok = 0;
        }
    }
    return ok;
}

int db_store_final_index(PrincipleNode *root,
                         AISystem *systems, int system_count,
                         Culture *cultures, int culture_count,
                         double *overall_scores) {
    int i, ok = 1;
    if (!g_conn || !root || !systems) return 0;

    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int rc;
        double penalty = risk_penalty_for(root, systems[i].ai_system_id, risks, &rc);
        double ethical  = compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural = compute_overall_cultural_compatibility(
                root, systems[i].ai_system_id, cultures, culture_count);
        const char *rl = penalty >= 0.50 ? "High" : penalty >= 0.25 ? "Medium" : "Low";

        if (!db_insert_final_index(systems[i].system_name,
                ethical, cultural, rl, overall_scores[i])) ok = 0;
    }
    return ok;
}

int db_store_dashboard_summary(PrincipleNode *root,
                               AISystem *systems, int system_count,
                               Culture *cultures, int culture_count,
                               double *overall_scores) {
    int i, ok = 1;
    if (!g_conn || !root || !systems) return 0;

    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int rc;
        double penalty = risk_penalty_for(root, systems[i].ai_system_id, risks, &rc);
        double ethical  = compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural = compute_overall_cultural_compatibility(
                root, systems[i].ai_system_id, cultures, culture_count);
        const char *rl = penalty >= 0.50 ? "High" : penalty >= 0.25 ? "Medium" : "Low";

        /* Rank: how many systems have a higher score? */
        int better = 0, j;
        for (j = 0; j < system_count; j++)
            if (overall_scores[j] > overall_scores[i]) better++;
        int rank = ((better + 1) * 100 + system_count - 1) / system_count;

        double projected = ethical + 0.20;
        if (strcmp(rl, "High") == 0) projected += 0.05;
        if (projected > 0.95) projected = 0.95;

        if (!db_insert_dashboard_summary(systems[i].system_name,
                ethical, cultural, rl, rank, projected)) ok = 0;
    }
    return ok;
}
