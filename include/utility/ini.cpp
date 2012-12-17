#include "ini.hpp"

#include <strstream>
#include <fstream>
#include "../unicode/string.hpp"


#pragma warning(disable: 4503)


namespace utility
{
	
	namespace detail
	{
		template < typename CharT, typename IniMapT >
		void parse(std::basic_istream<CharT> &&stream, IniMapT &maps)
		{
			std::basic_string<CharT> line;
			std::basic_string<TCHAR> cur_section;

			while(std::getline(stream, line))
			{
				stdex::trim(line);

				if( line.empty() )
					continue;

				// comment
				if( line[0] == '#' ||
					line[0] == ';' )
					continue;

				// section
				size_t start_pos = line.find('[');
				size_t end_pos = 0;
				if( start_pos != std::basic_string<CharT>::npos )
				{
					end_pos = line.rfind(']');

					// error
					if( end_pos == std::basic_string<CharT>::npos )
					{
						assert(0);
						continue;
					}

					cur_section = unicode::to(line.substr(start_pos + 1, end_pos - start_pos - 1));
					maps[cur_section];

					continue;
				}


				size_t key_pos = line.find('=');
				if( key_pos != std::basic_string<CharT>::npos )
				{
					ini::key_type key = unicode::to(line.substr(0, key_pos));
					stdex::trim(key);

					ini::value_type value = unicode::to(line.substr(key_pos + 1, line.length()));
					stdex::trim(value);

					maps[cur_section][key] = value;
				}
			}
		}
	}


	ini::ini(const stdex::tString &path)
	{
		stdex::tIfstream in;

		in.imbue(std::locale("chs"));
		in.open(path.c_str());

		assert(in.good());
		if( !in )
			return;

		detail::parse(std::move(in), ini_maps_);
	}

	ini::ini(const char *buffer, size_t len)
	{
		std::istringstream is(buffer);

		detail::parse(std::move(is), ini_maps_);
	}

	ini::ret_helper ini::get_val(const section_type &section, const key_type &key) const
	{
		assert(!ini_maps_.empty());

		auto section_iter = ini_maps_.find(section);
		if( section_iter == ini_maps_.end() )
			return ret_helper(_T(""));
		auto key_iter = section_iter->second.find(key);
		if( key_iter == section_iter->second.end() )
			return ret_helper(_T(""));

		return ret_helper(key_iter->second);
	}

	std::vector<stdex::tString> ini::get_all_sections() const
	{
		std::vector<stdex::tString> vec;
		vec.reserve(ini_maps_.size());

		std::for_each(ini_maps_.begin(), ini_maps_.end(), [&vec](const ini_map_type::value_type &val)
		{
			vec.push_back(val.first);
		});

		return std::move(vec);
	}
}