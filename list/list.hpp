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

	template <class T, class Pointer>
	struct list_node
	{
		// type aliases
		using value_type = T;
		using nodeptr = typename std::pointer_traits<Pointer>::template rebind<list_node>; // I am using the rebind because I cannot know what will be used as pointer
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
			::new (std::addressof(head->next_)) nodeptr(head);
			::new (std::addressof(head->prev_)) nodeptr(head);
			
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
			std::allocator_traits<NodeAlloc>::destroy(allocator, std::addressof(ptr->value_));
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

	// list_node head will store first element(next_) and last element(prev_) 
	template <class T, class Alloc = std::allocator<T>>
	class list
	{
		friend list_const_iterator<list<T, Alloc>>;
		// type aliases
	public:
		// value type aliases
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// allocator types aliases and node pointer aliases
		using allocator_type = Alloc;
		using list_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
		using list_allocator_traits = std::allocator_traits<list_allocator>;
		using node_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<list_node<T, typename std::allocator_traits<Alloc>::void_pointer>>;
		using node_allocator_traits = std::allocator_traits<node_allocator_type>;
		using node_type = list_node<T, typename node_allocator_traits::void_pointer>;
		using nodeptr = typename node_allocator_traits::pointer;

		// iterator aliases
		using iterator = list_iterator<list<T, Alloc>>;
		using const_iterator = list_const_iterator<list<T, Alloc>>;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		// list data
	private:
		node_allocator_type allocator_;
		nodeptr head_;
		size_type size_;

		// Ctors and dtor
	public:
		list() noexcept : list(allocator_type{}, 0) {}

		explicit list(const allocator_type& allocator) : list(allocator, 0){}

	private:
		// helper initialize ctor
		list(const allocator_type& alloc,
			size_type size) : allocator_{ alloc },
			head_{ node_type::create_head(allocator_) },
			size_{ size }	{ }

	public:
		explicit list(size_type count, const allocator_type& allocator = allocator_type{}) : list(allocator, 0)
		{
			for (size_type i{}; i < count; ++i) {
				emplace_back();
			}
		}

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
			construct_range(first.get_pointer(), last.get_pointer(), where);
		}

		void construct_range(iterator first, iterator last, nodeptr where)
		{
			construct_range(static_cast<const_iterator>(first), last, where);
		}

		void construct_range(nodeptr first, const nodeptr last, nodeptr where)
		{
			if (first == last) return;

			size_type count = std::distance(first, last);

			size_type i{};
			auto next = where->next_;
			nodeptr node{};
			while (i++ < count) {
				node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next, where, *first);
				where = node;
				++first;
			}
			next->prev_ = node;
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
		explicit list(size_type count,
			const_reference value,
			const allocator_type allocator = allocator_type{}) : list(allocator, count)
		{
			construct_n_copies(count, value, head_);
		}


		template <class Iter, std::enable_if_t<is_iterator<Iter>::value, int> = 0>
		list(Iter first, Iter last, const allocator_type& allocator = allocator_type{}) : list(allocator, std::distance(first, last))
		{
			construct_range(first, last, head_);
		}

		list(const list& rhs) : list(std::allocator_traits<allocator_type>::select_on_container_copy_construction(rhs.allocator_), rhs.size_)
		{
			construct_range(rhs.head_->next_, rhs.head_, head_);
		}

		list(const list& rhs, const allocator_type& allocator) : list(allocator, rhs.size_)
		{
			construct_range(rhs.head_->next_, rhs.head_, head_);
		}

		list(list&& rhs) : allocator_{ std::move(rhs.allocator_) }, head_{node_type::create_head(allocator_)}, size_{rhs.size_}
		{
			this->operator=(std::move(rhs));
		}

		list(std::initializer_list<value_type> list, 
			 const allocator_type& allocator = allocator_type{}) : list(allocator, list.size())
		{
			construct_range(list.begin(), list.end(), head_);
		}

	private:
		void tidy() noexcept
		{
			if (head_) {
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
			if (this == std::addressof(rhs)) return *this;
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

	private:
		void move_range_construct(nodeptr first, nodeptr last, nodeptr where)
		{
			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, where->next_, where, std::move(first->value_));
				where->next_->prev_ = node;
				where->next_ = node;
				where = node;
				first = first->next_;
			}
		}

	public:
		// FIXME: may cause problems
		list& operator=(list&& rhs)
		{
			if (this == std::addressof(rhs)) return *this;

			if (allocator_ == rhs.allocator_)	 {
				if (head_) {
					erase_range(head_->next_, head_);
				}
				std::swap(head_, rhs.head_);
				size_		= rhs.size_;
			}
			else if (list_allocator_traits::propagate_on_container_move_assignment::value) {
				tidy(); // use old allocator to free the storage
				allocator_	= std::move(rhs.allocator_);
				head_		= node_type::create_head(allocator_);
				size_		= rhs.size_;
				move_range_construct(rhs.head_->next_, rhs.head_, head_); // move all elements from rhs
			}
			else {
				assign(rhs.begin(), rhs.end());
			}
			rhs.size_ = 0;
	
			return *this;
		}

		list& operator=(std::initializer_list<T> ilist) 
		{
			assign(ilist.begin(), ilist.end());

			return *this;
		}

		template <class Iter, std::enable_if_t<is_iterator<Iter>::value || std::is_pointer<Iter>::value, int> = 0>
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
			assign(ilist.begin(), ilist.end());
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

			size_ += std::distance(first, last);
			auto node = where->prev_;
			construct_range(first, last, node);
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


		void push_back(const_reference value)
		{
			emplace_back(value);
		}

		void push_back(value_type&& value)
		{
			emplace_back(std::move(value));
		}

		template<class...Args>
		reference emplace_back(Args&&... what)
		{
			++size_;
			auto node = allocator_.allocate(1);
			node_allocator_traits::construct(allocator_, node, head_, head_->prev_, std::forward<Args>(what)...);
			head_->prev_->next_ = node;
			head_->prev_ = node;

			return node->value_;
		}
		
		void pop_back() noexcept
		{
			assert(size_ != 0 && "cannot pop from empty container");

			auto node = head_->prev_->prev_;
			node_type::free_node(allocator_, head_->prev_);
			head_->prev_ = node;
			node->next_ = head_;
			--size_;
		}


		void push_front(const_reference value)
		{
			emplace_front(value);
		}

		void push_front(value_type&& value)
		{
			emplace_front(std::move(value));
		}

		template <class... Args>
		reference emplace_front(Args&&...what)
		{
			++size_;
			
			auto node = allocator_.allocate(1);
			node_allocator_traits::construct(allocator_, node, head_->next_, head_, std::forward<Args>(what)...);
			head_->next_->prev_ = node;
			head_->next_ = node;

			return node->value_;
		}

		void pop_front() noexcept
		{
			assert(size_ != 0 && "cannot pop on empty container");
			--size_;
			auto node = head_->next_->next_;
			node_type::free_node(allocator_, head_->next_);
			head_->next_ = node;
			node->prev_ = head_;
		}


		void resize(size_type new_size)
		{
			if (size_ < new_size) {
				for (size_type i{ size_ }; i < new_size; ++i) {
					emplace_back();
				}
			}
			else {
				while (new_size < size_) {
					pop_back();
				}
			}
		}

		void resize(size_type new_size, const_reference value) 
		{
			if (size_ < new_size) {
				construct_n_copies(new_size - size_, value, head_->prev_);
			}
			else {
				while (new_size < size_) {
					pop_back();
				}
			}
		}


		void swap(list& rhs) noexcept(std::allocator_traits<node_allocator_type>::is_always_equal::value)
		{
			if (this != std::addressof(rhs)) {
				if (std::allocator_traits<node_allocator_type>::is_always_equal::value) {
					std::swap(allocator_, rhs.allocator_);
				}

				std::swap(head_, rhs.head_);
				std::swap(size_, rhs.size_);
			}
		}

	// Operations
	private:
		void unchecked_splice(nodeptr first, nodeptr last, nodeptr where)
		{
			first->prev_->next_ = last;
			auto tmp = first->prev_;

			where->prev_->next_ = first;
			first->prev_ = where->prev_;
			where->prev_ = last->prev_;
			last->prev_->next_ = where;

			last->prev_ = tmp;
		}

	public:
		template <class Cmp = std::less<value_type>>
		void merge(list& rhs, Cmp cmp = Cmp{})
		{
			if (this == std::addressof(rhs)) return;
			merge(std::move(rhs), cmp);
		}

	private:
		// handmade sorted because std::sorted uses iterators (I have no unchecked iterators for now)
		template<class Cmp>
		bool is_sorted(const list& list, Cmp cmp)
		{
			auto node = list.head_->next_;
			auto end = list.head_->prev_;
			for (; node != end; node = node->next_) {
				if (cmp(node->next_->value_, node->value_)) {
					return false;
				}
			}
			return true;
		}

		//FIXME: unchecked
		template <class Cmp>
		nodeptr unchecked_merge(nodeptr lhsfirst, nodeptr lhslast, nodeptr rhsfirst, nodeptr rhslast, Cmp cmp) noexcept
		{
			auto node = lhsfirst->prev_;
			while (lhsfirst != lhslast && rhsfirst != rhslast) {
				if (!cmp(rhsfirst->value_, lhsfirst->value_)) {
					node->next_ = lhsfirst;
					node->next_->prev_ = node;
					node = node->next_;
					lhsfirst = lhsfirst->next_;
				}
				else {
					node->next_ = rhsfirst;
					node->next_->prev_ = node;
					node = node->next_;
					rhsfirst = rhsfirst->next_;
				}
			}

			while (lhsfirst != lhslast) {
				node->next_ = lhsfirst;
				node->next_->prev_ = node;
				node = node->next_;
				lhsfirst = lhsfirst->next_;
			}

			while (rhsfirst != rhslast) {
				node->next_ = rhsfirst;
				node->next_->prev_ = node;
				node = node->next_;
				rhsfirst = rhsfirst->next_;
			}

			return node;
		}

	public:
		template <class Cmp = std::less<value_type>>
		void merge(list&& rhs, Cmp cmp = Cmp{})
		{
			if (this == std::addressof(rhs)) return;

			assert(get_allocator() == rhs.get_allocator() && "list allocator incompatible for merge");

			if (rhs.size_ == 0) return;

			if (size_ == 0) {
				assert(is_sorted(rhs, cmp) && "sequence not ordered");
				
				if (!head_) {
					head_ = node_type::create_head(allocator_);
				}

				unchecked_splice(rhs.begin().get_pointer(), rhs.end().get_pointer(), head_);
				size_ = rhs.size_;
				rhs.size_ = 0;
				return;
			}

			assert(is_sorted(*this, cmp) && is_sorted(rhs, cmp) && "sequence not ordered");
	
			auto lhsnode = head_->next_;
			auto rhsnode = rhs.head_->next_;
			auto node = unchecked_merge(lhsnode, head_, rhsnode, rhs.head_, cmp);

			head_->prev_ = node;
			node->next_ = head_;
			size_ = size_ + rhs.size_;

			rhs.head_->next_ = rhs.head_;
			rhs.head_->prev_ = rhs.head_;
			rhs.size_ = 0;
		}


		void splice(const_iterator pos, list& rhs) noexcept
		{
			splice(pos, std::move(rhs), rhs.begin(), rhs.end());
		}

		void splice(const_iterator pos, list&& rhs) noexcept
		{
			splice(pos, std::move(rhs), rhs.begin(), rhs.end());
		}

		void splice(const_iterator pos, list& rhs, const_iterator what)
		{
			splice(pos, std::move(rhs), what);
		}

		void splice(const_iterator pos, list&& rhs, const_iterator it)
		{
			assert(this != std::addressof(rhs) && "splicing the same object");
			auto where = pos.get_pointer();
			auto what = it.get_pointer();
			assert(what != rhs.head_ && "cannot move rhs head");
			assert(rhs.size_ != 0 && "moving from empty container");
			rhs.range_verify(what);

			++size_;
			--rhs.size_;

			what->prev_->next_ = what->next_;
			what->next_->prev_ = what->prev_;

			where->prev_->next_ = what;
			what->prev_ = where->prev_->prev_;
			where->prev_ = what;
			what->next_ = where;
		}

		void splice(const_iterator pos, list& rhs, const_iterator first, const_iterator last) noexcept
		{
			splice(pos, std::move(rhs), first, last);
		}

		void splice(const_iterator pos, list&& rhs, const_iterator first, const_iterator last) noexcept
		{
			assert(this != std::addressof(rhs) && "splicing same object");
			auto begin = first.get_pointer();
			auto end = last.get_pointer();
			auto where = pos.get_pointer();
			assert(rhs.size_ != 0 && "moving from empty container");
			assert(begin != end && "trying to splice empty range");
			rhs.range_verify(begin);
			rhs.range_verify(end);

			size_type range_size = std::distance(first, last);
			rhs.size_ -= range_size;
			size_ += range_size;

			unchecked_splice(begin, end, where);
		}


		void remove(const_reference value) noexcept
		{
			auto node = head_->next_;
			while (node != head_)
			{
				auto tmp = node->next_;
				if (node->value_ == value) {
					node->prev_->next_ = node->next_;
					node->next_->prev_ = node->prev_;
					
					node_type::free_node(allocator_, node);
					--size_;
				}
				node = tmp;
			} 
		}

		template <class Predicate>
		void remove_if(Predicate pred)
		{
			auto node = head_->next_;
			while (node != head_)
			{
				auto tmp = node->next_;
				if (pred(node->value_)) {
					node->prev_->next_ = node->next_;
					node->next_->prev_ = node->prev_;

					node_type::free_node(allocator_, node);
					--size_;
				}
				node = tmp;
			}
		}


		void reverse() noexcept
		{
			if (!head_ || size_ == 0) return;
			auto begin = head_->next_;
			auto end = head_->prev_;

			size_type i{};
			auto half_size = size_ / 2;
			while (i < half_size) {
				auto first = begin;
				auto last = end;
				begin = begin->next_;
				end = end->prev_;

				last->prev_->next_ = first;
				last->next_->prev_ = first;

				first->prev_->next_ = last;
				first->next_->prev_ = last;

				std::swap(first->next_, last->next_);
				std::swap(last->prev_, first->prev_);

				++i;
			}
		}

		void unique() noexcept
		{
			auto node = head_->next_;
			while (node != head_->prev_) {
				if (node->next_->value_ == node->value_) {
					auto tmp = node->next_;
					node->next_ = node->next_->next_;
					node->next_->prev_ = node;
					node_type::free_node(allocator_, tmp);
					--size_;
				}
				else {
					node = node->next_;
				}
			}
		}

		template <class BinaryPredicate>
		void unique(BinaryPredicate pred)
		{
			auto node = head_->next_;
			while (node != head_->prev_) {
				if (pred(node->value_, node->next_->value_)) {
					auto tmp = node->next_;
					node->next_ = node->next_->next_;
					node->next_->prev_ = node;
					node_type::free_node(allocator_, tmp);
					--size_;
				}
				else {
					node = node->next_;
				}
			}
		}

	private:
		template <class BinaryPred>
		nodeptr Sort(nodeptr begin, size_type size, BinaryPred pred)
		{
			if (size <= 1) return begin;

			auto mid = size / 2;
			auto left = begin;
			auto right = begin;
			for (size_type j{}; j < mid; ++j) {
				right = right->next_;
			}

			left = Sort(left, mid, pred);
			right = Sort(right, size - mid, pred);

			auto end = right;
			for (size_type j{}; j < size - mid; ++j) {
				end = end->next_;
			}

			auto node = unchecked_merge(left, right, right, end, pred);
			end->prev_ = node;
			node->next_ = end;

			// FIXME: That's stupid, but works-_-
			for (size_type i{}; i < size; ++i) {
				end = end->prev_;
			}

			return end; // new begin
		}
	public:
		template <class BinaryPred = std::less<value_type>>
		void sort(BinaryPred pred = BinaryPred{})
		{
			if (head_) {
				Sort(head_->next_, size_, pred);
			}
		}

	};

	template <class T, class Alloc>
	[[nodiscard]] bool operator==(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		if (std::addressof(rhs) == std::addressof(lhs)) return true;
		auto lhssize = lhs.size();
		auto rhssize = rhs.size();
		if (lhssize != rhssize) return false;

		auto lhsnode = lhs.begin().get_pointer();
		auto rhsnode = rhs.begin().get_pointer();
		for (std::size_t i{}; i < lhssize; ++i) {
			if (!(lhsnode->value_ == rhsnode->value_))
			{
				return false;
			}
			rhsnode = rhsnode->next_;
			lhsnode = lhsnode->next_;
		}
		return true;
	}

	template <class T, class Alloc>
	[[nodiscard]] bool operator!=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <class T, class Alloc>
	[[nodiscard]] bool operator<(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		if (std::addressof(rhs) == std::addressof(lhs)) return true;
		auto lhssize = lhs.size();
		auto rhssize = rhs.size();
		if (lhssize != rhssize) return false;

		auto lhsnode = lhs.begin().get_pointer();
		auto rhsnode = rhs.begin().get_pointer();
		for (std::size_t i{}; i < lhssize; ++i) {
			if ( rhsnode->value_ < lhsnode->value_)
			{
				return false;
			}
			rhsnode = rhsnode->next_;
			lhsnode = lhsnode->next_;
		}
		return true;
	}

	template <class T, class Alloc>
	[[nodiscard]] bool operator>(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		return rhs < lhs;
	}

	template <class T, class Alloc>
	[[nodiscard]] bool operator<=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		return !(rhs < lhs);
	}

	template <class T, class Alloc>
	[[nodiscard]] bool operator>=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs) noexcept
	{
		return !(lhs < rhs);
	}
}

namespace std {
	template <class T, class Alloc>
	void swap(my_lib::list<T, Alloc>& lhs, my_lib::list<T, Alloc>& rhs) noexcept(lhs.swap(rhs))
	{
		lhs.swap(rhs);
	}
}

#endif
