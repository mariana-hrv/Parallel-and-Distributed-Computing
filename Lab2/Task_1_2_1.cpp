
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

using namespace std;

queue<int> dataQueue;
mutex m;
bool dataReady = false;

// Перевірка числа на простоту
bool IsPrime(int number) {
    if (number < 2)
        return false;

    for (int i = 2; i <= number / i; i++) {
        if (number % i == 0)
            return false;
    }

    return true;
}

// Потік підготовки даних
void DataPreparation() {
    cout << "Enter integers (q to finish):" << endl;

    int number;

    while (cin >> number) {
        lock_guard<mutex> lock(m);
        dataQueue.push(number);
    }

    {
        lock_guard<mutex> lock(m);
        dataReady = true;
    }
}

// Потік обробки даних
void DataProcessing() {
    unique_lock<mutex> lk(m);

    while (!dataReady) {
        lk.unlock();
        this_thread::sleep_for(chrono::milliseconds(100));
        lk.lock();
    }

    queue<int> numbers;
    numbers.swap(dataQueue);
    lk.unlock();

    cout << "Prime numbers:";

    bool found = false;

    while (!numbers.empty()) {
        int number = numbers.front();
        numbers.pop();

        if (IsPrime(number)) {
            cout << " " << number;
            found = true;
        }
    }

    if (!found)
        cout << " none";

    cout << endl;
}

int main() {
    thread preparation(DataPreparation);
    preparation.detach();

    thread processing(DataProcessing);
    processing.join();

    return 0;
}
