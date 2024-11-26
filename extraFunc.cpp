#include <iostream>
#include <thread> // For sleep functionality
#include <chrono>

#include "extraFunc.h"

void extraFunction1() {
    std::cout << "This is extra function 1" << std::endl;
}

void extraFunction2() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void extraFunction3() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
}