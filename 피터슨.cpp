#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

//volatile int sum = 0;
std::atomic <int> sum = 0;
std::mutex mylock;
int num_threads;

volatile int victim = 0;
volatile bool flag[2] = { false, false };


void Lock(int myID)
{
	int yourID = 1 - myID;
	flag[myID] = true;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	victim = myID;
	while (flag[yourID] && victim == myID) {}
}

void Unlock(int myID)
{
	flag[myID] = false;
}


void thread_func(int myID)
{

	for (auto i = 0; i < 50000000 / num_threads; ++i)
	{
		Lock(myID);
		//mylock.lock();
		//_asm lock add sum, 2;
		sum = sum + 2;
		//mylock.unlock();
		Unlock(myID);
	}

}




int main()
{
	int a;
	std::cin >> a;
	std::vector<std::thread*> threads;
	for (num_threads = 1; num_threads <= 16; num_threads *= 2)
	{
		sum = 0;
		threads.clear();
		auto t = std::chrono::high_resolution_clock::now();
		for (auto j = 0; j < num_threads; ++j)	threads.emplace_back(new std::thread(thread_func, num_threads));

		for (auto th : threads) {
			th->join();
			delete th;
		}
		auto d = std::chrono::high_resolution_clock::now() - t;

		std::cout << "Thread num : " << num_threads << std::endl;
		std::cout << "Sum : " << sum << std::endl;
		std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl << std::endl;
	}
	return 0;
}


