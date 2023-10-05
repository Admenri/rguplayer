// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINER_LINK_LIST_H_
#define BASE_CONTAINER_LINK_LIST_H_

#include <stdint.h>

namespace base {

template <typename T>
class LinkNode {
 public:
  LinkNode(T *data) : prev_(nullptr), next_(nullptr), data_(data) {}

  LinkNode(const LinkNode &) = delete;
  LinkNode &operator=(const LinkNode &) = delete;

  LinkNode *Prev() { return prev_; }
  LinkNode *Next() { return next_; }
  T *Data() { return data_; }

  virtual ~LinkNode() {
    if (prev_ && next_) {
      next_->prev_ = prev_;
      prev_->next_ = next_;
    }
  }

 private:
  template <typename T>
  friend class LinkList;

  LinkNode<T> *prev_;
  LinkNode<T> *next_;
  T *data_;
};

template <typename T>
class LinkList {
 public:
  LinkList() : root_(nullptr), size_(0) {
    root_.prev_ = &root_;
    root_.next_ = &root_;
  }

  LinkList(const LinkList &) = delete;
  LinkList &operator=(const LinkList) = delete;

  void PushFront(LinkNode<T> &node) {
    root_.next_->prev_ = &node;
    node.prev_ = &root_;
    node.next_ = root_.next_;
    root_.next_ = &node;

    size_++;
  }

  void PushBack(LinkNode<T> &node) {
    root_.prev_->next_ = &node;
    node.next_ = &root_;
    node.prev_ = root_.prev_;
    root_.prev_ = &node;

    size_++;
  }

  void InsertBefore(LinkNode<T> &node, LinkNode<T> &prev) {
    node.next_ = &prev;
    node.prev_ = prev.prev_;
    prev.prev_->next_ = &node;
    prev.prev_ = &node;

    size_++;
  }

  void Remove(LinkNode<T> &node) {
    if (!node.next_) return;

    node.prev_->next_ = node.next_;
    node.next_->prev_ = node.prev_;

    node.prev_ = 0;
    node.next_ = 0;

    size_--;
  }

  void Clear() {
    Remove(root_);
    root_.prev_ = &root_;
    root_.next_ = &root_;

    size_ = 0;
  }

  bool Empty() const { return root_.next_ == &root_; }

  LinkNode<T> *begin() { return root_.next_; }
  LinkNode<T> *end() { return &root_; }

 private:
  LinkNode<T> root_;
  size_t size_;
};

}  // namespace base

#endif  // BASE_CONTAINER_LINK_LIST_H_