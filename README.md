#Single-producer single-consumer queue implementation in C++

This is a simple header-only single-producer single-consumer queue
implementation written in C++11.  It is not designed to be very performant or
scalable, you have been warned!  If you are interested in performant thread-safe
queues take a look at:

 * [Intel TBB](https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_queue_cls.htm)
 * [Folly ProducerConsumerQueue](https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_queue_cls.htm)
 * [Boost SPSC Queue](http://www.boost.org/doc/libs/1_58_0/doc/html/boost/lockfree/spsc_queue.html)

This queue would be more scalable if the buffer was made fixed size.

##Ownership transfer

One of the key concepts I wanted to explore in this implementation was the idea
of 'ownership transfer'.  In other words, I wanted to make it clear in the API
that the thread writing messages to the queue relinquished responsibility for
destroying the messages to the thread reading messages from the queue.  This is
done by using `unique_ptr`s to pass messages.  If you are not familiar with
rvalues and `std::move()` then I suggest reading up on them before using this
code.

##Example

```
spsc_queue<int> q;

// Send [0, 9] over the queue
thread producer([&q]() {
	spsc_writer<int> w(q);
	for (int i = 0; i < num_msgs; i++) {
		w.push(unique_ptr<int>(new int(i)));
	}
});

// Print integers received from the queue.  The output will be:
// 0
// 1
// ...
// 8
// 9
thread consumer([&q, &msgs_rcvd]() {
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
```
