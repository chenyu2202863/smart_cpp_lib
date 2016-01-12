#ifndef __DATABASE_TABLE_HPP
#define __DATABASE_TABLE_HPP

#include <string>

#include "stream.hpp"



namespace database {

class table
{
	struct impl;
	impl *impl_;


	friend ostream& operator << (ostream & , const table &);
	friend istream& operator >> (istream & , table &);
	friend bool operator==(const table &lhs, const table &rhs);

	friend class table_iterator;

public:
	table();
	~table();

private:
	table(const table &);
	table &operator=(const table &);

public:
	void reserve_col(size_t col_cnt);
	void reserve_row(size_t row_cnt);

	size_t insert_col(const std::string &name);
	size_t insert_row();

	void set_cell(size_t col_i, size_t row_i, const void *data, size_t size);

	size_t size() const;
	void clear();

	size_t col_cnt() const;
	size_t row_cnt() const;

	const std::string &col_name(size_t index) const;

	std::pair<const char *, size_t> get_data(size_t row_i, size_t col_i) const;
	std::pair<const char *, size_t> get_data(size_t row_i, const std::string &name) const;
};


ostream &operator<<(ostream &os, const table &t);
istream &operator>>(istream &os, table &t);


bool operator==(const table &lhs, const table &rhs);
bool operator!=(const table &lhs, const table &rhs);
}


#endif