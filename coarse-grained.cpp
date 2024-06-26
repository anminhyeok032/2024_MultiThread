#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <set>

using namespace std;
using namespace chrono;

constexpr int MAX_THREADS = 16;


class NODE {
public:
	int v;
	NODE* next;
	NODE() : v(-1), next(nullptr) {}
	NODE(int x) : v(x), next(nullptr) {}
};

class null_mutex
{
public:
	void lock() {}
	void unlock() {}
};


class SET {
	NODE head, tail;
	mutex ll;
public:
	SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
	}
	bool ADD(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		if (curr->v != x) {
			NODE* node = new NODE{ x };
			node->next = curr;
			prev->next = node;
			ll.unlock();
			return true;
		}
		else
		{
			ll.unlock();
			return false;
		}
	}

	bool REMOVE(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		if (curr->v != x) {
			ll.unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			delete curr;
			ll.unlock();
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		bool res = (curr->v == x);
		ll.unlock();
		return res;
	}
	void print20()
	{
		NODE* p = head.next;
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		NODE* p = head.next;
		while (p != &tail) {
			NODE* t = p;
			p = p->next;
			delete t;
		}
		head.next = &tail;
	}
};

class STD_SET {
	std::set <int> std_set;
public:
	STD_SET()
	{
	}
	bool ADD(int x)
	{
		if (std_set.count(x) == 1)
			return false;
		std_set.insert(x);
		return true;
	}

	bool REMOVE(int x)
	{
		if (std_set.count(x) == 0)
			return false;
		std_set.erase(x);
		return true;
	}

	bool CONTAINS(int x)
	{
		return std_set.count(x) == 1;
	}
	void print20()
	{
		int count = 20;
		for (auto x : std_set) {
			cout << x << ", ";
			if (--count == 0) break;
		}
		cout << endl;
	}

	void clear()
	{
		std_set.clear();
	}
};

SET my_set;   // 성긴 동기화
//STD_SET my_set;

constexpr int RANGE = 1000;
constexpr int LOOP = 4000000;


void worker(int num_threads)
{
	for (int i = 0; i < LOOP / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % RANGE;
			my_set.ADD(v);
			break;
		}
		case 1: {
			int v = rand() % RANGE;
			my_set.REMOVE(v);
			break;
		}
		case 2: {
			int v = rand() % RANGE;
			my_set.CONTAINS(v);
			break;
		}
		}
	}
}

int main()
{
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		vector <thread> threads;
		my_set.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker, num_threads);
		for (auto& th : threads)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();
		my_set.print20();
		cout << num_threads << " Threads.  Exec Time : " << exec_ms << endl;
	}
}