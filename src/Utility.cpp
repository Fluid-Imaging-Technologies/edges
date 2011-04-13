/*
 * $Id: Utility.cpp,v 1.1 2009/03/12 18:26:18 scott Exp $
 *
 */

#include "Utility.h"

/*
  =======================================================================================
  =======================================================================================
*/
bool file_exists(const char *filename)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (filename && *filename) {
		if (GetFileAttributesEx(filename, GetFileExInfoStandard, &fileInfo)) {
			return true;
		}
	}

	return false;
}

/*
  =======================================================================================
  Only use for files less then 4GB
  =======================================================================================
*/
unsigned long get_file_size(const char *filename)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (filename && *filename) {
		if (GetFileAttributesEx(filename, GetFileExInfoStandard, &fileInfo)) {
			return fileInfo.nFileSizeLow;
		}
	}

	return 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool directory_exists(const char *dirname)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (dirname && *dirname) {
		if (!GetFileAttributesEx(dirname, GetFileExInfoStandard, &fileInfo)) {
			return false;
		}

		if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}
	}

	return false;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool is_directory_read_only(const char *dir)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	// if we can't access it, it is read only
	if (!GetFileAttributesEx(dir, GetFileExInfoStandard, &fileInfo)) {
		return true;
	}

	if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
		return true;
	}

	return false;
}

/*
  =======================================================================================
  =======================================================================================
*/
long count_lines_in_file(HANDLE fh)
{
	unsigned long oldPos, newPos, bytes_read, file_size, total_bytes_read_so_far, next_read;
	long num_lines;
	char *buff;

	if (!fh) {
		return -1;
	}

	file_size = GetFileSize(fh, 0);

	if (INVALID_FILE_SIZE == file_size) {
		return -1;
	}
	
	if (file_size == 0) {
		return 0;
	}

	buff = new char[1032];

	if (!buff) {
		return -1;
	}

	memset(buff, 0, 1032);

	// save the original position before starting
	oldPos = SetFilePointer(fh, 0, 0, FILE_CURRENT);

	if (INVALID_SET_FILE_POINTER == oldPos) {
		delete [] buff;
		return -1;
	}

	// now count the number of line feeds '\n' in the file
	if (INVALID_SET_FILE_POINTER == SetFilePointer(fh, 0, 0, FILE_BEGIN)) {
		delete [] buff;
		return -1;
	}

	num_lines = 0;
	total_bytes_read_so_far = 0;
	
	while (total_bytes_read_so_far < file_size) {
		if (total_bytes_read_so_far + 1024 < file_size) {
			next_read = 1024;
		}
		else {
			next_read = file_size - total_bytes_read_so_far;
		}

		bytes_read = 0;

		if (!ReadFile(fh, buff, next_read, &bytes_read, NULL)) {
			num_lines = 0;
			break;
		}

		if (bytes_read == 0) {
			num_lines = 0;
			break;
		}

		for (unsigned int i = 0; i < bytes_read; i++) {
			if (buff[i] == '\n') {
				num_lines++;
			}
		}

		total_bytes_read_so_far += bytes_read;
	}

	// restore the old position before returing
	newPos = SetFilePointer(fh, oldPos, 0, FILE_BEGIN);

	if (newPos != oldPos) {
		delete [] buff;
		return -1;
	}

	delete [] buff;

	return num_lines;
}
