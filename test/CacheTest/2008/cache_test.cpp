#include "stdafx.h"
#include "unit_test/tut/tut/tut.hpp"

#include "cache/cache.hpp"
#include "extend_stl/string/algorithm.hpp"

#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <vector>
#include <limits>
#include <random>




namespace tut
{
	struct cache_test
	{};

	typedef test_group<cache_test> tf;
	typedef tf::object object;
	tf cache_test_group("cache_test");

	std::string Create(const int &key)
	{
		return stdex::to_string(key);
	}

	template < typename T, typename U >
	void element_erase(const T &key, const U &value)
	{
		std::ostringstream os;
		os << "erase key: " << key << "\t value:" << value << std::endl; 
	
		::OutputDebugStringA(os.str().c_str());
	}

	template<>
	template<>
	void object::test<1>()
	{
		set_test_name("lru");

		typedef cache::cache_t<int, std::string, cache::lru_t> Cache;

		Cache cache(10);

		for(size_t i = 0; i != 10; ++i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());

		for(size_t i = 10; i != 0; --i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());

	}

	template<>
	template<>
	void object::test<2>()
	{
		set_test_name("lfu");

		typedef cache::cache_t<int, std::string, cache::lfu_t> Cache;

		Cache cache(10);

		for(size_t i = 0; i != 10; ++i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());

		for(size_t i = 10; i != 0; --i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());
	}


	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("fifo");

		typedef cache::cache_t<int, std::string, cache::fifo_t> Cache;

		Cache cache(10);

		for(size_t i = 0; i != 10; ++i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());

		for(size_t i = 10; i != 0; --i)
		{
			std::pair<const std::string, bool> ret = cache.get(i);
			if( !ret.second )
				cache.insert(i, Create(i)/*, &element_erase<int, std::string>*/);

			//std::cout << ret.first << std::endl;
		}

		ensure(cache.size() == cache.capacity());
	}


	template < typename T >
	T *allocator(const size_t &size)
	{
		return (char *)::operator new(size);
	}

	template < typename T >
	void deletor(T *p)
	{
		::operator delete(p);
	}

	std::tr1::shared_ptr<char> CreateBuffer(const __int64 &key)
	{
		std::tr1::shared_ptr<char> buf(allocator<char>(0x1000), &deletor<char>);

		return buf;
	}


	struct ScopeTimer
	{
		DWORD start_;
		DWORD stop_;
		ScopeTimer()
			: start_(0)
			, stop_(0)
		{
			start_ = ::GetTickCount();
		}
		~ScopeTimer()
		{
			stop_ = ::GetTickCount();
			std::cout << "--- " << stop_ - start_ << " ---" << std::endl;
		}
	};
	ptrdiff_t myrandom (ptrdiff_t i) { return rand()%i;}
	// Functions to create collections of test keys 

	// General pattern 
	template <typename K> 
	std::vector<K> make_keys(size_t); 

	// Integer specialization: 
	// generate n unique integer keys 
	template <> 
	std::vector<int> make_keys<int>(size_t n) 
	{ 
		std::tr1::mt19937 eng;  // a core engine class£ºMersenne Twister generator
		//std::tr1::normal_distribution<double> dist; 
		std::tr1::uniform_int<int>  unif(0, n * 20); 

		std::set<int> r; 
		do 
		{ 
			r.insert(static_cast<int>(unif(eng)));
		} while (r.size()<n); 
		return std::vector<int>(r.begin(),r.end()); 
	} 

	// String specialization: 
	// generate n unique long strings 
	template <> 
	std::vector<std::wstring> make_keys<std::wstring>(size_t n) 
	{ 
		assert(n < std::numeric_limits<size_t>::max());

		// Sufficiently long key lengths favour comparator 
		// based containers over hashed 
		const size_t keylength=256; 

		std::set<std::wstring> r; 
		do { 
			std::wstring s; 
			for (size_t i=0;i<keylength;++i) 
				s+=static_cast<wchar_t>(1+ rand() % (n * 10)); 
			r.insert(s); 
		} while(r.size()<n); 
		return std::vector<std::wstring>(r.begin(),r.end()); 
	} 


	// Test the cache object (capacity n) with k keys 
	// and a accesses.  Benchmark adapts to CACHE::key_type, 
	// but expects an int-like CACHE::value_type 
	template <typename CACHE> 
	void benchmark( 
		CACHE& lru, 
		size_t n, 
		size_t k, 
		size_t a 
		) 
	{ 

		typedef typename CACHE::key_type key_type; 

		// The set of m keys to be used for testing 
		std::vector<key_type> keys = make_keys<key_type>(k); 

		// Create a random access sequence plus some "primer" 
		std::vector<key_type> seq; 
		for (size_t i=0;i<a+n;++i) 
			seq.push_back(keys[i%k]); 
		std::random_shuffle(seq.begin(),seq.end(),myrandom); 


		// Setup timing 
		ScopeTimer timer;
		std::cout << k << ": ";


		for (size_t i=0;i<n;++i) 
		{
			std::pair<const int, bool> ret = lru.get(seq[i]);
			if( !ret.second )
				lru.insert(seq[i], i, &element_erase<key_type, typename CACHE::value_type>);
		}


		int t=0; 
		for(std::vector<key_type>::const_iterator it = seq.begin()+n; it!=seq.end(); ++it) 
		{
			std::pair<const int, bool> ret = lru.get(*it);
			if( ret.second )
				ret.first;
		}

		// Avoid "optimised away" 
		volatile int force=0; 
		force=t; 
	} 

	// Some lightweight functions to cache 
	// for the key types of interest 

	template <typename K> 
	int fn(const K&); 

	template <> 
	int fn<int>(const int& x) 
	{ 
		return x; 
	} 

	template <> 
	int fn<std::wstring>(const std::wstring& x) 
	{ 
		return x.size(); 
	} 


	template <>
	template <>
	void object::test<4>()
	{
		set_test_name("performance");

		const size_t n = 1000; 

		typedef cache::cache_t<int, int, cache::lru_t> LRUCache;
		LRUCache lru(n); 

		std::cout  << std::endl << "lru:" << std::endl;

		// Test a variety of cache load levels 
		benchmark(lru,n,n/2    ,1024*n); 
		benchmark(lru,n,(7*n)/8,1024*n); 
		benchmark(lru,n,n      ,1024*n); 
		benchmark(lru,n, 1.2 * n,		1024*n); 
		benchmark(lru,n, 2 * n    ,1024*n); 
		benchmark(lru,n, 10 * n    ,1024*n);
		benchmark(lru,n, 100 * n    ,1024*n);
		benchmark(lru,n, 1000 * n    ,1024*n);

		

		std::cout << std::endl << "lfu:" << std::endl;

		typedef cache::cache_t<int, int, cache::lfu_t> LFUCache;
		LFUCache lru2(n); 

		// Test a variety of cache load levels 
		benchmark(lru2,n,n/2    ,1024*n); 
		benchmark(lru2,n,(7*n)/8,1024*n); 
		benchmark(lru2,n,n      ,1024*n); 
		benchmark(lru2,n, 1.2 * n,		1024*n); 
		benchmark(lru2,n, 2 * n    ,1024*n); 
		benchmark(lru2,n, 10 * n    ,1024*n);
		benchmark(lru2,n, 100 * n    ,1024*n);
		benchmark(lru2,n, 1000 * n    ,1024*n);

		

		std::cout << std::endl << "fifo:" << std::endl;
		typedef cache::cache_t<int, int, cache::fifo_t> FIFOCache;
		FIFOCache lru3(n); 

		// Test a variety of cache load levels 
		benchmark(lru3,n,n/2    ,1024*n); 
		benchmark(lru3,n,(7*n)/8,1024*n); 
		benchmark(lru3,n,n      ,1024*n); 
		benchmark(lru3,n, 1.2 * n,		1024*n); 
		benchmark(lru3,n, 2 * n    ,1024*n); 
		benchmark(lru3,n, 10 * n    ,1024*n);
		benchmark(lru3,n, 100 * n    ,1024*n);
		benchmark(lru3,n, 1000 * n    ,1024*n);

		
	}
}