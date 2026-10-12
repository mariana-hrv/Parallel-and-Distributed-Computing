#include <iostream>
#include <thread>
#include <list>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <atomic>
#include <functional>
#include <utility>
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

 // Завдання 1.2.6
 // Обмін даними із захистом від deadlock

class someData {
public:
    string name;
    string surname;
    string address;
    int age = 0;
};

class exchangePerson {
private:
    someData data;
    mutex m;

    static void Print(const string& title,
                      const someData& person) {
        cout << title << endl;
        cout << "Name: " << person.name << endl;
        cout << "Surname: " << person.surname << endl;
        cout << "Address: " << person.address << endl;
        cout << "Age: " << person.age << endl << endl;
    }

public:
    static void JohnDoe(exchangePerson& person) {
        lock_guard<mutex> lock(person.m);

        person.data.name = "John";
        person.data.surname = "Doe";
        person.data.address = "Unknown";
        person.data.age = 120;
    }

    static void JacobSmith(exchangePerson& person) {
        lock_guard<mutex> lock(person.m);

        person.data.name = "Jacob";
        person.data.surname = "Smith";
        person.data.address = "Known";
        person.data.age = 1;
    }

    static void Swap(exchangePerson& first,
                     exchangePerson& second) {
        if (&first == &second) {
            cout << "Cannot swap the same object!"
                 << endl;
            return;
        }

        std::lock(first.m, second.m);

        lock_guard<mutex> lock1(first.m, adopt_lock);
        lock_guard<mutex> lock2(second.m, adopt_lock);

        cout << "BEFORE SWAP:" << endl;
        Print("Person 1:", first.data);
        Print("Person 2:", second.data);

        swap(first.data, second.data);

        cout << "AFTER SWAP:" << endl;
        Print("Person 1:", first.data);
        Print("Person 2:", second.data);
    }

    // Завдання 1.2.7
    // Обмін із використанням unique_lock

    static void SwapUnique(exchangePerson& first,
                           exchangePerson& second) {
        if (&first == &second) {
            cout << "Cannot swap the same object!"
                 << endl;
            return;
        }

        unique_lock<mutex> lock1(first.m, defer_lock);
        unique_lock<mutex> lock2(second.m, defer_lock);

        std::lock(lock1, lock2);

        cout << "BEFORE SWAP:" << endl;

        Print("Person 1:", first.data);
        Print("Person 2:", second.data);

        swap(first.data, second.data);

        cout << "AFTER SWAP:" << endl;

        Print("Person 1:", first.data);
        Print("Person 2:", second.data);
    }

};

void Task126() {
    exchangePerson person1;
    exchangePerson person2;

    atomic<int> completed{0};

    thread t1([&]() {
        exchangePerson::JohnDoe(person1);
        ++completed;
    });
    t1.detach();

    thread t2([&]() {
        exchangePerson::JacobSmith(person2);
        ++completed;
    });
    t2.detach();

    while (completed.load() < 2) {
        this_thread::sleep_for(
            chrono::milliseconds(1));
    }

    thread t3(exchangePerson::Swap,
              ref(person1), ref(person2));

    t3.join();
}

void Task127() {
    exchangePerson person1;
    exchangePerson person2;

    atomic<int> completed{0};

    thread t1([&]() {
        exchangePerson::JohnDoe(person1);
        ++completed;
    });
    t1.detach();

    thread t2([&]() {
        exchangePerson::JacobSmith(person2);
        ++completed;
    });
    t2.detach();

    while (completed.load() < 2) {
        this_thread::sleep_for(
            chrono::milliseconds(1));
    }

    thread t3(exchangePerson::SwapUnique,
              ref(person1), ref(person2));

    t3.join();
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
        
    case 126:
        Task126();
        break;
        
    case 127:
        Task127();
        break;

    default:
        cout << "Unknown task" << endl;
    }

    return 0;
}
