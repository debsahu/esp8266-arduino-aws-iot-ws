/*
This file is part of aws-mqtt-websockets.

aws-mqtt-websockets is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

aws-mqtt-websockets is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with aws-mqtt-websockets.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef __CIRCULARBYTEBUFFER_H_
#define __CIRCULARBYTEBUFFER_H_

//#define DEBUG_CBB(...) os_printf( __VA_ARGS__ )

#ifndef DEBUG_CBB
#define DEBUG_CBB(...)
#define NODEBUG_CBB
#endif

class CircularByteBuffer {
public:

	CircularByteBuffer () {
		data = NULL;
		size = 0;
		capacity = 0;
	}
	~CircularByteBuffer(){
		if (data!=NULL)
			free (data);
	}

	void clear ()
	{
		size = 0;
		begin = 0;
		end = 0;
	}

	void deallocate () {
		if (data!=NULL) {
			free (data);
			data = NULL;
		}
	}

	void init (long capacity) {
		if (data!=NULL)
			free (data);
		data = (byte*) malloc (capacity);
		size = 0;
		begin = 0;
		end = 0;
		this->capacity = capacity;
	}

	long getSize () {
		return size;
	}

	byte peek () {
		return data[begin];
	}
	void push (byte b) {
		if (size+1 == capacity) {
			DEBUG_CBB ("buffer full");
			return;
		}
		data[end] = b;
		end = (end+1) % capacity;
		size += 1;

	}

	byte pop () {
		if (size == 0){
			DEBUG_CBB ("buffer empty");
			return 0;
		}
		byte ret = data[begin];
		begin = (begin+1) % capacity;
		size -= 1;
		return ret;

	}

	void push (byte* b, long len){
		if (size+len >= capacity) {
			DEBUG_CBB ("buffer full");
			return;
		}
		if ( (end + len)  <= capacity ) {
			memcpy ((void*)&data[end],b,len);
			end += len;
		} else {
			long endSide = capacity - end;
			memcpy ((void*)&data[end],b,endSide);
			end = 0;
			memcpy ((void*)&data[end],b+endSide,len-endSide);
			end += len-endSide;
		}
		size += len;

	}

	byte* pop (byte* b, long len){
		if ( (size - len) < 0 ) {
			DEBUG_CBB ("buffer empty");
			return NULL;
		}
		if ( (begin + len)  <= capacity ) {
			memcpy (b,(void*)&data[begin],len);
			begin += len;
		} else {
			long endSide = capacity - begin;
			memcpy (b,(void*)&data[begin],endSide);
			begin = 0;
			memcpy (b+endSide,(void*)&data[begin],len-endSide);
			begin += len-endSide;
		}
		size -= len;
	}

private:
	byte* data;
	long capacity;
	long size;
	long begin;
	long end;
};



#endif
