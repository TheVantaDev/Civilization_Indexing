# AI Civilisation - Ethical Alignment System

A C-based system that evaluates AI systems' ethical alignment across multiple civilizational ethical principles. This project analyzes AI decision-making behaviors against established ethical frameworks from diverse cultural traditions.

## Project Overview

This system implements a tree-based data structure to organize ethical principles and measures how AI systems align with these principles based on their decision-making behaviors. It combines ethical philosophy with computational analysis to provide ethical alignment scores.

## Features

- **Multi-Principle Ethics Framework**: Incorporates ethical principles from various civilizations (Sanatan Dharma, Buddhism, Confucianism, Islamic Ethics, Greek Philosophy, Indigenous Traditions)
- **Behavior Tracking**: Monitor AI system decisions and their alignment with ethical principles
- **Tree-Based Organization**: Binary search tree structure for efficient principle lookup
- **Alignment Scoring**: Calculate ethical alignment scores based on behavior attributes (transparency, bias, outcomes)
- **Culture Support**: Track cultural priorities and sensitivities

## Project Structure

```
├── main.c              # Entry point and program orchestration
├── ethics.c            # Ethical principle management (BST implementation)
├── culture.c           # Cultural data loading and management
├── behavior.c          # AI behavior tracking and attachment
├── scoring.c           # Alignment score computation
├── types.h             # Data structure definitions
├── ethics.csv          # Ethical principles dataset
├── ai_systems.csv      # AI system metadata dataset
├── culture.csv         # Cultural priorities dataset
├── ai_behavior.csv     # AI system behavior records
└── README.md           # This file
```

## Data Structures

### PrincipleNode
Binary search tree node containing:
- Ethical principle details (ID, civilization, principle name, domain, weight)
- Linked list of associated behaviors
- Left and right child pointers

### BehaviorNode
Linked list node representing AI decisions:
- Record ID and AI system ID
- Decision type, affected group, outcome
- Transparency level

### Culture
Structure containing cultural information:
- Culture ID, region
- Priority (e.g., fairness, privacy)
- Sensitivity level

### AISystem
Structure containing AI system metadata:
- AI system ID and system name
- Application domain and developer
- Deployment region

## Compilation

### Requirements
- GCC compiler
- Windows/Unix environment

### Build Command
```bash
gcc main.c ethics.c culture.c behavior.c scoring.c -o main.exe
```

## Usage

### Running the Program
```bash
.\main.exe
```

### Output
The program generates sponsor-aligned ethical intelligence outputs:
```
1. Intermediate AI scoring table by ethical principle
2. Cultural compatibility table by region
3. Ethical risk detection table with risk levels
4. Final Ethical AI Civilization Index table
5. Recommendations and projected score improvements
6. Dashboard summary with global ethics rank
```

## File Descriptions

### main.c
- Program entry point
- Loads ethics principles, AI metadata, cultures, and behaviors
- Computes principle-level alignment, culture compatibility, and risk summaries
- Prints final Ethical AI Civilization Index, recommendations, and dashboard blocks

### ethics.c
- `create_principle()`: Create new ethical principle nodes
- `insert_principle()`: Insert principles into BST
- `find_principle()`: Search for principles in BST
- `load_ethics()`: Load principles from CSV file

### culture.c
- `load_cultures()`: Parse and load cultural data from CSV
- Stores up to 10 cultures in memory

### behavior.c
- `create_behavior()`: Create behavior records
- Rule-based mapping: Link behaviors to relevant ethical principles
- `load_behaviors()`: Parse AI behavior data from CSV

### scoring.c
- `transparency_to_score()`: Convert transparency levels to scores
- `bias_penalty()`: Apply penalties for biased or rejected outcomes
- `compute_principle_alignment_for_ai()`: Calculate principle-wise alignment
- `compute_culture_compatibility_for_ai()`: Calculate region compatibility
- `evaluate_risks_for_ai()`: Detect risk categories and levels

### types.h
- `PrincipleNode`: Binary search tree structure for ethics
- `BehaviorNode`: Linked list for tracking behaviors
- `Culture`: Culture information structure
- `AISystem`: AI metadata structure
- `RiskSummary`: Risk category and severity output structure
- Function declarations for module interfaces

## CSV Data Format

### ethics.csv
```
ethics_id,civilization,ethical_principle,domain,weight
1,Sanatan Dharma,Dharma (Duty & fairness),Justice,0.9
2,Buddhism,Compassion,Social welfare,0.85
...
```

### culture.csv
```
culture_id,culture_region,ethical_priority,sensitivity_level
1,South Asia,fairness,High
2,Europe,privacy,High
...
```

### ai_systems.csv
```
ai_system_id,system_name,application_domain,developer,deployment_region
101,HireSmart AI,Recruitment,TechCorp,Global
102,MedAssist AI,Healthcare diagnosis,HealthTech Labs,Europe
...
```

### ai_behavior.csv
```
record_id,ai_system_id,decision_type,affected_group,outcome,transparency
1,101,Candidate screening,Minority applicants,Rejected,Low
2,101,Candidate ranking,All applicants,Ranked,Medium
...
```

## Algorithm Details

### Alignment Score Calculation
1. For each AI system, traverse the ethics principles tree
2. Find all behaviors associated with that AI system
3. Calculate individual behavior scores based on:
   - Principle weight
   - Transparency level (High=0.9, Medium=0.6, Low=0.3)
   - Bias penalty (Biased/Rejected outcomes → 0.7, others → 1.0)
4. Average scores across the tree (in-order traversal)

### Transparency Scoring
- **High**: 0.9
- **Medium**: 0.6
- **Low**: 0.3

### Bias Penalties
- **Biased or Rejected outcomes**: 0.7 multiplier
- **Other outcomes**: 1.0 multiplier (no penalty)

## Ethical Frameworks

The system incorporates principles from:
- **Sanatan Dharma**: Focus on duty and fairness
- **Buddhism**: Emphasis on compassion
- **Confucianism**: Social harmony
- **Islamic Ethics**: Justice (Adl)
- **Greek Philosophy**: Virtue ethics
- **Indigenous Traditions**: Environmental stewardship

## Compilation Notes

- Uses standard C libraries (stdio.h, stdlib.h, string.h)
- No external dependencies required
- Compatible with GCC on Windows/Unix systems
- Uses POSIX file I/O for CSV parsing

## Future Enhancements

- Add more AI systems and behavior records
- Implement dynamic principle weighting
- Add statistical analysis of alignment trends
- Support for more detailed behavior attributes
- Web-based visualization interface
- Multi-threaded processing for large datasets

## Author Notes

This project demonstrates:
- Tree data structure implementation
- Linked list management
- CSV file parsing in C
- Recursive algorithm design
- Cross-cultural ethical analysis

## License

Open source project - Use freely for educational purposes.

## Contact

For questions or contributions, please open an issue or submit a pull request.
