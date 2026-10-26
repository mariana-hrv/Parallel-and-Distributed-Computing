
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

mutex m;
mutex coutMutex;
condition_variable cv;
int i = 0;

// Потоки, які очікують сигнал
void Waits(int number) {
    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Thread " << number
             << ": waiting for signal..." << endl;
    }

    unique_lock<mutex> lk(m);

    cv.wait(lk, [] {
        return i == 1;
    });

    lk.unlock();

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Thread " << number
             << ": waiting finished!" << endl;
    }
}

// Потік, який надсилає сигнали
void Awake() {
    this_thread::sleep_for(
        chrono::milliseconds(100));

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Awake: first notify_all() (i = 0)"
             << endl;
    }

    cv.notify_all();

    this_thread::sleep_for(
        chrono::milliseconds(100));

    {
        lock_guard<mutex> lk(m);
        i = 1;
    }

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Awake: second notify_all() (i = 1)"
             << endl;
    }

    cv.notify_all();
}

int main() {
    thread t1(Waits, 1);
    thread t2(Waits, 2);
    thread t3(Waits, 3);

    thread t4(Awake);

    t4.join();

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
