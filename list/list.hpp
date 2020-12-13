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
#include "my_utilities.hpp" // my custom library

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

		using nodeptr = typename MyList::nodeptr;
		using value_type = typename MyList::value_type;
		using pointer = typename MyList::const_pointer;
		using reference = typename MyList::const_reference;
		using difference_type = typename MyList::difference_type;

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
			assert(flag && "out of range iterator");
		}

		void offset_verify(difference_type offset) const noexcept
		{
			if (ptr_ != mylist_->head_) {
				bool flag{};
				for (auto node = const_cast<nodeptr>(mylist_->head_->next_); node != mylist_->head_; node = node->next_) {
					if (ptr_ == node) {
						flag = true;
						break;
					}
				}
				assert(flag && "out of range iterator");
			}

			if (offset > 0) {
				if (mylist_->size_ == 0) {
					assert(false && "cannot increment iterator on empty container");
				}
			}
			else {
				if (mylist_->size_ == 0) {
					assert(false && "cannot decrement iterator on empty container");
				}
			}
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
			offset_verify(1);
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
			offset_verify(-1);
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

	public:
		nodeptr& get_pointer() noexcept
		{
			return ptr_;
		}

		const nodeptr& get_pointer() const noexcept
		{
			return ptr_;
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
	-------------------------------------
	|* Constructors:					|
	| list()							|
	| list(std::initializer_list<T>)	|
	|* Destructor						|
	-------------------------------------	
	
	-------------------------------------
	|* Member functions:				|
	| operator=(const list&)			|
	| operator=(list&&)					|
	| operator=(std::initializer_list<T>|
	| assign(size_type, const_reference)|
	| assign(Iter, const Iter)			|
	| assign(std::initializer_list<T>)	|
	| get_allocator()					|
	-------------------------------------
	
	-------------------------------------
	|* Element access:					|
	| front() (2 overloads)				|
	| back() (2 overloads)				|
	-------------------------------------

	-------------------------------------
	|* Iterators:						|
	| begin() (2 overloads)				|
	| end() (2 overloads)				|
	| cbegin()							|
	| cend()							|
	| rbegin() (2 overloads)			|
	| rend() (2 overloads)				|
	| crbegin()							|
	| crend()							|
	-------------------------------------

	-------------------------------------
	|* Capacity:						|
	| empty()							|
	| size()							|
	| max_size()						|				
	-------------------------------------

	-------------------------------------------------------------
	|* Modifiers:												|
	| clear()													|
	| insert(const_iterator, const_reference)					|
	| insert(const_iterator, value_type&&)						|
	| insert(const_iterator, size_type count, const_reference)	|
	| insert(const_iterator, Iter first, Iter last)				|
	| insert(const_iterator, std::initializer_list<value_type>) |
	| emplace(const iterator, Args&&...)						|
	| erase(const_iterator)										|
	| erase(const_iterator, const_iterator)						|

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
		void construct_range(Iter first, const Iter last, nodeptr where)
		{
			if (first == last) return;

			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next_, where, *first);
				where->next_->prev_= node;
				where->next_ = node;
				where = node;
				++first;
			}
		}

		void construct_range(const_iterator first, const_iterator last, nodeptr where)
		{
			if (first == last) return;

			size_type count = std::distance(first, last);
			size_type i{};
			while (i++ < count) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next_, where, *first);
				where->next_->prev_ = node;
				where->next_ = node;
				where = node;
				++first;
			}
		}

		void construct_range(iterator first, iterator last, nodeptr where)
		{
			construct_range(static_cast<const_iterator>(first), last, where);
		}

		void construct_range(nodeptr first, const nodeptr last, nodeptr where)
		{
			if (first == last) return;

			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next_, where, first->value_);
				where->next_->prev_ = node;
				where->next_ = node;
				where = node;
				first = first->next_;
			}
		}

		void construct_n_copies(size_type count, const_reference value, nodeptr where)
		{
			for (size_type i{}; i < count; ++i) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next_, where, value);
				where->next_->prev_= node;
				where->next_ = node;
				where = node;
			}
		}

		void erase_range(nodeptr first, nodeptr last)
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
			construct_range(list.begin(), list.end(), head_);
		}

	private:
		void tidy() noexcept
		{
			if (head_ != nullptr) {
				if (size_ != 0) {
					node_type::free_all_nonhead(allocator_, head_);
					node_type::free_without_value(allocator_, head_);
				}
				else {
					node_type::free_without_value(allocator_, head_);
				}
			}
		}

	public:
		~list() noexcept
		{
			tidy();
		}

	// Member functions (operator=, assign, get_allocator)
	public:
		list& operator=(const list& rhs)
		{
			auto rhs_size	= rhs.size_;
			auto rhs_head	= rhs.head_;
			auto rhs_node	= rhs_head->next_;
			auto node		= head_->next_;
			if (size_ == 0) {
				construct_range(rhs_head->next_, rhs_head, head_);
			}
			else if (size_ <= rhs_size) {
				for (; node != head_; rhs_node = rhs_node->next_, node = node->next_) {
					node->value_ = rhs_node->value_;
				}

				// if size != list.size_ append remain elements
				if (size_ < rhs_size) {
					construct_range(rhs_node, rhs_head, head_);
				}
			}
			else {
				for (; rhs_node != rhs_head; rhs_node = rhs_node->next_, node = node->next_) {
					node->value_ = rhs_node->value_;
				}
				erase_range(node, head_);
			}
			size_ = rhs.size_;
			
			return *this;
		}

		list& operator=(list&& rhs) 
		{
			if (allocator_ == rhs.allocator_) {
				tidy();
				head_		= std::move(rhs.head_); // is it reasonable?
				rhs.head_	= nullptr;
				size_		= rhs.size_;
			}
			else if (list_allocator_traits::propagate_on_container_move_assignment::value) {
				tidy(); // use old allocator to free the storage
				allocator_	= std::move(rhs.allocator_);
				head_		= node_type::create_head(allocator_);
				size_		= rhs.size_;
				construct_range(rhs.head_->next_, rhs.head_, head_); // copy all elements from rhs
			}
			else {
				assign(rhs.begin(), rhs.end());
			}

			return *this;
		}

		// FIXME: same code as operator= (list)
		list& operator=(std::initializer_list<T> ilist) 
		{
			auto ilbeg = ilist.begin();
			auto ilsize = ilist.size();
			if (size_ == 0) {
				construct_range(ilbeg, ilist.end(), head_);
			}
			if (auto node = head_->next_;  size_ <= ilsize) {
				for (; node != head_; node = node->next_) {
					node->value_ = *(ilbeg++);
				}

				if (size_ < ilsize) {
					construct_range(ilbeg, ilist.end(), head_);
				}
			}
			else {
				for (size_type i{}; i < ilsize; ++i, node = node->next_) {
					node->value_ = *(ilbeg++);
				}

				erase_range(node, head_);
			}

			size_ = ilsize;

			return *this;
		}

		template <class Iter, std::enable_if_t<is_iterator<Iter>::value, int> = 0>
		void assign(Iter first, const Iter last)
		{
			size_type new_size = std::distance(first, last);
			if (size_ == 0) {
				construct_range(first, last, head_);
			}
			else if (auto node = head_->next_; size_ <= new_size) {
				for (; node != head_; node = node->next_) {
					node->value_ = *(first++);
				}

				if (size_ < new_size) {
					construct_range(first, last, head_);
				}
			}
			else {
				for (size_type i{}; i < new_size; ++i, node = node->next_) {
					node->value_ = *(first++);
				}

				erase_range(node, head_);
			}
			size_ = new_size;
		}

		void assign(size_type count, const_reference value)
		{
			if (size_ == 0) {
				construct_n_copies(count, value, head_);
			}
			else if (auto node = head_->next_; size_ <= count) {
				for (; node != head_; node = node->next_) {
					node->value_ = value;
				}

				if (size_ < count) {
					for (size_type i{ size_ }; i < count; ++i, node = node->next_) {
						node->value_ = value;
					}
				}
			}
			else {
				for (size_type i{}; i < count; ++i, node = node->next_) {
					node->value_ = value;
				}
				erase_range(node, head_);
			}

			size_ = count;
		}

		void assign(std::initializer_list<T> ilist)
		{
			*this = ilist;
		}
		
		[[nodiscard]] allocator_type get_allocator() const noexcept
		{
			return static_cast<allocator_type>(allocator_);
		}

	// Element access
	public:
		[[nodiscard]] reference front()
		{
			assert(size_ != 0 && "front() on empty container");
			return head_->next_->value_;
		}

		[[nodiscard]] const_reference front() const
		{
			assert(size_ != 0 && "front() on empty container");
			return head_->next_->value_;
		}

		[[nodiscard]] reference back()
		{
			assert(size_ != 0 && "back() on empty container");
			return head_->prev_->value_;
		}

		[[nodiscard]] const_reference back() const
		{
			assert(size_ != 0 && "back() on empty container");
			return head_->prev_->value_;
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

	// Modifiers
	public:
		void clear() noexcept
		{
			node_type::free_all_nonhead(allocator_, head_);
			head_->next_ = head_;
			head_->prev_ = head_;
			size_ = 0;
		}

	private:
		void range_verify(const nodeptr& ptr) const noexcept
		{
			if (ptr == head_) return;

			auto node = head_->next_;
			bool flag{};
			for (; node != head_; node = node->next_) {
				if (ptr == node) {
					flag = true;
					break;
				}
			}
			assert(flag && "out of range iterator");
		}

	public:
		iterator insert(const_iterator pos, const_reference value)
		{
			return emplace(pos, value);
		}

		iterator insert(const_iterator pos, value_type&& value)
		{
			return emplace(pos, std::move(value));
		}

		iterator insert(const_iterator pos, size_type count, const_reference value)
		{
			auto where = pos.get_pointer();
			range_verify(where);

			auto node = where->prev_;
			construct_n_copies(count, value, node);
			size_ += count;
			return iterator{ this, node->next_ };
		}

		template <class Iter, std::enable_if_t <is_iterator<Iter>::value || std::is_pointer<Iter>::value, int> = 0>
		iterator insert(const_iterator pos, Iter first, Iter last)
		{
			auto where = pos.get_pointer();
			range_verify(where);

			auto node = where->prev_;
			construct_range(first, last, node);
			size_ += std::distance(first, last);
			return iterator{ this, node->next_ };
		}

		template <class... Args>
		iterator emplace(const_iterator pos, Args&&...what)
		{
			auto where = pos.get_pointer();
			range_verify(where);

			auto node = allocator_.allocate(1);
			node_allocator_traits::construct(allocator_, node, where, where->prev_, std::forward<Args>(what)...);
			where->prev_->next_ = node;
			where->prev_ = node;
			++size_;

			return iterator{ this, node };
		}

		iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
		{
			return insert(pos, ilist.begin(), ilist.end());
		}

		iterator erase(const_iterator pos)
		{
			auto where = pos.get_pointer();
			assert(where != head_ && "cannot erase out of range iterator");
			range_verify(where);

			auto result = where->next_;
			erase_range(where, where->next_);
			--size_;
			return iterator(this, result);
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			auto begin = first.get_pointer();
			assert(begin != head_ && "cannot erase out of range iterator");
			range_verify(begin);

			auto end = last.get_pointer();
			range_verify(end);

			size_ -= std::distance(first, last);
			erase_range(begin, end);
			return iterator{ this, end };
		}

	};
}

#endif