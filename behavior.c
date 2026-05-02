#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "db.h"

static int is_valid_transparency(char *value) {
    return strcmp(value, "Low")==0 || strcmp(value, "Medium")==0 || strcmp(value, "High")==0;
}

static void sanitize_csv_field(char *s) {
    int i;
    int w=0;
    char tmp[128];
    if (!s) return;
    for (i=0; s[i] && w<127; i++) {
        char c=s[i];
        if (c=='\r' || c=='\n') continue;
        if (c==',' || c=='|') c=' ';
        if ((unsigned char)c<32) continue;
        tmp[w++]=c;
    }
    while (w>0 && isspace((unsigned char)tmp[w-1])) w--;
    tmp[w]='\0';
    strcpy(s, tmp);
}

static int read_command_output(char *command, char *out, int out_size) {
    FILE *pipe;
    int total=0;
    int n;

    if (!command || !out || out_size<=0) return 0;
    out[0]='\0';

#ifdef _WIN32
    pipe=_popen(command, "r");
#else
    pipe=popen(command, "r");
#endif
    if (!pipe) return 0;

    while (!feof(pipe) && total<out_size-1) {
        n=(int)fread(out+total, 1, (size_t)(out_size-1-total), pipe);
        if (n<=0) break;
        total+=n;
    }
    out[total]='\0';

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return total;
}

static int extract_json_response_text(char *json, char *out, int out_size) {
    char *p;
    int w=0;
    if (!json || !out || out_size<=0) return 0;
    out[0]='\0';

    p=strstr(json, "\"response\":\"");
    if (!p) return 0;
    p+=12;

    while (*p && w<out_size-1) {
        if (*p=='\\') {
            p++;
            if (*p=='n') out[w++]='\n';
            else if (*p=='r') {
                /* ignore */
            } else if (*p=='t') out[w++]='\t';
            else if (*p=='\\') out[w++]='\\';
            else if (*p=='\"') out[w++]='\"';
            else if (*p=='/') out[w++]='/';
            else if (*p=='\0') break;
            else out[w++]=*p;
            if (*p) p++;
            continue;
        }

        if (*p=='\"') break;
        out[w++]=*p;
        p++;
    }
    out[w]='\0';
    return w;
}

static int next_behavior_record_id(char *filename) {
    FILE *fp=fopen(filename, "r");
    char line[512];
    int max_id=0;

    if (!fp) return 1;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        int rec_id=0;
        if (sscanf(line, "%d,", &rec_id)==1 && rec_id>max_id) {
            max_id=rec_id;
        }
    }
    fclose(fp);
    return max_id+1;
}

static void load_ai_system_ids(char *filename, int *ids, int *count, int max_ids) {
    FILE *fp=fopen(filename, "r");
    char line[1024];
    int c=0;
    if (!ids || !count || max_ids<=0) return;
    *count=0;
    if (!fp) return;

    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp) && c<max_ids) {
        int ai_id;
        if (sscanf(line, "%d,", &ai_id)==1) {
            ids[c++]=ai_id;
        }
    }
    fclose(fp);
    *count=c;
}

static void build_ai_id_list(char *buffer, int buffer_size, int *ids, int count) {
    int i;
    int used=0;
    if (!buffer || buffer_size<=0) return;
    buffer[0]='\0';
    if (!ids || count<=0) {
        snprintf(buffer, (size_t)buffer_size, "101,102,103");
        return;
    }
    for (i=0; i<count; i++) {
        int written=snprintf(buffer+used, (size_t)(buffer_size-used), (i==0)?"%d":",%d", ids[i]);
        if (written<=0 || used+written>=buffer_size) break;
        used+=written;
    }
}

int append_live_behaviors_from_ollama(char *behavior_csv, char *ai_systems_csv, char *model, int max_rows) {
    FILE *prompt_fp;
    FILE *out;
    int ids[64];
    int id_count=0;
    int next_id;
    char id_list[256];
    char prompt[2500];
    char command[1024];
    char response_text[32768];
    char *line;
    int appended=0;

    if (!behavior_csv || !model || max_rows<=0) return 0;

    load_ai_system_ids(ai_systems_csv, ids, &id_count, 64);
    build_ai_id_list(id_list, sizeof(id_list), ids, id_count);

    snprintf(prompt, sizeof(prompt),
             "Generate %d realistic NEW AI behavior events for a live ethics monitoring system. "
             "Return ONLY plain text lines with this exact pipe-delimited schema and no header: "
             "ai_system_id|decision_type|affected_group|outcome|transparency\\n"
             "Rules: ai_system_id must be one of [%s]. transparency must be exactly High or Medium or Low. "
             "Keep decision_type and group short (max 3 words), avoid commas and pipes. "
             "outcome should be one word from: Ranked, Approved, Rejected, Biased, Balanced, Reviewed.",
             max_rows, id_list);

    prompt_fp=fopen("ollama_prompt_tmp.txt", "w");
    if (!prompt_fp) return 0;
    fprintf(prompt_fp, "%s", prompt);
    fclose(prompt_fp);

#ifdef _WIN32
    snprintf(command, sizeof(command),
             "type ollama_prompt_tmp.txt | ollama run %s",
             model);
#else
    snprintf(command, sizeof(command),
             "cat ollama_prompt_tmp.txt | ollama run %s",
             model);
#endif

    if (read_command_output(command, response_text, (int)sizeof(response_text))<=0) {
        remove("ollama_prompt_tmp.txt");
        return 0;
    }
    remove("ollama_prompt_tmp.txt");

    out=fopen(behavior_csv, "a");
    if (!out) return 0;

    next_id=next_behavior_record_id(behavior_csv);
    line=strtok(response_text, "\n");
    while (line && appended<max_rows) {
        int ai_id;
        char decision[128];
        char group[128];
        char outcome[128];
        char transparency[16];
        if (sscanf(line, "%d|%127[^|]|%127[^|]|%127[^|]|%15s",
                   &ai_id, decision, group, outcome, transparency)==5) {
            sanitize_csv_field(decision);
            sanitize_csv_field(group);
            sanitize_csv_field(outcome);
            sanitize_csv_field(transparency);
            if (is_valid_transparency(transparency)) {
                fprintf(out, "%d,%d,%s,%s,%s,%s\n",
                        next_id++, ai_id, decision, group, outcome, transparency);
                /* Also persist to MySQL (source = "ollama") */
                db_insert_behavior(ai_id, decision, group, outcome, transparency, "ollama");
                appended++;
            }
        }
        line=strtok(NULL, "\n");
    }

    fclose(out);
    return appended;
}

int contains_any(char *text, char *keys[], int n) {
    int i;
    for (i=0; i<n; i++) {
        if (strstr(text, keys[i])) return 1;
    }
    return 0;
}

int should_attach(PrincipleNode *p, BehaviorNode *b) {
    char *fairness_keys[] = {"hiring", "candidate", "screen", "rank", "recruit", "loan", "credit"};
    char *compassion_keys[] = {"minority", "underrepresented", "patient", "student", "welfare"};
    char *harmony_keys[] = {"recommend", "learning", "govern", "social", "community"};
    char *justice_keys[] = {"justice", "law", "diagnosis", "screen", "equal"};
    char *virtue_keys[] = {"behavior", "ranking", "decision", "review", "personal"};
    char *environment_keys[] = {"environment", "sustain", "carbon", "resource", "waste"};

    if (strstr(p->ethical_principle, "Dharma") || strstr(p->ethical_principle, "fairness")) {
        if (contains_any(b->decision_type, fairness_keys, 7) || strstr(b->outcome, "Rejected") || strstr(b->outcome, "Biased")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Compassion")) {
        if (contains_any(b->affected_group, compassion_keys, 5) || strstr(b->outcome, "Rejected")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Social harmony")) {
        if (contains_any(b->decision_type, harmony_keys, 5) || strstr(b->affected_group, "All")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Justice") || strstr(p->ethical_principle, "Adl")) {
        if (contains_any(b->decision_type, justice_keys, 5) || strstr(b->affected_group, "applicant") || strstr(b->affected_group, "Patients")) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Virtue")) {
        if (contains_any(b->decision_type, virtue_keys, 5)) {
            return 1;
        }
    }

    if (strstr(p->ethical_principle, "Environmental")) {
        if (contains_any(b->decision_type, environment_keys, 5) || contains_any(b->outcome, environment_keys, 5)) {
            return 1;
        }
    }

    return 0;
}

double behavior_alignment_value(PrincipleNode *p, BehaviorNode *b) {
    double t;
    double penalty;
    if (strcmp(b->transparency, "High")==0) t=0.9;
    else if (strcmp(b->transparency, "Medium")==0) t=0.6;
    else t=0.3;

    penalty=(strstr(b->outcome, "Biased") || strstr(b->outcome, "Rejected"))?0.7:1.0;
    return p->weight*t*penalty;
}

void update_principle_aggregates(PrincipleNode *p, BehaviorNode *b) {
    if (!p || !b) return;
    p->total_behavior_count++;
    p->sum_alignment+=behavior_alignment_value(p, b);

    if (strstr(b->outcome, "Biased") || strstr(b->outcome, "Rejected")) {
        p->high_bias_count++;
        p->fairness_hits_count++;
    }
    if (strcmp(b->transparency, "Medium")==0) p->medium_transparency_count++;
    if (strcmp(b->transparency, "Low")==0) {
        p->low_transparency_count++;
        p->fairness_hits_count++;
    }
}

BehaviorNode* clone_behavior(BehaviorNode *src) {
    BehaviorNode *b=(BehaviorNode*)malloc(sizeof(BehaviorNode));
    if (!b) return NULL;
    *b=*src;
    b->next=NULL;
    return b;
}

int behavior_index_bucket(int ai_id) {
    int b=ai_id%257;
    if (b<0) b+=257;
    return b;
}

BehaviorRef *behavior_index[257]={0};

void reset_behavior_index(void) {
    int i;
    for (i=0; i<257; i++) {
        BehaviorRef *cur=behavior_index[i];
        while (cur) {
            BehaviorRef *next=cur->next;
            free(cur);
            cur=next;
        }
        behavior_index[i]=NULL;
    }
}

void add_behavior_to_index(int ai_id, PrincipleNode *principle, BehaviorNode *behavior) {
    int bucket=behavior_index_bucket(ai_id);
    BehaviorRef *ref=(BehaviorRef*)malloc(sizeof(BehaviorRef));
    if (!ref) return;
    ref->ai_system_id=ai_id;
    ref->principle=principle;
    ref->behavior=behavior;
    ref->next=behavior_index[bucket];
    behavior_index[bucket]=ref;
}

BehaviorRef* get_behavior_refs_for_ai(int ai_id) {
    int bucket=behavior_index_bucket(ai_id);
    BehaviorRef *cur=behavior_index[bucket];
    BehaviorRef *head=NULL;
    BehaviorRef *tail=NULL;

    while (cur) {
        if (cur->ai_system_id==ai_id) {
            BehaviorRef *copy=(BehaviorRef*)malloc(sizeof(BehaviorRef));
            if (!copy) break;
            *copy=*cur;
            copy->next=NULL;
            if (!head) {
                head=tail=copy;
            } else {
                tail->next=copy;
                tail=copy;
            }
        }
        cur=cur->next;
    }

    return head;
}

void free_behavior_refs(BehaviorRef *head) {
    while (head) {
        BehaviorRef *next=head->next;
        free(head);
        head=next;
    }
}

void attach_behavior_recursive(PrincipleNode *root, BehaviorNode *b) {
    BehaviorNode *copy;
    if (!root || !b) return;

    attach_behavior_recursive(root->left, b);

    if (should_attach(root, b)) {
        copy=clone_behavior(b);
        if (copy) {
            copy->next=root->behaviors;
            root->behaviors=copy;
            update_principle_aggregates(root, copy);
            add_behavior_to_index(copy->ai_system_id, root, copy);
        }
    }

    attach_behavior_recursive(root->right, b);
}

BehaviorNode* create_behavior(int record_id, int ai_id,
                              char *decision,
                              char *group,
                              char *outcome,
                              char *trans) {
    BehaviorNode *b=malloc(sizeof(BehaviorNode));
    if (!b) return NULL;
    b->record_id=record_id;
    b->ai_system_id=ai_id;
    strcpy(b->decision_type, decision);
    strcpy(b->affected_group, group);
    strcpy(b->outcome, outcome);
    strcpy(b->transparency, trans);
    b->next=NULL;
    return b;
}

void load_behaviors(char *filename, PrincipleNode *ethics_root) {
    FILE *fp=fopen(filename, "r");
    if (!fp) {
        perror("Failed to open ai_behavior.csv");
        return;
    }
    reset_behavior_index();
    char line[512];
    fgets(line, sizeof(line), fp); // header

    while (fgets(line, sizeof(line), fp)) {
        int rec_id, ai_id;
        char decision[128], group[128], outcome[128], trans[16];

        // Ignore incompatible lines from external datasets accidentally appended.
        if (line[0]<'0' || line[0]>'9') {
            continue;
        }

        if (sscanf(line, "%d,%d,%127[^,],%127[^,],%127[^,],%15s",
                   &rec_id, &ai_id, decision, group, outcome, trans)==6) {
            BehaviorNode *b=create_behavior(rec_id, ai_id, decision, group, outcome, trans);
            if (!b) continue;
            attach_behavior_recursive(ethics_root, b);
            free(b);
        }
    }
    fclose(fp);
}
