#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <set>

using namespace std;
using namespace chrono;

constexpr int MAX_THREADS = 16;

// 각 NODE별 Lock 구현을 위해 mutex 추가
class NODE {
public:
	int v;
	NODE* next;
	mutex lock;
	NODE() : v(-1), next(nullptr) {}
	NODE(int x) : v(x), next(nullptr) {}
	
};

//=============
// 세밀한 동기화
//=============
class FINE_SET {
	NODE head, tail;

public:
	FINE_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
	}
	
	// 추가시 true 리턴
	// 각 노드를 지날때마다 락을 따로 걸어줌
	// return시 현재 지나고 있는 NODE의 락을 풀고 리턴함
	bool ADD(int x)
	{
		head.lock.lock();
		NODE* prev = &head;
		NODE* curr = prev->next;
		curr->lock.lock();
		while (curr->v < x) {
			prev->lock.unlock();
			prev = curr;
			curr = curr->next;
			curr->lock.lock();
		}
		if (curr->v != x)
		{
			NODE* node = new NODE{ x };
			node->next = curr;
			prev->next = node;
			curr->lock.unlock();
			prev->lock.unlock();
			return true;
		}
		else 
		{
			curr->lock.unlock();
			prev->lock.unlock();
			return false;
		}
	}

	// 삭제시 true 리턴
	// 각 노드를 지날때마다 락을 따로 걸어줌
	// return시 현재 지나고 있는 NODE의 락을 풀고 리턴함
	bool REMOVE(int x)
	{
		NODE* prev = &head;
		head.lock.lock();
		NODE* curr = prev->next;
		curr->lock.lock();
		while (curr->v < x)
		{
			prev->lock.unlock();
			prev = curr;
			curr = curr->next;
			curr->lock.lock();
		}
		if (curr->v != x) {
			curr->lock.unlock();
			prev->lock.unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			
			curr->lock.unlock();
			prev->lock.unlock();

			delete curr;
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		NODE* prev = &head;
		head.lock.lock();
		NODE* curr = prev->next;
		curr->lock.lock();
		while (curr->v < x)
		{
			prev->lock.unlock();

			prev = curr;
			curr = curr->next;

			curr->lock.lock();
		}
		bool res = (curr->v == x);
		curr->lock.unlock();
		prev->lock.unlock();
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


//SET my_set;		// 성긴 동기화
FINE_SET my_set;	// 세밀한 동기화
//STD_SET my_set;	// 싱글 스레드 no lock

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
		default: {
			cout << "Error\n"; 
			exit(-1);
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