#ifndef CHUNKFILE_H
#define CHUNKFILE_H

#include "Files.h"

typedef long ChunkType;

struct ChunkHeader
{
	unsigned long type;
	unsigned long size;
};

struct MiniChunkHeader
{
	unsigned char type;
	unsigned char size;
};

class ChunkReader
{
	static const int MAX_CHUNK_DEPTH = 256;

	File& m_file;
	long  m_position;
	long  m_size;
	long  m_offsets[ MAX_CHUNK_DEPTH ];
	long  m_miniSize;
	long  m_miniOffset;
	int   m_curDepth;

public:
	ChunkType   next();
	ChunkType   nextMini();
	void        skip();
	size_t      size();
	size_t      read(void* buffer, size_t size, bool check = true);
    size_t      tell() { return m_position; }
    bool        group() const { return m_size < 0; }

	ChunkReader(File& file);
};

class ChunkWriter
{
	static const int MAX_CHUNK_DEPTH = 256;
	
	template <typename HdrType>
	struct ChunkInfo
	{
		HdrType       hdr;
		unsigned long offset;
	};

	File&                      m_file;
	ChunkInfo<ChunkHeader>     m_chunks[ MAX_CHUNK_DEPTH ];
	ChunkInfo<MiniChunkHeader> m_miniChunk;
	int                        m_curDepth;

public:
	void beginChunk(ChunkType type);
	void beginMiniChunk(ChunkType type);
	void write(const void* buffer, size_t size);
	void endChunk();

	ChunkWriter(File& file);
};

#endif