#include <f5/threading/map.hpp>
#include <cassert>


void test_insert_or_assign() {
    f5::tsmap<int, std::unique_ptr<std::string>> map;

    map.insert_or_assign(1, std::make_unique<std::string>("one"));
    map.insert_or_assign(3, std::make_unique<std::string>("three"));
    map.insert_or_assign(2, std::make_unique<std::string>("two"));

    assert(map.find(1) && *map.find(1) == "one");
    assert(map.find(2) && *map.find(2) == "two");
    assert(map.find(3) && *map.find(3) == "three");

    map.insert_or_assign(2, std::make_unique<std::string>("2"));
    assert(map.find(2) && *map.find(2) == "2");

    assert(map.find(4) == nullptr);
}


void test_emplace_if_not_found() {
    f5::tsmap<int, std::unique_ptr<std::string>> map;

    map.emplace_if_not_found(1, std::make_unique<std::string>("one"));
    map.emplace_if_not_found(3, std::make_unique<std::string>("three"));
    map.emplace_if_not_found(2, std::make_unique<std::string>("two"));

    assert(map.find(1) && *map.find(1) == "one");
    assert(map.find(2) && *map.find(2) == "two");
    assert(map.find(3) && *map.find(3) == "three");

    map.emplace_if_not_found(2, std::make_unique<std::string>("2"));
    assert(map.find(2) && *map.find(2) == "two");

    assert(map.find(4) == nullptr);
}


int main() {
    test_insert_or_assign();
    test_emplace_if_not_found();
}

