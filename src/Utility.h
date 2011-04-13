/*
 * $Id: Utility.h,v 1.1 2009/03/12 18:26:18 scott Exp $
 *
 */

#pragma once
#ifndef UTILITY_H
#define UTILITY_H

#include "version.h"

bool file_exists(const char *filename);
unsigned long get_file_size(const char *filename);
bool directory_exists(const char *dirname);
bool is_directory_read_only(const char *dir);
long count_lines_in_file(HANDLE fh);


#endif // ifndef UTILITY_H
