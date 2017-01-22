#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <stdexcept>

class wexception
{
    std::wstring m_message;
public:
    const wchar_t* what() const { return m_message.c_str(); }
    wexception(const std::wstring& message) : m_message(message) {}
};

class wruntime_error : public wexception
{
public:
    wruntime_error(const std::wstring& message) : wexception(message) {}
};

class IOException : public wruntime_error
{
public:
	IOException(const std::wstring& message) : wruntime_error(message) {}
};

class ReadException : public IOException
{
public:
	ReadException() : IOException(L"Unable to read file") {}
};

class WriteException : public IOException
{
public:
	WriteException() : IOException(L"Unable to write file") {}
};

class FileNotFoundException : public IOException
{
public:
	FileNotFoundException(const std::wstring& filename)
		: IOException(L"Unable to find file:\n" + filename) {}
};

class BadFileException : public IOException
{
public:
	BadFileException()
		: IOException(L"Bad or corrupted file") {}
};

#endif