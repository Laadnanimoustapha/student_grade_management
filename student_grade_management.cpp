#include <algorithm>
#include <cctype>
#include <cmath>
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
