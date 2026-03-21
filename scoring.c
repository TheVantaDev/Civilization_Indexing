#include <string.h>
#include "types.h"

static double transparency_to_score(const char *t) {
    if (strcmp(t, "High")==0) return 0.9;
    if (strcmp(t, "Medium")==0) return 0.6;
    return 0.3; // Low or anything else
}

static double bias_penalty(const char *outcome) {
    if (strstr(outcome, "Biased") || strstr(outcome, "Rejected"))
        return 0.7;
    return 1.0;
}

static int principle_match(const PrincipleNode *root, const char *principle_name) {
    if (!principle_name || !principle_name[0]) return 1;
    return strcmp(root->ethical_principle, principle_name)==0;
}

static double normalize(double v) {
    if (v<0.0) return 0.0;
    if (v>1.0) return 1.0;
    return v;
}

static double sensitivity_multiplier(const char *sensitivity) {
    if (strcmp(sensitivity, "High")==0) return 1.00;
    if (strcmp(sensitivity, "Medium")==0) return 0.90;
    return 0.80;
}

static double culture_priority_fit(const char *priority, const PrincipleNode *p) {
    if (strstr(priority, "fairness") && (strstr(p->ethical_principle, "Dharma") || strstr(p->ethical_principle, "Justice"))) return 1.0;
    if (strstr(priority, "privacy") && (strstr(p->domain, "Law") || strstr(p->ethical_principle, "Justice"))) return 1.0;
    if (strstr(priority, "harmony") && strstr(p->ethical_principle, "Social harmony")) return 1.0;
    if (strstr(priority, "transparency") && (strstr(p->ethical_principle, "Virtue") || strstr(p->domain, "Governance"))) return 0.95;
    return 0.75;
}

double compute_principle_alignment_for_ai(PrincipleNode *root, int ai_id, const char *principle_name) {
    BehaviorNode *b;
    int count;
    double sum;
    double left;
    double right;

    if (!root) return 0.0;

    left=compute_principle_alignment_for_ai(root->left, ai_id, principle_name);
    right=compute_principle_alignment_for_ai(root->right, ai_id, principle_name);

    sum=0.0;
    count=0;
    b=root->behaviors;

    if (principle_match(root, principle_name)) {
        while (b) {
            if (b->ai_system_id==ai_id) {
                double t=transparency_to_score(b->transparency);
                double p=bias_penalty(b->outcome);
                sum+=normalize(root->weight*t*p);
                count++;
            }
            b=b->next;
        }
    }

    if (principle_name && principle_name[0]) {
        if (count>0) return sum/count;
        if (left>0.0) return left;
        if (right>0.0) return right;
        return 0.0;
    }

    if (count>0) {
        double self=sum/count;
        double total=self;
        int parts=1;
        if (left>0.0) {
            total+=left;
            parts++;
        }
        if (right>0.0) {
            total+=right;
            parts++;
        }
        return total/parts;
    }

    if (left>0.0 && right>0.0) return (left+right)/2.0;
    if (left>0.0) return left;
    if (right>0.0) return right;
    return 0.0;
}

double compute_alignment_for_ai(PrincipleNode *root, int ai_id) {
    return compute_principle_alignment_for_ai(root, ai_id, "");
}

static void accumulate_culture_score(PrincipleNode *root, int ai_id, const Culture *culture, double *sum, int *count) {
    BehaviorNode *b;
    if (!root) return;

    accumulate_culture_score(root->left, ai_id, culture, sum, count);

    b=root->behaviors;
    while (b) {
        if (b->ai_system_id==ai_id) {
            double align=normalize(root->weight*transparency_to_score(b->transparency)*bias_penalty(b->outcome));
            double fit=culture_priority_fit(culture->priority, root);
            double sensitivity=sensitivity_multiplier(culture->sensitivity);
            *sum+=align*fit*sensitivity;
            (*count)++;
        }
        b=b->next;
    }

    accumulate_culture_score(root->right, ai_id, culture, sum, count);
}

double compute_culture_compatibility_for_ai(PrincipleNode *root, int ai_id, const Culture *culture) {
    double sum=0.0;
    int count=0;
    accumulate_culture_score(root, ai_id, culture, &sum, &count);
    if (count==0) return 0.0;
    return normalize(sum/count);
}

double compute_overall_cultural_compatibility(PrincipleNode *root, int ai_id, const Culture *cultures, int culture_count) {
    int i;
    double total=0.0;
    if (culture_count<=0) return 0.0;
    for (i=0; i<culture_count; i++) {
        total+=compute_culture_compatibility_for_ai(root, ai_id, &cultures[i]);
    }
    return normalize(total/culture_count);
}

static void accumulate_risk_counts(PrincipleNode *root,
                                   int ai_id,
                                   int *total,
                                   int *high_bias,
                                   int *medium_transparency,
                                   int *low_transparency,
                                   int *fairness_hits) {
    BehaviorNode *b;
    if (!root) return;

    accumulate_risk_counts(root->left, ai_id, total, high_bias, medium_transparency, low_transparency, fairness_hits);

    b=root->behaviors;
    while (b) {
        if (b->ai_system_id==ai_id) {
            (*total)++;
            if (strstr(b->outcome, "Biased") || strstr(b->outcome, "Rejected")) {
                (*high_bias)++;
                (*fairness_hits)++;
            }
            if (strcmp(b->transparency, "Medium")==0) (*medium_transparency)++;
            if (strcmp(b->transparency, "Low")==0) {
                (*low_transparency)++;
                (*fairness_hits)++;
            }
        }
        b=b->next;
    }

    accumulate_risk_counts(root->right, ai_id, total, high_bias, medium_transparency, low_transparency, fairness_hits);
}

int evaluate_risks_for_ai(PrincipleNode *root, int ai_id, RiskSummary *out, int max_out) {
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

    accumulate_risk_counts(root, ai_id, &total, &high_bias, &medium_transparency, &low_transparency, &fairness_hits);
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
