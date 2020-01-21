#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <iomanip>
#include <iostream>
#include <systemc.h>

const int t_width = 12;
const int n_width = 7;

extern void log_(const char *comp);
extern void log(const char *comp, const char *msg);

// Log 1 value
template<class T>
void log(const char *comp, const char *n1, T v1) {
    log_(comp);
    std::cout << ": " << n1 << ": " << v1 << std::endl;
}

// Log 2 values
template<class T1, class T2>
void log(const char *comp, const char *n1, T1 v1, const char *n2, T2 v2) {
    log_(comp);
    std::cout << ": " << n1 << ": " << v1 << ", " << n2 << ": " << v2 << std::endl;
}

// Log 3 values
template<class T1, class T2, class T3>
void log(const char *comp, const char *n1, T1 v1, const char *n2, T2 v2, const char *n3, T3 v3) {
    log_(comp);
    std::cout << ": " << n1 << ": " << v1 << ", " << n2 << ": " << v2 << \
        ", " << n3 << ": " << v3 << std::endl;
}

#endif

