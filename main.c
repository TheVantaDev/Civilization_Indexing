#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static int load_ai_systems(const char *filename, AISystem *arr, int max) {
    FILE *fp=fopen(filename, "r");
    char line[1024];
    int count=0;

    if (!fp) {
        perror("Failed to open ai_systems.csv");
        return 0;
    }

    fgets(line, sizeof(line), fp); // header
    while (fgets(line, sizeof(line), fp) && count<max) {
        AISystem row;
        if (sscanf(line, "%d,%127[^,],%127[^,],%127[^,],%63[^\n]",
                   &row.ai_system_id,
                   row.system_name,
                   row.application_domain,
                   row.developer,
                   row.deployment_region)==5) {
            arr[count++]=row;
        }
    }

    fclose(fp);
    return count;
}

static double risk_to_penalty(const char *risk_level) {
    if (strcmp(risk_level, "High")==0) return 0.60;
    if (strcmp(risk_level, "Medium")==0) return 0.30;
    return 0.10;
}

static const char* overall_risk_level(double risk_penalty) {
    if (risk_penalty>=0.50) return "High";
    if (risk_penalty>=0.25) return "Medium";
    return "Low";
}

static void print_principle_scores_for_ai(PrincipleNode *root, int ai_id) {
    double score;
    if (!root) return;
    print_principle_scores_for_ai(root->left, ai_id);

    score=compute_principle_alignment_for_ai(root, ai_id, root->ethical_principle);
    printf("%-12d %-35s %.2f\n", ai_id, root->ethical_principle, score);

    print_principle_scores_for_ai(root->right, ai_id);
}

static int compute_rank_position(double current, const double *all_scores, int n) {
    int i;
    int better=0;
    for (i=0; i<n; i++) {
        if (all_scores[i]>current) better++;
    }
    return better+1;
}

static int export_intermediate_scores_csv(const char *filename, PrincipleNode *root, const AISystem *systems, int system_count) {
    int i;
    FILE *fp=fopen(filename, "w");
    if (!fp) return 0;

    fprintf(fp, "ai_system_id,system_name,ethical_principle,alignment_score\n");
    for (i=0; i<system_count; i++) {
        const PrincipleNode *stack[128];
        int top=0;
        const PrincipleNode *cur=root;

        while (cur || top>0) {
            while (cur) {
                stack[top++]=cur;
                cur=cur->left;
            }
            cur=stack[--top];
            fprintf(fp, "%d,%s,%s,%.2f\n",
                    systems[i].ai_system_id,
                    systems[i].system_name,
                    cur->ethical_principle,
                    compute_principle_alignment_for_ai(root, systems[i].ai_system_id, cur->ethical_principle));
            cur=cur->right;
        }
    }

    fclose(fp);
    return 1;
}

static int export_cultural_compatibility_csv(const char *filename,
                                             PrincipleNode *root,
                                             const AISystem *systems,
                                             int system_count,
                                             const Culture *cultures,
                                             int culture_count) {
    int i, j;
    FILE *fp=fopen(filename, "w");
    if (!fp) return 0;

    fprintf(fp, "ai_system_id,system_name,region,compatibility_score\n");
    for (i=0; i<system_count; i++) {
        for (j=0; j<culture_count; j++) {
            fprintf(fp, "%d,%s,%s,%.2f\n",
                    systems[i].ai_system_id,
                    systems[i].system_name,
                    cultures[j].region,
                    compute_culture_compatibility_for_ai(root, systems[i].ai_system_id, &cultures[j]));
        }
    }

    fclose(fp);
    return 1;
}

static int export_risk_detection_csv(const char *filename, PrincipleNode *root, const AISystem *systems, int system_count) {
    int i, j;
    FILE *fp=fopen(filename, "w");
    if (!fp) return 0;

    fprintf(fp, "ai_system_id,system_name,risk_category,risk_level,risk_score\n");
    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int risk_count=evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        for (j=0; j<risk_count; j++) {
            fprintf(fp, "%d,%s,%s,%s,%.2f\n",
                    systems[i].ai_system_id,
                    systems[i].system_name,
                    risks[j].risk_category,
                    risks[j].risk_level,
                    risks[j].score);
        }
    }

    fclose(fp);
    return 1;
}

static int export_final_index_csv(const char *filename,
                                  PrincipleNode *root,
                                  const AISystem *systems,
                                  int system_count,
                                  const Culture *cultures,
                                  int culture_count,
                                  double *overall_scores) {
    int i, j;
    FILE *fp=fopen(filename, "w");
    if (!fp) return 0;

    fprintf(fp, "system_name,ethical_alignment,cultural_compatibility,risk_level,overall_score\n");
    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int risk_count=evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        double risk_penalty=0.0;
        double ethical_align=compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural_compat=compute_overall_cultural_compatibility(root, systems[i].ai_system_id, cultures, culture_count);
        double overall;

        for (j=0; j<risk_count; j++) risk_penalty+=risk_to_penalty(risks[j].risk_level);
        risk_penalty/=risk_count;
        overall=(0.45*ethical_align) + (0.35*cultural_compat) + (0.20*(1.0-risk_penalty));
        if (overall>1.0) overall=1.0;
        if (overall<0.0) overall=0.0;
        overall_scores[i]=overall;

        fprintf(fp, "%s,%.2f,%.2f,%s,%.2f\n",
                systems[i].system_name,
                ethical_align,
                cultural_compat,
                overall_risk_level(risk_penalty),
                overall);
    }

    fclose(fp);
    return 1;
}

static int export_dashboard_csv(const char *filename,
                                PrincipleNode *root,
                                const AISystem *systems,
                                int system_count,
                                const Culture *cultures,
                                int culture_count,
                                const double *overall_scores) {
    int i, j;
    FILE *fp=fopen(filename, "w");
    if (!fp) return 0;

    fprintf(fp, "system_name,ethical_alignment,cultural_compatibility,risk_level,global_ethics_rank,projected_ethical_score\n");
    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int rank;
        int risk_count=evaluate_risks_for_ai(root, systems[i].ai_system_id, risks, 3);
        double risk_penalty=0.0;
        double projected;
        double ethical_align=compute_alignment_for_ai(root, systems[i].ai_system_id);
        double cultural_compat=compute_overall_cultural_compatibility(root, systems[i].ai_system_id, cultures, culture_count);

        for (j=0; j<risk_count; j++) risk_penalty+=risk_to_penalty(risks[j].risk_level);
        risk_penalty/=risk_count;

        rank=compute_rank_position(overall_scores[i], overall_scores, system_count);
        rank=(rank*100 + system_count - 1)/system_count;

        projected=ethical_align + 0.20;
        if (strcmp(overall_risk_level(risk_penalty), "High")==0) projected+=0.05;
        if (projected>0.95) projected=0.95;

        fprintf(fp, "%s,%.2f,%.2f,%s,%d,%.2f\n",
                systems[i].system_name,
                ethical_align,
                cultural_compat,
                overall_risk_level(risk_penalty),
                rank,
                projected);
    }

    fclose(fp);
    return 1;
}

static void print_recommendations(const AISystem *sys, const RiskSummary *risks, int risk_count, double projected_score) {
    int i;
    printf("\nAI System\n%s\n", sys->system_name);
    printf("\nDetected Issues\n");
    for (i=0; i<risk_count; i++) {
        if (strcmp(risks[i].risk_level, "Low")!=0) {
            printf("- %s risk (%s)\n", risks[i].risk_category, risks[i].risk_level);
        }
    }

    printf("\nRecommended Actions\n");
    if (strcmp(risks[1].risk_level, "Low")!=0) {
        printf("- Introduce Explainable AI models\n");
        printf("- Increase decision transparency\n");
    }
    if (strcmp(risks[0].risk_level, "Low")!=0) {
        printf("- Conduct fairness audits\n");
        printf("- Implement bias mitigation algorithms\n");
    }
    if (strcmp(risks[2].risk_level, "High")==0) {
        printf("- Add human oversight for high-impact decisions\n");
    }
    printf("\nPredicted Improvement\nProjected Ethical Score -> %.0f%%\n", projected_score*100.0);
}

static void print_dashboard(const AISystem *sys,
                            double ethical_align,
                            double cultural_compat,
                            const char *risk_level,
                            int rank,
                            double projected_score) {
    const char *insight;
    printf("\nEthical AI Civilization Dashboard\n");
    printf("AI System: %s\n\n", sys->system_name);
    printf("Metric                        Value\n");
    printf("Ethical Alignment             %.0f%%\n", ethical_align*100.0);
    printf("Cultural Compatibility        %.0f%%\n", cultural_compat*100.0);
    printf("Risk Level                    %s\n", risk_level);
    printf("Global Ethics Rank            %d / 100\n", rank);

    insight=(strcmp(risk_level, "High")==0)
        ? "The system violates core fairness principles due to biased outcomes and low transparency."
        : "The system is largely aligned but should still strengthen transparency and fairness controls.";
    printf("\nAI Insight\n%s\n", insight);
    printf("\nPredicted Improvement\nProjected Ethical Score -> %.0f%%\n", projected_score*100.0);
}

int main(void) {
    const int max_systems=64;
    const int max_cultures=64;
    int i, j;
    AISystem systems[64];
    Culture cultures[64];
    double overall_scores[64];

    PrincipleNode *ethics_root=load_ethics("ethics.csv");
    if (!ethics_root) {
        printf("Failed to load ethics.csv\n");
        return 1;
    }

    int system_count=load_ai_systems("ai_systems.csv", systems, max_systems);
    int culture_count=load_cultures("culture.csv", cultures, max_cultures);

    if (system_count<=0) {
        printf("No AI systems loaded from ai_systems.csv\n");
        free_ethics_tree(ethics_root);
        return 1;
    }

    printf("Loaded %d AI systems and %d cultures.\n", system_count, culture_count);

    load_behaviors("ai_behavior.csv", ethics_root);

    printf("\nIntermediate AI Scoring Table\n");
    printf("ai_system_id ethical_principle                    alignment_score\n");
    for (i=0; i<system_count; i++) {
        print_principle_scores_for_ai(ethics_root, systems[i].ai_system_id);
    }

    printf("\nCultural Compatibility Output\n");
    printf("ai_system_id region          compatibility_score\n");
    for (i=0; i<system_count; i++) {
        for (j=0; j<culture_count; j++) {
            double compat=compute_culture_compatibility_for_ai(ethics_root, systems[i].ai_system_id, &cultures[j]);
            printf("%-12d %-14s %.2f\n", systems[i].ai_system_id, cultures[j].region, compat);
        }
    }

    printf("\nEthical Risk Detection Output\n");
    printf("ai_system_id risk_category           risk_level\n");
    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int risk_count=evaluate_risks_for_ai(ethics_root, systems[i].ai_system_id, risks, 3);
        for (j=0; j<risk_count; j++) {
            printf("%-12d %-23s %s\n", systems[i].ai_system_id, risks[j].risk_category, risks[j].risk_level);
        }
    }

    printf("\nFinal Ethical AI Civilization Index\n");
    printf("AI System       Ethical Alignment  Cultural Compatibility  Risk Level  Overall Score\n");

    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int risk_count=evaluate_risks_for_ai(ethics_root, systems[i].ai_system_id, risks, 3);
        double ethical_align=compute_alignment_for_ai(ethics_root, systems[i].ai_system_id);
        double cultural_compat=compute_overall_cultural_compatibility(ethics_root, systems[i].ai_system_id, cultures, culture_count);
        double risk_penalty=0.0;
        double overall;

        for (j=0; j<risk_count; j++) {
            risk_penalty+=risk_to_penalty(risks[j].risk_level);
        }
        risk_penalty/=risk_count;

        overall=(0.45*ethical_align) + (0.35*cultural_compat) + (0.20*(1.0-risk_penalty));
        if (overall>1.0) overall=1.0;
        if (overall<0.0) overall=0.0;
        overall_scores[i]=overall;

        printf("%-15s %-17.0f%% %-23.0f%% %-10s %.0f%%\n",
               systems[i].system_name,
               ethical_align*100.0,
               cultural_compat*100.0,
               overall_risk_level(risk_penalty),
               overall*100.0);
    }

    printf("\nAdditional PASSIONIT PRUTL KALKI + AIDHARMA Dimensions\n");
    printf("AI System       Positive Soul  Negative Soul  Material PM  Material NM\n");
    for (i=0; i<system_count; i++) {
        double ethical_align=compute_alignment_for_ai(ethics_root, systems[i].ai_system_id);
        double cultural_compat=compute_overall_cultural_compatibility(ethics_root, systems[i].ai_system_id, cultures, culture_count);
        double positive_soul=(ethical_align*0.6 + cultural_compat*0.4);
        double negative_soul=1.0-positive_soul;
        double material_pm=positive_soul*0.9;
        double material_nm=negative_soul*0.9;
        printf("%-15s %-13.0f%% %-13.0f%% %-11.0f%% %.0f%%\n",
               systems[i].system_name,
               positive_soul*100.0,
               negative_soul*100.0,
               material_pm*100.0,
               material_nm*100.0);
    }

    for (i=0; i<system_count; i++) {
        RiskSummary risks[3];
        int risk_count=evaluate_risks_for_ai(ethics_root, systems[i].ai_system_id, risks, 3);
        double ethical_align=compute_alignment_for_ai(ethics_root, systems[i].ai_system_id);
        double cultural_compat=compute_overall_cultural_compatibility(ethics_root, systems[i].ai_system_id, cultures, culture_count);
        double risk_penalty=0.0;
        double projected;
        int rank;

        for (j=0; j<risk_count; j++) {
            risk_penalty+=risk_to_penalty(risks[j].risk_level);
        }
        risk_penalty/=risk_count;

        rank=compute_rank_position(overall_scores[i], overall_scores, system_count);
        rank=(rank*100 + system_count - 1)/system_count;

        projected=ethical_align + 0.20;
        if (strcmp(overall_risk_level(risk_penalty), "High")==0) projected+=0.05;
        if (projected>0.95) projected=0.95;

        print_recommendations(&systems[i], risks, risk_count, projected);
        print_dashboard(&systems[i], ethical_align, cultural_compat, overall_risk_level(risk_penalty), rank, projected);
    }

    if (!export_intermediate_scores_csv("export_intermediate_scoring.csv", ethics_root, systems, system_count)) {
        printf("Warning: failed to write export_intermediate_scoring.csv\n");
    }
    if (!export_cultural_compatibility_csv("export_cultural_compatibility.csv", ethics_root, systems, system_count, cultures, culture_count)) {
        printf("Warning: failed to write export_cultural_compatibility.csv\n");
    }
    if (!export_risk_detection_csv("export_risk_detection.csv", ethics_root, systems, system_count)) {
        printf("Warning: failed to write export_risk_detection.csv\n");
    }
    if (!export_final_index_csv("export_final_index.csv", ethics_root, systems, system_count, cultures, culture_count, overall_scores)) {
        printf("Warning: failed to write export_final_index.csv\n");
    }
    if (!export_dashboard_csv("export_dashboard.csv", ethics_root, systems, system_count, cultures, culture_count, overall_scores)) {
        printf("Warning: failed to write export_dashboard.csv\n");
    }

    // ===== VISUALIZATION SECTION =====
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                        ASCII VISUALIZATION                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");

    // Collect data for visualization
    double ethical_scores[64];
    double cultural_scores[64];
    const char *ai_names[64];

    for (i = 0; i < system_count; i++) {
        ethical_scores[i] = compute_alignment_for_ai(ethics_root, systems[i].ai_system_id);
        cultural_scores[i] = compute_overall_cultural_compatibility(ethics_root, systems[i].ai_system_id, cultures, culture_count);
        ai_names[i] = systems[i].system_name;
    }

    // Print comparison chart
    print_comparison_chart("AI Systems Ethics Comparison", ai_names, ethical_scores, cultural_scores, overall_scores, system_count);

    // Print risk charts for each AI
    for (i = 0; i < system_count; i++) {
        RiskSummary risks[3];
        int risk_count = evaluate_risks_for_ai(ethics_root, systems[i].ai_system_id, risks, 3);
        print_risk_chart(systems[i].system_name, risks, risk_count);
    }

    // Generate HTML Dashboard
    if (generate_html_dashboard("dashboard.html", systems, ethical_scores, cultural_scores, overall_scores, system_count)) {
        printf("\n Interactive HTML dashboard generated: dashboard.html\n");
        printf("   Open this file in your web browser to see interactive charts!\n");
    } else {
        printf("\n Failed to generate HTML dashboard\n");
    }

    // Generate GNUplot script (optional)
    if (generate_gnuplot_script("plot_ethics.gp", systems, overall_scores, system_count)) {
        printf(" GNUplot script generated: plot_ethics.gp\n");
        printf("   Run: gnuplot plot_ethics.gp (if you have GNUplot installed)\n");
    }

    free_ethics_tree(ethics_root);

    return 0;
}
