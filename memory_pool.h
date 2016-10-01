/*
 * Copyright 2016 Waizung Taam
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Create time: 2016-10-02 
 * Email: waizungtaam@gmail.com
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <new>


/* Single Thread Memory Pool Implementation */

class MemoryPool {
public:
  MemoryPool(int num_unit, int unit_size) :
    num_unit_(num_unit), unit_size_(unit_size), 
    list_memory_(nullptr), client_memory_(nullptr),
    allocated_unit_blocks_(nullptr), allocated_memory_blocks_(nullptr),
    free_memory_list_(nullptr), allocated_memory_list_(nullptr) {
    bool expandable = expand_pool();
    if (!expandable) {
      throw std::bad_alloc();
    }
  }
  ~MemoryPool() {
    free_allocated_unit_blocks();
    free_allocated_memory_blocks();
  }

  void* allocate() {
    bool expandable = true;
    if (free_memory_list_ == nullptr) {
      expandable = expand_pool();
    }
    if (expandable) {
      return allocate_unit();
    } else {
      return nullptr;
    }
  }

  void deallocate(void* p) {
    if (p == nullptr || allocated_memory_list_ == nullptr) {
      return;
    }
    free_unit(p);
  }

private:
  /* A single node of the memory pool list */
  struct Unit {
    void* address;
    Unit* next;
  };
  /* A block of Unit */
  struct UnitBlock {
    UnitBlock(unsigned char* a, UnitBlock* n) : 
      address(a), next(n) {}
    unsigned char* address;
    UnitBlock* next;    
  };
  /* A block of actual memory */
  struct MemoryBlock {
    MemoryBlock(unsigned char* a, MemoryBlock* n) : 
      address(a), next(n) {}
    unsigned char* address;
    MemoryBlock* next;    
  };

  bool expand_allocated_unit_blocks() {
    UnitBlock* new_unit_block = nullptr;
    if (allocated_unit_blocks_ == nullptr) {
      new_unit_block = new UnitBlock(list_memory_, nullptr);
    } else {
      new_unit_block = new UnitBlock(
                       list_memory_, allocated_unit_blocks_);
    }
    allocated_unit_blocks_ = new_unit_block;
  }
  bool expand_allocated_memory_blocks() {
    MemoryBlock* new_memory_block = nullptr;
    if (allocated_memory_blocks_ == nullptr) {
      new_memory_block = new MemoryBlock(client_memory_, nullptr);
    } else {
      new_memory_block = new MemoryBlock(
                         client_memory_, allocated_memory_blocks_);
    }
    allocated_memory_blocks_ = new_memory_block;
  }
  bool expand_free_memory_list() {
    Unit* new_unit = nullptr;
    for (int i = 0; i < num_unit_; ++i) {
      new_unit = reinterpret_cast<Unit*>(
                 list_memory_ + (i * sizeof(Unit)));
      new_unit->address = reinterpret_cast<void*>(
                          client_memory_ + (i * unit_size_));
      new_unit->next = free_memory_list_;
      free_memory_list_ = new_unit;
    }    
  }
  bool expand_pool() {
    list_memory_ = new(std::nothrow) unsigned char[
                   num_unit_ * sizeof(Unit)];    
    client_memory_ = new(std::nothrow) unsigned char[
                   num_unit_ * unit_size_];    
    if (client_memory_ == nullptr || list_memory_ == nullptr) {
      delete[] client_memory_;
      delete[] list_memory_;
      return false;
    }
    expand_allocated_unit_blocks();
    expand_allocated_memory_blocks();
    expand_free_memory_list();
    return true;
  }

  void free_allocated_unit_blocks() {
    UnitBlock* deleted_unit_block = nullptr;
    while (allocated_unit_blocks_ != nullptr) {
      deleted_unit_block = allocated_unit_blocks_;
      allocated_unit_blocks_ = allocated_unit_blocks_->next;
      delete[](deleted_unit_block->address);
      delete deleted_unit_block;
    }
  }
  void free_allocated_memory_blocks() {
    MemoryBlock* deleted_memory_block = nullptr;
    while (allocated_memory_blocks_ != nullptr) {
      deleted_memory_block = allocated_memory_blocks_;
      allocated_memory_blocks_ = allocated_memory_blocks_->next;
      delete[](deleted_memory_block->address);
      delete deleted_memory_block;
    }    
  }

  Unit* allocate_unit() {
    Unit* unit = free_memory_list_;
    free_memory_list_ = free_memory_list_->next;
    unit->next = allocated_memory_list_;
    allocated_memory_list_ = unit;
    return unit;
  }
  void free_unit(void* p) {
    Unit* unit = allocated_memory_list_;
    allocated_memory_list_ = allocated_memory_list_->next;
    unit->address = p;
    unit->next = free_memory_list_;
    free_memory_list_ = unit;
  }

  const int num_unit_;  // number of units in the pool
  const int unit_size_;  // size of each unit

  unsigned char* list_memory_;  // memory of the memory pool list
  unsigned char* client_memory_;  // memory of the client data

  UnitBlock* allocated_unit_blocks_;  // allocated UnitBlock s
  MemoryBlock* allocated_memory_blocks_;  // allocated MemoryBlock s

  Unit* free_memory_list_;  // the units that are free in the pool
  Unit* allocated_memory_list_;  // the units that are occupyed
};

#endif  // MEMORY_POOL_H