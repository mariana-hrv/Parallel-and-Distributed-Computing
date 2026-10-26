
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

mutex m;
mutex coutMutex;

condition_variable cv;
condition_variable doneCv;

int i = 0;
bool stopWaiting = false;
bool oneFinished = false;

// Функція очікування сигналу
void Waits(int number) {
    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Thread " << number
             << ": waiting for signal..." << endl;
    }

    unique_lock<mutex> lk(m);

    cv.wait(lk, [] {
        return i == 1 || stopWaiting;
    });

    if (stopWaiting) {
        return;
    }

    lk.unlock();

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Message from Thread "
             << number << endl;
    }

    {
        lock_guard<mutex> lock(m);
        oneFinished = true;
    }

    doneCv.notify_one();
}

// Функція надсилання сигналу
void Notify() {
    this_thread::sleep_for(
        chrono::milliseconds(100));

    {
        lock_guard<mutex> lock(m);
        i = 1;
    }

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Notify: sending notify_one()"
             << endl;
    }

    cv.notify_one();
}

int main() {
    thread t1(Waits, 1);
    thread t2(Waits, 2);
    thread t3(Waits, 3);

    thread t4(Notify);

    t4.join();

    // Очікуємо реакції одного потоку
    {
        unique_lock<mutex> lk(m);
        doneCv.wait(lk, [] {
            return oneFinished;
        });

        stopWaiting = true;
    }

    // Завершуємо два інші потоки
    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Main: stopping remaining threads"
             << endl;
    }

    cv.notify_all();

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
