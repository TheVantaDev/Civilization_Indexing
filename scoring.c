#include <string.h>
#include <stdlib.h>
#include "types.h"

double transparency_to_score(char *t) {
    if (strcmp(t, "High")==0) return 0.9;
    if (strcmp(t, "Medium")==0) return 0.6;
    return 0.3; // Low or anything else
}

double bias_penalty(char *outcome) {
    if (strstr(outcome, "Biased") || strstr(outcome, "Rejected"))
        return 0.7;
    return 1.0;
}

int principle_match(PrincipleNode *root, char *principle_name) {
    if (!principle_name || !principle_name[0]) return 1;
    return strcmp(root->ethical_principle, principle_name)==0;
}

double normalize(double v) {
    if (v<0.0) return 0.0;
    if (v>1.0) return 1.0;
    return v;
}

double sensitivity_multiplier(char *sensitivity) {
    if (strcmp(sensitivity, "High")==0) return 1.00;
    if (strcmp(sensitivity, "Medium")==0) return 0.90;
    return 0.80;
}

double culture_priority_fit(char *priority, PrincipleNode *p) {
    if (strstr(priority, "fairness") && (strstr(p->ethical_principle, "Dharma") || strstr(p->ethical_principle, "Justice"))) return 1.0;
    if (strstr(priority, "privacy") && (strstr(p->domain, "Law") || strstr(p->ethical_principle, "Justice"))) return 1.0;
    if (strstr(priority, "harmony") && strstr(p->ethical_principle, "Social harmony")) return 1.0;
    if (strstr(priority, "transparency") && (strstr(p->ethical_principle, "Virtue") || strstr(p->domain, "Governance"))) return 0.95;
    return 0.75;
}

double compute_principle_alignment_for_ai(PrincipleNode *root, int ai_id, char *principle_name) {
    BehaviorRef *refs;
    BehaviorRef *cur;
    int count=0;
    double sum=0.0;

    (void)root;
    refs=get_behavior_refs_for_ai(ai_id);
    cur=refs;

    while (cur) {
        if (!principle_name || !principle_name[0] || strcmp(cur->principle->ethical_principle, principle_name)==0) {
            double t=transparency_to_score(cur->behavior->transparency);
            double p=bias_penalty(cur->behavior->outcome);
            sum+=normalize(cur->principle->weight*t*p);
            count++;
        }
        cur=cur->next;
    }

    free_behavior_refs(refs);
    if (count==0) return 0.0;
    return sum/count;
}

double compute_alignment_for_ai(PrincipleNode *root, int ai_id) {
    return compute_principle_alignment_for_ai(root, ai_id, "");
}

double compute_culture_compatibility_for_ai(PrincipleNode *root, int ai_id, Culture *culture) {
    BehaviorRef *refs;
    BehaviorRef *cur;
    double sum=0.0;
    int count=0;

    (void)root;
    refs=get_behavior_refs_for_ai(ai_id);
    cur=refs;

    while (cur) {
        double align=normalize(cur->principle->weight*transparency_to_score(cur->behavior->transparency)*bias_penalty(cur->behavior->outcome));
        double fit=culture_priority_fit(culture->priority, cur->principle);
        double sensitivity=sensitivity_multiplier(culture->sensitivity);
        sum+=align*fit*sensitivity;
        count++;
        cur=cur->next;
    }

    free_behavior_refs(refs);
    if (count==0) return 0.0;
    return normalize(sum/count);
}

double compute_overall_cultural_compatibility(PrincipleNode *root, int ai_id, Culture *cultures, int culture_count) {
    int i;
    double total=0.0;
    if (culture_count<=0) return 0.0;
    for (i=0; i<culture_count; i++) {
        total+=compute_culture_compatibility_for_ai(root, ai_id, &cultures[i]);
    }
    return normalize(total/culture_count);
}

int evaluate_risks_for_ai(PrincipleNode *root, int ai_id, RiskSummary *out, int max_out) {
    BehaviorRef *refs;
    BehaviorRef *cur;
    int i;
    int total=0;
    int high_bias=0;
    int medium_transparency=0;
    int low_transparency=0;
    int fairness_hits=0;
    double bias_risk;
    double transparency_risk;
    double social_risk;

    RiskSummary defaults[3] = {
        {"Hiring bias", "Low", 0.0},
        {"Lack of transparency", "Low", 0.0},
        {"Social inequality", "Low", 0.0}
    };

    if (!root || !out || max_out<=0) return 0;

    for (i=0; i<3 && i<max_out; i++) out[i]=defaults[i];

    refs=get_behavior_refs_for_ai(ai_id);
    cur=refs;
    while (cur) {
        total++;
        if (strstr(cur->behavior->outcome, "Biased") || strstr(cur->behavior->outcome, "Rejected")) {
            high_bias++;
            fairness_hits++;
        }
        if (strcmp(cur->behavior->transparency, "Medium")==0) medium_transparency++;
        if (strcmp(cur->behavior->transparency, "Low")==0) {
            low_transparency++;
            fairness_hits++;
        }
        cur=cur->next;
    }
    free_behavior_refs(refs);

    if (total==0) return (max_out<3)?max_out:3;

    bias_risk=(double)high_bias/total;
    transparency_risk=((double)low_transparency + 0.5*(double)medium_transparency)/total;
    social_risk=(double)fairness_hits/total;

    out[0].score=normalize(bias_risk);
    out[1].score=normalize(transparency_risk);
    out[2].score=normalize(social_risk);

    if (out[0].score>=0.50) strcpy(out[0].risk_level, "High");
    else if (out[0].score>=0.25) strcpy(out[0].risk_level, "Medium");

    if (out[1].score>=0.50) strcpy(out[1].risk_level, "High");
    else if (out[1].score>=0.25) strcpy(out[1].risk_level, "Medium");

    if (out[2].score>=0.50) strcpy(out[2].risk_level, "High");
    else if (out[2].score>=0.25) strcpy(out[2].risk_level, "Medium");

    return (max_out<3)?max_out:3;
}
