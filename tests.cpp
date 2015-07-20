#include "spsc_queue.h"

#include "gtest/gtest.h"

#include <atomic>
#include <memory>
#include <thread>

using namespace std;
using namespace mndy;

TEST(SpscQueueTest, SimpleOneThread) {
	spsc_queue<int> q;
	spsc_reader<int> r(q);
	spsc_writer<int> w(q);

	for (int i = 0; i < 8; i++) {
		w.push(unique_ptr<int>(new int(i)));
		unique_ptr<int> ptr;
		bool valid = r.pop(ptr);
		ASSERT_TRUE(valid);
		if (valid) {
			ASSERT_EQ(*ptr, i);
		}
	}

	w.close();
	unique_ptr<int> ptr;
	ASSERT_FALSE(r.pop(ptr));
	ASSERT_EQ(ptr, nullptr);
}

TEST(SpscQueueTest, SimpleTwoThread) {
	spsc_queue<int> q;

	const int num_msgs = 8;
	atomic<int> msgs_rcvd(0);

	thread producer([&q, &num_msgs]() {
		spsc_writer<int> w(q);
		for (int i = 0; i < num_msgs; i++) {
			w.push(unique_ptr<int>(new int(i)));
		}
	});

	thread consumer([&q, &msgs_rcvd]() {
		spsc_reader<int> r(q);
		while(true) {
			unique_ptr<int> ptr;
			bool valid = r.pop(ptr);
			if (valid) {
				ASSERT_EQ(msgs_rcvd++, *ptr);
			} else {
				return;
			}
		}
	});

	producer.join();
	consumer.join();

	ASSERT_EQ(msgs_rcvd.load(), num_msgs);
}
