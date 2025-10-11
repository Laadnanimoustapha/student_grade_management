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