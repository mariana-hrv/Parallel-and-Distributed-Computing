#include <iostream>
#include <thread>

using namespace std;

void Thread1() {
    cout << 1 << endl;
}

void Thread2() {
    cout << 2 << endl;
}

// Завдання 1.2.1
// Потоки без join() та detach()
void Task121() {
    thread t1(Thread1);
    thread t2(Thread2);
}

// Завдання 1.2.2
// Від'єднання потоків методом detach()
void Task122() {
    thread t1(Thread1);
    thread t2(Thread2);

    t1.detach();
    t2.detach();
}

int main() {
    const int task = 122;

    if (task == 121) {
        Task121();
    }
    else if (task == 122) {
        Task122();
    }

    return 0;
}
