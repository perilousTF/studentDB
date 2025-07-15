#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <optional>
#include <ctime>
#include <limits>
#include <algorithm>

using namespace std;

// --- Global Constants ---
const int NAME_SIZE = 50;
const int ADDR_SIZE = 100;
const int PAGE_SIZE = 5; // Number of records to display per page

// --- Data Structures ---
struct Student {
    int id;
    char name[NAME_SIZE];
    char address[ADDR_SIZE];
    time_t createdAt;
    time_t updatedAt;
    bool isActive;

    Student() : id(0), createdAt(0), updatedAt(0), isActive(false) {
        name[0] = '\0'; address[0] = '\0';
    }
    Student(int studentId, const string& studentName, const string& studentAddress)
        : id(studentId), isActive(true) {
        createdAt = time(nullptr); updatedAt = time(nullptr);
        strncpy(name, studentName.c_str(), NAME_SIZE - 1);
        name[NAME_SIZE - 1] = '\0';
        strncpy(address, studentAddress.c_str(), ADDR_SIZE - 1);
        address[ADDR_SIZE - 1] = '\0';
    }

    void print() const {
        if (!isActive) return;
        
        // Fixed: Use standard ctime instead of ctime_s
        char* createdStr = ctime(&createdAt);
        char* updatedStr = ctime(&updatedAt);
        
        // Remove newline characters
        if (createdStr) {
            createdStr[strcspn(createdStr, "\n")] = 0;
        }
        if (updatedStr) {
            updatedStr[strcspn(updatedStr, "\n")] = 0;
        }

        cout << "------------------------" << endl;
        cout << "ID:         " << id << endl;
        cout << "Name:       " << name << endl;
        cout << "Address:    " << address << endl;
        cout << "Created At: " << (createdStr ? createdStr : "Unknown") << endl;
        cout << "Updated At: " << (updatedStr ? updatedStr : "Unknown") << endl;
        cout << "------------------------" << endl;
    }
};

struct PageResult {
    vector<Student> students;
    int currentPage;
    int totalPages;
    int totalRecords;
};

// --- The Database Engine (Normal, In-line Structure) ---
class IndexedStudentDB {
private:
    string dataFilename;
    string indexFilename;
    fstream dataFile;
    unordered_map<int, streampos> id_index;

    void loadIndex() {
        ifstream indexIn(indexFilename, ios::binary);
        if (!indexIn) return;
        int id;
        streampos pos;
        while (indexIn.read(reinterpret_cast<char*>(&id), sizeof(id)) &&
               indexIn.read(reinterpret_cast<char*>(&pos), sizeof(pos))) {
            id_index[id] = pos;
        }
    }

    void saveIndex() {
        ofstream indexOut(indexFilename, ios::binary | ios::trunc);
        for (const auto& pair : id_index) {
            int id = pair.first;
            streampos pos = pair.second;
            indexOut.write(reinterpret_cast<const char*>(&id), sizeof(id));
            indexOut.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
        }
    }

public:
    IndexedStudentDB(const string& basename) {
        dataFilename = basename + ".dat";
        indexFilename = basename + ".idx";
        dataFile.open(dataFilename, ios::in | ios::out | ios::binary | ios::app);
        if (!dataFile.is_open() || dataFile.tellg() == 0) {
            dataFile.close();
            dataFile.open(dataFilename, ios::in | ios::out | ios::binary | ios::trunc);
        }
        loadIndex();
    }

    ~IndexedStudentDB() {
        if (dataFile.is_open()) {
            cout << "\nShutting down. Saving index..." << endl;
            saveIndex();
            dataFile.close();
        }
    }

    bool addStudent(const Student& s) {
        if (id_index.count(s.id)) {
            cerr << "Error: Student with ID " << s.id << " already exists." << endl;
            return false;
        }
        
        // Clear any error flags and go to end of file
        dataFile.clear();
        dataFile.seekp(0, ios::end);
        streampos newPos = dataFile.tellp();
        dataFile.write(reinterpret_cast<const char*>(&s), sizeof(Student));
        dataFile.flush(); // Ensure data is written immediately
        
        id_index[s.id] = newPos;
        return true;
    }

    optional<Student> findStudentById(int id) {
        if (id_index.count(id) == 0) return nullopt;
        streampos pos = id_index.at(id);
        
        // Clear any error flags and ensure we can read
        dataFile.clear();
        dataFile.seekg(pos);
        
        Student s;
        if (dataFile.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            if (s.isActive && s.id == id) return s;
        }
        return nullopt;
    }

    bool updateStudent(int id, const string& newName, const string& newAddress) {
        auto studentOpt = findStudentById(id);
        if (!studentOpt) {
            cerr << "Error: Cannot update non-existent student with ID " << id << endl;
            return false;
        }
        Student s = *studentOpt;
        strncpy(s.name, newName.c_str(), NAME_SIZE - 1);
        s.name[NAME_SIZE - 1] = '\0';
        strncpy(s.address, newAddress.c_str(), ADDR_SIZE - 1);
        s.address[ADDR_SIZE - 1] = '\0';
        s.updatedAt = time(nullptr);
        
        streampos pos = id_index.at(id);
        
        // Clear any error flags and ensure we can write
        dataFile.clear();
        dataFile.seekp(pos);
        dataFile.write(reinterpret_cast<const char*>(&s), sizeof(Student));
        dataFile.flush(); // Ensure data is written immediately
        
        return true;
    }

    bool deleteStudent(int id) {
        if (id_index.count(id) == 0) return false;
        auto studentOpt = findStudentById(id);
        if (!studentOpt) return false;
        Student s = *studentOpt;
        s.isActive = false;
        s.updatedAt = time(nullptr);
        
        streampos pos = id_index.at(id);
        
        // Clear any error flags and ensure we can write
        dataFile.clear();
        dataFile.seekp(pos);
        dataFile.write(reinterpret_cast<const char*>(&s), sizeof(Student));
        dataFile.flush(); // Ensure data is written immediately
        
        id_index.erase(id);
        return true;
    }

    PageResult getAllStudents(int pageNumber, int pageSize) {
        PageResult result;
        result.currentPage = pageNumber;
        vector<Student> pageStudents;
        int totalActiveRecords = 0;
        int recordsToSkip = (pageNumber - 1) * pageSize;

        dataFile.clear();
        dataFile.seekg(0, ios::beg);
        Student s;
        while (dataFile.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            if (s.isActive) {
                if (totalActiveRecords >= recordsToSkip && pageStudents.size() < pageSize) {
                    pageStudents.push_back(s);
                }
                totalActiveRecords++;
            }
        }
        result.students = pageStudents;
        result.totalRecords = totalActiveRecords;
        result.totalPages = (totalActiveRecords + pageSize - 1) / pageSize;
        if (result.totalPages == 0) result.totalPages = 1;
        return result;
    }

    PageResult searchBy(const string& query, bool byName, int pageNumber, int pageSize) {
        PageResult result;
        result.currentPage = pageNumber;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        vector<Student> pageStudents;
        int totalMatchingRecords = 0;
        int recordsToSkip = (pageNumber - 1) * pageSize;

        dataFile.clear();
        dataFile.seekg(0, ios::beg);
        Student s;
        while (dataFile.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            if (s.isActive) {
                string fieldToCompare = byName ? s.name : s.address;
                transform(fieldToCompare.begin(), fieldToCompare.end(), fieldToCompare.begin(), ::tolower);
                if (fieldToCompare.find(lowerQuery) != string::npos) {
                    if (totalMatchingRecords >= recordsToSkip && pageStudents.size() < pageSize) {
                        pageStudents.push_back(s);
                    }
                    totalMatchingRecords++;
                }
            }
        }
        result.students = pageStudents;
        result.totalRecords = totalMatchingRecords;
        result.totalPages = (totalMatchingRecords + pageSize - 1) / pageSize;
        if (result.totalPages == 0) result.totalPages = 1;
        return result;
    }
};

// --- Command-Line Interface (CLI) ---

void printMenu() {
    cout << "\n===== Student Database Menu =====" << endl;
    cout << "1. Add New Student" << endl;
    cout << "2. Find Student by ID" << endl;
    cout << "3. Update Student" << endl;
    cout << "4. Delete Student" << endl;
    cout << "5. List All Students" << endl;
    cout << "6. Search by Name" << endl;
    cout << "7. Search by Address" << endl;
    cout << "0. Exit" << endl;
    cout << "===============================" << endl;
    cout << "Enter your choice: ";
}

int getIntegerInput() {
    int value;
    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return value;
}

void handleAddStudent(IndexedStudentDB& db) {
    cout << "Enter Student ID: ";
    int id = getIntegerInput();
    string name, address;
    cout << "Enter Name: ";
    getline(cin, name);
    cout << "Enter Address: ";
    getline(cin, address);
    if (db.addStudent({id, name, address})) {
        cout << "Student added successfully." << endl;
    }
}

void handleFindStudent(IndexedStudentDB& db) {
    cout << "Enter ID to find: ";
    int id = getIntegerInput();
    if (auto s = db.findStudentById(id)) {
        s->print();
    } else {
        cout << "Student with ID " << id << " not found." << endl;
    }
}

void handleUpdateStudent(IndexedStudentDB& db) {
    cout << "Enter ID of student to update: ";
    int id = getIntegerInput();
    if (!db.findStudentById(id)) {
        cout << "Student with ID " << id << " does not exist." << endl;
        return;
    }
    string name, address;
    cout << "Enter new Name: ";
    getline(cin, name);
    cout << "Enter new Address: ";
    getline(cin, address);
    if (db.updateStudent(id, name, address)) {
        cout << "Student updated successfully." << endl;
    }
}

void handleDeleteStudent(IndexedStudentDB& db) {
    cout << "Enter ID of student to delete: ";
    int id = getIntegerInput();
    if (db.deleteStudent(id)) {
        cout << "Student with ID " << id << " deleted successfully." << endl;
    } else {
        cout << "Failed to delete student with ID " << id << ". May not exist." << endl;
    }
}

void displayPagedResults(const PageResult& page) {
    cout << "\n--- Page " << page.currentPage << " of " << page.totalPages
         << " (Total Matching Records: " << page.totalRecords << ") ---" << endl;
    if (page.students.empty()) {
        cout << "No records found on this page." << endl;
    } else {
        for (const auto& s : page.students) {
            s.print();
        }
    }
    if (page.totalPages > 1) {
        cout << "Navigation: ";
        if (page.currentPage > 1) cout << "(P)revious  ";
        if (page.currentPage < page.totalPages) cout << "(N)ext  ";
        cout << "(Q)uit View" << endl;
        cout << "Enter choice: ";
    }
}

void handleListAllStudents(IndexedStudentDB& db) {
    int currentPage = 1;
    while (true) {
        PageResult page = db.getAllStudents(currentPage, PAGE_SIZE);
        displayPagedResults(page);
        if (page.totalPages <= 1) { cin.get(); break; } // Wait for enter if only one page
        char navChoice;
        cin >> navChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        navChoice = tolower(navChoice);
        if (navChoice == 'n' && currentPage < page.totalPages) {
            currentPage++;
        } else if (navChoice == 'p' && currentPage > 1) {
            currentPage--;
        } else if (navChoice == 'q') {
            break;
        } else {
            cout << "Invalid navigation choice." << endl;
        }
    }
}

void handleSearch(IndexedStudentDB& db, bool byName) {
    string query;
    cout << "Enter search term: ";
    getline(cin, query);
    int currentPage = 1;
    while (true) {
        PageResult page = db.searchBy(query, byName, currentPage, PAGE_SIZE);
        displayPagedResults(page);
        if (page.totalPages <= 1) { cin.get(); break; } // Wait for enter if only one page
        char navChoice;
        cin >> navChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        navChoice = tolower(navChoice);
        if (navChoice == 'n' && currentPage < page.totalPages) {
            currentPage++;
        } else if (navChoice == 'p' && currentPage > 1) {
            currentPage--;
        } else if (navChoice == 'q') {
            break;
        } else {
            cout << "Invalid navigation choice." << endl;
        }
    }
}

// --- Main Program Entry Point ---
int main() {
    IndexedStudentDB db("school_db");
    
    while (true) {
        printMenu();
        int choice = getIntegerInput();
        switch (choice) {
            case 1: handleAddStudent(db); break;
            case 2: handleFindStudent(db); break;
            case 3: handleUpdateStudent(db); break;
            case 4: handleDeleteStudent(db); break;
            case 5: handleListAllStudents(db); break;
            case 6: handleSearch(db, true); break;
            case 7: handleSearch(db, false); break;
            case 0: return 0; // Destructor will be called here to save the index
            default: cout << "Invalid choice. Please try again." << endl;
        }
    }
    return 0;
}