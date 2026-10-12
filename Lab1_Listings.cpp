#include <iostream>
#include <thread>
#include <list>
#include <algorithm>
#include <chrono>
#include <mutex>

using namespace std;

// =====================================
// Завдання 1.2.1
// Створення потоків без join() і detach()
// =====================================

void Thread1() {
    cout << 1 << endl;
}

void Thread2() {
    cout << 2 << endl;
}

void Task121() {
    thread t1(Thread1);
    thread t2(Thread2);
}

// =====================================
// Завдання 1.2.2
// Від'єднання потоків методом detach()
// =====================================

void Task122() {
    thread t1(Thread1);
    thread t2(Thread2);

    t1.detach();
    t2.detach();
}

// =====================================
// Завдання 1.2.3
// Паралельний доступ до глобального списку
// =====================================

list<int> l;

void AddToList(int startValue) {
    for (int i = 0; i < 10; i++) {
        int value = startValue + i;
        l.push_back(value);

        cout << "AddToList: added " << value << endl;

        this_thread::sleep_for(
            chrono::milliseconds(20));
    }
}

void ListContains(int searchValue) {
    for (int i = 0; i < 10; i++) {
        bool found =
            find(l.begin(), l.end(), searchValue)
            != l.end();

        cout << "ListContains: attempt " << i + 1
             << ", value " << searchValue
             << (found ? " FOUND" : " NOT FOUND")
             << endl;

        this_thread::sleep_for(
            chrono::milliseconds(25));
    }
}

void Task123() {
    l.clear();

    int startValue = 1;
    int searchValue = 5;

    thread t1(AddToList, startValue);
    thread t2(ListContains, searchValue);

    t1.join();
    t2.join();
}

 // Завдання 1.2.4
 // Синхронізований доступ до списку

list<int> l124;
mutex m124;

void AddToList124(int startValue) {
    for (int i = 0; i < 10; i++) {
        int value = startValue + i;

        m124.lock();
        l124.push_back(value);
        cout << "AddToList: added " << value << endl;
        m124.unlock();

        this_thread::sleep_for(chrono::milliseconds(20));
    }
}

void ListContains124(int searchValue) {
    for (int i = 0; i < 10; i++) {
        m124.lock();

        bool found =
            find(l124.begin(), l124.end(), searchValue)
            != l124.end();

        cout << "ListContains: attempt " << i + 1
             << ", value " << searchValue
             << (found ? " FOUND" : " NOT FOUND")
             << endl;

        m124.unlock();

        this_thread::sleep_for(chrono::milliseconds(25));
    }
}

void Task124() {
    l124.clear();

    thread t1(AddToList124, 1);
    thread t2(ListContains124, 5);

    t1.join();
    t2.join();
}

 // Завдання 1.2.5
 // Використання std::lock_guard

list<int> l125;
mutex m125;
int checksCompleted125 = 0;

void AddToList125(int value) {
    lock_guard<mutex> lock(m125);

    l125.push_back(value);

    cout << "AddToList: added "
         << value << endl;
}

void ListContains125(int searchValue, int attempt) {
    lock_guard<mutex> lock(m125);

    bool found =
        find(l125.begin(), l125.end(), searchValue)
        != l125.end();

    cout << "ListContains: attempt " << attempt
         << ", value " << searchValue
         << (found ? " FOUND" : " NOT FOUND")
         << endl;

    ++checksCompleted125;
}

void Task125() {
    l125.clear();
    checksCompleted125 = 0;

    for (int i = 0; i < 10; i++) {
        thread t1(AddToList125, i + 1);
        t1.detach();

        thread t2(ListContains125, 5, i + 1);
        t2.detach();
    }

    while (true) {
        bool finished;

        {
            lock_guard<mutex> lock(m125);
            finished = (l125.size() == 10 &&
                        checksCompleted125 == 10);
        }

        if (finished)
            break;

        this_thread::sleep_for(
            chrono::milliseconds(10));
    }
}

// =====================================
// Головна функція
// =====================================

int main() {
    int task = 125;

    switch (task) {
    case 121:
        Task121();
        break;

    case 122:
        Task122();
        break;

    case 123:
        Task123();
        break;

    case 124:
        Task124();
        break;
        
    case 125:
        Task125();
        break;

    default:
        cout << "Unknown task" << endl;
    }

    return 0;
}
