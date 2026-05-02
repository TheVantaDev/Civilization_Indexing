/* scenarios.c  –  Pre-defined real-world AI ethics scenarios
 *
 * Each scenario is tied to one AI system. When the program runs,
 * it sends a situation-specific prompt to Ollama so the generated
 * behavior rows are contextually meaningful, not random.
 *
 * Add / edit scenarios here without touching any other file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "db.h"

/* ------------------------------------------------------------------ *
 *  Scenario definition                                                *
 * ------------------------------------------------------------------ */
typedef struct {
    int  ai_system_id;
    char name[128];
    char situation[2048];   /* the real-world case description */
} Scenario;

/* ------------------------------------------------------------------ *
 *  Pre-defined scenarios  (one per AI system)                        *
 *  Edit these to change what case study gets analyzed each run.      *
 * ------------------------------------------------------------------ */
static Scenario SCENARIOS[] = {
    {
        101,
        "HireSmart AI – Tech Hiring Discrimination Case",
        "A Fortune 500 software company's AI hiring tool is under investigation after "
        "internal audits revealed it rejects 70 percent of female applicants at the resume "
        "screening stage. The AI was trained on 10 years of historical hiring data that "
        "already skewed heavily toward male candidates from IIT colleges. Candidates from "
        "Tier-2 cities and non-English-medium backgrounds are automatically flagged as "
        "low priority regardless of their actual skills or experience. Affected groups "
        "include women, applicants from rural districts, and candidates without brand-name "
        "college degrees. The company provides no transparency into how decisions are made."
    },
    {
        102,
        "MedAssist AI – Rural Healthcare Inequality Case",
        "A private hospital chain across 12 cities uses an AI for medical triage and "
        "treatment approval decisions. Reports from doctors show that elderly patients aged "
        "65 and above from rural districts receive Rejected outcomes for standard procedures "
        "at three times the rate of urban patients with identical diagnoses. The AI scores "
        "patients on an undisclosed risk index and approves or rejects treatment plans "
        "without providing any reasoning to the attending physicians. Minority communities "
        "and patients with lower income indicators are disproportionately affected. "
        "Whistleblowers claim the model penalizes patients who lack private insurance. "
        "Transparency is near zero and no human override mechanism exists."
    },
    {
        103,
        "EduGuide AI – Socioeconomic Educational Bias Case",
        "A government-backed education AI platform deployed across 500 schools in 5 states "
        "recommends academic tracks to students. Analysis by education researchers shows "
        "the AI consistently recommends advanced STEM and honors tracks to students from "
        "upper-income schools while routing students from low-income government schools to "
        "vocational and basic tracks regardless of their actual exam scores. Students from "
        "minority communities and first-generation learners face a 45 percent lower "
        "acceptance rate for advanced program recommendations compared to peers with "
        "identical grades from wealthier backgrounds. The platform affects 2 million "
        "students annually and its recommendation logic has never been audited. "
        "Parents and teachers are given no explanation when a student is downgraded."
    }
};

#define SCENARIO_COUNT ((int)(sizeof(SCENARIOS)/sizeof(SCENARIOS[0])))

/* ------------------------------------------------------------------ *
 *  Private helpers (same as in behavior.c — kept local)             *
 * ------------------------------------------------------------------ */
static int sc_is_valid_transparency(const char *v) {
    return strcmp(v,"Low")==0 || strcmp(v,"Medium")==0 || strcmp(v,"High")==0;
}

static void sc_sanitize(char *s) {
    int i, w=0;
    char tmp[256];
    if (!s) return;
    for (i=0; s[i] && w<255; i++) {
        char c=s[i];
        if (c=='\r'||c=='\n') continue;
        if (c==','||c=='|')   c=' ';
        if ((unsigned char)c<32) continue;
        tmp[w++]=c;
    }
    while (w>0 && (unsigned char)tmp[w-1]<=32) w--;
    tmp[w]='\0';
    strcpy(s,tmp);
}

static int sc_read_pipe(const char *cmd, char *out, int sz) {
    FILE *p;
    int total=0, n;
    out[0]='\0';
#ifdef _WIN32
    p=_popen(cmd,"r");
#else
    p=popen(cmd,"r");
#endif
    if (!p) return 0;
    while (!feof(p) && total<sz-1) {
        n=(int)fread(out+total,1,(size_t)(sz-1-total),p);
        if (n<=0) break;
        total+=n;
    }
    out[total]='\0';
#ifdef _WIN32
    _pclose(p);
#else
    pclose(p);
#endif
    return total;
}

static int sc_next_id(const char *csv) {
    FILE *fp=fopen(csv,"r");
    char line[512];
    int max_id=0;
    if (!fp) return 1;
    fgets(line,sizeof(line),fp);
    while (fgets(line,sizeof(line),fp)) {
        int id=0;
        if (sscanf(line,"%d,",&id)==1 && id>max_id) max_id=id;
    }
    fclose(fp);
    return max_id+1;
}

/* ------------------------------------------------------------------ *
 *  Core function: run one scenario through Ollama                    *
 * ------------------------------------------------------------------ */
static int run_scenario(Scenario *sc, const char *behavior_csv,
                        const char *model, int rows_per_scenario) {
    char prompt[4096];
    char command[512];
    char response[32768];
    FILE *pf, *out;
    char *line;
    int appended=0;
    int next_id;

    /* Build the situation-aware prompt */
    snprintf(prompt, sizeof(prompt),
        "You are an AI ethics analyst. Read the following real-world case and generate "
        "%d realistic behavior log entries that this AI system (ID: %d) would produce "
        "based on the described situation.\n\n"
        "SITUATION:\n%s\n\n"
        "Return ONLY plain pipe-delimited lines, NO header, NO explanation, NO numbering:\n"
        "ai_system_id|decision_type|affected_group|outcome|transparency\n\n"
        "STRICT RULES:\n"
        "- ai_system_id must be exactly %d\n"
        "- decision_type: max 3 words, directly related to the situation\n"
        "- affected_group: a specific group mentioned or implied in the situation\n"
        "- outcome: exactly one of: Ranked, Approved, Rejected, Biased, Balanced, Reviewed\n"
        "- transparency: exactly one of: High, Medium, Low\n"
        "- No commas or pipes inside any field\n"
        "- Make outcomes reflect the bias or ethical problem described\n"
        "- Generate exactly %d lines, nothing else.",
        rows_per_scenario,
        sc->ai_system_id,
        sc->situation,
        sc->ai_system_id,
        rows_per_scenario);

    /* Write prompt to temp file */
    pf = fopen("ollama_scenario_tmp.txt","w");
    if (!pf) return 0;
    fprintf(pf,"%s",prompt);
    fclose(pf);

    /* Call Ollama */
#ifdef _WIN32
    snprintf(command,sizeof(command),
             "type ollama_scenario_tmp.txt | ollama run %s", model);
#else
    snprintf(command,sizeof(command),
             "cat ollama_scenario_tmp.txt | ollama run %s", model);
#endif

    printf("[SCENARIO] Running: %s\n", sc->name);
    if (sc_read_pipe(command, response, sizeof(response)) <= 0) {
        remove("ollama_scenario_tmp.txt");
        printf("[SCENARIO] Ollama did not respond for scenario: %s\n", sc->name);
        return 0;
    }
    remove("ollama_scenario_tmp.txt");

    /* Append valid rows to CSV + MySQL */
    out = fopen(behavior_csv, "a");
    if (!out) return 0;
    next_id = sc_next_id(behavior_csv);

    line = strtok(response, "\n");
    while (line && appended < rows_per_scenario) {
        int ai_id;
        char decision[128], group[128], outcome[128], trans[16];

        /* Skip lines that look like explanations or headers */
        if (line[0] < '0' || line[0] > '9') { line=strtok(NULL,"\n"); continue; }

        if (sscanf(line, "%d|%127[^|]|%127[^|]|%127[^|]|%15s",
                   &ai_id, decision, group, outcome, trans) == 5) {
            sc_sanitize(decision);
            sc_sanitize(group);
            sc_sanitize(outcome);
            sc_sanitize(trans);
            if (sc_is_valid_transparency(trans) && ai_id == sc->ai_system_id) {
                fprintf(out, "%d,%d,%s,%s,%s,%s\n",
                        next_id++, ai_id, decision, group, outcome, trans);
                db_insert_behavior(ai_id, decision, group, outcome, trans, "ollama_scenario");
                appended++;
            }
        }
        line = strtok(NULL, "\n");
    }

    fclose(out);
    printf("[SCENARIO] Appended %d contextual behavior rows for '%s'\n",
           appended, sc->name);
    return appended;
}

/* ------------------------------------------------------------------ *
 *  Public API: called from main.c                                    *
 * ------------------------------------------------------------------ */
int append_scenario_behaviors(const char *behavior_csv,
                              const char *model,
                              int rows_per_scenario) {
    int i;
    int total = 0;

    printf("\n========== SCENARIO-DRIVEN BEHAVIOR GENERATION ==========\n");
    for (i = 0; i < SCENARIO_COUNT; i++) {
        printf("\n[%d/%d] Scenario: %s\n", i+1, SCENARIO_COUNT, SCENARIOS[i].name);
        total += run_scenario(&SCENARIOS[i], behavior_csv, model, rows_per_scenario);
    }
    printf("\n[SCENARIO] Total scenario-based rows appended: %d\n", total);
    printf("==========================================================\n\n");
    return total;
}

/* ------------------------------------------------------------------ *
 *  Print all scenarios (for dashboard / logging)                            *
 * ------------------------------------------------------------------ */
void print_scenarios(void) {
    int i;
    printf("\nPre-defined Scenarios Loaded:\n");
    for (i = 0; i < SCENARIO_COUNT; i++) {
        printf("  [%d] AI System %d — %s\n",
               i+1, SCENARIOS[i].ai_system_id, SCENARIOS[i].name);
    }
    printf("\n");
}
