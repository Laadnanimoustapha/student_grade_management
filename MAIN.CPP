#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstdlib> // For system()

using namespace std;

// --- Utils for UI ---
namespace UI {
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void pause() {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    void printBanner() {
        cout << "================================================================================\n";
        cout << "   _____ _______ _    _ _____  ______ _   _ _______    \n";
        cout << "  / ____|__   __| |  | |  __ \\|  ____| \\ | |__   __|   \n";
        cout << " | (___    | |  | |  | | |  | | |__  |  \\| |  | |      \n";
        cout << "  \\___ \\   | |  | |  | | |  | |  __| | . ` |  | |      \n";
        cout << "  ____) |  | |  | |__| | |__| | |____| |\\  |  | |      \n";
        cout << " |_____/   |_|   \\____/|_____/|______|_| \\_|  |_|      \n";
        cout << "                                                       \n";
        cout << "      G R A D E   M A N A G E M E N T   S Y S T E M    \n";
        cout << "================================================================================\n";
    }

    void printLine() {
        cout << "+----------+----------------------+----------+----------+----------+----------+-------+\n";
    }
}

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
        UI::clearScreen();
        UI::printBanner();
        cout << "\n[ ADD NEW STUDENT ]\n";
        
        Student s;
        cout << ">> Enter Student ID: ";
        while (!(cin >> s.id)) {
            cout << "!! Invalid input. Enter a number: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore(); // Clear newline

        // Check for duplicate ID
        for (const auto& existing : students) {
            if (existing.id == s.id) {
                cout << "!! Error: Student with ID " << s.id << " already exists.\n";
                UI::pause();
                return;
            }
        }

        cout << ">> Enter Name: ";
        getline(cin, s.name);

        cout << ">> Enter Math Grade (0-100): ";
        s.mathGrade = getValidGrade();
        cout << ">> Enter Science Grade (0-100): ";
        s.scienceGrade = getValidGrade();
        cout << ">> Enter English Grade (0-100): ";
        s.englishGrade = getValidGrade();

        students.push_back(s);
        cout << "\n[SUCCESS] Student added successfully!\n";
        saveData();
        UI::pause();
    }

    void displayAll() const {
        UI::clearScreen();
        UI::printBanner();
        cout << "\n[ STUDENT RECORDS ]\n";
        
        if (students.empty()) {
            cout << "\nNo students found in the system.\n";
            UI::pause();
            return;
        }

        printHeader();
        for (const auto& s : students) {
            printStudent(s);
        }
        UI::printLine();
        
        cout << "\nTotal Students: " << students.size() << "\n";
        UI::pause();
    }

    void searchStudent() const {
        UI::clearScreen();
        UI::printBanner();
        cout << "\n[ SEARCH STUDENT ]\n";

        int id;
        cout << ">> Enter Student ID to search: ";
        cin >> id;
        
        bool found = false;
        for (const auto& s : students) {
            if (s.id == id) {
                cout << "\nResult Found:\n";
                printHeader();
                printStudent(s);
                UI::printLine();
                found = true;
                break;
            }
        }
        if (!found) cout << "\n[INFO] Student with ID " << id << " not found.\n";
        UI::pause();
    }

    void deleteStudent() {
        UI::clearScreen();
        UI::printBanner();
        cout << "\n[ DELETE STUDENT ]\n";

        int id;
        cout << ">> Enter Student ID to delete: ";
        cin >> id;

        auto it = remove_if(students.begin(), students.end(), [id](const Student& s) {
            return s.id == id;
        });

        if (it != students.end()) {
            students.erase(it, students.end());
            cout << "\n[SUCCESS] Student deleted successfully.\n";
            saveData();
        } else {
            cout << "\n[INFO] Student not found.\n";
        }
        UI::pause();
    }

    void modifyGrades() {
        UI::clearScreen();
        UI::printBanner();
        cout << "\n[ MODIFY GRADES ]\n";

        int id;
        cout << ">> Enter Student ID to modify: ";
        cin >> id;

        for (auto& s : students) {
            if (s.id == id) {
                cout << "\nEditing grades for: " << s.name << "\n";
                cout << ">> New Math Grade: ";
                s.mathGrade = getValidGrade();
                cout << ">> New Science Grade: ";
                s.scienceGrade = getValidGrade();
                cout << ">> New English Grade: ";
                s.englishGrade = getValidGrade();
                
                cout << "\n[SUCCESS] Grades updated.\n";
                saveData();
                UI::pause();
                return;
            }
        }
        cout << "\n[INFO] Student not found.\n";
        UI::pause();
    }

private:
    float getValidGrade() {
        float grade;
        while (!(cin >> grade) || grade < 0 || grade > 100) {
            cout << "!! Invalid grade. Enter 0-100: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return grade;
    }

    void printHeader() const {
        UI::printLine();
        cout << "| " << left << setw(8) << "ID" 
             << " | " << setw(20) << "Name" 
             << " | " << setw(8) << "Math" 
             << " | " << setw(8) << "Science" 
             << " | " << setw(8) << "English" 
             << " | " << setw(8) << "Average" 
             << " | " << setw(5) << "Grade" << " |\n";
        UI::printLine();
    }

    void printStudent(const Student& s) const {
        cout << "| " << left << setw(8) << s.id 
             << " | " << setw(20) << s.name 
             << " | " << setw(8) << s.mathGrade 
             << " | " << setw(8) << s.scienceGrade 
             << " | " << setw(8) << s.englishGrade 
             << " | " << fixed << setprecision(2) << setw(8) << s.calculateAverage() 
             << " | " << setw(5) << s.getLetterGrade() << " |\n";
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
        UI::clearScreen();
        UI::printBanner();
        cout << "\n";
        cout << "    [1] Add Student\n";
        cout << "    [2] Display All Students\n";
        cout << "    [3] Search Student\n";
        cout << "    [4] Modify Grades\n";
        cout << "    [5] Delete Student\n";
        cout << "    [6] Exit\n\n";
        cout << ">> Select an option: ";
        
        if (!(cin >> choice)) {
            cout << "!! Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            UI::pause();
            continue;
        }

        cin.ignore(); // Clear buffer for any subsequent getlines if necessary for mixed input
        // (Though pure numeric choice doesn't strictly need it if next calls handle it well, 
        //  but safer for the pauses)

        switch (choice) {
            case 1: system.addStudent(); break;
            case 2: system.displayAll(); break;
            case 3: system.searchStudent(); break;
            case 4: system.modifyGrades(); break;
            case 5: system.deleteStudent(); break;
            case 6: cout << "\nExiting... Thank you for using Student Manager!\n"; break;
            default: 
                cout << "!! Invalid choice. Try again.\n";
                UI::pause();
        }
    } while (choice != 6);

    return 0;
}
