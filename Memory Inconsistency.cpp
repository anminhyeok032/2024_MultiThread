#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "Bakery.h"

volatile int sum;
//std::atomic <int> sum;
std::mutex mylock;
int num_threads;


const auto SIZE = 50000000;
//volatile int x, y;
std::atomic <int> x, y;

int trace_x[SIZE], trace_y[SIZE];



volatile bool done = false;
volatile int* bound;
//std::atomic_int* bound;
int num_error;



void ThreadFunc0()
{
	for (int i = 0; i < SIZE; i++) {
		x = i;
		//std::atomic_thread_fence(std::memory_order_seq_cst);
		trace_y[i] = y;
	}
}
void ThreadFunc1()
{
	for (int i = 0; i < SIZE; i++) {
		y = i;
		//std::atomic_thread_fence(std::memory_order_seq_cst);
		trace_x[i] = x;
	}
}


void ThreadFunc11()
{
	for (int j = 0; j <= 25000000; ++j) *bound = -(1 + *bound);
	done = true;
}
void ThreadFunc12()
{
	while (!done) {
		int v = *bound;
		if ((v != 0) && (v != -1)) {
			printf("%X", v);
			num_error++;
		}
	}
}

int main()
{
	//실습 16
	//bound = new int{ 0 };

	// 실습 17
	int ARR[32];
	long long temp = (long long)&ARR[31];
	temp = temp - (temp % 64);
	//temp -= 2;
	temp -= 1;		// 잘리는 위치가 바뀜
	bound = (int*)temp;
	*bound = 0;


	std::thread t11{ThreadFunc0};
	std::thread t12{ThreadFunc1};
	t11.join(); t12.join();

	// 실습 15
	/*int count = 0;
	for (int i = 0; i < SIZE; ++i)
		if (trace_x[i] == trace_x[i + 1])
			if (trace_y[trace_x[i]] == trace_y[trace_x[i] + 1]) {
				if (trace_y[trace_x[i]] != i) continue;
				count++;
			}
	std::cout << "Total Memory Inconsistency: " << count << std::endl;*/

	//// 실습 16 cpu를 믿자 error는 0번
	//std::cout << "Total Memory Inconsistency: " << num_error << std::endl;
	//delete bound;



	std::cout << "Total Memory Inconsistency: " << num_error << std::endl;

	return 0;
}


