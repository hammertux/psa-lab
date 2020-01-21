#include "helpers.h"

using namespace std;

// Name and time stamp
void log_(const char *comp) {
    cout << setw(t_width) << sc_time_stamp() << ": " << setw(n_width) << comp;
}

// Log msg
void log(const char *comp, const char *msg) {
    log_(comp);
    cout << ": " << msg << endl;
}


