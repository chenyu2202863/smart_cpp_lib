#ifndef __DATABASE_TABLE_ITERATOR_HPP
#define __DATABASE_TABLE_ITERATOR_HPP


#include <iterator>
#include <string>
#include "table.hpp"


namespace database {

	class table_iterator
		: public std::iterator<std::forward_iterator_tag, std::string>
	{
	private:
		bool is_eof_;
		size_t row_index_;
		size_t col_index_;
		table *val_;

	public:
		table_iterator()
			: is_eof_(true)
			, row_index_(0)
			, col_index_(0)
			, val_(nullptr)
		{}
		table_iterator(table &val)
			: is_eof_(false)
			, row_index_(0)
			, col_index_(0)
			, val_(&val)
		{}

	public:
		bool is_eof() const
		{
			return is_eof_;
		}

		void operator++()
		{
			++col_index_;

			if( col_index_ % val_->col_cnt() == 0 )
			{
				++row_index_;

				if( row_index_ == val_->row_cnt() && 
					col_index_ == val_->col_cnt() )
				{
					is_eof_ = true;
					return;
				}

				col_index_ = 0;
			}
		}

		std::string operator*() const
		{
			auto ret_val = val_->get_data(row_index_, col_index_);
			return std::string(ret_val.first, ret_val.second);
		}

		bool operator==(const table_iterator &rhs) const
		{
			return rhs.is_eof_ && is_eof_;
		}

		bool operator!=(const table_iterator &rhs) const
		{
			return !(*this == rhs);
		}
	};

}



#endif