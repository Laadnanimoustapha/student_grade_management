#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr float GPA_SCALE = 25.0f; // Converts percentage (0-100) to GPA (0-4.0)
    constexpr std::array<const char *, 14> MAIN_MENU_OPTIONS = {
        "1. Add Student",
        "2. Display All Students",
        "3. Search Student",
        "4. Sort Students",
        "5. Update Student",
        "6. Delete Student",
        "7. Display Class Statistics",
        "8. Save Data",
        "9. Load Data",
        "10. Export to CSV",
        "11. Import from CSV",
        "12. Show Top Performers",
        "13. Filter Students",
        "14. Export Progress Reports"
    };
}

class Student {
public:
    std::string id;
    std::string name;
    std::vector<float> grades;

    float calculateGPA() const {
        if (grades.empty()) {
            return 0.0f;
        }

        float sum = std::accumulate(grades.begin(), grades.end(), 0.0f);
        float average = sum / static_cast<float>(grades.size());
        float gpa = average / GPA_SCALE;

        if (gpa > 4.0f) {
            gpa = 4.0f;
        }
        if (gpa < 0.0f) {
            gpa = 0.0f;
        }
        return gpa;
    }
};

class GradeManager {
private:
    std::vector<Student> students;

    static std::string trim(const std::string &text) {
        const std::string whitespace = " \t\r\n";
        std::size_t start = text.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return "";
        }
        std::size_t end = text.find_last_not_of(whitespace);
        return text.substr(start, end - start + 1);
    }

    static std::string toLowerCopy(const std::string &text) {
        std::string result(text);
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return result;
    }

    static bool tryParseInt(const std::string &text, int &value) {
        std::stringstream ss(trim(text));
        int temp = 0;
        if (ss >> temp && ss.eof()) {
            value = temp;
            return true;
        }
        return false;
    }

    static bool tryParseUnsigned(const std::string &text, std::size_t &value) {
        std::stringstream ss(trim(text));
        long long temp = 0;
        if (ss >> temp && temp >= 0 && ss.eof()) {
            value = static_cast<std::size_t>(temp);
            return true;
        }
        return false;
    }

    static bool tryParseFloat(const std::string &text, float &value) {
        std::stringstream ss(trim(text));
        float temp = 0.0f;
        if (ss >> temp && ss.eof()) {
            value = temp;
            return true;
        }
        return false;
    }

    bool idExists(const std::string &id, const Student *skip = nullptr) const {
        return std::any_of(students.begin(), students.end(), [&](const Student &s) {
            return &s != skip && s.id == id;
        });
    }


    void printStudentRow(const Student &student, int rank) const {
        std::cout << std::left << std::setw(25) << student.name
                  << std::setw(15) << student.id
                  << std::setw(10) << std::fixed << std::setprecision(2) << student.calculateGPA()
                  << std::setw(5) << rank << '\n';
    }

    static std::string formatGradeTrend(const std::vector<float> &grades) {
        if (grades.size() < 2) {
            return "N/A";
        }

        std::size_t upCount = 0;
        std::size_t downCount = 0;
        std::size_t steadyCount = 0;
        for (std::size_t i = 1; i < grades.size(); ++i) {
            const float diff = grades[i] - grades[i - 1];
            if (std::fabs(diff) < 1e-3f) {
                ++steadyCount;
            } else if (diff > 0.0f) {
                ++upCount;
            } else {
                ++downCount;
            }
        }

        std::stringstream trend;
        trend << upCount << "↑ " << downCount << "↓ " << steadyCount << "→";
        return trend.str();
    }

    void displayStudentDetails(const Student &student) const {
        std::cout << "\nStudent Details" << '\n';
        std::cout << std::string(50, '-') << '\n';
        std::cout << "Name : " << student.name << '\n';
        std::cout << "ID   : " << student.id << '\n';
        std::cout << "GPA  : " << std::fixed << std::setprecision(2) << student.calculateGPA() << '\n';
        std::cout << "Grades (" << student.grades.size() << ")";
        if (student.grades.empty()) {
            std::cout << ": None" << '\n';
        } else {
            std::cout << ":\n";
            for (std::size_t i = 0; i < student.grades.size(); ++i) {
                std::cout << "  [" << (i + 1) << "] " << std::fixed << std::setprecision(2) << student.grades[i] << '\n';
            }
        }
        std::cout << "Grade Trend       : " << formatGradeTrend(student.grades) << '\n';
        if (!student.grades.empty()) {
            std::cout << "Highest Grade     : " << std::fixed << std::setprecision(2)
                      << *std::max_element(student.grades.begin(), student.grades.end()) << '\n';
            std::cout << "Lowest Grade      : " << std::fixed << std::setprecision(2)
                      << *std::min_element(student.grades.begin(), student.grades.end()) << '\n';
        }
        std::cout << std::string(50, '-') << '\n';
    }

    std::vector<float> promptGradesFromUser() {
        std::vector<float> grades;
        std::string input;
        int gradeCount = 0;

        while (true) {
            std::cout << "Enter number of grades to input: ";
            std::getline(std::cin, input);
            if (tryParseInt(input, gradeCount) && gradeCount >= 0) {
                break;
            }
            std::cout << "Invalid number. Please enter a non-negative integer." << '\n';
        }

        for (int i = 0; i < gradeCount; ++i) {
            float grade = 0.0f;
            while (true) {
                std::cout << "Enter grade #" << (i + 1) << " (0-100): ";
                std::getline(std::cin, input);
                if (tryParseFloat(input, grade) && grade >= 0.0f && grade <= 100.0f) {
                    grades.push_back(grade);
                    break;
                }
                std::cout << "Invalid grade. Please enter a number between 0 and 100." << '\n';
            }
        }

        return grades;
    }

    void computeRanks(std::vector<int> &ranks) const {
        if (students.empty()) {
            ranks.clear();
            return;
        }

        std::vector<std::size_t> order(students.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
            float gpaA = students[a].calculateGPA();
            float gpaB = students[b].calculateGPA();
            if (std::fabs(gpaA - gpaB) > 1e-4f) {
                return gpaA > gpaB;
            }
            return students[a].name < students[b].name;
        });

        ranks.assign(students.size(), 0);
        float previousGpa = -1.0f;
        int currentRank = 0;

        for (std::size_t i = 0; i < order.size(); ++i) {
            float gpa = students[order[i]].calculateGPA();
            if (i == 0) {
                currentRank = 1;
            } else if (std::fabs(gpa - previousGpa) > 1e-4f) {
                currentRank = static_cast<int>(i + 1);
            }
            ranks[order[i]] = currentRank;
            previousGpa = gpa;
        }
    }

public:
    void addStudent() {
        Student student;
        std::string input;

        std::cout << "\n--- Add Student ---" << '\n';
        while (true) {
            std::cout << "Enter student ID: ";
            std::getline(std::cin, student.id);
            student.id = trim(student.id);

            if (student.id.empty()) {
                std::cout << "ID cannot be empty. Please try again." << '\n';
                continue;
            }
            if (idExists(student.id)) {
                std::cout << "ID already exists. Please enter a unique ID." << '\n';
                continue;
            }
            break;
        }

        std::cout << "Enter student name: ";
        std::getline(std::cin, student.name);
        student.name = trim(student.name);
        while (student.name.empty()) {
            std::cout << "Name cannot be empty. Enter student name: ";
            std::getline(std::cin, student.name);
            student.name = trim(student.name);
        }

        student.grades = promptGradesFromUser();
        students.push_back(std::move(student));
        std::cout << "Student added successfully." << '\n';
    }

    void updateStudent() {
        if (students.empty()) {
            std::cout << "\nNo students available to update." << '\n';
            return;
        }

        std::cout << "\n--- Update Student ---" << '\n';
        std::cout << "Enter student ID to update: ";
        std::string id;
        std::getline(std::cin, id);
        id = trim(id);

        auto it = std::find_if(students.begin(), students.end(), [&](const Student &s) {
            return s.id == id;
        });

        if (it == students.end()) {
            std::cout << "No student found with ID: " << id << '\n';
            return;
        }

        bool modifying = true;
        while (modifying) {
            displayStudentDetails(*it);
            std::cout << "Choose an option:" << '\n';
            std::cout << "1. Update Name" << '\n';
            std::cout << "2. Update ID" << '\n';
            std::cout << "3. Replace All Grades" << '\n';
            std::cout << "4. Add Grade" << '\n';
            std::cout << "5. Modify Grade" << '\n';
            std::cout << "6. Remove Grade" << '\n';
            std::cout << "0. Finish Updating" << '\n';
            std::cout << "Your choice: ";

            std::string input;
            std::getline(std::cin, input);
            int choice = -1;
            tryParseInt(input, choice);

            switch (choice) {
            case 1: {
                std::cout << "Enter new name: ";
                std::getline(std::cin, it->name);
                it->name = trim(it->name);
                if (it->name.empty()) {
                    std::cout << "Name cannot be empty. Keeping previous value." << '\n';
                }
                break;
            }
            case 2: {
                std::cout << "Enter new ID: ";
                std::string newId;
                std::getline(std::cin, newId);
                newId = trim(newId);
                if (newId.empty()) {
                    std::cout << "ID cannot be empty. Keeping previous value." << '\n';
                } else if (idExists(newId, &(*it))) {
                    std::cout << "ID already exists. Keeping previous value." << '\n';
                } else {
                    it->id = std::move(newId);
                    std::cout << "ID updated successfully." << '\n';
                }
                break;
            }
            case 3: {
                std::cout << "Replacing all grades." << '\n';
                it->grades = promptGradesFromUser();
                break;
            }
            case 4: {
                std::cout << "Enter grade to add (0-100): ";
                std::getline(std::cin, input);
                float grade = 0.0f;
                if (tryParseFloat(input, grade) && grade >= 0.0f && grade <= 100.0f) {
                    it->grades.push_back(grade);
                    std::cout << "Grade added." << '\n';
                } else {
                    std::cout << "Invalid grade. No changes made." << '\n';
                }
                break;
            }
            case 5: {
                if (it->grades.empty()) {
                    std::cout << "No grades to modify." << '\n';
                    break;
                }
                std::cout << "Enter grade index to modify (1-" << it->grades.size() << "): ";
                std::getline(std::cin, input);
                int index = 0;
                if (tryParseInt(input, index) && index >= 1 && static_cast<std::size_t>(index) <= it->grades.size()) {
                    std::cout << "Enter new grade (0-100): ";
                    std::getline(std::cin, input);
                    float grade = 0.0f;
                    if (tryParseFloat(input, grade) && grade >= 0.0f && grade <= 100.0f) {
                        it->grades[static_cast<std::size_t>(index) - 1] = grade;
                        std::cout << "Grade updated." << '\n';
                    } else {
                        std::cout << "Invalid grade. No changes made." << '\n';
                    }
                } else {
                    std::cout << "Invalid index. No changes made." << '\n';
                }
                break;
            }
            case 6: {
                if (it->grades.empty()) {
                    std::cout << "No grades to remove." << '\n';
                    break;
                }
                std::cout << "Enter grade index to remove (1-" << it->grades.size() << "): ";
                std::getline(std::cin, input);
                int index = 0;
                if (tryParseInt(input, index) && index >= 1 && static_cast<std::size_t>(index) <= it->grades.size()) {
                    it->grades.erase(it->grades.begin() + (index - 1));
                    std::cout << "Grade removed." << '\n';
                } else {
                    std::cout << "Invalid index. No changes made." << '\n';
                }
                break;
            }
            case 0:
                modifying = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << '\n';
                break;
            }
        }
    }

    void deleteStudent() {
        if (students.empty()) {
            std::cout << "\nNo students available to delete." << '\n';
            return;
        }

        std::cout << "\n--- Delete Student ---" << '\n';
        std::cout << "Enter student ID to delete: ";
        std::string id;
        std::getline(std::cin, id);
        id = trim(id);

        auto it = std::find_if(students.begin(), students.end(), [&](const Student &s) {
            return s.id == id;
        });

        if (it == students.end()) {
            std::cout << "No student found with ID: " << id << '\n';
            return;
        }

        displayStudentDetails(*it);
        std::cout << "Are you sure you want to delete this student? (y/n): ";
        std::string confirmation;
        std::getline(std::cin, confirmation);
        if (!confirmation.empty() && (confirmation[0] == 'y' || confirmation[0] == 'Y')) {
            students.erase(it);
            std::cout << "Student deleted successfully." << '\n';
        } else {
            std::cout << "Deletion cancelled." << '\n';
        }
    }

    void displayCompactList(const std::vector<std::size_t> &indices) const {
        if (indices.empty()) {
            std::cout << "No matching students." << '\n';
            return;
        }

        std::vector<int> ranks;
        computeRanks(ranks);

        std::cout << std::left << std::setw(25) << "Name"
                  << std::setw(15) << "ID"
                  << std::setw(10) << "GPA"
                  << std::setw(5) << "Rank" << '\n';
        std::cout << std::string(55, '-') << '\n';

        for (std::size_t listIndex = 0; listIndex < indices.size(); ++listIndex) {
            const std::size_t studentIndex = indices[listIndex];
            if (studentIndex >= students.size()) {
                continue;
            }
            printStudentRow(students[studentIndex], ranks.empty() ? 0 : ranks[studentIndex]);
        }
    }

    void displayAll() const {
        if (students.empty()) {
            std::cout << "\nNo students available to display." << '\n';
            return;
        }

        std::vector<int> ranks;
        computeRanks(ranks);

        std::cout << "\n--- Student List ---" << '\n';
        std::cout << std::left << std::setw(25) << "Name"
                  << std::setw(15) << "ID"
                  << std::setw(10) << "GPA"
                  << std::setw(5) << "Rank" << '\n';
        std::cout << std::string(55, '-') << '\n';

       for (std::size_t idx = 0; idx < students.size(); ++idx) {
            printStudentRow(students[idx], ranks.empty() ? 0 : ranks[idx]);
        }
    }

    static void printHistogram(const std::array<std::size_t, 5> &buckets) {
        const std::array<const char *, 5> labels = {
            "90-100",
            "80-89",
            "70-79",
            "60-69",
            "0-59"
        };

        std::cout << "\nGPA Distribution (approx via percentages):" << '\n';
        for (std::size_t i = 0; i < buckets.size(); ++i) {
            std::cout << std::setw(7) << labels[i] << " | ";
            std::cout << std::string(buckets[i], '*') << " (" << buckets[i] << ")" << '\n';
        }
    }

    void displayStatistics() const {
        if (students.empty()) {
            std::cout << "\nNo students available for statistics." << '\n';
            return;
        }

        double totalGpa = 0.0;
        double totalGrades = 0.0;
        std::size_t gradeCount = 0;
        const Student *topStudent = nullptr;
        const Student *bottomStudent = nullptr;
        std::size_t honorsCount = 0;     // GPA >= 3.5
        std::size_t probationCount = 0;  // GPA < 2.0

        for (const auto &student : students) {
            float gpa = student.calculateGPA();
            totalGpa += gpa;

            if (topStudent == nullptr || gpa > topStudent->calculateGPA()) {
                topStudent = &student;
            }
            if (bottomStudent == nullptr || gpa < bottomStudent->calculateGPA()) {
                bottomStudent = &student;
            }

            if (gpa >= 3.5f) {
                ++honorsCount;
            }
            if (gpa < 2.0f) {
                ++probationCount;
            }

            for (float grade : student.grades) {
                totalGrades += grade;
                ++gradeCount;
            }
        }

        double averageGpa = totalGpa / static_cast<double>(students.size());
        double averageGrade = gradeCount > 0 ? totalGrades / static_cast<double>(gradeCount) : 0.0;

        std::array<std::size_t, 5> gradeBuckets = {0, 0, 0, 0, 0};
        for (const auto &student : students) {
            for (float grade : student.grades) {
                if (grade >= 90.0f) {
                    ++gradeBuckets[0];
                } else if (grade >= 80.0f) {
                    ++gradeBuckets[1];
                } else if (grade >= 70.0f) {
                    ++gradeBuckets[2];
                } else if (grade >= 60.0f) {
                    ++gradeBuckets[3];
                } else {
                    ++gradeBuckets[4];
                }
            }
        }

        std::cout << "\n--- Class Statistics ---" << '\n';
        std::cout << "Total students      : " << students.size() << '\n';
        std::cout << "Average GPA         : " << std::fixed << std::setprecision(2) << averageGpa << '\n';
        std::cout << "Average grade       : " << std::fixed << std::setprecision(2) << averageGrade << '\n';
        std::cout << "Students with honors: " << honorsCount << '\n';
        std::cout << "On academic probation: " << probationCount << '\n';
        if (topStudent != nullptr) {
            std::cout << "Top student         : " << topStudent->name << " (" << topStudent->id
                      << ", GPA: " << std::fixed << std::setprecision(2) << topStudent->calculateGPA() << ")" << '\n';
        }
        if (bottomStudent != nullptr) {
            std::cout << "Lowest GPA student  : " << bottomStudent->name << " (" << bottomStudent->id
                      << ", GPA: " << std::fixed << std::setprecision(2) << bottomStudent->calculateGPA() << ")" << '\n';
        }

        printHistogram(gradeBuckets);
    }

    void searchStudent() const {
        if (students.empty()) {
            std::cout << "\nNo students available to search." << '\n';
            return;
        }

        std::cout << "\n--- Search Student ---" << '\n';
        std::cout << "1. Search by ID" << '\n';
        std::cout << "2. Search by Name" << '\n';
        std::cout << "3. Search by GPA Range" << '\n';
        std::cout << "Choose an option: ";

        std::string input;
        std::getline(std::cin, input);
        int option = 0;
        tryParseInt(input, option);

        if (option == 1) {
            std::cout << "Enter ID: ";
            std::string id;
            std::getline(std::cin, id);
            id = trim(id);
            auto it = std::find_if(students.begin(), students.end(), [&](const Student &s) {
                return s.id == id;
            });

            if (it != students.end()) {
                displayStudentDetails(*it);
            } else {
                std::cout << "No student found with ID: " << id << '\n';
            }
        } else if (option == 2) {
            std::cout << "Enter name (partial matches allowed): ";
            std::string nameFragment;
            std::getline(std::cin, nameFragment);
            std::string target = toLowerCopy(trim(nameFragment));
            std::vector<std::size_t> matches;

            for (std::size_t i = 0; i < students.size(); ++i) {
                if (toLowerCopy(students[i].name).find(target) != std::string::npos) {
                    matches.push_back(i);
                }
            }

            if (matches.empty()) {
                std::cout << "No students matched the provided name." << '\n';
            } else {
                std::cout << "\nMatching students:" << '\n';
                displayCompactList(matches);
            }
        } else if (option == 3) {
            std::cout << "Enter minimum GPA (0 - 4.0): ";
            std::string minInput;
            std::getline(std::cin, minInput);
            float minGpa = 0.0f;
            if (!tryParseFloat(minInput, minGpa)) {
                std::cout << "Invalid minimum GPA." << '\n';
                return;
            }

            std::cout << "Enter maximum GPA (0 - 4.0): ";
            std::string maxInput;
            std::getline(std::cin, maxInput);
            float maxGpa = 4.0f;
            if (!tryParseFloat(maxInput, maxGpa)) {
                std::cout << "Invalid maximum GPA." << '\n';
                return;
            }

            if (minGpa > maxGpa) {
                std::swap(minGpa, maxGpa);
            }

            std::vector<std::size_t> matches;
            for (std::size_t i = 0; i < students.size(); ++i) {
                float gpa = students[i].calculateGPA();
                if (gpa >= minGpa && gpa <= maxGpa) {
                    matches.push_back(i);
                }
            }

            std::cout << "\nStudents with GPA between " << std::fixed << std::setprecision(2)
                      << minGpa << " and " << maxGpa << ":" << '\n';
            displayCompactList(matches);
        } else {
            std::cout << "Invalid option selected." << '\n';
        }
    }

    void exportSingleReport(const Student &student, const std::string &directory = "reports") const {
        std::error_code errorCode;
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory, errorCode);
            if (errorCode) {
                std::cout << "Failed to create reports directory: " << errorCode.message() << '\n';
                return;
            }
        }

        const std::string filePath = directory + "/" + student.id + "_report.txt";
        std::ofstream reportFile(filePath);
        if (!reportFile) {
            std::cout << "Could not open report file for student " << student.id << '\n';
            return;
        }

        reportFile << "Student Progress Report" << '\n';
        reportFile << std::string(60, '=') << '\n';
        reportFile << "Name             : " << student.name << '\n';
        reportFile << "ID               : " << student.id << '\n';
        reportFile << "GPA              : " << std::fixed << std::setprecision(2) << student.calculateGPA() << '\n';
        reportFile << "Grade Trend      : " << formatGradeTrend(student.grades) << '\n';
        if (!student.grades.empty()) {
            reportFile << "Highest Grade    : " << std::fixed << std::setprecision(2)
                       << *std::max_element(student.grades.begin(), student.grades.end()) << '\n';
            reportFile << "Lowest Grade     : " << std::fixed << std::setprecision(2)
                       << *std::min_element(student.grades.begin(), student.grades.end()) << '\n';
            reportFile << "Grades           : ";
            for (std::size_t i = 0; i < student.grades.size(); ++i) {
                reportFile << std::fixed << std::setprecision(2) << student.grades[i];
                if (i + 1 < student.grades.size()) {
                    reportFile << ", ";
                }
            }
            reportFile << '\n';
        } else {
            reportFile << "Grades           : None" << '\n';
        }

        reportFile << std::string(60, '=') << '\n';
        std::cout << "Progress report exported to " << filePath << '\n';
    }

    void saveToFile(const std::string &filename = "student_data.txt") const {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cout << "Failed to open " << filename << " for writing." << '\n';
            return;
        }

        outFile << "STUDENT_DB_V2" << '\n';
        outFile << students.size() << '\n';
        for (const auto &student : students) {
            outFile << student.id << '\n';
            outFile << student.name << '\n';
            outFile << student.grades.size() << '\n';
            for (std::size_t i = 0; i < student.grades.size(); ++i) {
                outFile << student.grades[i];
                if (i + 1 < student.grades.size()) {
                    outFile << ' ';
                }
            }
            outFile << '\n';
        }

        std::cout << "Data saved to " << filename << '\n';
    }

    void loadFromFile(const std::string &filename = "student_data.txt") {
        std::ifstream inFile(filename);
        if (!inFile) {
            std::cout << "No existing data file found. Starting with an empty database." << '\n';
            return;
        }

        students.clear();
        std::string line;
        if (!std::getline(inFile, line)) {
            std::cout << "Data file is empty." << '\n';
            return;
        }

        std::size_t studentCount = 0;
        bool usesNewFormat = (line == "STUDENT_DB_V2");
        if (usesNewFormat) {
            if (!std::getline(inFile, line) || !tryParseUnsigned(line, studentCount)) {
                std::cout << "Failed to read student count. Data file may be corrupted." << '\n';
                return;
            }
        } else {
            if (!tryParseUnsigned(line, studentCount)) {
                std::cout << "Failed to read student count. Data file may be corrupted." << '\n';
                return;
            }
        }

        bool dataTruncated = false;
        for (std::size_t i = 0; i < studentCount; ++i) {
            Student student;
            if (!std::getline(inFile, student.id)) {
                dataTruncated = true;
                break;
            }
            if (!std::getline(inFile, student.name)) {
                dataTruncated = true;
                break;
            }
            if (!std::getline(inFile, line)) {
                dataTruncated = true;
                break;
            }

            std::size_t gradeCount = 0;
            if (!tryParseUnsigned(line, gradeCount)) {
                dataTruncated = true;
                break;
            }

            if (!std::getline(inFile, line)) {
                dataTruncated = true;
                break;
            }

            student.grades.clear();
            if (!line.empty()) {
                std::stringstream gradeStream(line);
                float grade = 0.0f;
                while (gradeStream >> grade && student.grades.size() < gradeCount) {
                    if (grade >= 0.0f && grade <= 100.0f) {
                        student.grades.push_back(grade);
                    }
                }
            }

            while (student.grades.size() < gradeCount) {
                if (!std::getline(inFile, line)) {
                    dataTruncated = true;
                    break;
                }
                if (line.empty()) {
                    continue;
                }
                std::stringstream continuationStream(line);
                float grade = 0.0f;
                while (continuationStream >> grade && student.grades.size() < gradeCount) {
                    if (grade >= 0.0f && grade <= 100.0f) {
                        student.grades.push_back(grade);
                    }
                }
            }

            students.push_back(std::move(student));
        }

        if (dataTruncated) {
            std::cout << "Data file ended unexpectedly. Loaded " << students.size()
                      << " students before encountering an issue." << '\n';
        } else {
            std::cout << "Loaded " << students.size() << " students from " << filename << '\n';
        }
    }

    void sortStudents() {
        if (students.size() < 2) {
            std::cout << "\nNot enough students to sort." << '\n';
            return;
        }

        std::cout << "\n--- Sort Students ---" << '\n';
        std::cout << "1. Sort by Name (A-Z)" << '\n';
        std::cout << "2. Sort by GPA (High to Low)" << '\n';
        std::cout << "3. Sort by ID" << '\n';
        std::cout << "Choose an option: ";

        std::string input;
        std::getline(std::cin, input);
        int option = 0;
        tryParseInt(input, option);

        if (option == 1) {
            std::sort(students.begin(), students.end(), [&](const Student &a, const Student &b) {
                std::string nameA = toLowerCopy(a.name);
                std::string nameB = toLowerCopy(b.name);
                if (nameA == nameB) {
                    return a.id < b.id;
                }
                return nameA < nameB;
            });
            std::cout << "Students sorted alphabetically by name." << '\n';
        } else if (option == 2) {
            std::sort(students.begin(), students.end(), [&](const Student &a, const Student &b) {
                float gpaA = a.calculateGPA();
                float gpaB = b.calculateGPA();
                if (std::fabs(gpaA - gpaB) > 1e-4f) {
                    return gpaA > gpaB;
                }
                return a.name < b.name;
            });
            std::cout << "Students sorted by GPA in descending order." << '\n';
        } else if (option == 3) {
            std::sort(students.begin(), students.end(), [&](const Student &a, const Student &b) {
                return a.id < b.id;
            });
            std::cout << "Students sorted by ID." << '\n';
        } else {
            std::cout << "Invalid option selected." << '\n';
        }
    }

    // Add missing method stubs to make the code compile
    void exportToCSV() const {
        std::cout << "Export to CSV functionality not implemented yet." << '\n';
    }

    void importFromCSV() {
        std::cout << "Import from CSV functionality not implemented yet." << '\n';
    }

    void showTopPerformers() const {
        std::cout << "Show top performers functionality not implemented yet." << '\n';
    }

    void filterStudents() const {
        std::cout << "Filter students functionality not implemented yet." << '\n';
    }

    void exportProgressReports() const {
        std::cout << "Export progress reports functionality not implemented yet." << '\n';
    }
};

void showMenu() {
    std::cout << "\n===== Student Grade Management System =====" << '\n';
    for (const auto& option : MAIN_MENU_OPTIONS) {
        std::cout << option << '\n';
    }
    std::cout << "0. Exit" << '\n';
    std::cout << "Select an option: ";
}

int main() {
    GradeManager manager;
    manager.loadFromFile();

    bool running = true;
    while (running) {
        showMenu();
        std::string input;
        std::getline(std::cin, input);
        int choice = -1;

        // Use a local tryParseInt since the GradeManager one is private
        auto tryParseInt = [](const std::string &text, int &value) {
            std::string trimmed = text;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
            trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
            std::stringstream ss(trimmed);
            int temp = 0;
            if (ss >> temp && ss.eof()) {
                value = temp;
                return true;
            }
            return false;
        };

        if (!tryParseInt(input, choice)) {
            std::cout << "Invalid input. Please enter a number." << '\n';
            continue;
        }

        switch (choice) {
        case 1:
            manager.addStudent();
            break;
        case 2:
            manager.displayAll();
            break;
        case 3:
            manager.searchStudent();
            break;
        case 4:
            manager.sortStudents();
            break;
        case 5:
            manager.updateStudent();
            break;
        case 6:
            manager.deleteStudent();
            break;
        case 7:
            manager.displayStatistics();
            break;
        case 8:
            manager.saveToFile();
            break;
        case 9:
            manager.loadFromFile();
            break;
        case 10:
            manager.exportToCSV();
            break;
        case 11:
            manager.importFromCSV();
            break;
        case 12:
            manager.showTopPerformers();
            break;
        case 13:
            manager.filterStudents();
            break;
        case 14:
            manager.exportProgressReports();
            break;
        case 0:
            manager.saveToFile();
            std::cout << "Exiting program. Goodbye!" << '\n';
            running = false;
            break;
        default:
            std::cout << "Invalid choice. Please select a valid option." << '\n';
            break;
        }
    }

    return 0;
}
