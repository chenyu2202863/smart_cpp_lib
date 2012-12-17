#ifndef __ZIP_ZIP_HELPER_HPP
#define __ZIP_ZIP_HELPER_HPP

#include "zip/XZip.h"
#include "zip/XUnzip.h"

#include "../utility/smart_handle.hpp"


namespace zip
{
	template < typename T >
	struct close_zip_t
	{
		void operator()(T val)
		{
			CloseZip(val);
		}
	};

	typedef utility::smart_handle_t<HZIP, close_zip_t> auto_zip;


	// 解压
	template < typename BufferT >
	inline bool unzip(const BufferT &inBuf, const std::basic_string<TCHAR> &path)		
	{
		auto_zip hz = ::OpenZip(inBuf.first.get(), inBuf.second, ZIP_MEMORY, path.c_str());
		if( !hz.IsValid() )
			return false;

		ZIPENTRYW ze = {0}; 
		HRESULT hRes = ::GetZipItem(hz, -1 ,&ze); 
		if( hRes != S_OK )
			return false;

		int numitems = ze.index;
		for(int i=0; i != numitems; ++i)
		{ 
			hRes = ::GetZipItem(hz,i, &ze);
			if( hRes != S_OK )
				return false;

			hRes = ::UnzipItem(hz, i, ze.name, 0, ZIP_FILENAME);
			if( hRes != S_OK )
				return false;
		}

		return true;
	}

	template < typename BufferT >
	inline BufferT unzip(const BufferT &inBuf, BufferT (*MakeBufferFn)(size_t))
	{
		// 解压
		auto_zip hz = ::OpenZip(inBuf.first.get(), inBuf.second, ZIP_MEMORY, NULL);
		if( !hz.IsValid() )
			throw std::runtime_error("打开压缩文件出错!");

		ZIPENTRYW ze = {0}; 
		const int index = 0;
		HRESULT res = ::GetZipItemW(hz, index, &ze);
		if( res != S_OK )
			throw std::runtime_error("打开压缩文件出错!");

		BufferT outBuf = (*MakeBufferFn)(ze.unc_size);

		HRESULT zr = ZR_MORE; 
		unsigned long readSize = 0;
		const size_t unzipLen = 4096;
		while( zr == ZR_MORE )
		{
			const size_t leftLen = ze.unc_size - readSize;
			size_t readLen = std::min(leftLen, unzipLen);

			res = ::UnzipItem(hz,index, outBuf.first.get() + readSize, readLen, ZIP_MEMORY);

			if( res == S_OK )
				break;

			readSize += readLen;
		}

		assert(readSize == ze.unc_size);

		return outBuf;
	}
}


#endif