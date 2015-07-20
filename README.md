# Thread-safe queue implementation in C++

This is a simple header-only single-producer single-consumer queue
implementation written in C++11.  It is designed to explicitly transfer
ownership of dynamically allocated objects from one thread to another.  It is
not designed to be very performant or scalable, you have been warned!  If you
are interested in performant thread-safe queues take a look at:

 * [Intel TBB](https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_queue_cls.htm)
 * [Folly ProducerConsumerQueue](https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_queue_cls.htm)
 * [Boost SPSC Queue](http://www.boost.org/doc/libs/1_58_0/doc/html/boost/lockfree/spsc_queue.html)

## Ownership transfer

The key concept I wanted to explore in this implementation was the idea
of 'ownership transfer'.  In other words, I wanted to make it clear in the API
that the thread writing messages to the queue relinquished responsibility for
destroying the messages to the thread reading messages from the queue.  This is
done by using `unique_ptr`s to pass messages.  If you are not familiar with
rvalues and `std::move()` then I suggest reading up on them before using this
code.

Herb Sutter has written a nice blog post on the subject of using `unique_ptr`s 
as parameters [here](http://herbsutter.com/2013/06/05/gotw-91-solution-smart-pointer-parameters/).

## Example

```cpp
// Construct a queue - note that we can't perform operations on it directly - we
// need either a spsc_reader or spsc_writer object
spsc_queue<int> q;

// Send [0, 9] over the queue
thread producer([&q]() {
	// Create a spsc_writer so we can write messages into the spsc_queue
	spsc_writer<int> w(q);
	for (int i = 0; i < 10; i++) {
		// Create a unique_ptr<int> rvalue and move it into the queue
		w.push(unique_ptr<int>(new int(i)));
	}
	// spsc_writer closes the queue as it goes out of scope
});

// Print integers received from the queue.  The output will be:
// 0
// 1
// ...
// 8
// 9
thread consumer([&q]() {
	// Create a spsc_reader so we can read messages from the spsc_queue
	spsc_reader<int> r(q);
	while(true) {
		unique_ptr<int> ptr;
		
		// Pointer is moved from the queue into ptr
		bool valid = r.pop(ptr);
		if (valid) {
			// Dereference pointer to print - should check first for nullptr if
			// the producer might send one
			cout << *ptr << endl;
		} else {
			// Queue has been closed by the writer - exit thread
			return;
		}
	} // ptr is destroyed when it goes out of scope
});

producer.join();
consumer.join();
```
