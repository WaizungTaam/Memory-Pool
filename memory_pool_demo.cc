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

#include "memory_pool.h"

#include <chrono>
#include <cstddef>
#include <iostream>

const int LEN = 100000;

struct Foo {
  static MemoryPool* memory_pool_;

  void* operator new(std::size_t size) { return memory_pool_->allocate(); }
  void operator delete(void* p) { return memory_pool_->deallocate(p); }
};
MemoryPool* Foo::memory_pool_ = nullptr;

struct Bar {};


void demo(int num_unit) {
  MemoryPool pool(num_unit, sizeof(Foo));
  Foo::memory_pool_ = &pool;

  auto a_start = std::chrono::high_resolution_clock::now();
  Foo* arr[LEN];
  for (int i = 0; i < LEN; ++i) {
    arr[i] = new Foo();
  }
  for (int i = 0; i < LEN; ++i) {
    delete arr[i];
  }
  auto a_end = std::chrono::high_resolution_clock::now();

  auto b_start = std::chrono::high_resolution_clock::now();
  Bar* b[LEN];
  for (int i = 0; i < LEN; ++i) {
    b[i] = new Bar();
  }
  for (int i = 0; i < LEN; ++i) {
    delete b[i];
  }
  auto b_end = std::chrono::high_resolution_clock::now();

  std::cout << std::chrono::duration<double, std::milli>(
               a_end - a_start).count() << " ms\n"
            << std::chrono::duration<double, std::milli>(
               b_end - b_start).count() << " ms\n";  
}

int main() {
  auto num_units = { 2, 128, 1024 };

  for (auto num_unit : num_units) {
    std::cout << "num_unit: " << num_unit << "\n";
    demo(num_unit);
    std::cout << "\n";
  }
}


// $ valgrind --leak-check=full ./memory_pool_demo.o