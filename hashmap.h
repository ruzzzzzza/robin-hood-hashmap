// made by ruzzzzzza 30.01.23

#pragma once

#include <memory>
#include <vector>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:

    struct ElementOfHashMap { // contains info about element

        std::unique_ptr<std::pair<const KeyType, ValueType>> pair_ = nullptr;
        size_t dist_to_main_pos_ = 0;
        bool dead_ = false;

        ElementOfHashMap() = default;

        explicit ElementOfHashMap(const std::pair<const KeyType, ValueType> &element) {
            pair_ = std::make_unique<std::pair<const KeyType, ValueType>>(element);
        }

        ElementOfHashMap(const ElementOfHashMap &other) {
            dist_to_main_pos_ = other.dist_to_main_pos_;
            dead_ = other.dead_;
            if (other.pair_ == nullptr) {
                pair_ = nullptr;
            } else {
                pair_ = std::make_unique<std::pair<const KeyType, ValueType>>(*other.pair_);
            }
        }

        ElementOfHashMap &operator=(const ElementOfHashMap &other) {
            dist_to_main_pos_ = other.dist_to_main_pos_;
            dead_ = other.dead_;
            if (other.pair_ == nullptr) {
                pair_ = nullptr;
            } else {
                pair_ = std::make_unique<std::pair<const KeyType, ValueType>>(*other.pair_);
            }
            return *this;
        }

        ElementOfHashMap(ElementOfHashMap &&other) {
            dist_to_main_pos_ = std::move(other.dist_to_main_pos_);
            dead_ = std::move(other.dead_);
            pair_ = std::move(other.pair_);
        }

        ElementOfHashMap &operator=(ElementOfHashMap &&other) {
            dist_to_main_pos_ = std::move(other.dist_to_main_pos_);
            dead_ = std::move(other.dead_);
            pair_ = std::move(other.pair_);
            return *this;
        }

    };

    class iterator { // NOLINT
    public:

        iterator() = default;

        explicit iterator(size_t other_index, const HashMap *other_hash_map) : index_(other_index),
                                                                               hash_map_(other_hash_map) {}

        std::pair<const KeyType, ValueType> &operator*() {
            return *hash_map_->list_of_elements_[index_].pair_;
        }

        std::pair<const KeyType, ValueType> *operator->() {
            return &(*hash_map_->list_of_elements_[index_].pair_);
        }

        iterator &operator=(iterator other) {
            index_ = other.index_;
            hash_map_ = other.hash_map_;
            return *this;
        }

        iterator &operator++() {
            do {
                ++index_;
            } while (index_ != hash_map_->capacity_ && hash_map_->list_of_elements_[index_].pair_ == nullptr);
            return *this;
        }

        iterator operator++(int) {
            iterator new_pointer(*this);
            ++*this;
            return new_pointer;
        }

        bool operator==(const iterator &other) const {
            return (index_ == other.index_ && hash_map_ == other.hash_map_);
        }

        bool operator!=(const iterator &other) const {
            return !(*this == other); // NOLINT
        }

    private:
        size_t index_ = 0; // index of the element in map
        const HashMap *hash_map_ = nullptr;
    };

    class const_iterator { // NOLINT
    public:

        const_iterator() = default;

        explicit const_iterator(size_t other_index, const HashMap *other_hash_map) : index_(other_index),
                                                                                     hash_map_(other_hash_map) {}

        const std::pair<const KeyType, ValueType> &operator*() const {
            return *hash_map_->list_of_elements_[index_].pair_;
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return &(*hash_map_->list_of_elements_[index_].pair_);
        }

        const_iterator &operator=(const_iterator other) {
            index_ = other.index_;
            hash_map_ = other.hash_map_;
            return *this;
        }

        const_iterator &operator++() {
            do {
                ++index_;
            } while (index_ != hash_map_->capacity_ && hash_map_->list_of_elements_[index_].pair_ == nullptr);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator new_pointer(*this);
            ++*this;
            return new_pointer;
        }

        bool operator==(const const_iterator &other) const {
            return (index_ == other.index_ && hash_map_ == other.hash_map_);
        }

        bool operator!=(const const_iterator &other) const {
            return !(*this == other); // NOLINT
        }

    private:
        size_t index_ = 0; // index of the element in map
        const HashMap *hash_map_ = nullptr;
    };

    explicit HashMap(Hash hasher = Hash()) : hasher_(hasher) {

    }

    template<class Forwarditerator>
    HashMap(Forwarditerator begin, Forwarditerator end, Hash hasher = Hash()): hasher_(hasher) {
        for (Forwarditerator current = begin; current != end; ++current) {
            insert(*current);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash hasher = Hash()) : hasher_(hasher) {
        for (auto &element: list) {
            insert(element);
        }
    }

    HashMap(const HashMap &other, Hash hasher = Hash()) : hasher_(hasher) {
        list_of_elements_.clear();
        for (auto &u: other.list_of_elements_) {
            list_of_elements_.push_back(ElementOfHashMap(u));
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    Hash hash_function() const {
        return hasher_;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return (size_ == 0);
    }

    iterator end() {
        return iterator(capacity_, this);
    }

    const_iterator end() const {
        return const_iterator(capacity_, this);
    }

    iterator begin() {
        if (size_ == 0) {
            return this->end();
        }
        iterator iter(0, this); // first element in list
        if (this->list_of_elements_[0].pair_ != nullptr) { // if it contains value we return it
            return iter;
        }
        return ++iter; // if it doesn't contain value we return next, which contains
    }

    const_iterator begin() const {
        if (size_ == 0) {
            return this->end();
        }
        const_iterator iter(0, this);
        if (this->list_of_elements_[0].pair_ != nullptr) { // if it contains value we return it
            return iter;
        }
        return ++iter; // if it doesn't contain value we return next, which contains
    }

    iterator find(const KeyType &key) {
        if (capacity_ == 0) {
            return this->end();
        }
        size_t hash = hasher_(key);
        size_t pos = hash % capacity_;
        while (list_of_elements_[pos].dead_ ||
               (list_of_elements_[pos].pair_ != nullptr &&
                !((list_of_elements_[pos].pair_->first) == key))) { // robin hood with tombstones // NOLINT
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        return ((list_of_elements_[pos].pair_ == nullptr) ? this->end() : iterator(pos, this));
    }

    const_iterator find(const KeyType &key) const {
        if (capacity_ == 0) {
            return this->end();
        }
        size_t hash = hasher_(key);
        size_t pos = hash % capacity_;
        while (list_of_elements_[pos].dead_ ||
               (list_of_elements_[pos].pair_ != nullptr &&
                !((list_of_elements_[pos].pair_->first) == key))) { // robin hood with tombstones // NOLINT
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        return ((list_of_elements_[pos].pair_ == nullptr) ? this->end() : const_iterator(pos, this));
    }

    void insert(std::pair<KeyType, ValueType> element) {
        if (find(element.first) != this->end()) {
            return;
        }
        if (capacity_ == 0) {
            relax();
        }
        ElementOfHashMap current_element(element);
        size_t hash = hasher_(element.first);
        size_t pos = hash % capacity_;
        insert(pos, std::move(current_element), list_of_elements_); // inserting in list
        ++size_;
        relax();
    }

    void erase(KeyType key) {
        if (find(key) == this->end()) {
            return;
        }
        if (capacity_ == 0) {
            relax();
        }
        size_t hash = hasher_(key);
        size_t pos = hash % capacity_;
        while (list_of_elements_[pos].pair_ == nullptr || list_of_elements_[pos].pair_->first != key) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        list_of_elements_[pos].pair_ = nullptr;
        list_of_elements_[pos].dead_ = true; // it becomes tombstone
        list_of_elements_[pos].dist_to_main_pos_ = 0;
        --size_;
        relax();
    }

    ValueType &operator[](KeyType key) {
        if (find(key) == this->end()) {
            insert({key, ValueType()});
        }
        return find(key)->second;
    }

    const ValueType &at(KeyType key) const {
        if (find(key) == this->end()) {
            throw std::out_of_range("Error! Out of range!");
        }
        return find(key)->second;
    }

    void clear() {
        size_ = 0;
        capacity_ = 0;
        list_of_elements_.clear();
    }

private:

    Hash hasher_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    double max_load_factor_ = .5;
    double min_load_factor_ = .125;
    double the_best_load_factor_ = .25;

    std::vector<ElementOfHashMap> list_of_elements_;

    void
    insert(size_t pos, ElementOfHashMap current_element, std::vector<ElementOfHashMap> &list_of_elements) { // NOLINT
        while (list_of_elements[pos].pair_ != nullptr) { // robin hood
            if (current_element.dist_to_main_pos_ < list_of_elements[pos].dist_to_main_pos_) {
                std::swap(current_element, list_of_elements[pos]);
                ++current_element.dist_to_main_pos_;
            }
            ++pos;
            if (pos == list_of_elements.size()) {
                pos = 0;
            }
        }
        list_of_elements[pos] = current_element;
    }

    bool is_prime(size_t value) const {
        for (size_t i = 2; i * i <= value; ++i) {
            if (value % i == 0) {
                return false;
            }
        }
        return true;
    }

    size_t next_prime(size_t value) const {
        while (!is_prime(value)) {
            ++value;
        }
        return value;
    }

    void relax() { // looking at load factors
        if (capacity_ == 0) {
            list_of_elements_.push_back({});
            ++capacity_;
            return;
        }

        if (size_ == 0) {
            list_of_elements_.clear();
            list_of_elements_.push_back({});
            capacity_ = 1;
            return;
        }

        if (static_cast<double> (size_) <= min_load_factor_ * static_cast<double> (capacity_) ||
            static_cast<double> (size_) >= max_load_factor_ * static_cast<double> (capacity_)) {
            size_t new_capacity = next_prime(
                    static_cast<double>(size_) / the_best_load_factor_); // we want fewer collisions, so use prime nums
            std::vector<ElementOfHashMap> new_list(new_capacity);
            for (auto &current_element: list_of_elements_) {
                if (current_element.pair_ == nullptr) {
                    continue;
                }
                size_t hash = hasher_(current_element.pair_->first);
                size_t pos = hash % new_capacity;
                insert(pos, std::move(current_element), new_list);
            }
            capacity_ = new_capacity;
            swap(list_of_elements_, new_list);
        }
    }
};
