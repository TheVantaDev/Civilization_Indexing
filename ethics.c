#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

int principle_height(PrincipleNode *node) {
    if (!node) return 0;
    return node->height;
}

int max_int(int a, int b) {
    return (a>b)?a:b;
}

int get_balance(PrincipleNode *node) {
    if (!node) return 0;
    return principle_height(node->left)-principle_height(node->right);
}

void rebuild_threads_inorder(PrincipleNode *node, PrincipleNode **prev) {
    if (!node) return;

    rebuild_threads_inorder(node->left, prev);

    node->thread_prev=*prev;
    if (*prev) {
        (*prev)->thread_next=node;
    }
    node->thread_next=NULL;
    *prev=node;

    rebuild_threads_inorder(node->right, prev);
}

void rebuild_threads(PrincipleNode *root) {
    PrincipleNode *prev=NULL;
    rebuild_threads_inorder(root, &prev);
}

PrincipleNode* rotate_right(PrincipleNode *y) {
    PrincipleNode *x=y->left;
    PrincipleNode *t2=x->right;

    x->right=y;
    y->left=t2;

    y->height=1+max_int(principle_height(y->left), principle_height(y->right));
    x->height=1+max_int(principle_height(x->left), principle_height(x->right));

    return x;
}

PrincipleNode* rotate_left(PrincipleNode *x) {
    PrincipleNode *y=x->right;
    PrincipleNode *t2=y->left;

    y->left=x;
    x->right=t2;

    x->height=1+max_int(principle_height(x->left), principle_height(x->right));
    y->height=1+max_int(principle_height(y->left), principle_height(y->right));

    return y;
}

PrincipleNode* create_principle(int id, char *civ,
                                char *principle,
                                char *domain,
                                double weight) {
    PrincipleNode *node=malloc(sizeof(PrincipleNode));
    if (!node) return NULL;

    node->ethics_id=id;
    strcpy(node->civilization, civ);
    strcpy(node->ethical_principle, principle);
    strcpy(node->domain, domain);
    node->weight=weight;
    node->behaviors=NULL;

    node->height=1;
    node->total_behavior_count=0;
    node->high_bias_count=0;
    node->medium_transparency_count=0;
    node->low_transparency_count=0;
    node->fairness_hits_count=0;
    node->sum_alignment=0.0;
    node->thread_prev=NULL;
    node->thread_next=NULL;

    node->left=node->right=NULL;
    return node;
}

PrincipleNode* insert_principle(PrincipleNode *root, PrincipleNode *node) {
    int cmp;
    int balance;

    if (!root) return node;

    cmp=strcmp(node->ethical_principle, root->ethical_principle);
    if (cmp<0) {
        root->left=insert_principle(root->left, node);
    } else if (cmp>0) {
        root->right=insert_principle(root->right, node);
    } else {
        return root;
    }

    root->height=1+max_int(principle_height(root->left), principle_height(root->right));
    balance=get_balance(root);

    if (balance>1 && strcmp(node->ethical_principle, root->left->ethical_principle)<0) {
        return rotate_right(root);
    }
    if (balance<-1 && strcmp(node->ethical_principle, root->right->ethical_principle)>0) {
        return rotate_left(root);
    }
    if (balance>1 && strcmp(node->ethical_principle, root->left->ethical_principle)>0) {
        root->left=rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance<-1 && strcmp(node->ethical_principle, root->right->ethical_principle)<0) {
        root->right=rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}

PrincipleNode* find_principle(PrincipleNode *root, char *principle) {
    while (root) {
        int cmp=strcmp(principle, root->ethical_principle);
        if (cmp==0) return root;
        if (cmp<0) root=root->left;
        else root=root->right;
    }
    return NULL;
}

PrincipleNode* load_ethics(char *filename) {
    FILE *fp=fopen(filename, "r");
    char line[512];
    PrincipleNode *root=NULL;

    if (!fp) {
        perror("Failed to open ethics.csv");
        return NULL;
    }

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        int id;
        char civ[64], principle[128], domain[64];
        double weight;

        if (sscanf(line, "%d,%63[^,],%127[^,],%63[^,],%lf",
                   &id, civ, principle, domain, &weight)==5) {
            PrincipleNode *node=create_principle(id, civ, principle, domain, weight);
            if (node) root=insert_principle(root, node);
        }
    }

    fclose(fp);
    rebuild_threads(root);
    return root;
}

void free_behavior_list(BehaviorNode *head) {
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

void free_all_data(PrincipleNode *root) {
    free_ethics_tree(root);
    reset_behavior_index();
}
