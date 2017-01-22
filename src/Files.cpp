#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Files.h"
#include "Exceptions.h"
using namespace std;

struct PhysicalFile::Handle
{
	HANDLE hFile;
};

//
// PhysicalFile class
//
bool PhysicalFile::eof()
{
	return offset == size();
}

unsigned long PhysicalFile::size()
{
	return GetFileSize(handle->hFile, NULL);
}

void PhysicalFile::seek(unsigned long offset)
{
	this->offset = min(offset, size());
}

unsigned long PhysicalFile::tell()
{
	return offset;
}

size_t PhysicalFile::read(void* buffer, size_t count)
{
	DWORD read;
	SetFilePointer( handle->hFile, offset, NULL, FILE_BEGIN );
	if (!ReadFile(handle->hFile, buffer, (DWORD)count, &read, NULL))
	{
		read = 0;
	}
	offset += read;
	return read;
}

size_t PhysicalFile::write(const void* buffer, size_t count)
{
	DWORD written;
	SetFilePointer( handle->hFile, offset, NULL, FILE_BEGIN );
	if (!WriteFile(handle->hFile, buffer, (DWORD)count, &written, NULL))
	{
		written = 0;
	}
	offset += written;
	return written;
}

PhysicalFile::PhysicalFile(const wstring& filename, int mode)
    : handle(new Handle), offset(0)
{
    DWORD dwDesiredAccess = 0, dwShareMode = FILE_SHARE_READ, dwCreationDisposition = CREATE_ALWAYS;
    if (mode & FILEMODE_READ)   dwDesiredAccess |= GENERIC_READ;
    if (mode & FILEMODE_WRITE)  dwDesiredAccess |= GENERIC_WRITE;
    if (~mode & FILEMODE_WRITE) {
        dwCreationDisposition  = OPEN_EXISTING;
        dwShareMode           |= FILE_SHARE_WRITE;
    }

	handle->hFile = CreateFile(filename.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle->hFile == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		delete handle;
		if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
		{
			throw FileNotFoundException(filename);
		}
		throw IOException(L"Unable to open file" + filename);
	}
}

PhysicalFile::~PhysicalFile()
{
    CloseHandle(handle->hFile);
    delete handle;
}
