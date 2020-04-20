#include "vector.h"
#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>

//#define LOG_FILE_ASSIGMENT(LogFileName) \
//    if((this->log_ = fopen(LogFileName, "a")) == nullptr) { \
//        fprintf(stderr, "Can not open file %s for writing\n", LogFileName); \
//        this->log_ = nullptr; \
//    }

const char   DEFAULT_VEC_LOG_FILE[] = "vector.log";
const vec_t  TRESHOLD   = 10e5;
const size_t VALUE_ODDS = 30;

static FILE*  VecLogFileStream    = nullptr; //fopen(DEFAULT_VEC_LOG_FILE, "a");
static size_t VecDefStreamCounter = 0;

//Vector::Vector() :
//    buf_(new (std::nothrow) vec_t[maxsize_]),
//    size_(0),
//    maxsize_(VALUE_ODDS) {
//    LOG_FILE_ASSIGMENT(DEFAULT_VEC_LOG_FILE)
//    assert(buf_ != nullptr);
//}

FILE* VecLogFileAssigment() {
    if(VecDefStreamCounter == 0) {
        if((VecLogFileStream = fopen(DEFAULT_VEC_LOG_FILE, "a")) == nullptr) {
            fprintf(stderr, "Can not open file %s for writing\n", DEFAULT_VEC_LOG_FILE);
            VecLogFileStream = stderr;
        }
    }
    VecDefStreamCounter++;
    return VecLogFileStream;
}

Vector::Vector() :
    Vector(VALUE_ODDS) {
}

Vector::Vector(size_t size) :
    buf_(new (std::nothrow) vec_t[size]),
    size_(0),
    maxsize_(size),
    log_(VecLogFileAssigment()) {
    assert(buf_ != nullptr);
}

Vector::Vector(const Vector& vec) {
    if(!vec.Ok()) {
        std::cerr << "Can not copy destroyed vector!" << std::endl;
        return;
    }

    new (this) Vector(vec.maxsize_);

    //    vec_t* sup = new (std::nothrow) vec_t[vec.maxsize_]; //(vec_t*) malloc(vec.maxsize_ * sizeof(vec_t));
    //    if(sup == nullptr) {
    //        std::cerr << "Can not allocate memory!" << std::endl;
    //        return;
    //    }
    //    buf_     = sup;
    std::memcpy(buf_, vec.buf_, vec.size_ * sizeof(vec_t));
    size_    = vec.size_;
    //    maxsize_ = vec.maxsize_;
    log_ = VecLogFileAssigment();
}

Vector::~Vector() {
    delete[] buf_; //free(buf_);
    buf_ = nullptr;
    VecDefStreamCounter--;
    if(VecDefStreamCounter == 0) {
        if(log_ && log_ != stderr)
            fclose(log_);
        VecLogFileStream = nullptr;
    }
    log_ = nullptr;
}

vec_t Vector::operator[](int id) const {
    assert(id >= 0 && (size_t) id < size_);
    return buf_[id];
}

vec_t& Vector::operator[](int id) {
    assert(id >= 0 && (size_t) id < size_);
    if(this->Ok())
        return buf_[id];
    else {
        vec_t* ZerRet = new vec_t;
        return *ZerRet;
    }
}

Vector Vector::operator+(const Vector &that) const {
    const Vector& Long  = size_ >= that.size_ ? *this : that;
    const Vector& Short = size_ >= that.size_ ? that  : *this;

    Vector result(Long.size_);
    result.size_ = result.maxsize_;
    for(size_t i = 0; i < Long.size_; i++)
        result.buf_[i] = Long.buf_[i];
    for(size_t i = 0; i < Short.size_; i++)
        result.buf_[i] += Short.buf_[i];
    return result;
}

Vector Vector::operator-(const Vector &that) const {
    size_t ResSize = size_ >= that.size_ ? size_ : that.size_; // Now we know, what vector is longer
    Vector result(ResSize);         // now result.maxsize_ == ResSize
    result.size_ = result.maxsize_;
    for(size_t i = 0; i < size_; i++)
        result.buf_[i] = buf_[i];

    if(ResSize == size_)
        for(size_t i = 0; i < that.size_; i++)
            result.buf_[i] -= that.buf_[i];
    else {
        for(size_t i = 0; i < size_; i++)
            result.buf_[i] -= that.buf_[i];
        for(size_t i = size_; i < ResSize; i++)
            result.buf_[i] = -that.buf_[i];
    }
    return result;
}

void* Vector::operator new(size_t size, Vector* vec) { // inplace new (it is already exists in standart libruary)
    return vec;
}

Vector& Vector::operator=(const Vector &vec) {
    if(this == &vec)
        return *this;
    this->~Vector();
    return *new(this) Vector(vec);
}

int Vector::push_back(vec_t elem) {
    if(maxsize_ > size_)
        buf_[size_++] = elem;
    else {
        vec_t* NewBuff = new vec_t[(maxsize_ * 2) * sizeof(vec_t)];
        //(vec_t*) realloc(this->buf_, (maxsize_ * 2) * sizeof(vec_t)); // PROBLEMS!
        if(NewBuff == NULL) {
            fprintf(stderr, "Vector::Append(vec_t elem): memory allocate fail!\n");
            return -1;
        }
        else {
            maxsize_ *= 2;
            std::copy(buf_, buf_ + size_, NewBuff);
            delete[] buf_;
            buf_ = NewBuff;
            buf_[size_++] = elem;
        }
    }
    return 0;
}

vec_t Vector::pop_back() {
    if(this->Ok() == false)
        return 0;
    vec_t LastElem;
    if(size_ + VALUE_ODDS < maxsize_ / 2) {
        vec_t* sup = new vec_t[maxsize_ / 2];
        //(vec_t*)realloc(buf_, maxsize_ / 2);
        if(sup == nullptr) {
            std::cerr << "Can not reallocate memory!" << std::endl;
            return -1;
        }
        else {
            LastElem = buf_[size_ - 1];
            std::copy(buf_, buf_ + size_ - 1, sup);
        }
        maxsize_ /= 2;
        buf_ = sup;
    }
    --size_;
    return LastElem;

}

vec_t& Vector::at(int id) {
    if(id < 0 || (size_t)id >= size_) {
        std::cerr << "Index is out of size!" << std::endl;
        vec_t* ZerRet = new vec_t;
        return *ZerRet;
    }
    if(this->Ok())
        return buf_[id];
    vec_t* ZerRet = new vec_t;
    return *ZerRet;
}

bool Vector::Ok() const { // Don't check log file (it could be nullptr), if something go wrong, dumping vector
    bool check = true;
    if(buf_ == nullptr) {
        std::cerr << "Buffer is nullptr!" << std::endl;
        check = false;
    }
    if(maxsize_ == 0 ) {
        //std::cerr << "Maximum size of buffer is 0!" << std::endl;
        check = false;
    }
    if(size_ > maxsize_) {
        //std::cerr << "Size greater then maximum size!" << std::endl;
        check = false;
    }
    if(!check)
        this->Dump();
    return check;
}

size_t Vector::Size() const {
    this->Ok();
    return size_;
}

vec_t Vector::Front() const {
    return (*this)[0];
}

vec_t Vector::Back() const {
    return (*this)[size_ - 1];
}

int Vector::Dump() const {
    FILE* DumpFile = log_;
    if(log_ == nullptr) {
        std::cerr << "Vector's log file is nullptr!" << std::endl;
        //return -1;
        DumpFile = stderr;
    }
    fprintf(DumpFile, "DUUUMP (Vector %p):\nVector's buffer = %p", this, buf_);
    if(buf_ == nullptr)
        fprintf(DumpFile, " : There is empty buffer in vector!\n");
    else {
        fprintf(DumpFile, "\nmaxsize_ = %lu, size_ = %lu", maxsize_, size_);
        size_t DumpBuffSize = size_;
        if(maxsize_ < size_) {
            fprintf(DumpFile, " : Vector's size is greater than his maximal size!\n");
            DumpBuffSize = maxsize_;
        }
        fprintf(DumpFile, "\n{\n");
        for(size_t i = 0; i < DumpBuffSize; i++) {
            char SlashN = '\n';
            if(buf_[i] > TRESHOLD)
                SlashN = ' ';
            fprintf(DumpFile, "    [%3lu] = %f%c", i, buf_[i], SlashN);
            if(SlashN == ' ')
                fprintf(DumpFile, ": POISON?\n");
        }
        fprintf(DumpFile, "}\n\n");
    }
    return 0;
}

int Vector::ChangeLogFile(char* name) { // File opens for writing!
    FILE* NewLog = fopen(name, "w");
    if(NewLog == nullptr) {
        fprintf(stderr, "Can not open file %s for writing\n", name);
        return -1;
    }
    fclose(log_);
    log_ = NewLog;
    return 0;
}
