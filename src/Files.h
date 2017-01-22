#ifndef FILE_H
#define FILE_H

#include <string>

class File
{
protected:
	virtual ~File() {}

public:
	virtual bool          eof() = 0;
	virtual unsigned long size() = 0;
	virtual void          seek(unsigned long offset) = 0;
	virtual unsigned long tell() = 0;
	virtual size_t read(void* buffer, size_t count) = 0;
    virtual size_t write(const void* buffer, size_t count) = 0;
};

enum
{
    FILEMODE_READ  = 1,
    FILEMODE_WRITE = 2,
};

class PhysicalFile : public File
{
	struct Handle;

	Handle*       handle;
	unsigned long offset;

public:
	bool          eof();
	unsigned long size();
	void          seek(unsigned long offset);
	unsigned long tell();
	size_t read(void* buffer, size_t count);
    size_t write(const void* buffer, size_t count);

	PhysicalFile(const std::wstring& filename, int mode);
	~PhysicalFile();
};

#endif