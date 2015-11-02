#include "hpack.h"

#include <QDebug>
#include <QFile>

uint HPACKTableEntry::size() const
{
	return (name.length() + value.length() + 32);
}

HPACKDynamicTable::HPACKDynamicTable(uint maxSize)
	: _size(0)
	, _maxSize(maxSize)
{
}

void HPACKDynamicTable::insert(const HPACKTableEntry &entry)
{
	if (_size + entry.size() > _maxSize)
		shrink(qMax((uint)0, _maxSize - entry.size()));
	if (entry.size() <= _maxSize)
	{
		_table.insert(0, entry);
		_size += entry.size();
	}
}

HPACKTableEntry HPACKDynamicTable::entry(int index)
{
	return _table.value(index);
}

void HPACKDynamicTable::setMaxSize(uint maxSize)
{
	shrink(maxSize);
	_maxSize = maxSize;
}

void HPACKDynamicTable::shrink(uint maxSize)
{
	while (_size > maxSize)
	{
		HPACKTableEntry last = _table.last();
		_table.removeLast();
		_size -= last.size();
	}
}

QList<HPACKTableEntry> *HPACK::_staticTable = 0;
QList<HPACKTableEntry> HPACK::staticTable()
{
	if (!_staticTable)
	{
		_staticTable = new QList<HPACKTableEntry>;
		QFile in(":/hpack/static_table");
		if (!in.open(QIODevice::ReadOnly))
		{
			qCritical() << "Couldn't find the HPACK Static Table!";
			exit(1);
		}
		QByteArray line;
		while (!(line = in.readLine()).isEmpty())
		{
			line = line.trimmed();
			if (line.isEmpty() || line.startsWith('#'))
				continue;
			HPACKTableEntry entry;
			int tab = line.indexOf('\t');
			if (tab > 0)
			{
				entry.name = line.mid(0, tab).trimmed();
				entry.value = line.mid(tab).trimmed();
			}
			else
				entry.name = line;
			_staticTable->append(entry);
		}
		in.close();
	}
	return *_staticTable;
}

HPACK::HPACK(uint tableSize)
	: _dynTable(tableSize)
	, _error(false)
{
}

QList<HPACKTableEntry> HPACK::decode(const QByteArray &source)
{
	QList<HPACKTableEntry> headers;
	uint off = 0;
	while (off < source.length())
	{
		// Indexed Header Field
		if ((source[off] & 0x80) == 0x80) // 1x xx xx xx
		{
			uint bytesRead;
			quint64 id = decodeInteger(source.mid(off), 7, &bytesRead);
			if (_error || (id == 0))
			{
				_error = true;
				return QList<HPACKTableEntry>();
			}
			qDebug() << "indexed header field" << id;
			off += bytesRead;
			headers.append(tableEntry(id));
		}
		
		// Dynamic Table Size Update
		else if ((source[off] & 0xE0) == 0x20) // 00 1x xx xx
		{
			uint bytesRead;
			quint64 id = decodeInteger(source.mid(off), 5, &bytesRead);
			off += bytesRead;
			qDebug() << "dynamic table size upgrade" << id;
		}
		
		else
		{
			bool index;
			int n;
			
			// Literal Header Field with Incremental Indexing
			if ((source[off] & 0xC0) == 0x40) // 01 xx xx xx
			{
				n = 6;
				index = true;
			}
			
			// Literal Header Field without Indexing
			else if ((source[off] & 0xF0) == 0x00) // 00 00 xx xx
			{
				n = 4;
				index = false;
			}
			
			// Literal Header Field Never Indexed
			else if ((source[off] & 0xF0) == 0x10) // 00 01 xx xx
			{
				n = 4;
				index = false;
			}
			
			uint bytesRead;
			quint64 id = decodeInteger(source.mid(off), n, &bytesRead);
			if (_error)
				return QList<HPACKTableEntry>();
			off += bytesRead;
			qDebug() << "literal header field with incremental indexing" << id;
			HPACKTableEntry entry = tableEntry(id);
			if (id == 0)
			{
				bool huffman = source[off] & 0x80;
				quint64 nameLength = decodeInteger(source.mid(off), 7, &bytesRead);
				if (_error)
					return QList<HPACKTableEntry>();
				off += bytesRead;
				if (huffman)
					qDebug() << "TODO: implement huffman";
				else
					entry.name = source.mid(off, nameLength);
				off += nameLength;
			}
			bool huffman = source[off] & 0x80;
			quint64 valueLength = decodeInteger(source.mid(off), 7, &bytesRead);
			if (_error)
				return QList<HPACKTableEntry>();
			off += bytesRead;
			if (huffman)
				qDebug() << "TODO: implement huffman";
			else
				entry.value = source.mid(off, valueLength);
			off += valueLength;
			if (index)
				_dynTable.insert(entry);
			headers.append(entry);
		}
	}
	qDebug() << "finished decompressing";
	return headers;
}

quint64 HPACK::decodeInteger(const QByteArray &bytes, quint8 n, uint *bytesRead)
{
	if (bytes.size() == 0)
	{
		_error = true;
		return 0;
	}
	char mask = (char)(pow(2, n) - 1);
	quint64 dec = bytes[0] & mask;
	if (dec < mask)
	{
		if (bytesRead)
			*bytesRead = 1;
	}
	else
	{
		uint m = 0;
		char b = 0x80;
		uint i;
		for (i = 1; (b & 0x80) == 0x80; i++)
		{
			if (i >= bytes.size())
			{
				_error = true;
				return 0;
			}
			b = bytes[i];
			dec += (b & 0x7f) * (quint64)pow(2, m);
			m += 7;
		}
		if (bytesRead)
			*bytesRead = i;
	}
	return dec;
}

HPACKTableEntry HPACK::tableEntry(quint64 id)
{
	if (id >= 1 && id <= 61)
		return staticTable().at(id - 1);
	if (id > 61)
		return _dynTable.entry(id);
	return HPACKTableEntry();
}
