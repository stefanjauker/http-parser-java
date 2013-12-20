/*
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifndef _vector_h_
#define _vector_h_

#include <assert.h>
#include <string.h>
#include <stdlib.h>

// a minimalist replacement for STL vector to avoid link errors with VC++ 2010

#ifndef NDEBUG
#define MALLOC_CHECK_ 1
#endif

template <class T>
class vector {
private:
  T** _data;
  size_t _capacity;
  size_t _size;

  void expand(const size_t capacity);

public:
  vector(size_t capacity);
  vector(const vector<T>& src);
  ~vector();

  vector& operator= (const vector<T>& x);
  void push_back(T* element);
  void clear();
  T* at(size_t index);
  size_t size();
  size_t capacity();
  T** data();
};

template <class T>
void vector<T>::expand(const size_t capacity) {
  assert(capacity > _capacity);
  _data = (T**) realloc(_data, capacity * sizeof(T*));
  for (int i=_capacity; i < capacity; i++) {
    _data[i] = (T*) NULL;
  }
  _capacity = capacity;
}

template <class T>
vector<T>::vector(size_t capacity) {
  _size = 0;
  _capacity = capacity;
  _data = (T**) calloc(_capacity, sizeof(T*));
}

template <class T>
vector<T>::vector(const vector<T>& src) {
  _size = src.size();
  _capacity = _size;
  _data = (T**) calloc(_capacity, sizeof(T*));
  for (int i=0; i < _size; i++) {
    _data[i] = src.at[i];
  }
}

template <class T>
vector<T>& vector<T>::operator=(const vector<T>& src) {
  clear();
  _size = src.size();
  _capacity = _size;
  _data = (T**) realloc(_data, _capacity * sizeof(T*));
  for (int i=0; i < _size; i++) {
    _data[i] = src.at[i];
  }
  return *this;
}

template <class T>
vector<T>::~vector() {
  clear();
  _size = 0;
  free(_data);
}

template <class T>
void vector<T>::push_back(T* element) {
  if (_size >= _capacity) {
    // mimic growth strategy of ArrayList.java in jdk8
    expand(_capacity + (_capacity >> 1));
  }
  assert(_size < _capacity);
  _data[_size++] = element;
}

template <class T>
void vector<T>::clear() {
  for (int i=0; i < _size; i++) {
    delete _data[i];
  }
  _size = 0;
}

template <class T>
inline T* vector<T>::at(size_t index) {
  assert(index < _size);
  return _data[index];
}

template <class T>
inline size_t vector<T>::size() {
  return _size;
}

template <class T>
inline size_t vector<T>::capacity() {
  return _capacity;
}

template <class T>
inline T** vector<T>::data() {
  return _data;
}

#endif // _vector_h_
