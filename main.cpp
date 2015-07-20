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
			spsc_msg<int> msg = r.pop();
			if (msg.is_valid()) {
				cout << *msg.move_ptr() << endl;
			} else if (msg.is_closed()) {
				return;
			} else {
				continue;
			}
		}
	});

	producer.join();
	consumer.join();

	return 0;
}
