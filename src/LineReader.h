/*
 * $Id: LineReader.h,v 1.2 2009/03/12 18:26:06 scott Exp $
 *
 */

#pragma once
#ifndef LINE_READER_H
#define LINE_READER_H

#include "version.h"

class LineReader
{
public:
	LineReader(HANDLE fh);
	~LineReader();

	bool getNextLine(char *buff, unsigned long *max_len);

private:
	HANDLE _fh;
	char *_read_buff;
	unsigned long _read_buff_size;
	unsigned long _start_pos;
	unsigned long _end_pos;
};


#endif // ifndef LINE_READER_H


 