#pragma once
#include <array>

class Error {
    public:
        enum Code {
            kSuccess,
            kFull,
            kEmpty,
            kLastOfCode,
        };
    
        Error(Code code) : code_{code} {} //constructer

        operator bool() const { //cast to bool, const meads this fucntion doesn't change member variable.
            //member function can access to data member in class. this represents pointer to instance.
            return this -> code_ != kSuccess;
        }

        const char* Name() const { //return value is const ,which means can't be changed.
            return code_names_[static_cast<int>(this -> code_)]; //in case of kSuccuess, it is casted to 0.
        }

    private:
        static constexpr std::array<const char*,3> code_names_ = { //char* means string!
            "kSuccess",
            "kFull",
            "kEmpty",
        };

        Code code_;
};