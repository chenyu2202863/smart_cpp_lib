#include "table.hpp"

#include <vector>
#include <map>
#include <cassert>
#include <numeric>
#include <algorithm>


namespace database {


struct cell
{
	size_t col_index_;
	size_t row_index_;

	typedef std::allocator<char> allocator_type;
	typedef std::basic_string<char, std::char_traits<char>, allocator_type> data_type;

	data_type data_;

	cell()
		: col_index_(0)
		, row_index_(0)
	{}

	cell(size_t col_i, size_t row_i, const void *data, size_t len)
		: col_index_(col_i)
		, row_index_(row_i)
	{
		data_.resize(len);
		const char *tmp = static_cast<const char *>(data);
		std::copy(tmp, tmp + len, data_.begin());
	}

	cell(cell &&rhs)
		: col_index_(rhs.col_index_)
		, row_index_(rhs.row_index_)
		, data_(std::move(rhs.data_))
	{

	}

	cell &operator=(cell &&rhs)
	{
		if( &rhs != this )
		{
			col_index_	= rhs.col_index_;
			row_index_	= rhs.row_index_;
			data_		= std::move(rhs.data_);
		}

		return *this;
	}

private:
	cell(const cell &);
	cell &operator=(const cell &);

public:
	size_t size() const
	{
		// 数据属性大小
		size_t size = sizeof(row_index_) + sizeof(col_index_) + sizeof(data_.size()) + data_.size();
		return size;
	}

	void set(size_t col_i, size_t row_i, const void *data, size_t len)
	{
		col_index_ = col_i;
		row_index_ = row_i;

		data_.resize(len);
		const char *tmp = static_cast<const char *>(data);
		std::copy(tmp, tmp + len, data_.begin());
	}

	void clear()
	{
		col_index_ = 0;
		row_index_ = 0;
		data_.clear();
	}
};

ostream &operator<<(ostream &os, const cell &val)
{	
	os << val.col_index_ << val.row_index_ << val.data_.size();

	os.write(val.data_.data(), val.data_.size());

	return os;
}

istream &operator>>(istream &os, cell &val)
{	
	size_t size = 0;
	os >> val.col_index_ >> val.row_index_ >> size;
	val.data_.resize(size);

	if( size != 0 )
		os.read(&val.data_[0], size);

	return os;
}

bool operator==(const cell &lhs, const cell &rhs)
{
	return lhs.col_index_ == rhs.col_index_ &&
		lhs.row_index_ == rhs.row_index_ &&
		lhs.data_.size() == rhs.data_.size() &&
		memcmp(lhs.data_.data(), rhs.data_.data(), rhs.data_.size()) == 0;
}

bool operator!=(const cell &lhs, const cell &rhs)
{
	return !(lhs == rhs);
}



class col
{
	friend ostream& operator << (ostream & , const col &);
	friend istream& operator >> (istream & , col &);

	friend bool operator==(const col &lhs, const col &rhs);

public:
	typedef std::string name_type;
	typedef size_t id_type;

private:
	name_type name_;

public:
	col()
	{}

	col(const std::string &name)
		: name_(name)
	{}

	col(col &&r)
		: name_(std::move(r.name_))
	{
	}

	col &operator=(col &&r)
	{
		if( &r != this )
		{
			name_ = std::move(r.name_);
		}

		return *this;
	}

private:
	col(const col &);
	col &operator=(const col &);

public:
	void name(const name_type &name)
	{ name_ = name; }

	const name_type &name() const
	{ return name_; }


	size_t size() const
	{
		return sizeof(name_.length()) + name_.length() * sizeof(name_type::value_type);
	}

	void clear()
	{
		name_.clear();
	}
};

ostream &operator<<(ostream &os, const col &val)
{	
	os << val.name().length();
	os.write(val.name().c_str(), val.name().length());

	return os;
}

istream &operator>>(istream &os, col &val)
{	
	size_t len = 0;
	os >> len;

	if( len != 0 )
	{
		val.name_.resize(len);
		os.read(&val.name_[0], len);
	}

	return os;
}

bool operator==(const col &lhs, const col &rhs)
{
	return lhs.name_ == rhs.name_;
}

bool operator!=(const col &lhs, const col &rhs)
{
	return !(lhs == rhs);
}


class row
{
	typedef std::allocator<cell>				allocator_type;
	typedef std::vector<cell, allocator_type>	cells_type;
	cells_type cells_;

	friend ostream& operator << (ostream & , const row &);
	friend istream& operator >> (istream & , row &);

	friend bool operator==(const row &lhs, const row &rhs);

public:
	row()
	{}
	explicit row(size_t cnt)
	{
		cells_.resize(cnt);
	}

	row(row &&rhs)
		: cells_(std::move(rhs.cells_))
	{
	}

	row &operator=(row &&rhs)
	{
		if( &rhs != this )
		{
			cells_ = std::move(rhs.cells_);
		}

		return *this;
	}

private:
	row(const row &);
	row &operator=(const row &);

public:
	cell &operator[](size_t index)
	{
		assert(cells_.size() > index);
		return cells_[index];
	}

	const cell &operator[](size_t index) const
	{
		assert(cells_.size() > index);
		return cells_[index];
	}

	size_t size() const
	{
		// 得到所有数据大小
		size_t size = sizeof(cells_.size());
		size += std::accumulate(cells_.begin(), cells_.end(), 0, [](size_t cnt, const cell &val)->int
		{
			cnt += val.size();
			return cnt;
		});

		return size;
	}

	void reserve(size_t cnt)
	{
		cells_.reserve(cnt);
	}

	void clear()
	{
		std::for_each(cells_.begin(), cells_.end(), [](cell &val)
		{
			val.clear();
		});
		cells_.clear();
	}
};

ostream &operator<<(ostream &os, const row &val)
{	
	os << val.cells_.size();
	for(size_t i = 0; i != val.cells_.size(); ++i)
	{
		os << val.cells_[i];
	}
	return os;
}

istream &operator>>(istream &os, row &val)
{	
	size_t size = 0;
	os >> size;
	val.cells_.resize(size);

	for(size_t i = 0; i != size; ++i)
	{
		os >> val.cells_[i];
	}

	return os;
}

bool operator==(const row &lhs, const row &rhs)
{
	if( lhs.cells_.size() != rhs.cells_.size() )
		return false;

	for(size_t i = 0; i != rhs.cells_.size(); ++i)
	{
		if( rhs.cells_[i] != lhs.cells_[i] )
			return false;
	}

	return true;
}

bool operator!=(const row &lhs, const row &rhs)
{
	return !(lhs == rhs);
}


struct table::impl
{
	typedef std::allocator<col> col_allocator_type;
	typedef std::allocator<row> row_allocator_type;

	std::vector<col, col_allocator_type> cols_;
	std::vector<row, row_allocator_type> rows_;

	typedef std::allocator<std::pair<const std::string, size_t>> map_allocator_type;
	typedef std::map<std::string, size_t, std::less<std::string>, map_allocator_type> name_2_index_type;
	name_2_index_type name_2_index_;
};




table::table()
	: impl_(new impl)
{
}

table::~table()
{
	impl_->~impl();
}


void table::reserve_col(size_t max_size)
{
	impl_->cols_.reserve(max_size);
}

size_t table::insert_col(const col::name_type &name)
{
	impl_->cols_.push_back(col(name));
	impl_->name_2_index_[name] = impl_->cols_.size() - 1;

	return impl_->cols_.size() - 1;
}

size_t table::insert_row()
{
	impl_->rows_.push_back(row(impl_->cols_.size()));
	return impl_->rows_.size() - 1;
}

void table::reserve_row(size_t row_cnt)
{
	impl_->rows_.reserve(row_cnt);
}

void table::set_cell(size_t col_i, size_t row_i, const void *data, size_t size)
{
	cell &cell_val = impl_->rows_[row_i][col_i];
	cell_val.set(col_i, row_i, data, size);
}


size_t table::size() const
{
	size_t size = sizeof(impl_->cols_.size()) + sizeof(impl_->rows_.size());

	size += std::accumulate(impl_->cols_.begin(), impl_->cols_.end(), 0, [](size_t cnt, const col &val)->int
	{
		cnt += val.size();
		return cnt;
	});

	size += std::accumulate(impl_->rows_.begin(), impl_->rows_.end(), 0, [](size_t cnt, const row &val)->int
	{
		cnt += val.size();
		return cnt;
	});

	return size;
}

size_t table::row_cnt() const
{
	return impl_->rows_.size();
}

size_t table::col_cnt() const
{
	return impl_->cols_.size();
}

const std::string &table::col_name(size_t index) const
{
	return impl_->cols_[index].name();
}	

std::pair<const char *, size_t> table::get_data(size_t row_i, size_t col_i) const
{
	assert(col_i < impl_->cols_.size() && row_i < impl_->rows_.size());

	if( col_i < impl_->cols_.size() && row_i < impl_->rows_.size() )
	{
		const cell::data_type &val = impl_->rows_[row_i][col_i].data_;
		return std::make_pair(val.data(), val.size());
	}

	return std::make_pair((const char *)0, 0);
}

std::pair<const char *, size_t> table::get_data(size_t row_i, const std::string &name) const
{
	return get_data(row_i, impl_->name_2_index_[name]);
}

void table::clear()
{
	std::for_each(impl_->rows_.begin(), impl_->rows_.end(), [](row &val)
	{
		val.clear();
	});
	impl_->rows_.clear();

	std::for_each(impl_->cols_.begin(), impl_->cols_.end(), [](col &val)
	{
		val.clear();
	});
	impl_->cols_.clear();

	impl_->name_2_index_.clear();
}



ostream &operator<<(ostream &os, const table &t)
{
	os << t.impl_->cols_.size();
	for(size_t i = 0; i != t.impl_->cols_.size(); ++i)
	{
		os << t.impl_->cols_[i];
	}

	os << t.impl_->rows_.size();
	for(size_t i = 0; i != t.impl_->rows_.size(); ++i)
	{
		os << t.impl_->rows_[i];
	}


	return os;
}


istream &operator>>(istream &os, table &t)
{	
	size_t col_size = 0;
	os >> col_size;
	t.impl_->cols_.resize(col_size);

	for(size_t i = 0; i != col_size; ++i)
	{
		os >> t.impl_->cols_[i];

		t.impl_->name_2_index_[t.impl_->cols_[i].name()] = i;
	}

	size_t row_size = 0;
	os >> row_size;
	t.impl_->rows_.resize(row_size);

	for(size_t i = 0; i != row_size; ++i)
	{
		os >> t.impl_->rows_[i];
	}

	return os;
}


bool operator==(const table &lhs, const table &rhs)
{
	if( lhs.impl_->cols_.size() != rhs.impl_->cols_.size() &&
		lhs	.impl_->rows_.size() != rhs.impl_->rows_.size() )
		return false;

	return lhs.impl_->cols_ == rhs.impl_->cols_ &&
		lhs.impl_->rows_ == rhs.impl_->rows_;
}

bool operator!=(const table &lhs, const table &rhs)
{
	return !(lhs == rhs);
}

}