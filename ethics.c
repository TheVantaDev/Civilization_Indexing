#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

PrincipleNode* create_principle(int id, const char *civ,
                                const char *principle,
                                const char *domain,
                                double weight) {
    PrincipleNode *node=malloc(sizeof(PrincipleNode));
    node->ethics_id=id;
    strcpy(node->civilization, civ);
    strcpy(node->ethical_principle, principle);
    strcpy(node->domain, domain);
    node->weight=weight;
    node->behaviors=NULL;
    node->left=node->right=NULL;
    return node;
}

PrincipleNode* insert_principle(PrincipleNode *root, PrincipleNode *node) {
    if (root==NULL) return node;
    int cmp=strcmp(node->ethical_principle, root->ethical_principle);
    if (cmp<0) root->left=insert_principle(root->left, node);
    else if (cmp>0) root->right=insert_principle(root->right, node);
    // if equal, ignore or update
    return root;
}

PrincipleNode* find_principle(PrincipleNode *root, const char *principle) {
    if (root==NULL) return NULL;
    int cmp=strcmp(principle, root->ethical_principle);
    if (cmp==0) return root;
    else if (cmp<0) return find_principle(root->left, principle);
    else return find_principle(root->right, principle);
}

PrincipleNode* load_ethics(const char *filename) {
    FILE *fp=fopen(filename, "r");
    if (!fp) {
        perror("Failed to open ethics.csv");
        return NULL;
    }
    char line[512];
    PrincipleNode *root=NULL;

    // skip header
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        int id;
        char civ[64], principle[128], domain[64];
        double weight;
        if (sscanf(line, "%d,%63[^,],%127[^,],%63[^,],%lf",
                   &id, civ, principle, domain, &weight)==5) {
            PrincipleNode *node=create_principle(id, civ, principle, domain, weight);
            root=insert_principle(root, node);
        }
    }
    fclose(fp);
    return root;
}

static void free_behavior_list(BehaviorNode *head) {
    while (head) {
        BehaviorNode *next=head->next;
        free(head);
        head=next;
    }
}

void free_ethics_tree(PrincipleNode *root) {
    if (!root) return;
    free_ethics_tree(root->left);
    free_ethics_tree(root->right);
    free_behavior_list(root->behaviors);
    free(root);
}
