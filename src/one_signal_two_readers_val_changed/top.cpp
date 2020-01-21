#include <systemc.h>

#include "helpers.h"

SC_MODULE(Sender) {
    public:
    sc_port<sc_signal_inout_if<int>> out;

    SC_CTOR(Sender) {
        SC_THREAD(execute);
    }
    void execute() {
        int i = 0;

        log(name(), "starting");
        while (true) {
            for (int j = 0; j < 10; j++) {
                wait(1, SC_NS);
                log(name(), "writing", i);
                out->write(i);

            }
            ++i;
        }
    }
};

SC_MODULE(Receiver) {
    public:
    sc_port<sc_signal_in_if<int>> in;

    SC_CTOR(Receiver) {
        SC_THREAD(execute);
    }

    void execute() {
        while (1) {
            wait(in->value_changed_event());
            log(name(), "read", in->read());
        }
    }
};

int sc_main(int argc, char *argv[]) {
    Sender s("sender");
    Receiver r1("receiver_one");
    Receiver r2("receiver_two");

    // receivers will only trigger if value changes.
    sc_signal<int> sig1;

    // receivers will trigger even if value written is the same
    // sc_buffer<int> sig1;

    s.out(sig1);
    r1.in(sig1);
    r2.in(sig1);

    sc_start();
    return 0;
}
