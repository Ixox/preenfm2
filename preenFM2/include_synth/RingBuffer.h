/*
 * RingBuffer.h
 *
 *  Created on: Feb 13, 2011
 *      Author: xhosxe
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_


template <typename T, int size>
class RingBuffer {
public:
	RingBuffer() {
	    clear();
	}

	~RingBuffer() {
	};

	void clear() {
        this->head = 0;
        this->tail = 0;
	}

	void insert(T element) {
	    this->buf[this->tail] = element;
	    this->tail = (this->tail == size-1) ? 0 : this->tail + 1;
	}

	T remove() {
	    T element = this->buf[this->head];
	    this->head = (this->head == size-1) ? 0 : this->head + 1;
	    return element;
	}

	bool isFull() {
	   return (this->tail + 1 == this->head) ||
	        (this->tail == (size-1) && this->head == 0);
	}

	int getCount() {
	    int count = this->tail - this->head;
	    if (this->tail < this->head) {
	    	count += size;
	    }
	    return count;
	}

	void appendBlock(T* block, int number) {
		for (int k=0; k<number ; k++) {
			insert(block[k]);
		}
	}

private:
    volatile int head;
    volatile int tail;
    T buf[size+1];

};

#endif /* RINGBUFFER_H_ */
