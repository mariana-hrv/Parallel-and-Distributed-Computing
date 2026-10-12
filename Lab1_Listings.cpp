#include <iostream>
#include <thread>

using namespace std;

// Завдання 1.2.1
// Створення двох потоків без join() та detach()

void Thread1() {
    cout << 1 << endl;
}

void Thread2() {
    cout << 2 << endl;
}

int main() {
    thread t1(Thread1);
    thread t2(Thread2);

    return 0;
}

