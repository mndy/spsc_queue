#include "spsc_queue.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>

using namespace std;
using namespace mndy;

int main(int argc, char** argv) {
	spsc_queue<int> q;

	thread producer([&q]() {
		spsc_writer<int> w(q);
		for (int i = 0; i < 8; i++) {
			w.push(unique_ptr<int>(new int(i)));
		}
	});

	thread consumer([&q]() {
		spsc_reader<int> r(q);
		while(true) {
			unique_ptr<int> ptr;
			bool valid = r.pop(ptr);
			if (valid) {
				cout << *ptr << endl;
			} else {
				return;
			}
		}
	});

	producer.join();
	consumer.join();

	return 0;
}
