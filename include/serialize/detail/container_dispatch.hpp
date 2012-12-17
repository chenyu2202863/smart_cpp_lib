#ifndef __SERIALIZE_CONTAINER_DISPATCH_HPP
#define __SERIALIZE_CONTAINER_DISPATCH_HPP



#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <stack>
#include <queue>
#include <deque>


namespace serialize
{
	// forward declare
	template < typename CharT, typename OutT >
	class serialize_t;


	namespace detail
	{




		// -----------------------------------------------------------------
		// class ValueTraits

		template< typename ValTraitsT >
		class value_traits_t
		{
		public:
			typedef ValTraitsT								ValTraitsType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const ValTraitsType& val)
			{
				io << val;
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, ValTraitsType &val)
			{
				io >> val;
			}
		};



		template< typename FirstT, typename SecondT >
		class value_traits_t<std::pair<FirstT, SecondT>>
		{
		public:
			typedef std::pair<FirstT, SecondT>	PairTraitsType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const PairTraitsType& val)
			{
				value_traits_t<PairTraitsType::first_type>::push_dispatch(io, val.first);
				value_traits_t<PairTraitsType::second_type>::push_dispatch(io, val.second);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, PairTraitsType &val)
			{
				value_traits_t<PairTraitsType::first_type>::pop_dispatch(io, val.first);
				value_traits_t<PairTraitsType::second_type>::pop_dispatch(io, val.second);
			}
		};


		// 关联容器 -- map, multimap, set, multiset

		// -----------------------------------------------------------------
		// class associative_type_traits_t

		template< typename AssocTypeT >
		class associative_type_traits_t
		{
		public:
			typedef typename AssocTypeT::size_type		size_type;
			typedef typename AssocTypeT::key_type		key_type;
			typedef typename AssocTypeT::mapped_type	mapped_type;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const AssocTypeT& val)
			{
				value_traits_t<size_type>::push_dispatch(io, val.size());

				for(typename AssocTypeT::const_iterator iter = val.begin(); 
					iter != val.end(); ++iter)
				{
					value_traits_t<key_type>::push_dispatch(io, iter->first);
					value_traits_t<mapped_type>::push_dispatch(io, iter->second);
				}
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, AssocTypeT &val)
			{
				size_type valSize = 0;
				value_traits_t<size_type>::pop_dispatch(io, valSize);

				for(; valSize > 0; --valSize)
				{
					key_type key;
					value_traits_t<key_type>::pop_dispatch(io, key);

					mapped_type value;
					value_traits_t<mapped_type>::pop_dispatch(io, value);

					val.insert(std::make_pair(key, value));
				}
			}
		};


		// 线性容器--list, vector, deque

		// -----------------------------------------------------------------
		// class sequence_traits_t

		template< typename SequenceTypeT >
		class sequence_traits_t
		{
		public:
			typedef typename SequenceTypeT::size_type	size_type;	
			typedef typename SequenceTypeT::value_type	value_type;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const SequenceTypeT &val)
			{
				value_traits_t<size_type>::push_dispatch(io, val.size());

				for(SequenceTypeT::const_iterator iter = val.begin(); 
					iter != val.end(); ++iter)
				{
					value_traits_t<value_type>::push_dispatch(io, *iter);
				}
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, SequenceTypeT& val)
			{
				size_type valSize = 0;
				value_traits_t<size_type>::pop_dispatch(io, valSize);


				for(; valSize > 0; --valSize)
				{
					value_type element;
					value_traits_t<value_type>::pop_dispatch(io, element);

					val.push_back(element);
				}
			}
		};


		// STL map datatype serializer class.

		template< typename KeyT, typename DataT, typename LessT, typename AllocT > 
		class value_traits_t<std::map<KeyT, DataT, LessT, AllocT>> 
		{
		public:
			typedef std::map<KeyT, DataT, LessT, AllocT>	MapTraitType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const MapTraitType &val)
			{
				associative_type_traits_t<MapTraitType>::push_dispatch(io, val);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, MapTraitType& val)
			{
				associative_type_traits_t<MapTraitType>::pop_dispatch(io, val);
			}
		};


		// STL multimap datatype serializer class.

		template< typename KeyT, typename DataT, typename LessT, typename AllocT > 
		class value_traits_t<std::multimap<KeyT, DataT, LessT, AllocT>> 
		{
		public:
			typedef std::multimap<KeyT, DataT, LessT, AllocT> MultimapTraitType;


		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const MultimapTraitType &val)
			{
				associative_type_traits_t<MultimapTraitType>::push_dispatch(io, val);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, MultimapTraitType& val)
			{
				associative_type_traits_t<MultimapTraitType>::pop_dispatch(io, val);
			}
		};



		// STL set datatype serializer class.

		template< typename KeyT, typename LessT, typename AllocT > 
		class value_traits_t<std::set<KeyT, LessT, AllocT>> 
		{
		public:
			typedef std::set<KeyT, LessT, AllocT>		SetTratiType;
			typedef typename SetTratiType::size_type	size_type;
			typedef typename SetTratiType::value_type	value_type;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const SetTratiType& val)
			{
				value_traits_t<size_type>::push_dispatch(io, val.size());

				for(typename SetTratiType::const_iterator iter = val.begin(); 
					iter != val.end(); ++iter)
				{
					value_traits_t<value_type>::push_dispatch(io, *iter);
				}
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, SetTratiType &val)
			{
				size_type valSize = 0;
				value_traits_t<size_type>::pop_dispatch(io, valSize);

				for(; valSize > 0; --valSize)
				{
					value_type key;
					value_traits_t<value_type>::pop_dispatch(io, key);

					val.insert(key);
				}
			}
		};

		// STL multiset datatype serializer class.

		template<typename KeyT, typename LessT, typename AllocT > 
		class value_traits_t<std::multiset<KeyT, LessT, AllocT>> 
		{
		public:
			typedef std::multiset<KeyT, LessT, AllocT>		MultisetTraitType;
			typedef typename MultisetTraitType::size_type	size_type;
			typedef typename MultisetTraitType::value_type	value_type;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const MultisetTraitType& val)
			{
				value_traits_t<size_type>::push_dispatch(io, val.size());

				for(typename MultisetTraitType::const_iterator iter = val.begin(); 
					iter != val.end(); ++iter)
				{
					value_traits_t<value_type>::push_dispatch(io, *iter);
				}
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, MultisetTraitType &val)
			{
				size_type valSize = 0;
				value_traits_t<size_type>::pop_dispatch(io, valSize);

				for(; valSize > 0; --valSize)
				{
					value_type key;
					value_traits_t<value_type>::pop_dispatch(io, key);

					val.insert(key);
				}
			}
		};


		// STL list datatype serializer class.

		template< typename KeyT, typename AllocT > 
		class value_traits_t<std::list<KeyT, AllocT>>
		{
		public:
			typedef std::list<KeyT, AllocT> ListTraitType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const ListTraitType &val)
			{
				sequence_traits_t<ListTraitType>::push_dispatch(io, val);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, ListTraitType &val)
			{
				sequence_traits_t<ListTraitType>::pop_dispatch(io, val);
			}
		};

		//STL vector datatype serializer class.

		template< typename KeyT, typename AllocT > 
		class value_traits_t<std::vector<KeyT, AllocT>>
		{
		public:
			typedef std::vector<KeyT, AllocT> VectorTraitType;


		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const VectorTraitType &val)
			{
				sequence_traits_t<VectorTraitType>::push_dispatch(io, val);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, VectorTraitType &val)
			{
				sequence_traits_t<VectorTraitType>::pop_dispatch(io, val);
			}
		};


		// STL deque datatype serializer class.

		template< typename KeyT, typename AllocT >
		class value_traits_t<std::deque<KeyT, AllocT>>
		{
		public:
			typedef std::deque<KeyT, AllocT> DequeTraitType;


		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const DequeTraitType &val)
			{
				sequence_traits_t<DequeTraitType>::push_dispatch(io, val);
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, DequeTraitType &val)
			{
				sequence_traits_t<DequeTraitType>::pop_dispatch(io, val);
			}
		};

		template< typename ValueT, typename ContainerT >
		class value_traits_t<std::queue<ValueT, ContainerT>>
		{
		public:
			typedef std::queue<ValueT, ContainerT> QueueTraitType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const QueueTraitType &val)
			{
				static_assert(false, "no impl");
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, QueueTraitType &val)
			{
				static_assert(false, "no impl");
			}
		};

		template< typename ValueT, typename ContainerT >
		class value_traits_t<std::stack<ValueT, ContainerT>>
		{
		public:
			typedef std::stack<ValueT, ContainerT> StackTraitType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const StackTraitType &val)
			{
				static_assert(false, "no impl");
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, StackTraitType &val)
			{
				static_assert(false, "no impl");
			}
		};

		template< typename ValueT, typename ContainerT, typename LessT >
		class value_traits_t<std::priority_queue<ValueT, ContainerT, LessT>>
		{
		public:
			typedef std::priority_queue<ValueT, ContainerT, LessT> PriorityQueueTraitType;

		public:
			template < typename CharT, typename OutT >
			static void push_dispatch(serialize_t<CharT, OutT> &io, const PriorityQueueTraitType &val)
			{
				static_assert(false, "no impl");
			}

			template < typename CharT, typename OutT >
			static void pop_dispatch(serialize_t<CharT, OutT> &io, PriorityQueueTraitType &val)
			{
				static_assert(false, "no impl");
			}
		};
	}
}







#endif