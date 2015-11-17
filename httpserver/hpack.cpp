#include "hpack.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>

Huffman *Huffman::_huffmanTable = 0;
Huffman Huffman::huffmanTable()
{
	if (!_huffmanTable)
		_huffmanTable = new Huffman();
	return *_huffmanTable;
}

Huffman::Huffman()
{
	QFile in(":/hpack/huffman_table");
	if (!in.open(QIODevice::ReadOnly))
	{
		qCritical() << "Couldn't find the HPACK Huffman Table!";
		exit(1);
	}
	QByteArray line;
	while (!(line = in.readLine()).isEmpty())
	{
		line = line.trimmed().replace(" ", "");
		QBitArray bits(line.length());
		for (int i = 0; i < line.length(); i++)
			bits.setBit(i, line[i] == '1');
		table.append(bits);
	}
	in.close();
}

QBitArray Huffman::applyMask(const QBitArray &bits, const QBitArray &mask, int off)
{
	Q_ASSERT((bits.size() - off) >= mask.size());
	
	QBitArray ret(mask.size());
	for (int i = 0; i < mask.size(); i++)
		ret.setBit(i, bits.at(off + i) && mask.at(i));
	return ret;
}

QByteArray Huffman::decode(const QByteArray &in)
{
	QBitArray enc(in.length() * 8);
	for(int i = 0; i < in.length(); i++)
		for(int b = 0 ; b < 8; b++)
			enc.setBit(i * 8 + b, in.at(i) & (1<<(7-b)));
	QByteArray dec;
	int off = 0;
	while (off < enc.size())
	{
		bool found = false;
		for (int c = 0; c < huffmanTable().table.size(); c++)
		{
			QBitArray te = huffmanTable().table.at(c);
			if (te.size() > enc.size() - off)
				continue;
			QBitArray mask(te.size(), true);
			if (applyMask(enc, mask, off) == te)
			{
				dec.append((uchar)c);
				off += te.size();
				found = true;
				break;
			}
		}
		if (!found)
			break;
	}
	return dec;
}

QByteArray Huffman::encode(const QByteArray &in)
{
	qDebug() << "Huffman::encode(" << in << ")";
	QBitArray rem(0);
	QByteArray enc;
	for (char c : in)
	{
		QBitArray code = huffmanTable().table.at(c);
		qDebug() << c << ":" << code;
		QBitArray all(rem.size() + code.size());
		uint i = 0;
		for (int j = 0; j < rem.size(); j++)
			all.setBit(i++, rem.testBit(j));
		for (int j = 0; j < code.size(); j++)
			all.setBit(i++, code.testBit(j));
		qDebug() << rem << "+" << code << "=" << all;
		for (i = 0; i < all.size()/8; i++)
		{
			char byte = 0;
			for (int j = 0; j < 8; j++)
				if (all.testBit(i*8+j))
					byte |= (1<<(7-j));
			enc.append(byte);
		}
		i*=8;
		rem.resize(all.size()-i);
		for (int j = 0; i < all.size(); i++, j++)
			rem.setBit(j, all.testBit(i));
	}
	if (rem.size() != 0)
	{
		char byte = 0;
		int i;
		for (i = 0; i < rem.size(); i++)
			if (rem.testBit(i))
				byte |= (1<<(7-i));
		for (; i < 8; i++)
			byte |= (1<<(7-i)); // eos
		enc.append(byte);
	}
	return enc;
}

uint HPACKTableEntry::size() const
{
	return (name.length() + value.length() + 32);
}

bool HPACKTableEntry::operator== (const HPACKTableEntry &other) const
{
	if (value.isEmpty() || other.value.isEmpty())
		return (name == other.name);
	return ((name == other.name) && (value == other.value));
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

HPACKTableEntry HPACKDynamicTable::entry(int index) const
{
	return _table.value(index);
}

bool HPACKDynamicTable::contains(const HPACKTableEntry &entry) const
{
	return _table.contains(entry);
}

int HPACKDynamicTable::indexOf(const HPACKTableEntry &entry) const
{
	return _table.indexOf(entry);
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
	int off = 0;
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
					entry.name = Huffman::decode(source.mid(off, nameLength));
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
				entry.value = Huffman::decode(source.mid(off, valueLength));
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
	uchar mask = (uchar)(pow(2, n) - 1);
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
		int i;
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

QByteArray HPACK::encode(const QList<HPACKTableEntry> &headers)
{
	QByteArray enc;
	for (HPACKTableEntry entry : headers)
	{
		if (entry.value.isEmpty())
		{
			qWarning() << "HPACK: I won't encode a header field with empty value";
			continue;
		}
		
		quint64 id = tableEntry(entry);
		qDebug() << "HPACK:" << entry.name << "=" << entry.value << "; id:" << id;
		HPACKTableEntry tblEntry = id==0 ? HPACKTableEntry{ QByteArray(), QByteArray() } : tableEntry(id);
		if (!tblEntry.name.isEmpty() && !tblEntry.value.isEmpty())
		{
			QByteArray b = encodeInteger(id, 7);
			enc.append((char)(b.at(0) | 0x80));
			enc.append(b.mid(1));
			continue;
		}
		qDebug() << "TODO: Soll" << entry.name << "geindext werden?";
		QByteArray b = encodeInteger(id, 6);
		enc.append((char)(b.at(0) | 0x40));
		enc.append(b.mid(1));
		if (id == 0)
		{
			bool huffman = false; // never encode the name of the header
			QByteArray v = huffman ? Huffman::encode(entry.name) : entry.name;
			b = encodeInteger(v.length(), 7);
			enc.append(b.at(0) | (huffman ? 0x80 : 0));
			enc.append(b.mid(1));
			enc.append(v);
		}
		bool huffman = entry.value.length() >= 10;
		QByteArray v = huffman ? Huffman::encode(entry.value) : entry.value;
		b = encodeInteger(v.length(), 7);
		enc.append(b.at(0) | (huffman ? 0x80 : 0));
		enc.append(b.mid(1));
		enc.append(v);
		_dynTable.insert(entry);
	}
	return enc;
}

QByteArray HPACK::encodeInteger(quint64 i, quint8 n)
{
	QByteArray enc;
	if (i < pow(2, n) - 1)
	{
		enc.append((unsigned char)i);
		return enc;
	}
	enc.append((unsigned char)(pow(2, n) - 1));
	i -= pow(2, n) - 1;
	while (i >= 0x80)
	{
		enc.append((unsigned char)(i % 0x80 + 0x80));
		i /= 0x80;
	}
	enc.append((unsigned char)i);
	return enc;
}

HPACKTableEntry HPACK::tableEntry(quint64 id)
{
	if (id >= 1 && id <= 61)
		return staticTable().at(id - 1);
	if (id > 61)
		return _dynTable.entry(id - 62);
	return HPACKTableEntry();
}

quint64 HPACK::tableEntry(const HPACKTableEntry &entry)
{
	quint64 id = 0;
	if (staticTable().contains(entry))
		id = staticTable().indexOf(entry) + 1;
	else if (_dynTable.contains(entry))
		id = _dynTable.indexOf(entry) + 62;
	return id;
}
