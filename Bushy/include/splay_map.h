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
#include <iterator>
#include <type_traits>

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
private:
    struct node;
    struct base_node;

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

    // Iterator implementation. It is implemented as template, so we does not need
    // two iterator classes. It uses types from this map class, and is parametrized
    // by Value, which can be either const value_type (for constant iterator), or
    // value_type (for non-constant iterator).
    template<typename Value>
    class iterator_impl : public std::iterator<std::bidirectional_iterator_tag,
            Value,
            difference_type,
            typename std::conditional<std::template is_const<Value>::value, const_pointer, pointer>::type,
            typename std::conditional<std::template is_const<Value>::value, const_reference, reference>::type>
    {
    public:
        iterator_impl() : _node(nullptr), _proxy(nullptr) { }
        iterator_impl(const iterator_impl& other) = default; // default copy constructor; we just copy pointers
        ~iterator_impl() = default; // we do not need extra functionality here

        // Static assert, so this class is instantiated only with correct type.
        static_assert(std::is_same<typename std::remove_const<Value>::type, typename splay_map::value_type>::value, "Iterator error - invalid template instantiation!");

        // Conversion constructor from iterator to const iterator. We use a template hack so we cannot create
        // the non-constant iterator from constant iterator. We allow iterator creation only, if we can convert
        // the type ValueFrom to the type Value;
        template<typename ValueFrom>
        iterator_impl(const iterator_impl<ValueFrom>& other, typename std::enable_if<std::template is_convertible<ValueFrom, Value>::value, int>::type enable = int()) :
            _node(other._node),
            _proxy(other._proxy)
        {

        }

        iterator_impl& operator=(const iterator_impl& other) = default; // default copy assignment operator; we just copy pointers

        // Dereference operators
        typename iterator_impl::reference operator*() const { return _node->asNode()->value; }
        typename iterator_impl::pointer operator->() const { return &_node->asNode()->value; }

        iterator_impl& operator++()
        {
            _node = _proxy->_next(_node);
            return *this;
        }

        iterator_impl operator++(int)
        {
            iterator_impl temp(*this);
            _node = _proxy->_next(_node);
            return temp;
        }

        iterator_impl& operator--()
        {
            _node = _proxy->_prev(_node);
            return *this;
        }

        iterator_impl operator--(int)
        {
            iterator_impl temp(*this);
            _node = _proxy->_prev(_node);
            return temp;
        }

        bool operator==(const iterator_impl& other) const
        {
            const bool isNullLeft = !_proxy || &_proxy->_root == _node;
            const bool isNullRight = !other._proxy || &other._proxy->_root == other._node;

            if (isNullLeft != isNullRight)
            {
                // One iterator is end iterator, other is valid.
                return false;
            }

            if (isNullLeft && isNullRight)
            {
                // Both iterators are invalid (pass-the-end iterators)
                return true;
            }

            // Now, both iterators are valid, we compare if they points to the same
            // node (we can simply test the node pointers, because the map owns the node,
            // pointers are unique). So it cannot happen situation, where node pointers are
            // equal, but proxies not.

            return _node == other._node;
        }

        // Template version for comparation of constant and non-constant iterators
        template<typename OtherValue>
        bool operator==(const iterator_impl<OtherValue>& other) const
        {
            return (*this == const_cast_iterator(other));
        }

        bool operator!=(const iterator_impl& other) const
        {
            return !(*this == other);
        }

        // Template version for comparation of constant and non-constant iterators
        template<typename OtherValue>
        bool operator!=(const iterator_impl<OtherValue>& other) const
        {
            return !(*this == const_cast_iterator(other));
        }

        // Swaps the two iterators of the same type (restricted to the same type, we cannot
        // swap constant and non-constant iterator).
        void swap(iterator_impl& other)
        {
            std::swap(_node, other._node);
            std::swap(_proxy, other._proxy);
        }

    private:
        // Restricted constructor; used in the iterator casts. The const casts may be disturbing,
        // but they are needed, because we must allow create the iterator from the const object.
        // The constantness is achieved via interface (const functions of the map should not
        // return non-const iterator).
        explicit iterator_impl(const base_node* node, const splay_map* proxy) : _node(const_cast<base_node*>(node)), _proxy(const_cast<splay_map*>(proxy)) { }

        // Converts the other iterator type to this iterator type
        template<typename OtherValue>
        iterator_impl const_cast_iterator(const iterator_impl<OtherValue>& iterator) const
        {
            return iterator_impl(iterator._node, iterator._proxy);
        }

        // To allow use of private constructor in the splay map
        friend class splay_map;

        base_node* _node;
        splay_map* _proxy;
    };

    using iterator = iterator_impl<value_type>;
    using const_iterator = iterator_impl<const value_type>;

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
        _root{&_root, &_root, &_root},
        _comp(comp),
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
        splay_map(other._comp, std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator()))
    {
        insert(other.cbegin(), other.cend());
    }

    splay_map(const splay_map& other, const Allocator& alloc) :
        splay_map(other._comp, alloc)
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

    T& at(const Key& key)
    {
        base_node* node = _find(key);

        if (node != &_root)
        {
            return node->asNode()->value.second;
        }
        else
        {
            throw std::out_of_range("bushy::splay_map::at() - key not found!");
        }
    }

    const T& at(const Key& key) const
    {
        base_node* node = _find(key);

        if (node != &_root)
        {
            return node->asNode()->value.second;
        }
        else
        {
            throw std::out_of_range("bushy::splay_map::at() - key not found!");
        }
    }

    T& operator[](const Key& key);
    T& operator[]( Key&& key );

    const T& value(const Key& key, const T& defaultValue) const
    {
        base_node* node = _find(key);

        if (node != &_root)
        {
            return node->asNode()->value.second;
        }
        else
        {
            return defaultValue;
        }
    }

    reference front() { return *begin(); }
    const_reference front() const { return *cbegin(); }

    reference back() { return *rbegin(); }
    const_reference back() const { return *crbegin(); }

    // Iterators

    iterator begin() { return iterator(_root.left, this); }
    const_iterator begin() const { return const_iterator(_root.left, this); }
    const_iterator cbegin() const { return const_iterator(_root.left, this); }

    iterator end() { return iterator(&_root, this); }
    const_iterator end() const { return const_iterator(&_root, this); }
    const_iterator cend() const { return const_iterator(&_root, this); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    // Capacity

    bool empty() const { return _size == 0; }
    size_type size() const { return _size; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }

    // Modifiers

    void clear() { _cleanup(); }

    std::pair<iterator, bool> insert( const value_type& value );

    template<class P>
    std::pair<iterator, bool> insert( P&& value );
    std::pair<iterator, bool> insert( value_type&& value );
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

    size_type count(const Key& key) const
    {
        return _find(key) != &_root ? 1 : 0;
    }

    template<class K>
    size_type count(const K& x) const
    {
        return _find<K>(key) != &_root ? 1 : 0;
    }

    iterator find(const Key& key)
    {
        return iterator(_find(key), this);
    }

    const_iterator find(const Key& key) const
    {
        return const_iterator(_find(key), this);
    }

    template<class K>
    iterator find(const K& x)
    {
        return iterator(_find<K>(key), this);
    }

    template<class K>
    const_iterator find(const K& x) const
    {
        return const_iterator(_find<K>(key), this);
    }

    std::pair<iterator, iterator> equal_range(const Key& key)
    {
        return std::make_pair(lower_bound(key), upper_bound(key));
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const
    {
        return std::make_pair(lower_bound(key), upper_bound(key));
    }

    template<class K>
    std::pair<iterator, iterator> equal_range(const K& x)
    {
        return std::make_pair(lower_bound<K>(key), upper_bound<K>(key));
    }

    template<class K>
    std::pair<const_iterator, const_iterator> equal_range(const K& x) const
    {
        return std::make_pair(lower_bound<K>(key), upper_bound<K>(key));
    }

    iterator lower_bound(const Key& key)
    {
        return iterator(_lower_bound(key), this);
    }

    const_iterator lower_bound(const Key& key) const
    {
        return const_iterator(_lower_bound(key), this);
    }

    template<class K>
    iterator lower_bound(const K& x)
    {
        return iterator(_lower_bound<K>(key), this);
    }

    template<class K>
    const_iterator lower_bound(const K& x) const
    {
        return const_iterator(_lower_bound<K>(key), this);
    }

    iterator upper_bound(const Key& key)
    {
        return iterator(_upper_bound(key), this);
    }

    const_iterator upper_bound(const Key& key) const
    {
        return const_iterator(_upper_bound(key), this);
    }

    template<class K>
    iterator upper_bound(const K& x)
    {
        return iterator(_upper_bound<K>(key), this);
    }

    template<class K>
    const_iterator upper_bound(const K& x) const
    {
        return const_iterator(_upper_bound<K>(key), this);
    }

    inline key_compare key_comp() const { return _comp; }
    inline value_compare value_comp() const { return value_compare(_comp); }

    // Memory consumption
    static constexpr unsigned long memory_consumption_empty() { return sizeof(splay_map); }
    static constexpr unsigned long memory_consumption_item() { return sizeof(node); }

    // Estimates overall memory consumption
    unsigned long memory_consumption(unsigned long additional_item_memory = 0) const { return memory_consumption_empty() + _size * (memory_consumption_item() + additional_item_memory); }

private:
    // Base node containing only pointers (to the parent/left child/right child),
    // it is used as root of the tree, where parent points to the root of the tree,
    // and left/right pointers points to the min/max value of the tree.
    struct base_node
    {
        base_node* parent;
        base_node* left;
        base_node* right;

        inline node* asNode() { return static_cast<node*>(this); }
        inline const node* asNode() const { return static_cast<const node*>(this); }
    };

    // Rotates the tree node to the right. It does not checks if node/left_child
    // really exists, so please be careful when calling this function.
    //
    //              node                          left_child                *
    //             /    \                         /         \               *
    //      left_child  right_child  ==>       ll_child    node             *
    //       /     \                                      /     \           *
    //  ll_child  lr_child                             lr_child right_child *
    inline base_node* _right_rotate(base_node* node) const
    {
        base_node* parent = node->parent;
        base_node* left_child = node->left;
        base_node* lr_child = left_child->right;

        // Rotate right
        left_child->right = node;
        node->parent = left_child;

        // Move right grandchild of the left child
        node->left = lr_child;
        if (lr_child != &_root)
        {
            lr_child->parent = node;
        }

        // Fix the root
        left_child->parent = parent;
        if (parent != &_root)
        {
            if (parent->left == node)
            {
                parent->left = left_child;
            }
            else
            {
                parent->right = left_child;
            }
        }
        else
        {
            // Mark the new root!
            _root.parent = left_child;
        }

        return left_child;
    }

    // Destroys the entire tree.
    void _cleanup()
    {
        // First we must linearize tree (so the memory consumption of this object will be constant.
        base_node* work_node = _root.parent;

        while (work_node != &_root)
        {
            // If we have a left child of the current node, rotate the current node right!
            while (work_node->left != &_root)
            {
                work_node = _right_rotate(work_node);
            }

            // We have no left child. Move to the right in the next layer (linearize the tree).
            work_node = work_node->right;
        }

        work_node = _root.parent;

        while (work_node != &_root)
        {
            node* to_destroy = work_node->asNode();
            work_node = work_node->right;

            // Now, we delete the node
            _alloc.destroy(to_destroy);
            _alloc.deallocate(to_destroy, 1);
        }
    }

    // Finds the next node in the tree
    base_node* _next(base_node* node) const
    {
        if (node == &_root)
        {
            // "Cyclical" iteration over the range - we return the first node
            // to ensure the iterators will be valid.
            return _root.left;
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
        if (node == &_root)
        {
            // "Cyclical" iteration over the range - we return the last node
            // to ensure the iterators will be valid.
            return _root.right;
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
    base_node* _max(base_node* node) const
    {
        base_node* last = &_root;
        do
        {
            last = node;
            node = node->right;
        } while (node != &_root);

        return last;
    }

    // Finds the node with minimal value in the subtree
    base_node* _min(base_node* node) const
    {
        base_node* last = &_root;
        do
        {
            last = node;
            node = node->left;
        } while (node != &_root);

        return last;
    }

    // Finds the first right parent of the node
    base_node* _first_right_parent_of_node(base_node* node) const
    {
        if (node == &_root)
        {
            // Node is "end" node, return "end" iterator
            return &_root;
        }

        while (node->parent != &_root && node->parent->right == node)
        {
            // While it is right child, go up...
            node = node->parent;
        }

        return node->parent;
    }

    // Finds the first left parent of the node
    base_node* _first_left_parent_of_node(base_node* node) const
    {
        if (node == &_root)
        {
            // Node is "end" node, return "end" iterator
            return &_root;
        }

        while (node->parent != &_root && node->parent->left == node)
        {
            // While it is left child, go up...
            node = node->parent;
        }

        return node->parent;
    }

    // Finds the node with this key, returns root, if the node
    // with that key cannot be found.
    base_node* _find(const Key& key) const
    {
        base_node* current = _root.parent;

        while (current != &_root)
        {
            if (_comp(key, current->asNode()->value.first))
            {
                // Key is lesser than value in the current node, walk left
                current = current->left;
            }
            else if (_comp(current->asNode()->value.first, key))
            {
                // Key is greater than value in the current node, walk right
                current = current->right;
            }
            else
            {
                // Key is equal, we have found the node! Splay it to the root, if neccessary.
                if (_policy.find_policy.splay_hint())
                {
                    _splay(current);
                }

                break;
            }
        }

        return current;
    }

    // Finds the lower bound for particular key - first value, that is not less than key,
    // (so it is equal to the key or greater).
    base_node* _lower_bound(const Key& key) const
    {
        base_node* current = _root.parent;
        base_node* candidate = &_root;

        while (current != &_root)
        {
            if (_comp(current->asNode()->value.first, key)) // node is lesser than key
            {
                // Value is lesser - go right
                current = current->right;
            }
            else
            {
                // Value is greater or equal - go left and remember the new candidate for lower bound
                candidate = current;
                current = current->left;
            }
        }

        if (candidate != &_root && _policy.find_policy.splay_hint())
        {
            // Splay the node, if we should splay it (behave like find)
            _splay(candidate);
        }

        return candidate;
    }

    // Finds the upper bound for particular key - first value, that is greater than key,
    base_node* _upper_bound(const Key& key) const
    {
        base_node* current = _root.parent;
        base_node* candidate = &_root;

        while (current != &_root)
        {
            if (_comp(key, current->asNode()->value.first))
            {
                // Value is greater, remember it and go left
                candidate = current;
                current = current->left;
            }
            else
            {
                // Value is lesser or equal - go right
                current = current->right;
            }
        }

        if (candidate != &_root && _policy.find_policy.splay_hint())
        {
            // Splay the node, if we should splay it (behave like find)
            _splay(candidate);
        }

        return candidate;
    }

    // Finds the node with this key, returns root, if the node
    // with that key cannot be found. Template version, key can be
    // of different type.
    template<class K>
    base_node* _find(const K& key) const
    {
        base_node* current = _root.parent;

        while (current != &_root)
        {
            if (_comp(key, current->asNode()->value.first))
            {
                // Key is lesser than value in the current node, walk left
                current = current->left;
            }
            else if (_comp(current->asNode()->value.first, key))
            {
                // Key is greater than value in the current node, walk right
                current = current->right;
            }
            else
            {
                // Key is equal, we have found the node! Splay it to the root, if neccessary.
                if (_policy.find_policy.splay_hint())
                {
                    _splay(current);
                }

                break;
            }
        }

        return current;
    }

    // Finds the lower bound for particular key - first value, that is not less than key,
    // (so it is equal to the key or greater). Template version, key can be
    // of different type.
    template<class K>
    base_node* _lower_bound(const K& key) const
    {
        base_node* current = _root.parent;
        base_node* candidate = &_root;

        while (current != &_root)
        {
            if (_comp(current->asNode()->value.first, key)) // node is lesser than key
            {
                // Value is lesser - go right
                current = current->right;
            }
            else
            {
                // Value is greater or equal - go left and remember the new candidate for lower bound
                candidate = current;
                current = current->left;
            }
        }

        if (candidate != &_root && _policy.find_policy.splay_hint())
        {
            // Splay the node, if we should splay it (behave like find)
            _splay(candidate);
        }

        return candidate;
    }

    // Finds the upper bound for particular key - first value, that is greater than key.
    // Template version, key can be of different type.
    template<class K>
    base_node* _upper_bound(const K& key) const
    {
        base_node* current = _root.parent;
        base_node* candidate = &_root;

        while (current != &_root)
        {
            if (_comp(key, current->asNode()->value.first))
            {
                // Value is greater, remember it and go left
                candidate = current;
                current = current->left;
            }
            else
            {
                // Value is lesser or equal - go right
                current = current->right;
            }
        }

        if (candidate != &_root && _policy.find_policy.splay_hint())
        {
            // Splay the node, if we should splay it (behave like find)
            _splay(candidate);
        }

        return candidate;
    }

    // Ordinary node containing data
    struct node : public base_node
    {
        value_type value;
    };

    // Root of this map, parent points to the root of the tree,
    // left child is minimum of the tree, right child is the maximum
    // of the tree,
    mutable base_node _root;

    // Key comparator defined by the constructor
    Compare _comp;

    // We rebind the allocator to allocate nodes
    typedef typename Allocator::template rebind<node>::other NodeAllocator;

    // Node allocator
    NodeAllocator _alloc;

    // Actual count of elements in this map
    size_type _size;

    // Policy for behaviour of splaying the nodes in this splay tree
    Policy _policy;
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

template<class Key, class T, class Compare, class Alloc, class Policy>
void swap(bushy::splay_map<Key, T, Compare, Alloc, Policy>& lhs, bushy::splay_map<Key, T, Compare, Alloc, Policy>& rhs )
{
    lhs.swap(rhs);
}

template<class Key, class T, class Compare, class Alloc, class Policy>
void swap(typename bushy::splay_map<Key, T, Compare, Alloc, Policy>::iterator& lhs, typename bushy::splay_map<Key, T, Compare, Alloc, Policy>::iterator& rhs )
{
    lhs.swap(rhs);
}

template<class Key, class T, class Compare, class Alloc, class Policy>
void swap(typename bushy::splay_map<Key, T, Compare, Alloc, Policy>::const_iterator& lhs, typename bushy::splay_map<Key, T, Compare, Alloc, Policy>::const_iterator& rhs )
{
    lhs.swap(rhs);
}

}   // namespace std

#endif // BUSHY_SPLAY_MAP_H
