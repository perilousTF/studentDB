# Student Database Management System

A comprehensive C++ file-based database system for managing student records with indexing, pagination, and search capabilities.

## 📋 Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Database Architecture](#database-architecture)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

### Core Functionality
- **CRUD Operations**: Create, Read, Update, Delete student records
- **Persistent Storage**: File-based storage with automatic save/load
- **Indexing System**: Fast O(1) lookups using in-memory hash table
- **Soft Deletion**: Records are marked as inactive rather than physically deleted
- **Timestamp Tracking**: Automatic creation and update timestamps

### User Interface
- **Interactive CLI**: Clean, menu-driven command-line interface
- **Paginated Display**: Records shown in pages (5 per page by default)
- **Search Functionality**: Search by name or address with case-insensitive matching
- **Navigation Controls**: Easy page navigation for large datasets
- **Input Validation**: Robust error handling and input validation

### Advanced Features
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Memory Efficient**: Only loads required records into memory
- **Crash Recovery**: Index rebuilding on startup if needed
- **Duplicate Prevention**: Automatic prevention of duplicate student IDs

## 🔧 Prerequisites

- **C++ Compiler**: GCC 7.0+ or Clang 5.0+ with C++17 support
- **Operating System**: Windows 10+, Linux, or macOS
- **Memory**: Minimum 256MB RAM
- **Storage**: 10MB free disk space

## 🚀 Installation

### Clone the Repository
```bash
git clone https://github.com/perilousTF/studentDB.git
cd studentDB
```

### Compile the Program
```bash
# Using GCC
g++ -std=c++17 -O2 one.cpp -o student_db

# Using Clang
clang++ -std=c++17 -O2 one.cpp -o student_db

# For Windows with MinGW
g++ -std=c++17 -O2 one.cpp -o student_db.exe
```

### Run the Program
```bash
# Linux/macOS
./student_db

# Windows
student_db.exe
```

## 🎮 Usage

### Main Menu Options

```
===== Student Database Menu =====
1. Add New Student
2. Find Student by ID
3. Update Student
4. Delete Student
5. List All Students
6. Search by Name
7. Search by Address
0. Exit
===============================
```

### Example Usage

#### Adding a Student
```
Enter your choice: 1
Enter Student ID: 2206196
Enter Name: John Doe
Enter Address: 123 Main St, City, State
Student added successfully.
```

#### Searching Students
```
Enter your choice: 6
Enter search term: john
--- Page 1 of 1 (Total Matching Records: 1) ---
------------------------
ID:         2206196
Name:       John Doe
Address:    123 Main St, City, State
Created At: Thu Jul 10 04:02:26 2025
Updated At: Thu Jul 10 04:02:26 2025
------------------------
```

#### Pagination Navigation
```
--- Page 1 of 3 (Total Matching Records: 12) ---
[Student records displayed here]

Navigation: (N)ext  (Q)uit View
Enter choice: n
```


## 🏗️ Database Architecture

### File Structure
- **Data File (`.dat`)**: Binary file storing student records as fixed-size structs
- **Index File (`.idx`)**: Binary file storing ID-to-position mappings for fast lookups
- **In-Memory Index**: Hash table (`unordered_map`) for O(1) ID lookups

### Student Record Structure
```cpp
struct Student {
    int id;                    // Unique identifier
    char name[50];            // Student name
    char address[100];        // Student address
    time_t createdAt;         // Creation timestamp
    time_t updatedAt;         // Last update timestamp
    bool isActive;            // Soft deletion flag
};
```

### Performance Characteristics
- **Add Student**: O(1) average case
- **Find by ID**: O(1) lookup time
- **Update/Delete**: O(1) for existing records
- **Search by Name/Address**: O(n) linear scan
- **Pagination**: O(n) but memory efficient

## 📚 API Reference

### IndexedStudentDB Class

#### Constructor
```cpp
IndexedStudentDB(const string& basename)
```
Creates database with files `basename.dat` and `basename.idx`

#### Core Methods
```cpp
bool addStudent(const Student& s)                    // Add new student
optional<Student> findStudentById(int id)           // Find student by ID
bool updateStudent(int id, const string& name,      // Update student info
                  const string& address)
bool deleteStudent(int id)                          // Soft delete student
```

#### Query Methods
```cpp
PageResult getAllStudents(int pageNumber,           // Get paginated results
                         int pageSize)
PageResult searchBy(const string& query,           // Search with pagination
                   bool byName, 
                   int pageNumber, 
                   int pageSize)
```

### PageResult Structure
```cpp
struct PageResult {
    vector<Student> students;    // Students for current page
    int currentPage;            // Current page number
    int totalPages;             // Total number of pages
    int totalRecords;           // Total matching records
};
```

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **Make your changes**
4. **Add tests** (if applicable)
5. **Commit your changes**
   ```bash
   git commit -m "Add amazing feature"
   ```
6. **Push to the branch**
   ```bash
   git push origin feature/amazing-feature
   ```
7. **Open a Pull Request**

### Development Guidelines
- Follow C++17 standards
- Use meaningful variable names
- Add comments for complex logic
- Test on multiple platforms
- Keep functions focused and small

## 🐛 Known Issues

- Search operations are case-insensitive but require exact substring matches
- Large datasets (>10,000 records) may experience slower search performance
- Index file corruption can occur if program terminates unexpectedly

## 🔮 Future Enhancements

- [ ] Add data export/import (CSV, JSON)
- [ ] Implement B-tree indexing for better search performance
- [ ] Add data validation and constraints
- [ ] Create GUI version using Qt or similar
- [ ] Add networking support for client-server architecture
- [ ] Implement transaction support
- [ ] Add backup and restore functionality

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Your Name**
- GitHub: [@perilousTF](https://github.com/perilousTF)
- Email: pratyaypatra31@gmail.com

## 🙏 Acknowledgments

- Thanks to the C++ community for excellent documentation
- Inspired by traditional database management systems
- Built as a learning project for understanding file-based databases

---

⭐ **Star this repository if you find it helpful!** ⭐
