set terminal png size 800,600
set output 'ai_ethics_chart.png'
set title 'AI Ethics Scores'
set ylabel 'Score (%))'
set xlabel 'AI System'
set style data histogram
set style fill solid border -1
set boxwidth 0.9
set xtic rotate by -45 scale 0
set yrange [0:100]
plot '-' using 2:xtic(1) title 'Overall Score' linecolor rgb '#3498db'
'HireSmart AI' 41
'MedAssist AI' 59
'EduGuide AI' 60
e
