#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// --- Student Structure ---
struct Student {
    int id;
    string name;
    float mathGrade;
    float scienceGrade;
    float englishGrade;

    float calculateAverage() const {
        return (mathGrade + scienceGrade + englishGrade) / 3.0f;
    }

    char getLetterGrade() const {
        float avg = calculateAverage();
        if (avg >= 90) return 'A';
        if (avg >= 80) return 'B';
        if (avg >= 70) return 'C';
        if (avg >= 60) return 'D';
        return 'F';
    }
};

// --- Grade Management System Class ---
class GradeSystem {
private:
    vector<Student> students;
    const string filename = "students.txt";

public:
    GradeSystem() {
        loadData();
    }

    void addStudent() {
        Student s;
        cout << "\nEnter Student ID: ";
        while (!(cin >> s.id)) {
            cout << "Invalid input. Enter a number: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore(); // Clear newline

        // Check for duplicate ID
        for (const auto& existing : students) {
            if (existing.id == s.id) {
                cout << "Error: Student with ID " << s.id << " already exists.\n";
                return;
            }
        }

        cout << "Enter Name: ";
        getline(cin, s.name);

        cout << "Enter Math Grade (0-100): ";
        s.mathGrade = getValidGrade();
        cout << "Enter Science Grade (0-100): ";
        s.scienceGrade = getValidGrade();
        cout << "Enter English Grade (0-100): ";
        s.englishGrade = getValidGrade();

        students.push_back(s);
        cout << "Student added successfully!\n";
        saveData();
    }

    void displayAll() const {
        if (students.empty()) {
            cout << "\nNo students found.\n";
            return;
        }
        printHeader();
        for (const auto& s : students) {
            printStudent(s);
        }
    }

    void searchStudent() const {
        int id;
        cout << "\nEnter Student ID to search: ";
        cin >> id;
        
        bool found = false;
        for (const auto& s : students) {
            if (s.id == id) {
                printHeader();
                printStudent(s);
                found = true;
                break;
            }
        }
        if (!found) cout << "Student not found.\n";
    }

    void deleteStudent() {
        int id;
        cout << "\nEnter Student ID to delete: ";
        cin >> id;

        auto it = remove_if(students.begin(), students.end(), [id](const Student& s) {
            return s.id == id;
        });

        if (it != students.end()) {
            students.erase(it, students.end());
            cout << "Student deleted successfully.\n";
            saveData();
        } else {
            cout << "Student not found.\n";
        }
    }

    void modifyGrades() {
        int id;
        cout << "\nEnter Student ID to modify: ";
        cin >> id;

        for (auto& s : students) {
            if (s.id == id) {
                cout << "Editing grades for " << s.name << ":\n";
                cout << "New Math Grade: ";
                s.mathGrade = getValidGrade();
                cout << "New Science Grade: ";
                s.scienceGrade = getValidGrade();
                cout << "New English Grade: ";
                s.englishGrade = getValidGrade();
                cout << "Grades updated.\n";
                saveData();
                return;
            }
        }
        cout << "Student not found.\n";
    }

private:
    float getValidGrade() {
        float grade;
        while (!(cin >> grade) || grade < 0 || grade > 100) {
            cout << "Invalid grade. Enter 0-100: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return grade;
    }

    void printHeader() const {
        cout << "\n--------------------------------------------------------------------------------\n";
        cout << left << setw(10) << "ID" 
             << setw(20) << "Name" 
             << setw(10) << "Math" 
             << setw(10) << "Science" 
             << setw(10) << "English" 
             << setw(10) << "Average" 
             << setw(5) << "Grade" << "\n";
        cout << "--------------------------------------------------------------------------------\n";
    }

    void printStudent(const Student& s) const {
        cout << left << setw(10) << s.id 
             << setw(20) << s.name 
             << setw(10) << s.mathGrade 
             << setw(10) << s.scienceGrade 
             << setw(10) << s.englishGrade 
             << fixed << setprecision(2) << setw(10) << s.calculateAverage() 
             << setw(5) << s.getLetterGrade() << "\n";
    }

    void saveData() const {
        ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto& s : students) {
                outFile << s.id << "|" << s.name << "|" 
                        << s.mathGrade << "|" << s.scienceGrade << "|" << s.englishGrade << "\n";
            }
            outFile.close();
        } else {
            cerr << "Error: Could not save to file.\n";
        }
    }

    void loadData() {
        students.clear();
        ifstream inFile(filename);
        if (inFile.is_open()) {
            Student s;
            string line;
            while (getline(inFile, line)) {
                size_t pos1 = line.find('|');
                size_t pos2 = line.find('|', pos1 + 1);
                size_t pos3 = line.find('|', pos2 + 1);
                size_t pos4 = line.find('|', pos3 + 1);

                if (pos1 != string::npos && pos4 != string::npos) {
                    s.id = stoi(line.substr(0, pos1));
                    s.name = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    s.mathGrade = stof(line.substr(pos2 + 1, pos3 - pos2 - 1));
                    s.scienceGrade = stof(line.substr(pos3 + 1, pos4 - pos3 - 1));
                    s.englishGrade = stof(line.substr(pos4 + 1));
                    students.push_back(s);
                }
            }
            inFile.close();
        }
    }
};

// --- Main Menu ---
int main() {
    GradeSystem system;
    int choice;

    do {
        cout << "\n=== Student Grade Management System ===\n";
        cout << "1. Add Student\n";
        cout << "2. Display All Students\n";
        cout << "3. Search Student\n";
        cout << "4. Modify Grades\n";
        cout << "5. Delete Student\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: system.addStudent(); break;
            case 2: system.displayAll(); break;
            case 3: system.searchStudent(); break;
            case 4: system.modifyGrades(); break;
            case 5: system.deleteStudent(); break;
            case 6: cout << "Exiting...\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);

    return 0;
}
