#pragma once
#ifndef MY_LIB_LIST
#define MY_LIB_LIST

#include <memory>
#include <type_traits>
#include <iterator>
#include <utility>
#include <new>
#include <initializer_list>
#include <limits>
#include <cassert>

// Check for C++17
#ifdef _HAS_CXX17
#if _HAS_CXX17
#define STD_CXX17 1
#else
#define STD_CXX17 0
#endif
#elif __cplusplus == 201703L
#define STD_CXX17 1
#else
#define STD_CXX17 0
#endif
static_assert(STD_CXX17, "my_lib::list requires c++17");

namespace my_lib 
{
	/* 
	 * Structure of this file: 
	 * list_const_iterator
	 * list_iterator 
	 * list_node
	 * initializer_tag (for default initialize)
	 * list 
	 */
	
	 // Probably there are also will be a unchecked iterators 

	// CHECKME
	template <class MyList>
	class list_const_iterator
	{
	// type aliases
	public:
		using iterator_category = std::bidirectional_iterator_tag;

		using nodeptr			= typename MyList::nodeptr;
		using value_type		= typename MyList::value_type;
		using pointer			= typename MyList::const_pointer;
		using reference			= typename MyList::const_reference;
		using difference_type	= typename MyList::difference_type;

	private:
		const MyList* mylist_;
		nodeptr ptr_;
	
	// Ctors
	public:
		list_const_iterator(const MyList* list, const nodeptr ptr) noexcept : mylist_{ list }, ptr_{ ptr }{}
		
		list_const_iterator() noexcept = default;

	// helpers
	private:
		void range_verify() const noexcept
		{
			assert(ptr_ != mylist_->head_ && "past the end iterator");

			bool flag{};
			for (auto node = const_cast<nodeptr>(mylist_->head_->next_); node != mylist_->head_; node = node->next_) {
				if (ptr_ == node) {
					flag = true;
					break;
				}
			}
			assert("out of range iterator");
		}

		void is_comparable(const list_const_iterator& rhs) const noexcept
		{
			assert(mylist_ == rhs.mylist_ && "iterators incomparable");
		}

	// Access 
	public:
		[[nodiscard]] reference operator*() const noexcept
		{
			range_verify();
			return ptr_->value_;
		}
		
		[[nodiscard]] pointer operator->() const noexcept
		{
			range_verify();
			return std::pointer_traits<pointer>::pointer_to(**this);
		}

	// Increment / decrement
	public:
		list_const_iterator& operator++() noexcept
		{
			range_verify();
			ptr_ = ptr_->next_;
			return *this;
		}

		list_const_iterator operator++(int) noexcept
		{
			auto tmp{ *this };
			++* this;
			return tmp;
		}


		list_const_iterator& operator--() noexcept
		{
			range_verify();
			ptr_ = ptr_->prev_;
			return *this;
		}

		list_const_iterator operator--(int) noexcept
		{
			auto tmp{ *this };
			--* this;
			return tmp;
		}

	// Compare
	public:
		[[nodiscard]] bool operator ==(const list_const_iterator& rhs) const noexcept
		{
			is_comparable(rhs);
			return ptr_ == rhs.ptr_;
		}

		[[nodiscard]] bool operator !=(const list_const_iterator& rhs) const noexcept
		{
			return !(*this == rhs);
		}
	};

	// CHECKME
	template <class MyList>
	class list_iterator : public list_const_iterator<MyList>
	{
	// type aliases
	public:
		using mybase			= list_const_iterator<MyList>;
		using mybase::mybase; // ctors

		using nodeptr			= typename MyList::nodeptr;
		using value_type		= typename MyList::value_type;
		using pointer			= typename MyList::pointer;
		using reference			= typename MyList::reference;
		using difference_type	= typename MyList::difference_type;

	// public access
	public:
		[[nodiscard]] reference operator*() const noexcept
		{
			return const_cast<reference>(mybase::operator*());
		}

		[[nodiscard]] pointer operator->() const noexcept
		{
			return const_cast<pointer>(mybase::operator->());
		}

	// increment / decrement
	public:
		list_iterator& operator++() noexcept
		{
			mybase::operator++();
			return *this;
		}

		list_iterator operator++(int) noexcept
		{
			auto tmp{ *this };
			++* this;
			return tmp;
		}


		list_iterator& operator--() noexcept
		{
			mybase::operator--();
			return *this;
		}

		list_iterator operator--(int) noexcept
		{
			auto tmp{ *this };
			--* this;
			return tmp;
		}

	// compare
	public:
		using mybase::operator==;
		using mybase::operator!=;
	};

	// FIXME: try me bitch
	template <class T, class Pointer>
	struct list_node
	{
		// type aliases
		using value_type = T;
		using nodeptr = typename std::pointer_traits<Pointer>::template rebind<list_node>; // I am using the rebind because I cannot know what is will be used as pointer
		// for example, if smone will use unique_ptrs

		// data
		nodeptr		next_; // next node, or first if head
		nodeptr		prev_; // previous node, or last if head
		value_type	value_; // the stored value, unused if head

		// This is dummy realization, but I don't know how to fix it (there are should be singleton)
		template <class...Args>
		list_node(nodeptr next, nodeptr prev, Args&&... args) : next_{ next },
																prev_{ prev },
																value_{ std::forward<Args>(args)... }{}

		// copying
		list_node(const list_node&)				= delete;
		list_node& operator=(const list_node&)	= delete;
		
	// Create and destroy functions
	public:
		template <class NodeAlloc>
		[[nodiscard]] static nodeptr create_head(NodeAlloc& allocator)
		{
			nodeptr head = allocator.allocate(1);
			::new (&head->next_) nodeptr(head);
			::new (&head->prev_) nodeptr(head);
			
			return head;
		}

		template <class NodeAlloc>
		static void free_without_value(NodeAlloc& allocator, nodeptr ptr) noexcept
		{
			ptr->next_.~nodeptr();
			ptr->prev_.~nodeptr();
			std::allocator_traits<NodeAlloc>::deallocate(allocator, ptr, 1);
		}

		template <class NodeAlloc>
		static void free_node(NodeAlloc& allocator, nodeptr ptr) noexcept
		{
			std::allocator_traits<NodeAlloc>::destroy(allocator, &ptr->value_);
			free_without_value(allocator, ptr);
		}

		template <class NodeAlloc>
		static void free_all_nonhead(NodeAlloc& allocator, nodeptr head) noexcept
		{
			head->prev_->next_ = nullptr; // deleting from first to nullptr

			auto node = head->next_;
			for (nodeptr next; node; node = next) {
				next = node->next_;
				free_node(allocator, node);
			}
		}
	};

	struct initializer_tag {};

	/* Implemented:
	|-----------------------------------|
	|									|
	|* Constructors:					|
	| list()							|
	| list(std::initializer_list<T>)	|
	|* Destructor						|
	|-----------------------------------|	
	
	|-----------------------------------|
	|* Member functions:				|
	| operator=(const list&)			|
	| operator=(std::initializer_list<T>|
	| assign(Iter, const Iter)			|
	| assign(std::initializer_list<T>)	|
	| get_allocator()					|
	|-----------------------------------|
	
	|-----------------------------------|
	|* Iterators:						|
	| begin() (2 overloads)				|
	| end() (2 overloads)				|
	| cbegin()							|
	| cend()							|
	| rbegin() (2 overloads)			|
	| rend() (2 overloads)				|
	| crbegin()							|
	| crend()							|
	|-----------------------------------|

	|-----------------------------------|
	|* Capacity:						|
	| empty()							|
	| size()							|
	| max_size()						|				
	|-----------------------------------|
	*/
	// list_node head will store first element(next_) and last element(prev_) 
	template <class T, class Alloc = std::allocator<T>>
	class list
	{
		friend list_const_iterator<list<T, Alloc>>;
		// type aliases
	public:
		// value type aliases
		using value_type				= T;
		using pointer					= T*;
		using const_pointer				= const T*;
		using reference					= T&;
		using const_reference			= const T&;
		using size_type					= std::size_t;
		using difference_type			= std::ptrdiff_t;

		// allocator types aliases and node pointer aliases
		using allocator_type			= Alloc;
		using list_allocator			= typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
		using list_allocator_traits		= std::allocator_traits<list_allocator>;
		using node_allocator_type		= typename std::allocator_traits<Alloc>::template rebind_alloc<list_node<T, typename std::allocator_traits<Alloc>::void_pointer>>;
		using node_allocator_traits		= std::allocator_traits<node_allocator_type>;
		using node_type					= list_node<T, typename node_allocator_traits::void_pointer>;
		using nodeptr					= typename node_allocator_traits::pointer;

		// iterator aliases
		using iterator					= list_iterator<list<T, Alloc>>;
		using const_iterator			= list_const_iterator<list<T, Alloc>>;

		using reverse_iterator			= std::reverse_iterator<iterator>;
		using const_reverse_iterator	= std::reverse_iterator<const_iterator>;

		// list data
	private:
		node_allocator_type allocator_;
		nodeptr head_;
		size_type size_;

		// Ctors and dtor
	public:
		list() noexcept : head_{}, size_{}, allocator_{} {}

	private:
		// helper initialize ctor
		list(const allocator_type& alloc,
			size_type size) : allocator_{ static_cast<node_allocator_type>(alloc) },
			head_{ node_type::create_head(allocator_) },
			size_{ size }	{ }

	private:
		template <class Iter>
		void construct_range_unchecked(Iter first, const Iter last, nodeptr where)
		{
			if (first == last) return;

			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where, where->prev_, *first);
				where->prev_->next_ = node;
				where->prev_ = node;
				++first;
			}
		}

		void construct_range_unchecked(nodeptr first, const nodeptr last, nodeptr where)
		{
			if (first == last) return;

			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where, where->prev_, first->value_);
				where->prev_->next_ = node;
				where->prev_ = node;
				first = first->next_;
			}
		}

		void erase_unchecked(nodeptr first, nodeptr last)
		{
			first->prev_->next_ = last;
			last->prev_ = first->prev_;
			for (auto current = first->next_; first != last; first = current, current = current->next_) {
				node_type::free_node(allocator_, first);
			}
		}

	public:
		list(std::initializer_list<value_type> list, 
			 const allocator_type& allocator = allocator_type{}) : list(allocator, list.size())
		{
			construct_range_unchecked(list.begin(), list.end(), head_);
		}

		~list() noexcept
		{
			if (head_ != nullptr) {
				node_type::free_all_nonhead(allocator_, head_);
				node_type::free_without_value(allocator_, head_);
			}
		}

	// operator=, assign, get_allocator
	public:
		list& operator=(const list& list)
		{
			auto rhs_size	= list.size_;
			auto rhs_head	= list.head_;
			auto rhs_node	= rhs_head->next_;
			auto node		= head_->next_;
			if (size_ < rhs_size || size_ == rhs_size) {
				for (; node != head_; rhs_node = rhs_node->next_, node = node->next_) {
					node->value_ = rhs_node->value_;
				}

				// if size != list.size_ append remain elements
				if (size_ < rhs_size) {
					construct_range_unchecked(rhs_node, rhs_head, head_);
				}
			}
			else {
				for (; rhs_node != rhs_head; rhs_node = rhs_node->next_, node = node->next_) {
					node->value_ = rhs_node->value_;
				}
				erase_unchecked(node, head_);
			}
			size_ = list.size_;
			
			return *this;
		}

		// FIXME: same code as operator= (list)
		list& operator=(std::initializer_list<T> ilist) 
		{
			auto ilbeg = ilist.begin();
			auto ilsize = ilist.size();
			if (auto node = head_->next_;  size_ <= ilsize) {
				for (; node != head_; node = node->next_) {
					node->value_ = *(ilbeg++);
				}

				if (size_ < ilsize) {
					construct_range_unchecked(ilbeg, ilist.end(), head_);
				}
			}
			else {
				for (std::size_t i{}; i < ilsize; ++i, node = node->next_) {
					node->value_ = *(ilbeg++);
				}

				erase_unchecked(node, head_);
			}

			size_ = ilsize;

			return *this;
		}

		template <class Iter>
		void assign(Iter first, const Iter last)
		{
			auto new_size = std::distance(first, last);
			if (auto node = head_->next_; size_ <= new_size) {
				for (; node != head_; node = node->next_) {
					node->value_ = *(first++);
				}

				if (size_ < new_size) {
					construct_range_unchecked(first, last, head_);
				}
			}
			else {
				for (std::size_t i{}; i < new_size; ++i, node = node->next_) {
					node->value_ = *(first++);
				}

				erase_unchecked(node, head_);
			}
		}

		void assing(std::initializer_list<T> ilist)
		{
			*this = ilist;
		}
		
		[[nodiscard]] allocator_type get_allocator() const noexcept
		{
			return static_cast<allocator_type>(allocator_);
		}

	// Iterators
	public:
		[[nodiscard]] iterator begin() noexcept
		{
			return iterator(this, head_->next_);
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
			return const_iterator(this, head_->next_);
		}

		[[nodiscard]] iterator end() noexcept
		{
			return iterator(this, head_);
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
			return const_iterator(this, head_);
		}


		[[nodiscard]] const_iterator cbegin() const noexcept
		{
			return const_iterator(this, head_->next_);
		}

		[[nodiscard]] const_iterator cend() const noexcept
		{
			return const_iterator(this, head_);
		}


		[[nodiscard]] reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		[[nodiscard]] const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		[[nodiscard]] const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
		

		[[nodiscard]] const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

	// Capacity
	public:
		[[nodiscard]] bool empty() const noexcept
		{
			return size_ == 0;
		}

		[[nodiscard]] size_type size() const noexcept
		{
			return size_;
		}

		[[nodiscard]] size_type max_size() const noexcept
		{
			auto diff_max = static_cast<size_type>(std::numeric_limits<difference_type>::max());
			auto alnode_max = static_cast<size_type>(node_allocator_traits::max_size(allocator_));
			if (diff_max < alnode_max) return diff_max;
			return alnode_max;
		}
	};
}

#endif