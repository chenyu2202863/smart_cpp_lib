#ifndef __MMAP_FILEMAPPING_HPP
#define __MMAP_FILEMAPPING_HPP

#include "map_file.hpp"
#include "access_buffer.hpp"



namespace mmap
{

	// -------------------------------------------------------------------------
	// class HandleProxy	

	template < typename Owner >
	class proxy_t
	{
	public:
		typedef typename Owner::size_type	size_type;
		typedef typename Owner::pos_type	pos_type;
		typedef typename Owner::Utils		Utils;

	private: 
		Owner &owner_;

	public:
		enum { AllocationGranularityBits= Owner::AllocationGranularityBits };
		enum { AllocationGranularity	= Owner::AllocationGranularity	   };
		enum { AllocationGranularityMask= Owner::AllocationGranularityMask };
	
	public:
		proxy_t(Owner &owner)
			: owner_(owner)
		{}

	public:
		char *view_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			return owner_.view_segment(dwBasePage, dwPageCount);
		}

		char *access_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			return owner_.access_segment(dwBasePage, dwPageCount);
		}

		char *alloc_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			return owner_.alloc_segment(dwBasePage, dwPageCount);
		}
	};


	// -------------------------------------------------------------------------
	// class FileMapping

	// 对MapFile的一层包装，提供边界对齐的保护操作

	template<typename ConfigT>
	class file_mapping_t
		: public map_file_t<ConfigT>
	{
	private:
		typedef map_file_t<ConfigT>					base_type;

	public:
		typedef typename base_type::size_type		size_type;
		typedef typename base_type::pos_type		pos_type;

		typedef proxy_t<file_mapping_t>				HandleType;
		typedef base_type							Utils;

	private:
		DWORD total_pages_;

		enum { _AGBits = 16 };
		// 分配粒度
		enum { _AllocationGranularityInvBits = sizeof(DWORD) * 8 - _AGBits };


	public:
		enum { AllocationGranularityBits	= _AGBits };
		enum { AllocationGranularity		= (1 << _AGBits) };
		enum { AllocationGranularityMask	= (AllocationGranularity - 1) };

	private:
		file_mapping_t(const file_mapping_t &);
		file_mapping_t &operator=(const file_mapping_t &);


	public:
		file_mapping_t()
			: total_pages_(0)
		{}
		template < typename CharT >
		file_mapping_t(const std::basic_string<CharT> &path, pos_type *offset = NULL)
		{
			open(path, offset);
		}

	public:
		DWORD total_pages() const
		{
			return total_pages_;
		}

		void close()
		{
			base_type::close();
			total_pages_ = 0;
		}

		bool resize(pos_type size)
		{
			total_pages_ = (size + AllocationGranularityMask) >> AllocationGranularityBits;
	
			return base_type::resize(size);
		}

		template < typename CharT >
		bool open(const std::basic_string<CharT> &path, pos_type *offset = 0)
		{
			if( ConfigT::GetSizeOnOpen )
			{
				pos_type size = 0;
				bool hRes = base_type::open(path, &size);

				total_pages_ = static_cast<DWORD>((size + AllocationGranularityMask) >> AllocationGranularityBits);

				if( offset != 0 )
					*offset = size;

				return hRes;
			}
			else
			{
				total_pages_ = 0;
				return base_type::open(path, 0);
			}
		}

		char *view_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			assert(base_type::good());

			if( dwBasePage + dwPageCount > total_pages_ )
			{
				if( dwBasePage >= total_pages_ )
					return 0;
				else
					dwPageCount = total_pages_ - dwBasePage;
			}

			return reinterpret_cast<char *>(base_type::map(dwBasePage << AllocationGranularityBits, dwPageCount << AllocationGranularityBits));
		}

		char *access_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			assert(base_type::good());

			if( dwBasePage + dwPageCount > total_pages_ )
			{
				total_pages_ = dwBasePage + dwPageCount;

				base_type::resize(total_pages_ << AllocationGranularityBits);
			}

			return reinterpret_cast<char *>(base_type::map(dwBasePage << AllocationGranularityBits, dwPageCount << AllocationGranularityBits));
		}

		char *alloc_segment(DWORD dwBasePage, DWORD dwPageCount)
		{
			assert(base_type::good());

			dwBasePage += total_pages_;
			total_pages_ += dwPageCount;

			base_type::resize(total_pages_ << AllocationGranularityBits);

			return reinterpret_cast<char *>(base_type::map(dwBasePage << AllocationGranularityBits, dwPageCount << AllocationGranularityBits));
		}
		
	};


	typedef file_mapping_t<mapping_readwrite> file_mapping_readwrite;
	typedef file_mapping_t<mapping_readonly>  file_mapping_readonly;
}





#endif