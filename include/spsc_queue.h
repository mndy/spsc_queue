// A single-producer single-consumer thread-safe queue implementation written
// in C++11.
//
// GitHub:  https://github.com/mndy/spsc_queue
// Website: http://munday.io
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Michael Munday
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once
#ifndef _MNDY_SPSC_QUEUE_H_
#define _MNDY_SPSC_QUEUE_H_

#include <array>
#include <memory>
#include <mutex>
#include <utility>
#include <queue>

namespace mndy {

// Exception thrown if a reader has already been created for a queue.
//
struct spsc_reader_exists: public std::runtime_error {
	spsc_reader_exists():
		std::runtime_error("cannot create reader for queue: one already exists")
	{}
};

// Exception thrown if a writer has already been created for a queue.
//
struct spsc_writer_exists: public std::runtime_error {
	spsc_writer_exists():
		std::runtime_error("cannot create writer for queue: one already exists")
	{}
};

// A single-producer single-consumer thread safe queue.
//
// The queue cannot be used directly - it must be used with the spsc_reader
// and spsc_writer classes.  Only one instance of each respective class may be
// created for each spsc_queue instance.
template<typename T>
class spsc_queue;


// 
//
template<typename T>
class spsc_reader {
	spsc_queue<T>& q_;
	std::unique_ptr<T> value_;
public:
	spsc_reader(spsc_queue<T>&);
	virtual ~spsc_reader() {};
	std::unique_ptr<T> value();
	bool pop(std::unique_ptr<T>& ptr);
};

//
//
template<typename T>
class spsc_writer {
	spsc_queue<T>& q_;
public:
	spsc_writer(spsc_queue<T>&);
	virtual ~spsc_writer() { close(); }
	bool push(std::unique_ptr<T>);
	void close();
};

template<typename T>
class spsc_queue {
	friend class spsc_reader<T>;
	friend class spsc_writer<T>;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::queue<std::unique_ptr<T>> unsafe_q_;
	bool closed_, r_exists_, w_exists_;
public:
	spsc_queue(): closed_(false), r_exists_(false), w_exists_(false) {}
};

// Construct a spsc_reader for the given spsc_queue.
//
// Only one spsc_reader may be created per spsc_queue. If a spsc_reader already
// exists then this constructor will throw a spsc_reader_exists exception. 
template<typename T>
spsc_reader<T>::spsc_reader(spsc_queue<T>& q): q_(q) {
	std::unique_lock<std::mutex> lock(q_.mutex_);
	if (q_.r_exists_) {
		throw spsc_reader_exists();
	}
	q_.r_exists_ = true;
}

// Construct a spsc_writer for the given spsc_queue.
//
// Only one spsc_writer may be created per spsc_queue. If a spsc_writer already
// exists then this constructor will throw a spsc_writer_exists exception. 
template<typename T>
spsc_writer<T>::spsc_writer(spsc_queue<T>& q): q_(q) {
	std::unique_lock<std::mutex> lock(q_.mutex_);
	if (q_.w_exists_) {
		throw spsc_writer_exists();
	}
	q_.w_exists_ = true;
}

// Remove a token from the front of the queue.
//
// Will block until a token is available, or the queue is closed by the writer.
// When the queue is open a token will be moved into ptr and true will be
// returned.  When the queue is closed nullptr will be moved into ptr and false
// will be returned. Once the queue is closed it cannot be reopened.
template<typename T>
bool spsc_reader<T>::pop(std::unique_ptr<T>& ptr) {
	std::unique_lock<std::mutex> lock(q_.mutex_);
	// Only 2 iterations are possible - one of the conditions will be true after
	// waiting.
	do {
		if (q_.unsafe_q_.size() >= 1) {
			ptr = std::move(q_.unsafe_q_.front());
			q_.unsafe_q_.pop();
			return true;
		} else if (q_.closed_) {
			ptr = nullptr;
			return false;
		}
		q_.cv_.wait(lock);
	} while(true);
}

// Add a token to the back of the queue.
//
// Returns true if the token was successfully added to the queue and false if
// the token could not be added to the queue.  This call will only return false
// when the queue is closed.
template<typename T>
bool spsc_writer<T>::push(std::unique_ptr<T> v) {
	{
		std::unique_lock<std::mutex> lock(q_.mutex_);
		if (q_.closed_) {
			return false;
		}
		q_.unsafe_q_.push(std::move(v));
	}
	q_.cv_.notify_one();
	return true;
}

// Close the queue.
//
// Once the queue is closed no more tokens may be added to the queue.  close()
// is called autmatically by the destructor.
template<typename T>
void spsc_writer<T>::close() {
	{
		std::unique_lock<std::mutex> lock(q_.mutex_);
		q_.closed_ = true;
	}
	q_.cv_.notify_one();
}

}

#endif // _MNDY_SPSC_QUEUE_H_
