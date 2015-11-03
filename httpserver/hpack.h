#ifndef HPACK_H
#define HPACK_H

#include "httpglobal.h"

#include <QBitArray>
#include <QByteArray>
#include <QList>

/** This class is used to compress and/or decompress huffman code. */
class Huffman
{
private:
	static Huffman *_huffmanTable;
	static Huffman huffmanTable();
	explicit Huffman();
	QList<QBitArray> table;
	
public:
	static QBitArray applyMask(const QBitArray &bits, const QBitArray &mask, int off = 0);
	static QByteArray decode(const QByteArray &in);
};

/** An entry in a HPACK Table. */
struct HPACKTableEntry
{
	QByteArray name;
	QByteArray value;
	
	/** Returns the size of this entry. */
	uint size() const;
};

/** This class stores the HPACK Dynamic Table. */
class HPACKDynamicTable
{
	
public:
	/** Creates a new HPACK Dynamic Table with the given maximum size. */
	HPACKDynamicTable(uint maxSize = 4096);
	
	/** Inserts the element into the table. This could result in eviction of older elements. */
	void insert(const HPACKTableEntry &entry);
	/** Returns the element with the given index. */
	HPACKTableEntry entry(int index);
	
	/** Returns the current size of the Table. */
	uint size() const { return _size; }
	/** Returns the maximum size of the Table. */
	uint maxSize() const { return _maxSize; }
	
	/** Sets the maximum size of the Table. This could result in eviction of older elements. */
	void setMaxSize(uint maxSize);
	
private:
	/** Shrinks the table to the given size. */
	void shrink(uint maxSize);
	
	QList<HPACKTableEntry> _table;
	uint _size;
	uint _maxSize;
	
};

class HPACK
{
	
private:
	static QList<HPACKTableEntry> *_staticTable;
	static QList<HPACKTableEntry> staticTable();
	
public:
	HPACK(uint tableSize = 4096);
	
	QList<HPACKTableEntry> decode(const QByteArray &source);
	
	bool error() const { return _error; }
	void clearError() { _error = false; }
	
private:
	/** Returns the decoded integer with a n prefix. */
	quint64 decodeInteger(const QByteArray &bytes, quint8 n = 0, uint *bytesRead = 0);
	
	/** Returns the entry with the given id from either the static or the dynamic table. */
	HPACKTableEntry tableEntry(quint64 id);
	
	HPACKDynamicTable _dynTable;
	bool _error;
	
};

#endif // HPACK_H
