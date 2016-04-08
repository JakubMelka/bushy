//**************************** LICENSE *************************************
//
// Bushy - various search tree implementation library
// Copyright (C) 2016  Jakub Melka
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//**************************************************************************

#ifndef BUSHY_SPLAY_MAP_H
#define BUSHY_SPLAY_MAP_H

#include <memory>
#include <utility>
#include <functional>
#include <initializer_list>

namespace bushy
{

// Defines the splay mode policy. Specifies, when the splay operation
// will be triggered during some operations.
enum class splay_mode
{
    ALWAYS, // Node is splayed at every operation
    HALF,   // Node is splayed every second operation
    THIRD,  // Node is splayed every third operation
    FOURTH, // Node is splayed every fourth operation
    NEVER   // Node is never splayed
};

namespace impl
{

template<splay_mode>
struct splay_decider { };

template<>
struct splay_decider<splay_mode::ALWAYS>
{
    constexpr bool splay_hint() const { return true; }
};

template<>
struct splay_decider<splay_mode::HALF>
{
    int counter = 0;

    bool splay_hint() { return (++counter) & 1; }
};

template<>
struct splay_decider<splay_mode::THIRD>
{
    int counter = 0;

    bool splay_hint() { return !((++counter) % 3); }
};


template<>
struct splay_decider<splay_mode::FOURTH>
{
    int counter = 0;

    bool splay_hint() { return !((++counter) % 4); }
};

template<>
struct splay_decider<splay_mode::NEVER>
{
    constexpr bool splay_hint() const { return false; }
};

}   // namespace private

// Policy, which defines the behaviour of the splay
// tree during various operations.
template<splay_mode Insert, splay_mode Find>
struct splay_map_policy
{
    impl::splay_decider<Insert> insert_policy;
    impl::splay_decider<Find> find_policy;
};

template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>,
         typename Policy = splay_map_policy<splay_mode::FOURTH, splay_mode::THIRD>>
class splay_map
{
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const Key, T> value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef Compare key_compare;
    typedef Allocator allocator_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::allocator_traits<Allocator>::pointer pointer;
    typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;

    class iterator;
    class const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Value comparator class
    class value_compare
    {
    public:
        typedef bool result_type;
        typedef value_type first_argument_type;
        typedef value_type second_argument_type;

        inline bool operator()( const value_type& lhs, const value_type& rhs ) const
        {
            return comp(lhs.first, rhs.first);
        }

    protected:
        inline value_compare(Compare c) : comp(c) { }
        Compare comp;
    };

    // Constructors

    splay_map() : splay_map(Compare()) { }

    explicit splay_map(const Compare& comp, const Allocator& alloc = Allocator()) :
        root{&root, &root, &root},
        comp(comp),
        _alloc(alloc),
        _size(0)
    {

    }

    explicit splay_map(const Allocator& alloc) : splay_map(Compare(), alloc) { }

    template<class InputIterator>
    splay_map(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) :
        splay_map(comp, alloc)
    {
        insert(first, last);
    }

    template<class InputIterator>
    splay_map(InputIterator first, InputIterator last, const Allocator& alloc) :
        splay_map(alloc)
    {
        insert(first, last);
    }

    splay_map(const splay_map& other) :
        splay_map(other.comp, std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator()))
    {
        insert(other.cbegin(), other.cend());
    }

    splay_map(const splay_map& other, const Allocator& alloc) :
        splay_map(other.comp, alloc)
    {
        insert(other.cbegin(), other.cend());
    }

    // TODO: Fix this code!
    /*
    splay_map(splay_map&& other) :
        root{&root, &root, &root},
        comp(std::move(other.comp)),
        alloc(std::move(other.alloc)),
        size(other.size())
    {
        std::swap(root, other.root);
        other.size = 0;
    }

    splay_map(splay_map&& other, const Allocator& alloc) :
        root{&root, &root, &root},
        comp(std::move(other.comp)),
        alloc(alloc),
        size(other.size())
    {
        std::swap(root, other.root);
        other.size = 0;
    }*/

    splay_map(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) :
        splay_map(comp, alloc)
    {
        insert(init.begin(), init.end());
    }

    splay_map(std::initializer_list<value_type> init, const Allocator& alloc) :
        splay_map(alloc)
    {
        insert(init.begin(), init.end());
    }

    // Destructors
    ~splay_map() { clear(); }

    // Assign operator

    splay_map& operator=(const splay_map& other)
    {
        // First, clear the old data using old allocator
        clear();

        if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
        {
            // We must propagate the allocator to this object
            _alloc = other._alloc;
        }

        insert(other.cbegin(), other.cend());
    }

    splay_map& operator=(splay_map&& other);

    splay_map& operator=(std::initializer_list<value_type> ilist)
    {
        clear();
        insert(ilist.begin(), ilist.end());
    }

    // Allocator functions
    allocator_type get_allocator() const { return _alloc; }

    // Element access functions

    T& at(const Key& key);
    const T& at(const Key& key) const;

    T& operator[]( const Key& key );
    T& operator[]( Key&& key );

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    // Iterators

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const;

    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const;

    // Capacity

    bool empty() const;
    size_type size() const;
    size_type max_size() const;

    // Modifiers

    void clear();

    std::pair<iterator,bool> insert( const value_type& value );

    template<class P>
    std::pair<iterator,bool> insert( P&& value );
    std::pair<iterator,bool> insert( value_type&& value );
    iterator insert( const_iterator hint, const value_type& value );

    template< class P >
    iterator insert( const_iterator hint, P&& value );
    iterator insert( const_iterator hint, value_type&& value );

    template< class InputIt >
    void insert( InputIt first, InputIt last );

    void insert( std::initializer_list<value_type> ilist );

    template <class M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);

    template <class M>
    std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);

    template <class M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);

    template <class M>
    iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);

    template< class... Args >
    std::pair<iterator,bool> emplace( Args&&... args );

    template <class... Args>
    iterator emplace_hint( const_iterator hint, Args&&... args );

    template <class... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args);

    template <class... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args);

    template <class... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args);

    template <class... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args);

    iterator erase( const_iterator pos );
    iterator erase( iterator pos );
    iterator erase( const_iterator first, const_iterator last );
    size_type erase( const key_type& key );

    void swap( splay_map& other );

    // Lookup

    size_type count( const Key& key ) const;

    template< class K >
    size_type count( const K& x ) const;

    iterator find( const Key& key );

    const_iterator find( const Key& key ) const;

    template< class K > iterator find( const K& x );

    template< class K > const_iterator find( const K& x ) const;

    std::pair<iterator,iterator> equal_range( const Key& key );
    std::pair<const_iterator,const_iterator> equal_range( const Key& key ) const;

    template< class K >
    std::pair<iterator,iterator> equal_range( const K& x );

    template< class K >
    std::pair<const_iterator,const_iterator> equal_range( const K& x ) const;

    iterator lower_bound( const Key& key );
    const_iterator lower_bound( const Key& key ) const;

    template< class K >
    iterator lower_bound(const K& x);

    template< class K >
    const_iterator lower_bound(const K& x) const;

    iterator upper_bound( const Key& key );

    const_iterator upper_bound( const Key& key ) const;

    template< class K >
    iterator upper_bound( const K& x );

    template< class K >
    const_iterator upper_bound( const K& x ) const;

    inline key_compare key_comp() const { return comp; }
    inline value_compare value_comp() const { return value_compare(comp); }

    // Memory consumption
    static constexpr unsigned long memory_consumption_empty() { return sizeof(splay_map); }
    static constexpr unsigned long memory_consumption_item() { return sizeof(node); }

    // Estimates overall memory consumption
    unsigned long memory_consumption() const { return memory_consumption_empty() + _size * memory_consumption_item(); }

private:
    // Base node containing only pointers (to the parent/left child/right child),
    // it is used as root of the tree, where parent points to the root of the tree,
    // and left/right pointers points to the min/max value of the tree.
    struct base_node
    {
        base_node* parent;
        base_node* left;
        base_node* right;
    };

    // Finds the next node in the tree
    base_node* _next(base_node* node) const
    {
        if (node == &root)
        {
            // "Cyclical" iteration over the range - we return the first node
            // to ensure the iterators will be valid.
            return root->left;
        }

        if (node->right)
        {
            return _min(node->right);
        }
        else
        {
            return _first_right_parent_of_node(node);
        }
    }

    // Finds the previous node in the tree
    base_node* _prev(base_node* node) const
    {
        if (node == &root)
        {
            // "Cyclical" iteration over the range - we return the last node
            // to ensure the iterators will be valid.
            return root->right;
        }

        if (node->left)
        {
            return _max(node->left);
        }
        else
        {
            return _first_left_parent_of_node(node);
        }
    }

    // Finds the node with maximal value in the subtree
    base_node* _max(base_node* node)
    {
        base_node* last = nullptr;
        do
        {
            last = node;
            node = node->right;
        } while (node && node != &root);

        return last;
    }

    // Finds the node with minimal value in the subtree
    base_node* _min(base_node* node)
    {
        base_node* last = nullptr;
        do
        {
            last = node;
            node = node->left;
        } while (node && node != &root);

        return last;
    }

    // Ordinary node containing data
    struct node : public base_node
    {
        value_type value;
    };

    // Root of this map, parent points to the root of the tree,
    // left child is minimum of the tree, right child is the maximum
    // of the tree,
    base_node root;

    // Key comparator defined by the constructor
    Compare comp;

    // We rebind the allocator to allocate nodes
    typedef typename Allocator::template rebind<node>::other NodeAllocator;

    // Node allocator
    NodeAllocator _alloc;

    // Actual count of elements in this map
    size_type _size;
};

}   // namespace bushy

template< class Key, class T, class Compare, class Alloc >
bool operator==( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                 const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

template< class Key, class T, class Compare, class Alloc >
bool operator!=( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                 const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

template< class Key, class T, class Compare, class Alloc >
bool operator<( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

template< class Key, class T, class Compare, class Alloc >
bool operator<=( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                 const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

template< class Key, class T, class Compare, class Alloc >
bool operator>( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

template< class Key, class T, class Compare, class Alloc >
bool operator>=( const bushy::splay_map<Key,T,Compare,Alloc>& lhs,
                 const bushy::splay_map<Key,T,Compare,Alloc>& rhs );

namespace std
{

template<class Key, class T, class Compare, class Alloc>
void swap(bushy::splay_map<Key,T,Compare,Alloc>& lhs, bushy::splay_map<Key,T,Compare,Alloc>& rhs )
{
    lhs.swap(rhs);
}

}   // namespace std

#endif // BUSHY_SPLAY_MAP_H
