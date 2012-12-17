#include <Windows.h>
#include "Registry.h"
#include <shlwapi.h>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")  // for SHDeleteKey

// RegKey ----------------------------------------------------------------------

namespace win32
{
	namespace reg
	{
		reg_key::reg_key()
			: key_(0)
			, watch_event_(0) 
		{
		}

		reg_key::reg_key(HKEY rootkey, const TCHAR* subkey, REGSAM access)
			: key_(0)
			, watch_event_(0) 
		{
			if (rootkey) 
			{
				if (access & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_CREATE_LINK))
					create(rootkey, subkey, access);
				else
					open(rootkey, subkey, access);
			}
		}

		reg_key::~reg_key() 
		{
			close();
		}

		bool reg_key::create( HKEY rootkey, const TCHAR* subkey, REGSAM access )
		{
			DWORD disposition_value;
			return create(rootkey, subkey, &disposition_value, access);
		}

		bool reg_key::create( HKEY rootkey, const TCHAR* subkey, DWORD* disposition, REGSAM access )
		{
			close();

			return ERROR_SUCCESS == ::RegCreateKeyEx(rootkey, subkey, 0, NULL,
				REG_OPTION_NON_VOLATILE, access, NULL, &key_,
				disposition);
		}

		bool reg_key::create_key( const TCHAR* name, REGSAM access )
		{
			HKEY subkey = 0;
			bool suc = (ERROR_SUCCESS == ::RegCreateKeyEx(key_, name, 0, NULL, REG_OPTION_NON_VOLATILE,
				access, NULL, &subkey, NULL));
			close();

			key_ = subkey;
			return suc;
		}

		bool reg_key::open( HKEY rootkey, const TCHAR* subkey, REGSAM access )
		{
			close();
			return ERROR_SUCCESS == ::RegOpenKeyEx(rootkey, subkey, 0, access, &key_);
		}

		bool reg_key::open( const TCHAR* relative_key_name, REGSAM access )
		{
			HKEY subkey = NULL;
			LONG result = ::RegOpenKeyEx(key_, relative_key_name, 0, access, &subkey);

			// We have to close the current opened key before replacing it with the new
			// one.
			close();

			key_ = subkey;
			return result == ERROR_SUCCESS;
		}

		void reg_key::close() 
		{
			stop_watching();

			if (key_) 
			{
				::RegCloseKey(key_);
				key_ = 0;
			}
		}

		bool reg_key::has_value( const TCHAR* value_name ) const
		{
			return ERROR_SUCCESS == ::RegQueryValueEx(key_, value_name, 0, NULL, NULL, NULL);
		}

		std::uint32_t reg_key::get_value_cnt() const 
		{
			unsigned long count = 0;
			LONG result = ::RegQueryInfoKey(key_, NULL, 0, NULL, NULL, NULL, NULL, &count,
				NULL, NULL, NULL, NULL);
			return (result == ERROR_SUCCESS) ? count : 0;
		}

		bool reg_key::get_value_name_at( int index, std::basic_string<TCHAR>* name ) const
		{
			TCHAR buf[256] = {0};
			DWORD bufsize = sizeof(buf) / sizeof(buf[0]);
			LONG r = ::RegEnumValue(key_, index, buf, &bufsize, NULL, NULL, NULL, NULL);
			if (r == ERROR_SUCCESS)
				*name = buf;

			return r == ERROR_SUCCESS;
		}

		bool reg_key::delete_key( const TCHAR* name )
		{
			return ERROR_SUCCESS == ::SHDeleteKey(key_, name);
		}

		bool reg_key::delete_value( const TCHAR* name )
		{
			return ERROR_SUCCESS == ::RegDeleteValue(key_, name);
		}

		bool reg_key::read_value(const TCHAR *name, double &out_value)
		{
			DWORD type = REG_BINARY;

			double local_value(0);
			DWORD size = sizeof(local_value);
			bool result = read_value(name, &local_value, &size, &type);
			if (result) 
			{
				if( (type == REG_BINARY || type == REG_QWORD ) && 
					size == sizeof(local_value) )
					out_value = *reinterpret_cast<double *>(&local_value);
				else
					result = false;
			}

			return result;
		}

		bool reg_key::read_value(const TCHAR* name, std::basic_string<TCHAR> &out_value) const 
		{
			const size_t kMaxStringLength = 1024;  // This is after expansion.
			// Use the one of the other forms of ReadValue if 1024 is too small for you.
			TCHAR raw_value[kMaxStringLength] = {0};
			DWORD type = REG_SZ, size = sizeof(raw_value);
			bool result = read_value(name, raw_value, &size, &type);
			if ( result ) 
			{
				if (type == REG_SZ) 
				{
					out_value = raw_value;
				} 
				else if (type == REG_EXPAND_SZ) 
				{
					TCHAR expanded[kMaxStringLength] = {0};
					size = ExpandEnvironmentStrings(raw_value, expanded, kMaxStringLength);
					// Success: returns the number of TCHAR's copied
					// Fail: buffer too small, returns the size required
					// Fail: other, returns 0
					if (size == 0 || size > kMaxStringLength) 
					{
						result = false;
					} 
					else 
					{
						out_value = expanded;
					}
				}
			}

			return result;
		}

		bool reg_key::read_value(const TCHAR* name, void* data, DWORD *dsize, DWORD *dtype) const 
		{
			return ERROR_SUCCESS == ::RegQueryValueEx(key_, name, 0, dtype, reinterpret_cast<LPBYTE>(data), dsize);
		}

		bool reg_key::read_values( const TCHAR* name, std::vector<std::basic_string<TCHAR>> &values ) const
		{
			values.clear();

			DWORD type = REG_MULTI_SZ;
			DWORD size = 0;
			bool result = read_value(name, NULL, &size, &type);
			if ( !result || size == 0)
				return result;

			if (type != REG_MULTI_SZ)
				return false;

			std::vector<TCHAR> buffer(size / sizeof(TCHAR));
			result = read_value(name, &buffer[0], &size, NULL);
			if ( !result || size == 0)
				return result;

			// Parse the double-null-terminated list of strings.
			// Note: This code is paranoid to not read outside of |buf|, in the case where
			// it may not be properly terminated.
			const TCHAR* entry = &buffer[0];
			const TCHAR* buffer_end = entry + (size / sizeof(TCHAR));
			while (entry < buffer_end && entry[0] != _T('\0')) 
			{
				const TCHAR* entry_end = std::find(entry, buffer_end, _T('\0'));
				values.push_back(std::basic_string<TCHAR>(entry, entry_end));
				entry = entry_end + 1;
			}
			return true;
		}

		bool reg_key::write_value(const TCHAR *name, double in_value)
		{
			return write_value(name, &in_value, sizeof(in_value), REG_BINARY);
		}

		bool reg_key::write_value(const TCHAR * name, const TCHAR* in_value) 
		{
			return write_value(name, in_value, static_cast<DWORD>(sizeof(*in_value) * (_tcslen(in_value) + 1)), REG_SZ);
		}

		bool reg_key::write_value(const TCHAR* name, const void* data, DWORD dsize, DWORD dtype) 
		{
			return ERROR_SUCCESS == ::RegSetValueEx(key_, name, 0, dtype,
				reinterpret_cast<LPBYTE>(const_cast<void*>(data)), dsize);
		}

		bool reg_key::start_watching()
		{
			if (!watch_event_)
				watch_event_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);

			DWORD filter = REG_NOTIFY_CHANGE_NAME |
				REG_NOTIFY_CHANGE_ATTRIBUTES |
				REG_NOTIFY_CHANGE_LAST_SET |
				REG_NOTIFY_CHANGE_SECURITY;

			// Watch the registry key for a change of value.
			LONG result = ::RegNotifyChangeKeyValue(key_, TRUE, filter, watch_event_, TRUE);
			if (result != ERROR_SUCCESS)
			{
				::CloseHandle(watch_event_);
				watch_event_ = 0;
			}

			return result == ERROR_SUCCESS;
		}

		bool reg_key::is_changed() 
		{
			if (watch_event_) 
			{
				if (::WaitForSingleObject(watch_event_, 0) == WAIT_OBJECT_0) 
				{
					start_watching();
					return true;
				}
			}
			return false;
		}

		bool reg_key::stop_watching()
		{
			bool result = false;
			if (watch_event_) 
			{
				::CloseHandle(watch_event_);
				watch_event_ = 0;
				result = true;
			}
			return result;
		}

		// RegistryValueIterator ------------------------------------------------------

		reg_value_iterator::reg_value_iterator(HKEY root_key, const TCHAR* folder_key)
		{
			LONG result = ::RegOpenKeyEx(root_key, folder_key, 0, KEY_READ, &key_);
			if (result != ERROR_SUCCESS)
			{
				key_ = NULL;
			} 
			else 
			{
				DWORD count = 0;
				result = ::RegQueryInfoKey(key_, NULL, 0, NULL, NULL, NULL, NULL, &count,
					NULL, NULL, NULL, NULL);

				if (result != ERROR_SUCCESS) 
				{
					::RegCloseKey(key_);
					key_ = NULL;
				} 
				else 
				{
					index_ = count - 1;
				}
			}

			read();
		}

		reg_value_iterator::~reg_value_iterator() 
		{
			if (key_)
				::RegCloseKey(key_);
		}

		std::uint32_t reg_value_iterator::value_cnt() const 
		{
			unsigned long count = 0;
			LONG result = ::RegQueryInfoKey(key_, NULL, 0, NULL, NULL, NULL, NULL,
				&count, NULL, NULL, NULL, NULL);
			if (result != ERROR_SUCCESS)
				return 0;

			return count;
		}

		bool reg_value_iterator::valid() const 
		{
			return key_ != NULL && index_ >= 0;
		}

		void reg_value_iterator::operator++()
		{
			--index_;
			read();
		}

		bool reg_value_iterator::read()
		{
			if (valid()) 
			{
				DWORD ncount = sizeof(name_) / sizeof(name_[0]);
				value_size_ = sizeof(value_);
				LONG r = ::RegEnumValue(key_, index_, name_, &ncount, NULL, &type_,
					reinterpret_cast<BYTE*>(value_), &value_size_);
				if (ERROR_SUCCESS == r)
					return true;
			}

			name_[0] = '\0';
			value_[0] = '\0';
			value_size_ = 0;
			return false;
		}

		// RegistryKeyIterator --------------------------------------------------------

		reg_key_iterator::reg_key_iterator(HKEY root_key, const TCHAR* folder_key)
		{
			LONG result = ::RegOpenKeyEx(root_key, folder_key, 0, KEY_READ, &key_);
			if (result != ERROR_SUCCESS) 
			{
				key_ = NULL;
			} 
			else 
			{
				DWORD count = 0;
				LONG result = ::RegQueryInfoKey(key_, NULL, 0, NULL, &count, NULL, NULL,
					NULL, NULL, NULL, NULL, NULL);

				if (result != ERROR_SUCCESS) 
				{
					::RegCloseKey(key_);
					key_ = NULL;
				} 
				else 
				{
					index_ = count - 1;
				}
			}

			read();
		}

		reg_key_iterator::~reg_key_iterator() 
		{
			if (key_)
				::RegCloseKey(key_);
		}

		std::uint32_t reg_key_iterator::sub_key_cnt() const
		{
			unsigned long count = 0;
			LONG result = ::RegQueryInfoKey(key_, NULL, 0, NULL, &count, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL);
			if (result != ERROR_SUCCESS)
				return 0;

			return count;
		}

		bool reg_key_iterator::valid() const 
		{
			return key_ != NULL && index_ >= 0;
		}

		void reg_key_iterator::operator++() 
		{
			--index_;
			read();
		}

		bool reg_key_iterator::read() 
		{
			if (valid())
			{
				DWORD ncount = sizeof(name_) / sizeof(name_[0]);
				FILETIME written;
				LONG r = ::RegEnumKeyEx(key_, index_, name_, &ncount, NULL, NULL,
					NULL, &written);
				if (ERROR_SUCCESS == r)
					return true;
			}

			name_[0] = _T('\0');
			return false;
		}
	}
}

