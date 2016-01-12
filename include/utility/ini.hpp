#ifndef __UTILITY_INI_HPP
#define __UTILITY_INI_HPP

/** @ini.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* ini文件访问，支持ascii、unicode字符集
*/


#include <unordered_map>
#include <string>

#include "../extend_stl/unicode.hpp"
#include "../extend_stl/string/algorithm.hpp"


namespace utility
{
	/**
	* @class <ini>
	* @brief 线程相关局部存储
	*
	* 非线程安全
	*/
	class ini
	{
		struct ret_helper
		{
			const stdex::tString &buffer_;

			ret_helper(const stdex::tString &buffer)
				: buffer_(buffer)
			{}

			template < typename T >
			operator T()
			{
				return stdex::to_number(buffer_);
			}

			operator const stdex::tString &() const
			{
				return buffer_;
			}
		};

	public:
		typedef stdex::tString section_type;
		typedef stdex::tString key_type;
		typedef stdex::tString value_type;
		
	private:
		typedef std::unordered_map<key_type, value_type> key_value_map_type;
		typedef std::unordered_map<section_type, key_value_map_type> ini_map_type;

		ini_map_type ini_maps_;
		
	public:
		/**
		* @brief 构造函数，需要传入ini文件绝对路径
		* @param <path> <ini文件绝对路径>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <如果文件不存在，则不进行解析>
		* @remarks <无>
		*/
		explicit ini(const stdex::tString &path);

		/**
		* @brief 构造函数，需要传入内存缓冲区和缓冲区大小
		* @param <path> <ini文件绝对路径>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		ini(const char *buffer, size_t len);

	private:
		ini(const ini &);
		ini &operator=(const ini &);

	public:
		/**
		* @brief 获取指定section中指定key的值
		* @param <section> <section段>
		* @param <key> <在section段中key关键字>
		* @exception <无任何异常抛出>
		* @return <根据接收者类型，隐式转换该值>
		* @note <无>
		* @remarks <无>
		*/
		ret_helper get_val(const section_type &section, const key_type &key) const;

		/**
		* @brief 获取所有section
		* @param <无>
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <返回所有section>
		* @note <无>
		* @remarks <无>
		*/
		std::vector<stdex::tString> get_all_sections() const;
	};
}


#endif