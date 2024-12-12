#include<iostream>
#include<exception>
bool checkAndFixProblem() {
    // try to fix problem
    // return true;

    // fix problem fail
    return false;
}

void mightGoWrong() {
    if (!checkAndFixProblem()) {
        throw std::runtime_error("Something went wrong and couldn't be fixed");
    }
}

int main() {
    try {
        mightGoWrong();
        std::cout << "This line will not be executed if exception is thrown"
            << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: "
            << e.what() << std::endl;
    }
    std::cout << "Program continues after catching the exception"
        << std::endl;
    return 0;
}
