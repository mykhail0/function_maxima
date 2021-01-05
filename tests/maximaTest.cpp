#include "gtest/gtest.h"
#include "../src/function_maxima.h"
#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <thread>
#include <chrono>

#define GTEST_COUT std::cerr << "\033[1;36m[          ] [ INFO ]\033[0m"



// Testy Peczara

class Secret {
public:
  int get() const {
    return value;
  }
  bool operator<(const Secret &a) const {
    return value < a.value;
  }
  static Secret create(int v) {
    return Secret(v);
  }
  bool operator==(const Secret &a) const {
    return value == a.value;
  }
private:
  Secret(int v) : value(v) {
  }
  int value;
};

template<typename A, typename V>
struct same {
  bool operator()(const typename FunctionMaxima<A, V>::point_type &p,
                  const std::pair<A, V> &q) {
    return !(p.arg() < q.first) && !(q.first < p.arg()) &&
           !(p.value() < q.second) && !(q.second < p.value());
  }
};

template<typename A, typename V>
bool fun_equal(const FunctionMaxima<A, V> &F,
               const std::initializer_list<std::pair<A, V>> &L) {
  return F.size() == L.size() &&
         std::equal(F.begin(), F.end(), L.begin(), same<A, V>());
}

template<typename A, typename V>
bool fun_mx_equal(const FunctionMaxima<A, V> &F,
                  const std::initializer_list<std::pair<A, V>> &L) {
  return static_cast<typename FunctionMaxima<A, V>::size_type>(std::distance(F.mx_begin(), F.mx_end())) == L.size() &&
         std::equal(F.mx_begin(), F.mx_end(), L.begin(), same<A, V>());
}

TEST(example, example) {
  FunctionMaxima<int, int> fun;
  fun.set_value(0, 1);
  ASSERT_TRUE(fun_equal(fun, {{0, 1}}));
  ASSERT_TRUE(fun_mx_equal(fun, {{0, 1}}));

  fun.set_value(0, 0);
  ASSERT_TRUE(fun_equal(fun, {{0, 0}}));
  ASSERT_TRUE(fun_mx_equal(fun, {{0, 0}}));

  fun.set_value(1, 0);
  fun.set_value(2, 0);
  ASSERT_TRUE(fun_equal(fun, {{0, 0}, {1, 0}, {2, 0}}));
  ASSERT_TRUE(fun_mx_equal(fun, {{0, 0}, {1, 0}, {2, 0}}));

  fun.set_value(1, 1);
  ASSERT_TRUE(fun_mx_equal(fun, {{1, 1}}));

  fun.set_value(2, 2);
  ASSERT_TRUE(fun_mx_equal(fun, {{2, 2}}));
  fun.set_value(0, 2);
  fun.set_value(1, 3);
  ASSERT_TRUE(fun_mx_equal(fun, {{1, 3}}));

  try {
    // Nie ma wartości na tym argumencie
    auto c = fun.value_at(4);
    (void) c;
    ASSERT_TRUE(false);
  } catch (InvalidArg &e) {
    std::cout << e.what() << std::endl;
  }

  fun.erase(1);
  ASSERT_TRUE(fun.find(1) == fun.end());
  ASSERT_TRUE(fun_mx_equal(fun, {{0, 2}, {2, 2}}));

  fun.set_value(-2, 0);
  fun.set_value(-1, -1);
  ASSERT_TRUE(fun_mx_equal(fun, {{0, 2}, {2, 2}, {-2, 0}}));

  std::vector<FunctionMaxima<Secret, Secret>::point_type> v;
  {
    FunctionMaxima<Secret, Secret> temp;
    temp.set_value(Secret::create(1), Secret::create(10));
    temp.set_value(Secret::create(2), Secret::create(20));
    v.push_back(*temp.begin());
    v.push_back(*temp.mx_begin());
  }
  ASSERT_TRUE(v[0].arg().get() == 1);
  ASSERT_TRUE(v[0].value().get() == 10);
  ASSERT_TRUE(v[1].arg().get() == 2);
  ASSERT_TRUE(v[1].value().get() == 20);

  // To powinno działać szybko.
  FunctionMaxima<int, int> big;
  using size_type = decltype(big)::size_type;
  const size_type N = 100000;
  for (size_type i = 1; i <= N; ++i) {
    big.set_value(i, i);
  }
  size_type counter = 0;
  for (size_type i = 1; i <= N; ++i) {
    big.set_value(i, big.value_at(i) + 1);
    for (auto it = big.mx_begin(); it != big.mx_end(); ++it) {
      ++counter;
    }
  }
  ASSERT_TRUE(counter == 2 * N - 1);
  big = fun;
}

// Własne testy
const int SPECIAL_THROW_VALUE = 42;

class ThrowsOnCompare {
public:
  int get() const {
    return value;
  }
  bool operator<(const ThrowsOnCompare &a) const {
    if (a.value == SPECIAL_THROW_VALUE) {
        // Kto powiedział ze to ma być std::exception :)
        throw std::string("BOOM");
    }
    return value < a.value;
  }
  static ThrowsOnCompare create(int v) {
    return ThrowsOnCompare(v);
  }
  private:
  explicit ThrowsOnCompare(int v) : value(v) {
  }
  int value;
};

TEST(throwOnCompare, sizeNotChangingAfterChangesValue) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    temp.set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(20));
    ASSERT_EQ(temp.size(), 2);
    temp.set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(15));
    ASSERT_EQ(temp.size(), 2);
}

TEST(throwOnCompare, valueChangesProperly) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    temp.set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(20));
    temp.set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(15));
    ASSERT_EQ(temp.size(), 2);
    auto c = temp.value_at(ThrowsOnCompare::create(2));
    ASSERT_EQ(c.get(), 15);
}

TEST(throwOnCompare, exceptionHandlingProperly) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    EXPECT_THROW(
        {
            temp.set_value(ThrowsOnCompare::create(SPECIAL_THROW_VALUE), ThrowsOnCompare::create(20));
        }, 
        std::string);
    // Wyjątek się rzucił, nie powinno być dodanej liczby.
    ASSERT_EQ(temp.size(), 1);
}

TEST(throwOnCompare, exceptionHandlingProperlySwapped) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    EXPECT_THROW(
        {
            temp.set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(SPECIAL_THROW_VALUE));
        }, 
        std::string);
    // Wyjątek się rzucił, nie powinno być dodanej liczby.
    ASSERT_EQ(temp.size(), 1);
}

TEST(throwOnCompare, funNotChanging) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    temp.set_value(ThrowsOnCompare::create(50), ThrowsOnCompare::create(4)); // nie maksimum lokalne
    temp.set_value(ThrowsOnCompare::create(60), ThrowsOnCompare::create(7));

    EXPECT_THROW(
        {
            temp.set_value(ThrowsOnCompare::create(SPECIAL_THROW_VALUE), ThrowsOnCompare::create(100));
        }, 
    std::string);
    fun_equal(temp, {
            {ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)},
            {ThrowsOnCompare::create(50), ThrowsOnCompare::create(4)},
            {ThrowsOnCompare::create(60), ThrowsOnCompare::create(7)}
    });
}

TEST(throwOnCompare, localMaximasNotChanging) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    temp.set_value(ThrowsOnCompare::create(50), ThrowsOnCompare::create(4)); // nie maksimum lokalne
    temp.set_value(ThrowsOnCompare::create(60), ThrowsOnCompare::create(7));

    EXPECT_THROW(
        {
            temp.set_value(ThrowsOnCompare::create(SPECIAL_THROW_VALUE), ThrowsOnCompare::create(100));
        }, 
    std::string);
    fun_mx_equal(temp, {{ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)},
                        {ThrowsOnCompare::create(60), ThrowsOnCompare::create(7)}});
}

TEST(throwOnCompare, funNotChangedAfterThrow) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10));
    EXPECT_THROW(
        {
            temp.set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(SPECIAL_THROW_VALUE));
        }, 
        std::string);
    // Wyjątek się rzucił, nie powinno być ZMIENIONEJ liczby
    ASSERT_EQ(temp.size(), 1);
    ASSERT_EQ(temp.value_at(ThrowsOnCompare::create(1)).get(), 10);
}

// Tutaj moze byc robionych duzo kopii i będą długie, porownywanie dwoch slow tez moze dlugo trwac
int compareNum = 0;

class ExpensiveSecret {
public:
  std::string get() const {
    return value;
  }
  int getCompareNum() const {
    return compareNum;
  }
  bool operator<(const ExpensiveSecret &a) const {
    compareNum++;
    return value < a.value;
  }
  static ExpensiveSecret create(std::string v) {
    return ExpensiveSecret(v);
  }
  bool operator==(const ExpensiveSecret &a) const {
    return value == a.value;
  }
private:
  ExpensiveSecret(std::string v) : value(v) {
  }
  std::string value;
};

TEST(expensiveCompare, checkEqualLocalMaximas) {
    FunctionMaxima<ExpensiveSecret, ExpensiveSecret> temp;
    temp.set_value(ExpensiveSecret::create("Ala"), ExpensiveSecret::create("MaKota"));
    temp.set_value(ExpensiveSecret::create("Kot"), ExpensiveSecret::create("MaKota"));
    temp.set_value(ExpensiveSecret::create("Tom"), ExpensiveSecret::create("MaKota"));
    // Wszystkie wartości są takie same, więc są trzy lokalne maksima wg treści
    fun_mx_equal(temp, {{ExpensiveSecret::create("Ala"), ExpensiveSecret::create("MaKota")},
                        {ExpensiveSecret::create("Kot"), ExpensiveSecret::create("MaKota")},
                        {ExpensiveSecret::create("Tom"), ExpensiveSecret::create("MaKota")}
                        });
}

TEST(expensiveCompare, checkEqualLocalMaximasAfterUpdate) {
    FunctionMaxima<ExpensiveSecret, ExpensiveSecret> temp;
    temp.set_value(ExpensiveSecret::create("Ala"), ExpensiveSecret::create("MaKota"));
    temp.set_value(ExpensiveSecret::create("Kot"), ExpensiveSecret::create("MaKota"));
    temp.set_value(ExpensiveSecret::create("Tom"), ExpensiveSecret::create("MaKota"));
    // Wszystkie wartości są takie same, więc są trzy lokalne maksima wg treści
    temp.set_value(ExpensiveSecret::create("Luiza"), ExpensiveSecret::create("MialaKota"));
    fun_mx_equal(temp, {{ExpensiveSecret::create("Ala"), ExpensiveSecret::create("MaKota")},
                        {ExpensiveSecret::create("Luiza"), ExpensiveSecret::create("MialaKota")}
                        });
    temp.set_value(ExpensiveSecret::create("Radek"), ExpensiveSecret::create("StracilKota"));
}

std::string random_string(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz?:[]{}|$%^&*((_-+=";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

TEST(expensiveCompare, stringsGeneration) {
    for (int i = 0; i < 1000; i++) {
        auto s = random_string(5);
        (void) s;
    }
}

TEST(expensiveCompare, performanceTest) {
    int compareNumBegin = compareNum;
    FunctionMaxima<ExpensiveSecret, ExpensiveSecret> temp;
    for (int i = 0; i < 10000; i++) {
        auto s = random_string(5);
        temp.set_value(ExpensiveSecret::create(s), ExpensiveSecret::create("AlaMaKota"));
    }
    int compareNumMiddle = compareNum;
    GTEST_COUT << "Liczba wykonanych operacji po dodaniu 10000 słow < to " << compareNumMiddle - compareNumBegin << "\n";
    GTEST_COUT << "To jest na jedno dodanie ~" << ((compareNumMiddle - compareNumBegin) / 10000) / 13 << "porownań\n";
    int res1size = temp.size();
    GTEST_COUT << "W zbiorze jest aktualnie " << res1size << " elementow\n";
    // Teraz jest sprawdzenie czy się zaburzy za duzo i za dlugo
    temp.set_value(ExpensiveSecret::create("ka"), ExpensiveSecret::create("BasiaMaKota"));
    int res2size = temp.size();
    ASSERT_LE(res1size, res2size);
    GTEST_COUT << "Ustawienie nowego lokalnego maksimum wykonalo " << compareNum - compareNumMiddle << " porownan\n";
}

