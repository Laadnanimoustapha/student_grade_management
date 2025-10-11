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