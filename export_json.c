/* export_json.c  –  Write all computed results to data.json
 *
 * The HTML dashboard reads this file via fetch('data.json').
 * Call export_all_to_json() at the end of main() after all scoring.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

/* ------------------------------------------------------------------ *
 *  Helpers                                                            *
 * ------------------------------------------------------------------ */
static double risk_to_penalty_j(const char *risk_level) {
    if (strcmp(risk_level, "High")   == 0) return 0.60;
    if (strcmp(risk_level, "Medium") == 0) return 0.30;
    return 0.10;
}

static const char *overall_risk_level_j(double rp) {
    if (rp >= 0.50) return "High";
    if (rp >= 0.25) return "Medium";
    return "Low";
}

/* Escape a string for JSON (handle quotes, backslashes, control chars) */
static void json_escape(const char *in, char *out, int out_size) {
    int w = 0;
    out[0] = '\0';
    for (; *in && w < out_size - 2; in++) {
        unsigned char c = (unsigned char)*in;
        if (c == '"')       { out[w++] = '\\'; out[w++] = '"'; }
        else if (c == '\\') { out[w++] = '\\'; out[w++] = '\\'; }
        else if (c == '\n') { out[w++] = '\\'; out[w++] = 'n'; }
        else if (c == '\r') { out[w++] = '\\'; out[w++] = 'r'; }
        else if (c == '\t') { out[w++] = '\\'; out[w++] = 't'; }
        else if (c >= 32)   { out[w++] = (char)c; }
    }
    out[w] = '\0';
}

/* ------------------------------------------------------------------ *
 *  Main export function                                               *
 * ------------------------------------------------------------------ */
int export_all_to_json(const char *filename,
                       PrincipleNode *root,
                       AISystem      *systems,   int system_count,
                       Culture       *cultures,  int culture_count,
                       double        *overall_scores) {
    int i, j;
    char esc[256];
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "[JSON] Failed to open %s for writing\n", filename);
        return 0;
    }

    fprintf(fp, "{\n");

    /* -------- 1. systems ------------------------------------------ */
    fprintf(fp, "  \"systems\": [\n");
    for (i = 0; i < system_count; i++) {
        json_escape(systems[i].system_name,       esc, sizeof(esc));
        fprintf(fp, "    {\"ai_system_id\":%d, \"system_name\":\"%s\"",
                systems[i].ai_system_id, esc);
        json_escape(systems[i].application_domain, esc, sizeof(esc));
        fprintf(fp, ", \"application_domain\":\"%s\"", esc);
        json_escape(systems[i].developer,           esc, sizeof(esc));
        fprintf(fp, ", \"developer\":\"%s\"", esc);
        json_escape(systems[i].deployment_region,   esc, sizeof(esc));
        fprintf(fp, ", \"deployment_region\":\"%s\"}%s\n",
                esc, (i < system_count - 1) ? "," : "");
    }
    fprintf(fp, "  ],\n");

    /* -------- 2. intermediate_scoring ------------------------------ */
    fprintf(fp, "  \"intermediate_scoring\": [\n");
    {
        int first = 1;
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
                if (!first) fprintf(fp, ",\n");
                json_escape(systems[i].system_name, esc, sizeof(esc));
                fprintf(fp, "    {\"ai_system_id\":%d, \"system_name\":\"%s\"",
                        systems[i].ai_system_id, esc);
                json_escape(cur->ethical_principle, esc, sizeof(esc));
                fprintf(fp, ", \"ethical_principle\":\"%s\", \"alignment_score\":%.4f}",
                        esc, score);
                first = 0;
                cur = cur->right;
            }
        }
    }
    fprintf(fp, "\n  ],\n");

    /* -------- 3. cultural_compatibility ---------------------------- */
    fprintf(fp, "  \"cultural_compatibility\": [\n");
    {
        int first = 1;
        for (i = 0; i < system_count; i++) {
            for (j = 0; j < culture_count; j++) {
                double compat = compute_culture_compatibility_for_ai(
                    root, systems[i].ai_system_id, &cultures[j]);
                if (!first) fprintf(fp, ",\n");
                json_escape(systems[i].system_name, esc, sizeof(esc));
                fprintf(fp, "    {\"ai_system_id\":%d, \"system_name\":\"%s\"",
                        systems[i].ai_system_id, esc);
                json_escape(cultures[j].region, esc, sizeof(esc));
                fprintf(fp, ", \"region\":\"%s\", \"compatibility_score\":%.4f}",
                        esc, compat);
                first = 0;
            }
        }
    }
    fprintf(fp, "\n  ],\n");

    /* -------- 4. risk_detection ------------------------------------ */
    fprintf(fp, "  \"risk_detection\": [\n");
    {
        int first = 1;
        for (i = 0; i < system_count; i++) {
            RiskSummary risks[3];
            int rc = evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
            for (j = 0; j < rc; j++) {
                if (!first) fprintf(fp, ",\n");
                json_escape(systems[i].system_name, esc, sizeof(esc));
                fprintf(fp, "    {\"ai_system_id\":%d, \"system_name\":\"%s\"",
                        systems[i].ai_system_id, esc);
                json_escape(risks[j].risk_category, esc, sizeof(esc));
                fprintf(fp, ", \"risk_category\":\"%s\"", esc);
                json_escape(risks[j].risk_level, esc, sizeof(esc));
                fprintf(fp, ", \"risk_level\":\"%s\", \"risk_score\":%.4f}",
                        esc, risks[j].score);
                first = 0;
            }
        }
    }
    fprintf(fp, "\n  ],\n");

    /* -------- 5. final_index --------------------------------------- */
    fprintf(fp, "  \"final_index\": [\n");
    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int rc = evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        double rp = 0.0;
        for (j = 0; j < rc; j++) rp += risk_to_penalty_j(risks[j].risk_level);
        if (rc > 0) rp /= rc;
        double ethical  = compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural = compute_overall_cultural_compatibility(
            root, systems[i].ai_system_id, cultures, culture_count);
        json_escape(systems[i].system_name, esc, sizeof(esc));
        fprintf(fp, "    {\"system_name\":\"%s\", \"ethical_alignment\":%.4f"
                    ", \"cultural_compatibility\":%.4f"
                    ", \"risk_level\":\"%s\", \"overall_score\":%.4f}%s\n",
                esc, ethical, cultural,
                overall_risk_level_j(rp), overall_scores[i],
                (i < system_count - 1) ? "," : "");
    }
    fprintf(fp, "  ],\n");

    /* -------- 6. dashboard_summary --------------------------------- */
    fprintf(fp, "  \"dashboard\": [\n");
    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int rc = evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        double rp = 0.0;
        for (j = 0; j < rc; j++) rp += risk_to_penalty_j(risks[j].risk_level);
        if (rc > 0) rp /= rc;
        double ethical  = compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural = compute_overall_cultural_compatibility(
            root, systems[i].ai_system_id, cultures, culture_count);
        const char *rl = overall_risk_level_j(rp);

        int better = 0;
        for (j = 0; j < system_count; j++)
            if (overall_scores[j] > overall_scores[i]) better++;
        int rank = ((better + 1) * 100 + system_count - 1) / system_count;

        double projected = ethical + 0.20;
        if (strcmp(rl, "High") == 0) projected += 0.05;
        if (projected > 0.95)        projected  = 0.95;

        json_escape(systems[i].system_name, esc, sizeof(esc));
        fprintf(fp,
            "    {\"system_name\":\"%s\""
            ", \"ethical_alignment\":%.4f"
            ", \"cultural_compatibility\":%.4f"
            ", \"risk_level\":\"%s\""
            ", \"global_ethics_rank\":%d"
            ", \"projected_ethical_score\":%.4f}%s\n",
            esc, ethical, cultural, rl, rank, projected,
            (i < system_count - 1) ? "," : "");
    }
    fprintf(fp, "  ]\n");

    fprintf(fp, "}\n");
    fclose(fp);
    printf("[JSON] Exported all results to %s\n", filename);
    return 1;
}
