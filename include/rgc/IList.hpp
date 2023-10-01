#ifndef RENDERGRAPHCOMPILER_ILIST_HPP
#define RENDERGRAPHCOMPILER_ILIST_HPP

#include <cassert>
#include <unordered_map>

namespace rgc {

template <typename T> class IList;
template <typename T, bool Reverse> class IListIterator;

template <typename T> class IListNode {
public:
  explicit IListNode(IListNode *prev = nullptr, IListNode *next = nullptr)
      : m_prev{prev}, m_next{next} {}
  virtual ~IListNode() = default;

private:
  bool m_not_connected() const { return !m_prev && !m_next; }

  IListNode *m_prev;
  IListNode *m_next;
  friend class IList<T>;
  friend class IListIterator<T, true>;
  friend class IListIterator<T, false>;
};

template <typename T, bool Reverse = false> class IListIterator {
public:
  explicit IListIterator(IListNode<T> *current) : m_current{current} {}

  using difference_type = void;
  using value_type = T *;
  using pointer = void;
  using reference = void;
  using iterator_category = std::bidirectional_iterator_tag;

  T *operator*() const { return static_cast<T *>(m_current); }

  auto &operator++() {
    m_current = Reverse ? m_current->m_prev : m_current->m_next;
    return *this;
  }

  auto &operator++(int) {
    auto ret = *this;
    operator++();
    return ret;
  }

  auto &operator--() {
    m_current = Reverse ? m_current->m_next : m_current->m_prev;
    return *this;
  }

  auto &operator--(int) {
    auto ret = *this;
    operator--();
    return ret;
  }

  bool operator==(const IListIterator &another) const {
    return m_current == another.m_current;
  }

private:
  IListNode<T> *m_current;
};

template <typename T> using IListForwardIterator = IListIterator<T, false>;

template <typename T> using IListReverseIterator = IListIterator<T, false>;

template <typename T> class IList {
public:
  static_assert(std::derived_from<T, IListNode<T>>,
                "IListNode<T> must be a base class for T");
  auto begin() const { return IListForwardIterator<T>{m_head}; }

  auto end() const { return IListForwardIterator<T>{nullptr}; }

  auto rbegin() const { return IListReverseIterator<T>{m_tail}; }

  auto rend() const { return IListReverseIterator<T>{nullptr}; }

  T *front() const { return static_cast<T *>(m_head); }

  T *back() const { return static_cast<T *>(m_tail); }

  auto size() const { return m_container.size(); }

  bool empty() const { return m_container.empty(); }

  void insertAfter(IListNode<T> *node, IListNode<T> *after) {
    assert(node && "can't emplace null node");
    assert(node->m_not_connected() && "can insert only unconnected node");
    assert(!m_container.contains(node) && "double insertion");
    m_container.emplace(node, node);
    if (after == nullptr) {
      // Insert before head
      if (m_head == nullptr) {
        m_head = node;
        m_tail = node;
        return;
      }
      m_head->m_prev = node;
      node->m_next = m_head;
      m_head = node;
      return;
    }
    assert(m_container.contains(after) && "'after' node is not registered");

    if (after == m_tail)
      m_tail = node;
    else {
      auto *next = after->m_next;
      assert(next && "null next node");
      next->m_prev = node;
    }
    after->m_next = node;
    node->m_prev = after;
  }

  void insertBefore(IListNode<T> *node, IListNode<T> *before) {
    assert(node && "can't emplace null node");
    assert(node->m_not_connected() && "can insert only unconnected node");
    assert(!m_container.contains(node) && "double insertion");
    m_container.emplace(node, node);
    if (before == nullptr) {
      // Insert after tail
      if (m_tail == nullptr) {
        m_head = node;
        m_tail = node;
        return;
      }
      m_tail->m_next = node;
      node->m_prev = m_tail;
      m_tail = node;
      return;
    }
    assert(m_container.contains(before) && "'before' node is not registered");

    if (before == m_head)
      m_head = node;
    else {
      auto *prev = before->m_prev;
      assert(prev && "null prev node");
      prev->m_next = node;
    }
    before->m_prev = node;
    node->m_next = before;
  }

  void push_back(IListNode<T> *node) { insertAfter(node, m_tail); }

  void push_front(IListNode<T> *node) { insertBefore(node, m_head); }

  void erase(IListNode<T> *node) {
    assert(m_container.contains(node) && "erased node is not registered");
    auto *next = node->m_next;
    auto *prev = node->m_prev;
    if (next) {
      next->m_prev = prev;
    }
    if (prev) {
      prev->m_next = next;
    }
    if (node == m_head) {
      m_head = next;
    }
    if (node == m_tail) {
      m_tail = prev;
    }
    m_container.erase(node);
  }

private:
  IListNode<T> *m_head = nullptr;
  IListNode<T> *m_tail = nullptr;
  std::unordered_map<IListNode<T> *, std::unique_ptr<IListNode<T>>> m_container;
};

} // namespace rgc
#endif // RENDERGRAPHCOMPILER_ILIST_HPP
