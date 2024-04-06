#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>	
#include <set>
#include <vector>



const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;
int num_threads;



class NODE {
public:
	int key;
	NODE* next;
	int v;
	//NODE() : v(-1), next(nullptr) {}
	NODE() { next = NULL; }
	NODE(int key_value) {
		next = NULL;
		key = key_value;
	}
	~NODE() {}
};



class SET {
	NODE head, tail;
	std::mutex glock;
public:
	SET()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~SET() {}
	void Init()
	{
		NODE* ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE* pred = &head;
		glock.lock();
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			glock.unlock();
			return false;
		}
		else {
			NODE* node = new NODE(key);
			node->next = curr;
			pred->next = node;
			glock.unlock();
			return true;
		}

	}
	bool Remove(int key)
	{
		NODE* pred, * curr;
		pred = &head;
		glock.lock();
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}

	}
	bool Contains(int key)
	{
		NODE* pred, * curr;
		pred = &head;
		glock.lock();
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}
	}
};

SET my_set;

void worker(int num_threads)
{
	int key;
	for (int i = 0; i < NUM_TEST / (num_threads); i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			my_set.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			my_set.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			my_set.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}




int main()
{
	std::vector<std::thread*> threads;
	for (num_threads = 1; num_threads <= 16; num_threads *= 2)
	{
		threads.clear();


		auto t = std::chrono::high_resolution_clock::now();
		for (auto j = 0; j < num_threads; ++j)	threads.emplace_back(new std::thread(worker, num_threads));

		for (auto th : threads) {
			th->join();
			delete th;
		}
		threads.clear();
		auto d = std::chrono::high_resolution_clock::now() - t;


		std::cout << "Thread num : " << num_threads << std::endl;
		std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl << std::endl;
	}
	return 0;
}