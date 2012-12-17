#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

// Utility class to read, write and manipulate the Windows Registry.
// Registry vocabulary primer: a "key" is like a folder, in which there
// are "values", which are <name, data> pairs, with an associated data type.
//
// Note:
// ReadValue family of functions guarantee that the return arguments
// are not touched in case of failure.


namespace win32
{
	namespace reg
	{
		class reg_key
		{
		private:
			HKEY key_;  // The registry key being iterated.
			HANDLE watch_event_;

		public:
			reg_key();
			reg_key(HKEY rootkey, const TCHAR* subkey, REGSAM access);
			~reg_key();

		public:
			bool create(HKEY rootkey, const TCHAR* subkey, REGSAM access);

			bool create(HKEY rootkey, const TCHAR* subkey, DWORD* disposition, REGSAM access);

			// Creates a subkey or open it if it already exists.
			bool create_key(const TCHAR* name, REGSAM access);

			// Opens an existing reg key.
			bool open(HKEY rootkey, const TCHAR* subkey, REGSAM access);

			// Opens an existing reg key, given the relative key name.
			bool open(const TCHAR* relative_key_name, REGSAM access);

			// Closes this reg key.
			void close();

			// Returns false if this key does not have the specified value, of if an error
			// occurrs while attempting to access it.
			bool has_value(const TCHAR* value_name) const;

			// Returns the number of values for this key, of 0 if the number cannot be
			// determined.
			std::uint32_t get_value_cnt() const;

			// Determine the nth value's name.
			bool get_value_name_at(int index, std::basic_string<TCHAR>* name) const;

			// True while the key is valid.
			bool valid() const { return key_ != NULL; }

			// Kill a key and everything that live below it; please be careful when using
			// it.
			bool delete_key(const TCHAR* name);

			// Deletes a single value within the key.
			bool delete_value(const TCHAR* name);

			// Getters:

			template < typename T >
			bool read_value(const TCHAR *name, T &out_value) const;

			bool read_value(const TCHAR *name, double &out_value);
			
			// Returns a string value. If |name| is NULL or empty, returns the default
			// value, if any.
			bool read_value(const TCHAR* name, std::basic_string<TCHAR> &out_value) const;
			// Reads a REG_MULTI_SZ registry field into a vector of strings. Clears
			// |values| initially and adds further strings to the list. Returns
			// ERROR_CANTREAD if type is not REG_MULTI_SZ.
			bool read_values(const TCHAR* name, std::vector<std::basic_string<TCHAR>> &out_values) const;

			// Returns raw data. If |name| is NULL or empty, returns the default
			// value, if any.
			bool read_value(const TCHAR* name, void* data, DWORD *dsize, DWORD *dtype) const;

			// Setters:

			template < typename T >
			bool write_value(const TCHAR *name, const T &in_val);

			bool write_value(const TCHAR *name, double in_value);

			// Sets a string value.
			bool write_value(const TCHAR* name, const TCHAR* in_value);

			// Sets raw data, including type.
			bool write_value(const TCHAR* name, const void* data, DWORD dsize, DWORD dtype);

			// Starts watching the key to see if any of its values have changed.
			// The key must have been opened with the KEY_NOTIFY access privilege.
			bool start_watching();

			// If StartWatching hasn't been called, always returns false.
			// Otherwise, returns true if anything under the key has changed.
			// This can't be const because the |watch_event_| may be refreshed.
			bool is_changed();

			// Will automatically be called by destructor if not manually called
			// beforehand.  Returns true if it was watching, false otherwise.
			bool stop_watching();

			inline bool is_watching() const { return watch_event_ != 0; }
			HANDLE watch_event() const { return watch_event_; }
			HKEY native_handle() const { return key_; }
			operator HKEY() const { return native_handle(); }
		};

		// Iterates the entries found in a particular folder on the registry.
		// For this application I happen to know I wont need data size larger
		// than MAX_PATH, but in real life this wouldn't neccessarily be
		// adequate.
		class reg_value_iterator 
		{
		public:
			reg_value_iterator(HKEY root_key, const TCHAR* folder_key);

			~reg_value_iterator();

			std::uint32_t value_cnt() const;

			// True while the iterator is valid.
			bool valid() const;

			// Advances to the next registry entry.
			void operator++();

			const TCHAR* name() const { return name_; }
			const TCHAR* value() const { return value_; }
			std::uint32_t value_size() const { return value_size_; }
			std::uint32_t type() const { return type_; }

			int index() const { return index_; }

		private:
			// Read in the current values.
			bool read();

			// The registry key being iterated.
			HKEY key_;

			// Current index of the iteration.
			int index_;

			// Current values.
			TCHAR name_[MAX_PATH];
			TCHAR value_[MAX_PATH];
			DWORD value_size_;
			DWORD type_;
		};

		class reg_key_iterator 
		{
		public:
			reg_key_iterator(HKEY root_key, const TCHAR* folder_key);

			~reg_key_iterator();

			std::uint32_t sub_key_cnt() const;

			// True while the iterator is valid.
			bool valid() const;

			// Advances to the next entry in the folder.
			void operator++();

			const TCHAR* name() const { return name_; }

			int index() const { return index_; }

		private:
			// Read in the current values.
			bool read();

			// The registry key being iterated.
			HKEY key_;

			// Current index of the iteration.
			int index_;

			TCHAR name_[MAX_PATH];
		};


		template < typename T >
		bool reg_key::write_value(const TCHAR *name, const T &in_value)
		{
			static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, 
				"T must be a pod type");

			DWORD in = *reinterpret_cast<const DWORD *>(&in_value);
			return write_value(name, &in_value, static_cast<DWORD>(sizeof(in)), REG_DWORD);
		}



		template < typename T >
		bool reg_key::read_value(const TCHAR *name, T &out_value) const
		{
			static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, 
				"T must be a pod type");

			DWORD type = REG_DWORD;

			DWORD local_value(0);
			DWORD size = sizeof(local_value);
			bool result = read_value(name, &local_value, &size, &type);
			if (result) 
			{
				if( (type == REG_DWORD || type == REG_BINARY || type == REG_QWORD ) && 
					size == sizeof(local_value) )
					out_value = *reinterpret_cast<T *>(&local_value);
				else
					result = false;
			}

			return result;
		}
	}
}


#endif  // BASE_WIN_REGISTRY_H_
