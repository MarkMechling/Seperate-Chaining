#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type &;
  using const_reference = const key_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  //using key_compare = std::less<key_type>;   // B+-Tree
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing

private:
	struct Element {
		key_type key;
		Element *next;
		~Element() { delete next;};
	};
	struct list {
		Element *head{nullptr};
		bool end{false};
		~list() {delete head;};
	};
  list *table {nullptr};
  size_type table_size {0};
  size_type number_of_elements {0};
  float max_load_factor {0.7};

  Element *add(const key_type &key);
  Element *locate(const key_type &key) const;
  size_type h(const key_type &key) const { return hasher{}(key) % table_size; }
  size_type g(const key_type &) const { return 1; }
  void reserve(size_type n);
  void rehash(size_type n);

public:
  ADS_set() { rehash(N); }
  ADS_set(std::initializer_list<key_type> ilist): ADS_set{} { insert(ilist); }
  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} { insert(first,last); }
  ADS_set(const ADS_set &other): ADS_set{} {
    reserve(other.number_of_elements);
    for(const auto&k: other) {
      add(k);
    }
  }

  ~ADS_set() { delete[] table; }

  ADS_set &operator=(const ADS_set &other){
    if(this == &other)
      return *this;
    ADS_set tmp{other};
    swap(tmp);
    return *this;
  }
  ADS_set &operator=(std::initializer_list<key_type> ilist) {
    ADS_set tmp{ilist};
    swap(tmp);
    return *this;
  }

  size_type size() const { return number_of_elements; }
  bool empty() const { return number_of_elements == 0; }

  void insert(std::initializer_list<key_type> ilist) {
    for(const auto& Element : ilist){
      if(!count(Element))
	add(Element);
    }
  }

  std::pair<iterator,bool> insert(const key_type &key) {

    if(auto p {locate(key)}) {
    size_type idx {h(key)};
    list *position_t {&table[idx]};
	return {iterator{position_t,p}, false};
    }
    reserve(number_of_elements+1);
    size_type idx {h(key)};
    list *position_t {&table[idx]};
    return {iterator{position_t,add(key)}, true};
  }

  template<typename InputIt> void insert(InputIt first, InputIt last);

  void clear() {
    delete[] table;
    number_of_elements = 0;
    table_size = N;
    table = new list[table_size+1];
    table[table_size].end = true;
  }

  size_type count(const key_type &key) const { return !!locate(key);}

  iterator find(const key_type &key) const {
    size_type idx {h(key)};
    list *position_t {&table[idx]};
    if(auto p{locate(key)})
	return iterator{position_t,p};
    return end();
  }

    size_type erase(const key_type &key) {
	if(count(key)) {
		size_type idx {h(key)};
		Element *tmp{table[idx].head};
		Element *before{nullptr};
		while(tmp!=nullptr){

		  if(key_equal{}(tmp->key, key) && tmp == table[idx].head) {
		    table[idx].head = tmp->next;
		    tmp->next = nullptr;
		    delete tmp;
		    --number_of_elements;
                    return 1;
		  }
		  if(key_equal{}(tmp->key, key) && before){
	 	    before->next = tmp->next;
		    tmp->next=nullptr;
		    delete tmp;
		    before = nullptr;
		    delete before;
		    --number_of_elements;
		    return 1;
		  }
		  before = tmp;
		  tmp = tmp->next;
		}
	}
	return 0;
  }

  const_iterator begin() const {
    for(size_type idx{0}; idx < table_size; ++idx){
      if(table[idx].head){
	 list *position_t{&table[idx]};
	 Element *position_l{table[idx].head};
 	 return const_iterator{position_t,position_l};
      }
    }
    return end();
  }

  const_iterator end() const {
    list *position_t{&table[table_size]};
    return const_iterator{position_t};
  }

  void swap(ADS_set &other) {
    using std::swap;
    swap(table, other.table);
    swap(number_of_elements, other.number_of_elements);
    swap(table_size, other.table_size);
    swap(max_load_factor, other.max_load_factor);
  }

  void dump(std::ostream &o = std::cerr) const;

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if (lhs.number_of_elements != rhs.number_of_elements) return false;
    for (const auto &k: lhs) if (!rhs.count(k)) return false;
    return true;
  }
  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
  typename ADS_set<Key,N>::Element *ADS_set<Key,N>::add(const key_type& key) {

	size_type idx {h(key)};
	Element *new_Element = new Element;
	new_Element->key = key;
	new_Element->next = table[idx].head;
	table[idx].head = new_Element;
	++number_of_elements;
	return table[idx].head;
  }


template <typename Key, size_t N>
  typename ADS_set<Key,N>::Element *ADS_set<Key,N>::locate(const key_type& key) const {

	size_type idx {h(key)};
		Element *tmp{table[idx].head};
		while(tmp != nullptr) {
			if(key_equal{}(tmp->key, key))
				return tmp;
			else
				tmp = tmp->next;
		}
	return nullptr;
  }

template <typename Key, size_t N>
  template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {

	for(auto it{first}; it!=last; ++it) {
		if(!count(*it)) {
			reserve(number_of_elements+1);
			add(*it);
		}
	}
  }

 template <typename Key, size_t N>
  void ADS_set<Key,N>::dump(std::ostream &o) const {

	o << "number_of_elements = " << number_of_elements << " table_size = " << table_size << "\n";
	for(size_type idx{0}; idx < table_size; ++idx) {
		o << idx << ": ";
		Element *tmp{table[idx].head};
		while(tmp != nullptr){
			o << " --> " << tmp->key;
			tmp = tmp->next;
		}
		if(!table[idx].end)
		  o << " 0";
		o << "\n";
	}
	if(table[table_size].end)
		o << " 1" << "\n";
  }

template <typename Key, size_t N>
void ADS_set<Key,N>::reserve(size_type n) {
  if (table_size * max_load_factor >= n) return;
  size_type new_table_size {table_size};
  while (new_table_size * max_load_factor < n) ++(new_table_size *= 2);
  rehash(new_table_size);
}

template <typename Key, size_t N>
  void ADS_set<Key,N>::rehash(size_type n) {

	size_type new_table_size {std::max(N, std::max(n, size_type(number_of_elements / max_load_factor)))};
	list *new_table {new list[new_table_size+1]};
	new_table[new_table_size].end = true;
	size_type old_table_size {table_size};
	list *old_table {table};
	number_of_elements = 0;
	table = new_table;
	table_size = new_table_size;

	for(size_type idx{0}; idx < old_table_size; ++idx) {
		Element *tmp{old_table[idx].head};
		while(tmp != nullptr){
			add(tmp->key);
			tmp = tmp->next;
		}
	}
	delete[] old_table;
  }

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;
private:
  list *p_table;
  Element *p_list;
  void skip() {while(p_table->head == nullptr && p_table->end != true) ++p_table; p_list = p_table->head; }
public:
  explicit Iterator(list *p_table = nullptr, Element *p_list = nullptr):
	            p_table{p_table}, p_list{p_list}
	            {}
  reference operator*() const {return p_list->key;}
  pointer operator->() const {return &p_list->key;}
  Iterator &operator++() {
   if(p_list->next!=nullptr)
     p_list = p_list->next;
   else{
     ++p_table;
     p_list = p_table->head;
     skip();
       }
   return *this;
  }
  Iterator operator++(int){ auto rc{*this}; ++*this; return rc; }
  friend bool operator==(const Iterator &lhs, const Iterator &rhs)
	 {
	  return lhs.p_table == rhs.p_table && lhs.p_list == rhs.p_list;
	 }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs){return !(lhs == rhs);}
};
template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }


#endif // ADS_SET_H
