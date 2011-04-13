/*
 * $Id: LineReader.cpp,v 1.1.1.1 2009/03/10 19:46:34 scott Exp $
 *
 */


#include "LineReader.h"

/*
  =============================================================================
  =============================================================================
*/
LineReader::LineReader(HANDLE fh)
{
	_fh = fh;
	_read_buff = NULL;
	_read_buff_size = 0;
	_start_pos = _end_pos = 0;
}

/*
  =============================================================================
  =============================================================================
*/
LineReader::~LineReader()
{
	_fh = 0;

	if (_read_buff) {
		delete [] _read_buff;
	}

	_read_buff_size = 0;
	_start_pos = _end_pos = 0;
}

/*
  =============================================================================
  _start_pos points to the beginning of unread data

  _end_pos points one byte past the end of unread data or the location to start 
     copying the next ReadFile

  _read_buff[_start_pos] to _read_buff[_end_pos - 1] is the data available
     to copy to users buffer

  _read_buff[_end_pos] to _read_buff[_read_buff_size] is guaranteed all zeros

  _read_buff[0] to _read_buff[_start_pos - 1] has already been seen by client
     and we don't copy it again
  =============================================================================
*/
#define READ_BUFF_SIZE (65536 - 64)
bool LineReader::getNextLine(char *buff, unsigned long *max_len)
{
	char *p;
	unsigned long user_buff_size;
	unsigned long num_bytes;

	if (!max_len) {
		return false;
	}

	user_buff_size = *max_len;
	*max_len = 0;

	if (!_fh) {
		return false;
	}

	if (!_read_buff) {
		_read_buff = new char[READ_BUFF_SIZE + 64];

		if (!_read_buff) {
			return false;
		}

		_read_buff_size = READ_BUFF_SIZE;
		_start_pos = _end_pos = 0;
		memset(_read_buff, 0, READ_BUFF_SIZE + 64);
	}

	p = strchr(_read_buff + _start_pos, '\n');

	while (!p) {
		// need to do another read
		if (_start_pos > 0) {
			// move uncopied data to the start
			memmove(_read_buff, _read_buff + _start_pos, _end_pos - _start_pos);
			_end_pos = _end_pos - _start_pos;
			_start_pos = 0;
			// zero the remaining _read_buff
			memset(_read_buff + _end_pos, 0, _read_buff_size - _end_pos);
		}
		else if (_end_pos != 0) {
			// need a bigger read buffer
			// TODO: back to this in a minute
			return false;
		}

		// now read as much as we can			
		if (!ReadFile(_fh, _read_buff + _end_pos, _read_buff_size - _end_pos, &num_bytes, NULL)) {
			// no more data and no end line found
			return false;
		}

		if (num_bytes == 0) {
			// no more data and no end line found
			return false;
		}

		_end_pos += num_bytes;

		p = strchr(_read_buff + _start_pos, '\n');
	}
	
	num_bytes = 1 + (p - &_read_buff[_start_pos]);
	*max_len = num_bytes;

	if (num_bytes > user_buff_size) {
		return false;
	}

	if (!buff) {
		return false;
	}

	memcpy(buff, _read_buff + _start_pos, num_bytes);
	buff[num_bytes] = 0;

	_start_pos += num_bytes;

	return true;
}
