// 낙천적 동기화
// 세밀한 동기화는 잠금의 획득과 해제가 너무 잦음
// ㄴ 이동시 잠금을 하지 않음
// 잠금을 획득하지 않고 검색한 후, 해당 노드를 잠구고, validate 함수로 잠긴 노드가 정확한지 확인한다.
// 확인했을때 이상한 경우 잠금 해제 후 처음부터 다시 확인한다.
// 잠금을 획득하지 않고 이동하기 때문에 삭제된 노드로 이동할 수 있다.
// ㄴ 해당 문제를 위해 제거된 노드를 통해 이동하게 함 - delete를 일단 하지 않음
//		ㄴ Memory Leak => 나중에 해결

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <mutex>
#include <set>

using namespace std;
using namespace chrono;

constexpr int MAX_THREADS = 16;


class NODE {
	std::mutex  nl;
public:
	int v;
	NODE* next;
	bool removed;
	NODE() : v(-1), next(nullptr), removed(false) {}
	NODE(int x) : v(x), next(nullptr), removed(false) {}
	void Lock()
	{
		nl.lock();
	}
	void Unlock()
	{
		nl.unlock();
	}
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

class F_SET {
	NODE head, tail;
public:
	F_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
	}
	bool ADD(int x)
	{
		NODE* prev = &head;
		prev->Lock();
		NODE* curr = prev->next;
		curr->Lock();
		while (curr->v < x) {
			prev->Unlock();
			prev = curr;
			curr = curr->next;
			curr->Lock();
		}
		if (curr->v != x) {
			NODE* node = new NODE{ x };
			node->next = curr;
			prev->next = node;
			prev->Unlock(); curr->Unlock();
			return true;
		}
		else
		{
			prev->Unlock(); curr->Unlock();
			return false;
		}
	}

	bool REMOVE(int x)
	{
		NODE* prev = &head;
		prev->Lock();
		NODE* curr = prev->next;
		curr->Lock();
		while (curr->v < x) {
			prev->Unlock();
			prev = curr;
			curr = curr->next;
			curr->Lock();
		}
		if (curr->v != x) {
			prev->Unlock(); curr->Unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			prev->Unlock(); curr->Unlock();
			delete curr;
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		NODE* prev = &head;
		prev->Lock();
		NODE* curr = prev->next;
		curr->Lock();
		while (curr->v < x) {
			prev->Unlock();
			prev = curr;
			curr = curr->next;
			curr->Lock();
		}
		bool res = (curr->v == x);
		prev->Unlock(); curr->Unlock();
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

// 낙천적 동기화
class Z_SET {
	NODE head, tail;
public:
	Z_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
	}

	bool validate(const NODE* prev, const NODE* curr)
	{
		return (false == prev->removed) && (false == curr->removed) && (prev->next == curr);
	}

	bool ADD(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->Lock(); curr->Lock();
			if (true == validate(prev, curr)) {
				if (curr->v != x) {
					NODE* node = new NODE{ x };
					node->next = curr;
					prev->next = node;
					prev->Unlock(); curr->Unlock();
					return true;
				}
				else
				{
					prev->Unlock(); curr->Unlock();
					return false;
				}
			}
			else {
				prev->Unlock(); curr->Unlock();
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true)
		{
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->Lock(); curr->Lock();
			if (true == validate(prev, curr))
			{
				if (curr->v != x) {
					prev->Unlock(); curr->Unlock();
					return false;
				}
				else {
					curr->removed = true;
					prev->next = curr->next;
					prev->Unlock(); curr->Unlock();
					//delete curr;
					return true;
				}
			}
			else
			{
				prev->Unlock(); curr->Unlock();
			}
		}
	}

	bool CONTAINS(int x)
	{
		while (true)
		{
			NODE* curr = head.next;
			while (curr->v < x) {
				curr = curr->next;
			}
			return (curr->v == x) && (false == curr->removed);
		}
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

class O_SET {
	NODE head, tail;
public:
	O_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
	}

	bool validate(const NODE* prev, const NODE* curr)
	{
		NODE* ptr = &head;
		while (ptr->v <= prev->v) {
			if (ptr == prev)
				return ptr->next == curr;
			ptr = ptr->next;
		}
		return false;
	}

	bool ADD(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->Lock(); curr->Lock();
			if (true == validate(prev, curr)) {
				if (curr->v != x) {
					NODE* node = new NODE{ x };
					node->next = curr;
					prev->next = node;
					prev->Unlock(); curr->Unlock();
					return true;
				}
				else
				{
					prev->Unlock(); curr->Unlock();
					return false;
				}
			}
			else {
				prev->Unlock(); curr->Unlock();
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true)
		{
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->Lock(); curr->Lock();
			if (true == validate(prev, curr))
			{
				if (curr->v != x) {
					prev->Unlock(); curr->Unlock();
					return false;
				}
				else {
					prev->next = curr->next;
					prev->Unlock(); curr->Unlock();
					//delete curr;
					return true;
				}
			}
			else
			{
				prev->Unlock(); curr->Unlock();
			}
		}
	}

	bool CONTAINS(int x)
	{
		while (true)
		{
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->Lock(); curr->Lock();
			if (true == validate(prev, curr))
			{
				bool res = (curr->v == x);
				prev->Unlock(); curr->Unlock();
				return res;
			}
			else
			{
				prev->Unlock(); curr->Unlock();
			}
		}
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

//SET my_set;   // 성긴 동기화
//F_SET my_set;	// 세밀한 동기화
//O_SET my_set;	// 낙천적 동기화
Z_SET my_set;	// 게으른 동기화
//STD_SET my_set;

constexpr int RANGE = 1000;
constexpr int LOOP = 4000000;

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

void worker_check(vector<HISTORY>* history, int num_threads)
{
	for (int i = 0; i < LOOP / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % RANGE;
			history->emplace_back(0, v, my_set.ADD(v));
			break;
		}
		case 1: {
			int v = rand() % RANGE;
			history->emplace_back(1, v, my_set.REMOVE(v));
			break;
		}
		case 2: {
			int v = rand() % RANGE;
			history->emplace_back(2, v, my_set.CONTAINS(v));
			break;
		}
		}
	}
}

void check_history(array <vector <HISTORY>, MAX_THREADS>& history, int num_threads)
{
	array <int, RANGE> survive = {};
	cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (my_set.CONTAINS(i)) {
				cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == my_set.CONTAINS(i)) {
				cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	cout << " OK\n";
}

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
	cout << "Checking Error...\n";
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		vector <thread> threads;
		array<vector<HISTORY>, MAX_THREADS> history;
		my_set.clear();

		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker_check, &history[i], num_threads);
		for (auto& th : threads)
			th.join();
		auto end_t = high_resolution_clock::now();

		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		my_set.print20();
		cout << num_threads << " Threads.  Exec Time : " << exec_ms << endl;
		check_history(history, num_threads);
	}

	cout << "\n\nBenchmarking...\n";
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