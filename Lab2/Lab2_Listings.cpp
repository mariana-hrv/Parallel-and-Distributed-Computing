// Laboratory work No. 2: Parallel and Distributed Computing
// Seven complete program listings in one C++ source file.
// To run another task in Visual Studio, change LAB_TASK below from 1 to 2..7.
// Every task preserves its own main() and is compiled separately by the preprocessor.
// Thus only one main() is active at a time.
#ifndef LAB_TASK
#define LAB_TASK 1
#endif

#if LAB_TASK < 1 || LAB_TASK > 7
#error LAB_TASK must be between 1 and 7
#endif


// ============================================================================
// TASK 1.2.1
// ============================================================================
#if LAB_TASK == 1

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

#endif // LAB_TASK == 1


// ============================================================================
// TASK 1.2.2
// ============================================================================
#if LAB_TASK == 2

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

#endif // LAB_TASK == 2


// ============================================================================
// TASK 1.2.3
// ============================================================================
#if LAB_TASK == 3

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

#endif // LAB_TASK == 3


// ============================================================================
// TASK 1.2.4
// ============================================================================
#if LAB_TASK == 4

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

queue<int> dataQueue;
mutex m;
condition_variable cv;
bool inputFinished = false;

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
        {
            lock_guard<mutex> lock(m);
            dataQueue.push(number);
        }

        cv.notify_one();
    }

    {
        lock_guard<mutex> lock(m);
        inputFinished = true;
    }

    cv.notify_one();
}

// Потік обробки даних
void DataProcessing() {
    queue<int> primeNumbers;

    while (true) {
        unique_lock<mutex> lk(m);

        cv.wait(lk, [] {
            return !dataQueue.empty() || inputFinished;
            });

        if (dataQueue.empty() && inputFinished)
            break;

        int number = dataQueue.front();
        dataQueue.pop();
        lk.unlock();

        if (IsPrime(number))
            primeNumbers.push(number);
    }

    cout << "Prime numbers:";

    if (primeNumbers.empty()) {
        cout << " none";
    }

    while (!primeNumbers.empty()) {
        cout << " " << primeNumbers.front();
        primeNumbers.pop();
    }

    cout << endl;
}

int main() {
    thread preparation(DataPreparation);
    preparation.detach();

    thread processing(DataProcessing);
    processing.join();

    return 0;
}

#endif // LAB_TASK == 4


// ============================================================================
// TASK 1.2.5
// ============================================================================
#if LAB_TASK == 5

#include <iostream>
     #include <future>
     #include <vector>
     #include <chrono>
     #include <cmath>
     #include <iomanip>

     using namespace std;

     // Обчислення n-го простого числа
     int NthPrime(int n) {
         int limit = (n < 6) ? 15 :
             static_cast<int>(
                 n * (log(n) + log(log(n)))) + 10;
        vector<unsigned char> composite(limit + 1, 0);

        for (int p = 2; p <= limit / p; ++p) {
            if (!composite[p]) {
                for (int k = p * p; k <= limit; k += p) {
                    composite[k] = 1;
                }
            }
        }

        int found = 0;

        for (int x = 2; x <= limit; ++x) {
            if (!composite[x] && ++found == n) {
                return x;
            }
        }

        return -1;
    }

     // Дослідження одного режиму
     double RunExperiment(int n, launch mode, const char* modeName) {
         cout << "\nMode: " << modeName << endl;

        future<int> primeFuture = async(mode, NthPrime, n);

        cout << "Choose a function of n:" << endl;
        cout << "1 - sqrt(n), 2 - sin(n), 3 - ln(n): ";

        int choice;
        cin >> choice;

        double functionValue;

        if (choice == 1)
            functionValue = sqrt(static_cast<double>(n));
        else if (choice == 2)
            functionValue = sin(static_cast<double>(n));
        else if (choice == 3)
            functionValue = log(static_cast<double>(n));
        else {
            cout << "Invalid choice. Using sqrt(n)." << endl;
            functionValue = sqrt(static_cast<double>(n));
        }

        cout << "Function value: "
            << fixed << setprecision(4)
            << functionValue << endl;

        auto start = chrono::steady_clock::now();

        int prime = primeFuture.get();

        auto finish = chrono::steady_clock::now();

         double waitMs =
                       chrono::duration<double, milli>(finish - start).count();

        cout << "Prime number #" << n
            << ": " << prime << endl;

        cout << "Waiting time for get(): "
            << fixed << setprecision(2)
            << waitMs << " ms" << endl;

        return waitMs;
    }

    int main() {
        int n;

        cout << "Enter n (1 to 1000000): ";
        cin >> n;

        if (!cin || n < 1 || n > 1000000) {
            cout << "Invalid n." << endl;
            return 1;
        }

        double deferredMs =
            RunExperiment(n, launch::deferred, "deferred");

        double asyncMs =
                RunExperiment(n, launch::async, "async");

          cout << "\nComparison:" << endl;
          cout << "deferred: " << deferredMs << " ms" << endl;
          cout << "async:    " << asyncMs << " ms" << endl;

          return 0;
     }

#endif // LAB_TASK == 5


// ============================================================================
// TASK 1.2.6
// ============================================================================
#if LAB_TASK == 6

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <deque>
#include <vector>
#include <cmath>
#include <utility>

using namespace std;

// Глобальні черги завдань і номерів
deque<packaged_task<int(int)>> taskQueue;
deque<int> numberQueue;

mutex m;
condition_variable cv;
bool inputFinished = false;

// Обчислення n-го простого числа
int NthPrime(int n) {
    int limit = (n < 6) ? 15 :
        static_cast<int>(
            n * (log(n) + log(log(n)))) + 10;

    vector<unsigned char> composite(limit + 1, 0);

    for (int p = 2; p <= limit / p; ++p) {
        if (!composite[p]) {
            for (int k = p * p; k <= limit; k += p) {
                composite[k] = 1;
            }
        }
    }

    int found = 0;

    for (int x = 2; x <= limit; ++x) {
        if (!composite[x] && ++found == n) {
            return x;
        }
    }

    return -1;
}

// Робочий потік
void Worker() {
    while (true) {
        packaged_task<int(int)> task;
        int n;

        {
             unique_lock<mutex> lk(m);

             cv.wait(lk, [] {
                 return !taskQueue.empty() || inputFinished;
                 });

             if (taskQueue.empty() && inputFinished)
                 break;

             task = move(taskQueue.front());
             taskQueue.pop_front();

             n = numberQueue.front();
             numberQueue.pop_front();
        }

        // Виконання завдання
        task(n);
    }
}

int main() {
    vector<pair<int, future<int>>> results;

    // Запуск незалежного робочого потоку
    thread worker(Worker);

    cout << "Enter n (1 to 1000000)." << endl;
    cout << "Enter q to finish." << endl;

    int n;

    while (true) {
        cout << "n: ";

        if (!(cin >> n))
            break;
    if (n < 1 || n > 1000000) {
        cout << "Invalid n." << endl;
        continue;
    }

    packaged_task<int(int)> task(NthPrime);

    future<int> result = task.get_future();

    {
        lock_guard<mutex> lk(m);

        taskQueue.push_back(move(task));
        numberQueue.push_back(n);
    }

    results.emplace_back(n, move(result));

    cv.notify_one();

    cout << "Task added for n = " << n << endl;
}

// Повідомлення про завершення введення
{
    lock_guard<mutex> lk(m);
    inputFinished = true;
}

cv.notify_one();

// Очікування завершення робочого потоку
worker.join();

cout << "\nCalculation results:" << endl;

for (auto& item : results) {
    cout << "Prime number #" << item.first
        << ": " << item.second.get() << endl;
}

cout << "All tasks completed." << endl;
     return 0;
}

#endif // LAB_TASK == 6


// ============================================================================
// TASK 1.2.7
// ============================================================================
#if LAB_TASK == 7

#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <chrono>
#include <cmath>
#include <vector>
#include <iomanip>
#include <functional>

using namespace std;

// Обчислення n-го простого числа
int NthPrime(int n) {
    int limit = (n < 6) ? 15 :
        static_cast<int>(
            n * (log(n) + log(log(n)))) + 10;

     vector<unsigned char> composite(limit + 1, 0);
    for (int p = 2; p <= limit / p; ++p) {
        if (!composite[p]) {
            for (int k = p * p; k <= limit; k += p) {
                composite[k] = 1;
            }
        }
    }

    int found = 0;

    for (int x = 2; x <= limit; ++x) {
        if (!composite[x] && ++found == n)
            return x;
    }

    return -1;
}

// Перший робочий потік
void CalculatePrimes(
    int n,
    promise<int> firstPromise,
    promise<bool> signalPromise,
    promise<int> secondPromise)
{
    // Обчислення n-го простого числа
    int firstPrime = NthPrime(n);
    firstPromise.set_value(firstPrime);

    // Сигнал другому потоку
    signalPromise.set_value(true);

    // Обчислення (n * 10)-го простого числа
    int secondPrime = NthPrime(n * 10);
    secondPromise.set_value(secondPrime);
}

// Другий робочий потік
void CalculateSquareRoot(
    int n,
    future<bool> signalFuture,
    mutex& coutMutex)
{
    // Очікування сигналу від першого потоку
    bool ready = signalFuture.get();

    if (ready) {
        this_thread::sleep_for(
            chrono::seconds(2));

        double result = sqrt(static_cast<double>(n));

        lock_guard<mutex> lock(coutMutex);

        cout << "Square root of " << n
            << ": " << fixed << setprecision(4)
            << result << endl;
    }
}

int main() {
    int n;

    cout << "Enter n (1 to 100000): ";
    cin >> n;

    if (!cin || n < 1 || n > 100000) {
        cout << "Invalid n." << endl;
        return 1;
    }

    // Локальний м'ютекс для виведення
    mutex coutMutex;

    // Створення promise
    promise<int> firstPromise;
    promise<int> secondPromise;
    promise<bool> signalPromise;

    // Отримання відповідних future
    future<int> firstFuture =
        firstPromise.get_future();

    future<int> secondFuture =
        secondPromise.get_future();
future<bool> signalFuture =
    signalPromise.get_future();

// Запуск першого робочого потоку
thread t1(
    CalculatePrimes,
    n,
    move(firstPromise),
    move(signalPromise),
    move(secondPromise)
);

// Запуск другого робочого потоку
thread t2(
    CalculateSquareRoot,
    n,
    move(signalFuture),
    ref(coutMutex)
);

// Отримання першого результату
int firstPrime = firstFuture.get();

{
    lock_guard<mutex> lock(coutMutex);

    cout << "Prime number #" << n
        << ": " << firstPrime << endl;
}

// Отримання другого результату
int secondPrime = secondFuture.get();

{
    lock_guard<mutex> lock(coutMutex);

    cout << "Prime number #" << n * 10
        << ": " << secondPrime << endl;
}

// Очікування завершення обох потоків
t1.join();
      t2.join();

      cout << "All threads completed." << endl;

      return 0;
}

#endif // LAB_TASK == 7
