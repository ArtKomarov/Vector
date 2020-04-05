#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>

typedef double vec_t; // Vector elemnt's type

extern const char  DEFAULT_VEC_LOG_FILE[]; // Name of default log file (for dump)
extern const vec_t TRESHOLD;               // Treshold for elements value to print warning (for dump)
extern const vec_t VALUE_ODDS;             // Odds (maxsize_ - size_) for reducing vector in pop_back

struct Vector {
private:
    vec_t  *buf_ = nullptr; // Memory for elements
    size_t size_;           // Number of current vector elements
    size_t maxsize_;        // Size of allocated memory
    FILE* log_   = nullptr; // Log file descriptor for dump
public:
    // Constructors/Destructor
    Vector ();                  // Default constructor
    Vector (int size);          // Create empty vector of size "size", initialized by zeroes
    Vector (const Vector &vec); // Copy constructor
    ~Vector();                  // Destructor

    //Operators
    Vector& operator= (const Vector &vec);        // Copy operator
    vec_t operator[] (int id) const;              // Get element by id
    Vector operator+ (const Vector &vec) const;   // Addition two vectors
    Vector operator- (const Vector &vec) const;   // Subtraction two vectors
    void* operator new(size_t size);              // Default new
    void* operator new(size_t size, Vector* vec); // Only usage constructor to 'vec' (Using in operator=)

    //Other functions
    int    push_back (vec_t elem); // Append element to the end of vector
    vec_t  pop_back  ();           // Get last element of the vector and delete him
    bool   Ok        () const;     // Check fields of vector
    size_t Size      () const;     // Get vector size
    vec_t  Front     () const;     // Get first element
    vec_t  Back      () const;     // Get last element
    int    Dump      () const;     // Print whole vector to a log file
    int ChangeLogFile(char* name); // Set log file for dumping
};

#endif // VECTOR_H
