#include <string.h>
#include "types.h"

static double transparency_to_score(const char *t) {
    if (strcmp(t, "High")==0) return 0.9;
    if (strcmp(t, "Medium")==0) return 0.6;
    return 0.3; // Low or anything else
}

static double bias_penalty(const char *outcome) {
    // Very simple rule: if contains "Biased" or "Rejected" then penalty
    if (strstr(outcome, "Biased") || strstr(outcome, "Rejected"))
        return 0.7;
    return 1.0;
}

// Compute alignment score for a given AI system id, traversing tree
double compute_alignment_for_ai(PrincipleNode *root, int ai_id) {
    if (!root) return 0.0;

    double left=compute_alignment_for_ai(root->left, ai_id);
    double right=compute_alignment_for_ai(root->right, ai_id);

    // compute for this node
    BehaviorNode *b=root->behaviors;
    int count=0;
    double sum=0.0;

    while (b) {
        if (b->ai_system_id==ai_id) {
            double t=transparency_to_score(b->transparency);
            double p=bias_penalty(b->outcome);
            double s=root->weight*t*p;
            sum+=s;
            count++;
        }
        b=b->next;
    }

    double self_avg=(count>0)?(sum/count):0.0;

    // combine with left and right averages (very simple merge)
    // Here we just average non-zero parts.
    double total=self_avg;
    int parts=(self_avg>0)?1:0;
    if (left>0) { total+=left; parts++; }
    if (right>0) { total+=right; parts++; }

    if (parts==0) return 0.0;
    return total/parts;
}
