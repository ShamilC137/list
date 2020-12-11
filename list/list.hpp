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

	template <class MyList>
	class list_const_iterator
	{

	};

	template <class MyList>
	class list_iterator : list_const_iterator<MyList>
	{

	};

	// FIXME: try me bitch
	// FIXME: reasonability of Pointer
	template <class T, class Pointer>
	struct list_node
	{
		// type aliases
		using value_type = T;
		using nodeptr = typename std::pointer_traits<Pointer>::template rebind<list_node>; // FIXME: check the reasonability of it

		// data
		nodeptr		next_; // next node, or first if head
		nodeptr		prev_; // previous node, or last if head
		value_type	value_; // the stored value, unused if head

		template <class...Args>
		list_node(nodeptr next, nodeptr prev, Args&&... args) : next_{ next },
																prev_{ prev },
																value_{ std::forward<Args>(args)... }{}

		// copying
		list_node(const list_node&) = delete;
		list_node& operator=(const list_node&) = delete;
		
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
		static void free_all_nodes(NodeAlloc& allocator, nodeptr head) noexcept
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

		// allocator types aliases and node pointer aliase
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
			size_type size) :	allocator_{ static_cast<node_allocator_type>(alloc) },
								head_{node_type::create_head(allocator_)},
								size_{size}	{ }

	private:
		template <class Iter>
		void construct_range_unchecked(Iter first, Iter last)
		{
			if (first == last) return;

			while (first != last) {
				auto node = allocator_.allocate(1);
				node_allocator_traits::construct(allocator_, node, head_, head_->prev_, *first);
				head_->prev_->next_ = node;
				head_->prev_ = node;
				++first;
			}
		}

	public:
		list(std::initializer_list<value_type> list, 
			 const allocator_type& allocator = allocator_type{}) : list(allocator, list.size())
		{
			construct_range_unchecked(list.begin(), list.end());
		}

		~list() noexcept
		{
			if(head_ != nullptr)	node_type::free_all_nodes(allocator_, head_);
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