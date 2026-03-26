#include <stdio.h>
#include <string.h>
#include "types.h"

// ASCII Bar Chart in Terminal
void print_ascii_chart(const char *title, const char *labels[], const double values[], int count) {
    int i, j;
    const int max_bar_width = 50;

    printf("\n%s\n", title);
    printf("=============================================================================\n");

    for (i = 0; i < count; i++) {
        int bar_length = (int)(values[i] * max_bar_width);
        printf("%-20s | ", labels[i]);

        // Draw bar
        for (j = 0; j < bar_length; j++) {
            printf("█");
        }

        printf(" %.0f%%\n", values[i] * 100.0);
    }

    printf("=============================================================================\n");
}

// Multi-series comparison chart
void print_comparison_chart(const char *title,
                           const char *ai_names[],
                           const double ethical[],
                           const double cultural[],
                           const double overall[],
                           int count) {
    int i, j;
    const int max_bar_width = 40;

    printf("\n%s\n", title);
    printf("=============================================================================\n");

    for (i = 0; i < count; i++) {
        printf("\n%s:\n", ai_names[i]);

        // Ethical Alignment
        printf("  Ethical Align  | ");
        for (j = 0; j < (int)(ethical[i] * max_bar_width); j++) printf("█");
        printf(" %.0f%%\n", ethical[i] * 100.0);

        // Cultural Compatibility
        printf("  Cultural Fit   | ");
        for (j = 0; j < (int)(cultural[i] * max_bar_width); j++) printf("▓");
        printf(" %.0f%%\n", cultural[i] * 100.0);

        // Overall Score
        printf("  Overall Score  | ");
        for (j = 0; j < (int)(overall[i] * max_bar_width); j++) printf("▒");
        printf(" %.0f%%\n", overall[i] * 100.0);
    }

    printf("=============================================================================\n");
}

// Risk Level Visualization
void print_risk_chart(const char *ai_name, const RiskSummary risks[], int risk_count) {
    int i, j;
    const int max_bar_width = 50;

    printf("\n%s - Risk Assessment\n", ai_name);
    printf("=============================================================================\n");

    for (i = 0; i < risk_count; i++) {
        int bar_length = (int)(risks[i].score * max_bar_width);
        char *color;

        // Color coding with ASCII
        if (strcmp(risks[i].risk_level, "High") == 0) color = "🔴";
        else if (strcmp(risks[i].risk_level, "Medium") == 0) color = "🟡";
        else color = "🟢";

        printf("%-23s %s | ", risks[i].risk_category, color);

        for (j = 0; j < bar_length; j++) {
            printf("■");
        }

        printf(" %.0f%% (%s)\n", risks[i].score * 100.0, risks[i].risk_level);
    }

    printf("=============================================================================\n");
}

// Generate HTML Dashboard with Chart.js
int generate_html_dashboard(const char *filename,
                           const AISystem *systems,
                           const double ethical[],
                           const double cultural[],
                           const double overall[],
                           int system_count) {
    FILE *fp = fopen(filename, "w");
    int i;

    if (!fp) return 0;

    fprintf(fp, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(fp, "    <meta charset=\"UTF-8\">\n");
    fprintf(fp, "    <title>AI Civilization Ethics Dashboard</title>\n");
    fprintf(fp, "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n");
    fprintf(fp, "    <style>\n");
    fprintf(fp, "        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n");
    fprintf(fp, "        h1 { color: #333; text-align: center; }\n");
    fprintf(fp, "        .chart-container { width: 80%%; margin: 20px auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n");
    fprintf(fp, "        .legend { text-align: center; margin: 10px; font-size: 14px; color: #666; }\n");
    fprintf(fp, "    </style>\n");
    fprintf(fp, "</head>\n<body>\n");

    fprintf(fp, "    <h1>🌍 AI Civilization Ethics Dashboard</h1>\n");

    // Overall Scores Chart
    fprintf(fp, "    <div class=\"chart-container\">\n");
    fprintf(fp, "        <h2>Overall Ethical Scores Comparison</h2>\n");
    fprintf(fp, "        <canvas id=\"overallChart\"></canvas>\n");
    fprintf(fp, "    </div>\n");

    // Detailed Metrics Chart
    fprintf(fp, "    <div class=\"chart-container\">\n");
    fprintf(fp, "        <h2>Detailed Ethical Metrics</h2>\n");
    fprintf(fp, "        <canvas id=\"detailedChart\"></canvas>\n");
    fprintf(fp, "    </div>\n");

    // Radar Chart
    fprintf(fp, "    <div class=\"chart-container\">\n");
    fprintf(fp, "        <h2>Multi-dimensional Ethics Profile</h2>\n");
    fprintf(fp, "        <canvas id=\"radarChart\"></canvas>\n");
    fprintf(fp, "    </div>\n");

    fprintf(fp, "    <script>\n");

    // Data arrays
    fprintf(fp, "        const aiSystems = [");
    for (i = 0; i < system_count; i++) {
        fprintf(fp, "'%s'%s", systems[i].system_name, (i < system_count - 1) ? ", " : "");
    }
    fprintf(fp, "];\n");

    fprintf(fp, "        const ethicalScores = [");
    for (i = 0; i < system_count; i++) {
        fprintf(fp, "%.2f%s", ethical[i] * 100, (i < system_count - 1) ? ", " : "");
    }
    fprintf(fp, "];\n");

    fprintf(fp, "        const culturalScores = [");
    for (i = 0; i < system_count; i++) {
        fprintf(fp, "%.2f%s", cultural[i] * 100, (i < system_count - 1) ? ", " : "");
    }
    fprintf(fp, "];\n");

    fprintf(fp, "        const overallScores = [");
    for (i = 0; i < system_count; i++) {
        fprintf(fp, "%.2f%s", overall[i] * 100, (i < system_count - 1) ? ", " : "");
    }
    fprintf(fp, "];\n\n");

    // Overall Bar Chart
    fprintf(fp, "        new Chart(document.getElementById('overallChart'), {\n");
    fprintf(fp, "            type: 'bar',\n");
    fprintf(fp, "            data: {\n");
    fprintf(fp, "                labels: aiSystems,\n");
    fprintf(fp, "                datasets: [{\n");
    fprintf(fp, "                    label: 'Overall Ethical Score (%%)',\n");
    fprintf(fp, "                    data: overallScores,\n");
    fprintf(fp, "                    backgroundColor: ['#e74c3c', '#3498db', '#2ecc71'],\n");
    fprintf(fp, "                    borderWidth: 1\n");
    fprintf(fp, "                }]\n");
    fprintf(fp, "            },\n");
    fprintf(fp, "            options: {\n");
    fprintf(fp, "                responsive: true,\n");
    fprintf(fp, "                scales: { y: { beginAtZero: true, max: 100 } }\n");
    fprintf(fp, "            }\n");
    fprintf(fp, "        });\n\n");

    // Grouped Bar Chart
    fprintf(fp, "        new Chart(document.getElementById('detailedChart'), {\n");
    fprintf(fp, "            type: 'bar',\n");
    fprintf(fp, "            data: {\n");
    fprintf(fp, "                labels: aiSystems,\n");
    fprintf(fp, "                datasets: [\n");
    fprintf(fp, "                    {\n");
    fprintf(fp, "                        label: 'Ethical Alignment',\n");
    fprintf(fp, "                        data: ethicalScores,\n");
    fprintf(fp, "                        backgroundColor: '#9b59b6'\n");
    fprintf(fp, "                    },\n");
    fprintf(fp, "                    {\n");
    fprintf(fp, "                        label: 'Cultural Compatibility',\n");
    fprintf(fp, "                        data: culturalScores,\n");
    fprintf(fp, "                        backgroundColor: '#f39c12'\n");
    fprintf(fp, "                    },\n");
    fprintf(fp, "                    {\n");
    fprintf(fp, "                        label: 'Overall Score',\n");
    fprintf(fp, "                        data: overallScores,\n");
    fprintf(fp, "                        backgroundColor: '#1abc9c'\n");
    fprintf(fp, "                    }\n");
    fprintf(fp, "                ]\n");
    fprintf(fp, "            },\n");
    fprintf(fp, "            options: {\n");
    fprintf(fp, "                responsive: true,\n");
    fprintf(fp, "                scales: { y: { beginAtZero: true, max: 100 } }\n");
    fprintf(fp, "            }\n");
    fprintf(fp, "        });\n\n");

    // Radar Chart
    fprintf(fp, "        new Chart(document.getElementById('radarChart'), {\n");
    fprintf(fp, "            type: 'radar',\n");
    fprintf(fp, "            data: {\n");
    fprintf(fp, "                labels: ['Ethical Alignment', 'Cultural Compatibility', 'Overall Score'],\n");
    fprintf(fp, "                datasets: [\n");
    for (i = 0; i < system_count; i++) {
        const char *colors[] = {"#e74c3c", "#3498db", "#2ecc71"};
        fprintf(fp, "                    {\n");
        fprintf(fp, "                        label: '%s',\n", systems[i].system_name);
        fprintf(fp, "                        data: [%.2f, %.2f, %.2f],\n",
                ethical[i] * 100, cultural[i] * 100, overall[i] * 100);
        fprintf(fp, "                        borderColor: '%s',\n", colors[i % 3]);
        fprintf(fp, "                        backgroundColor: '%s33'\n", colors[i % 3]);
        fprintf(fp, "                    }%s\n", (i < system_count - 1) ? "," : "");
    }
    fprintf(fp, "                ]\n");
    fprintf(fp, "            },\n");
    fprintf(fp, "            options: {\n");
    fprintf(fp, "                responsive: true,\n");
    fprintf(fp, "                scales: { r: { beginAtZero: true, max: 100 } }\n");
    fprintf(fp, "            }\n");
    fprintf(fp, "        });\n");

    fprintf(fp, "    </script>\n");
    fprintf(fp, "</body>\n</html>\n");

    fclose(fp);
    return 1;
}

// Generate GNUplot script
int generate_gnuplot_script(const char *filename,
                            const AISystem *systems,
                            const double scores[],
                            int system_count) {
    FILE *fp = fopen(filename, "w");
    int i;

    if (!fp) return 0;

    fprintf(fp, "set terminal png size 800,600\n");
    fprintf(fp, "set output 'ai_ethics_chart.png'\n");
    fprintf(fp, "set title 'AI Ethics Scores'\n");
    fprintf(fp, "set ylabel 'Score (%%))'\n");
    fprintf(fp, "set xlabel 'AI System'\n");
    fprintf(fp, "set style data histogram\n");
    fprintf(fp, "set style fill solid border -1\n");
    fprintf(fp, "set boxwidth 0.9\n");
    fprintf(fp, "set xtic rotate by -45 scale 0\n");
    fprintf(fp, "set yrange [0:100]\n");
    fprintf(fp, "plot '-' using 2:xtic(1) title 'Overall Score' linecolor rgb '#3498db'\n");

    for (i = 0; i < system_count; i++) {
        fprintf(fp, "'%s' %.0f\n", systems[i].system_name, scores[i] * 100);
    }
    fprintf(fp, "e\n");

    fclose(fp);
    return 1;
}
