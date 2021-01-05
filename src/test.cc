#include "function_maxima.h"

#include <cassert>
#include <iostream>
#include <vector>

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
    FunctionMaxima<int, int>::point_type p {nullptr, nullptr};
}
