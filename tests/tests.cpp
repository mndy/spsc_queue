#include <spsc_queue.h>

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

TEST(SpscQueueTest, TwoReaderError) {
	bool exception_thrown = false;
	try {
		spsc_queue<int> q;
		spsc_reader<int> r1(q);
		spsc_reader<int> r2(q);
	} catch (const spsc_reader_exists& e) {
		exception_thrown = true;	
	}

	ASSERT_TRUE(exception_thrown);
}

TEST(SpscQueueTest, TwoWriterError) {
	bool exception_thrown = false;
	try {
		spsc_queue<int> q;
		spsc_writer<int> w1(q);
		spsc_writer<int> w2(q);
	} catch (const spsc_writer_exists& e) {
		exception_thrown = true;	
	}

	ASSERT_TRUE(exception_thrown);
}

TEST(SpscQueueTest, EarlyDestruction) {
	class destruct_count {
		atomic<int>& count_;
	public:
		destruct_count(atomic<int>& count): count_(count) {}
		~destruct_count() { count_++; }
	};

	atomic<int> destructs_called(0);
	{
		spsc_queue<destruct_count> q;
		spsc_writer<destruct_count> w(q);

		for (int i = 0; i < 100; i++) {
			w.push(unique_ptr<destruct_count>(
				new destruct_count(destructs_called))
			);
		}
	}
	ASSERT_EQ(destructs_called, 100);
}

TEST(SpscQueueTest, Loop) {
	const int num_msgs(128);
	spsc_queue<int> q1, q2;

	thread write_first([&q1, &q2, &num_msgs]() {
		unique_ptr<int> ptr(new int(0));
		spsc_reader<int> r(q2);
		{
			spsc_writer<int> w(q1);
			for (int i = 0; i < num_msgs; ++i) {
				ASSERT_TRUE(w.push(move(ptr)));
				ASSERT_TRUE(r.pop(ptr));
				++(*ptr);
			}
		}
		ASSERT_TRUE(ptr != nullptr);
		ASSERT_EQ(num_msgs * 2, *ptr);
		ASSERT_FALSE(r.pop(ptr));
	});

	thread read_first([&q1, &q2, &num_msgs]() {
		unique_ptr<int> ptr;
		spsc_reader<int> r(q1);
		{
			spsc_writer<int> w(q2);
			for (int i = 0; i < num_msgs; ++i) {
				ASSERT_TRUE(r.pop(ptr));
				++(*ptr);
				ASSERT_TRUE(w.push(move(ptr)));
			}
		}
		ASSERT_FALSE(r.pop(ptr));
	});

	write_first.join();
	read_first.join();
}

