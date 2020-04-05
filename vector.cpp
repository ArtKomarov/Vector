#include "vector.h"
#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>

#define LOG_FILE_ASSIGMENT(LogFileName) \
    if((this->log_ = fopen(LogFileName, "w")) == nullptr) { \
        fprintf(stderr, "Can not open file %s for writing\n", LogFileName); \
        this->log_ = nullptr; \
    }

const char  DEFAULT_VEC_LOG_FILE[] = "vector.log";
const vec_t TRESHOLD   = 10e5;
const vec_t VALUE_ODDS = 30;

Vector::Vector() :
    size_(VALUE_ODDS),
    maxsize_(0) {
    buf_ = new vec_t[size_];
    LOG_FILE_ASSIGMENT(DEFAULT_VEC_LOG_FILE)
}

Vector::Vector(int size) :
    size_(0),
    maxsize_(size) {
    buf_ = new vec_t[size]; //(vec_t*) calloc(size, sizeof(vec_t));
    LOG_FILE_ASSIGMENT(DEFAULT_VEC_LOG_FILE)
    assert(buf_ != nullptr);
}

Vector::Vector(const Vector& vec) {
    if(!vec.Ok()) {
        std::cerr << "Can not copy destroyed vector!" << std::endl;
        return;
    }
    vec_t* sup = new (std::nothrow) vec_t[vec.maxsize_]; //(vec_t*) malloc(vec.maxsize_ * sizeof(vec_t));
    if(sup == nullptr) {
        std::cerr << "Can not allocate memory!" << std::endl;
        return;
    }
    buf_     = sup;
    std::memcpy(buf_, vec.buf_, vec.size_ * sizeof(vec_t));
    size_    = vec.size_;
    maxsize_ = vec.maxsize_;
    LOG_FILE_ASSIGMENT(DEFAULT_VEC_LOG_FILE)
}

Vector::~Vector() {
    delete[] buf_; //free(buf_);
    buf_ = nullptr;
    if(log_)
        fclose(log_);
    log_ = nullptr;
}

vec_t Vector::operator[](int id) const {
    assert(id >= 0 && (size_t) id < size_);
    if(this->Ok())
        return buf_[id];
    else
        return -1;
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

void* Vector::operator new(size_t size) {
    return ::operator new(size);
}

void* Vector::operator new(size_t size, Vector* vec){
    return vec;
}

Vector& Vector::operator=(const Vector &vec) {
    fclose(log_);
    delete[] buf_;
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
    if(log_ == nullptr) {
        std::cerr << "Vector's log file is nullptr!" << std::endl;
        return -1;
    }
    fprintf(log_, "DUUUMP:\nVector's buffer = %p", buf_);
    if(buf_ == nullptr)
        fprintf(log_, " : There is empty buffer in vector!\n");
    else {
        fprintf(log_, "\nmaxsize_ = %lu, size_ = %lu", maxsize_, size_);
        if(maxsize_ < size_)
            fprintf(log_, " : Vector's size is greater than his maximal size!\n");
        else {
            fprintf(log_, "\n{\n");
            for(size_t i = 0; i < size_; i++) {
                char SlashN = '\n';
                if(buf_[i] > TRESHOLD)
                    SlashN = ' ';
                fprintf(log_, "    [%3lu] = %f%c", i, buf_[i], SlashN);
                if(SlashN == ' ')
                    fprintf(log_, ": POISON?\n");
            }
        }
        fprintf(log_, "}\n\n");
    }
    return 0;
}

int Vector::ChangeLogFile(char* name) {
    FILE* NewLog = fopen(name, "w");
    if(NewLog == nullptr) {
        fprintf(stderr, "Can not open file %s for writing\n", name);
        return -1;
    }
    fclose(log_);
    log_ = NewLog;
    return 0;
}
