#include "function_maxima.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <array>

using namespace std;

class Secret {
public:
    int get() const { return value; }

    bool operator<(const Secret &a) const { return value < a.value; }

    static Secret create(int v) { return Secret(v); }

private:
    Secret(int v) : value(v) {}

    int value;
};

int main() {
    // Should not compile.
    // FunctionMaxima<int, int>::point_type p {nullptr, nullptr};

    FunctionMaxima <Secret, Secret> func;

    Secret s1 = Secret::create(1);
    Secret s2 = Secret::create(2);
    Secret s3 = Secret::create(3);
    Secret s4 = Secret::create(3);
    Secret s11 = Secret::create(11);
    Secret s22 = Secret::create(22);
    Secret s33 = Secret::create(33);
    Secret s44 = Secret::create(44);
    func.set_value(s1, s11);
    func.set_value(s2, s22);
    func.set_value(s3, s33);
    func.set_value(s3, s33);
    func.set_value(s4, s44);

    for(auto it = func.mx_begin(); it != func.mx_end(); it++){
        cout << it->value().get() << endl;
    }
}
