#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "Bakery.h"

#include <stdbool.h>

volatile int sum;
std::mutex mylock;
int num_threads;


int x = 0;
bool CAS(int* addr, int expect, int update)
{
	return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_int*> (addr), &expect, update);
}

void CAS_LOCK() {
	while (false == CAS(&x, 0, 1));
}

void CAS_UNLOCK()
{
	CAS(&x, 1, 0);
}

void thread_func(int id)
{

	for (auto i = 0; i < 5000'0000 / num_threads; ++i)
	{
		//mylock.lock();
		CAS_LOCK();
		sum = sum + 2;
		CAS_UNLOCK();
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


