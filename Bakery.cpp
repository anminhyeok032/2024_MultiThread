#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "Bakery.h"

volatile int sum;
std::mutex mylock;
int num_threads;

Bakery bakery;

void thread_func(int id)
{

	for (auto i = 0; i < 500'0000 / num_threads; ++i)
	{
		//mylock.lock();
		bakery.Lock(id);
		sum = sum + 2;
		bakery.UnLock(id);
		//mylock.unlock();
	}

}


int main()
{

	std::vector<std::thread*> threads;
	for (num_threads = 1; num_threads <= 8; num_threads *= 2)
	{
		sum = 0;
		threads.clear();

		bakery = { num_threads };

		auto t = std::chrono::high_resolution_clock::now();
		for (auto j = 0; j < num_threads; ++j)	threads.emplace_back(new std::thread(thread_func, j));

		for (auto th : threads) {
			th->join();
			delete th;
		}
		threads.clear();
		auto d = std::chrono::high_resolution_clock::now() - t;


		std::cout << "Thread num : " << num_threads << std::endl;
		std::cout << "Sum : " << sum << std::endl;
		std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl << std::endl;
	}
	return 0;
}


