// Copyright 2023 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
#pragma once

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>

#include <cstdint>
#include <cstring>

#include "pw_function/function.h"
#include "pw_span/span.h"
#include "pw_string/util.h"
#include "pw_thread_zephyr/config.h"

namespace pw::thread {

class Thread;  // Forward declare Thread which depends on Context.

}  // namespace pw::thread

namespace pw::thread::zephyr {

class Options;

// At the moment, Zephyr RTOS doesn't support dynamic thread stack allocation
// (due to various alignment and size requirements on different architectures).
// Still, we separate the context in two parts:
//
//   1) Context which just contains the Thread Control Block (k_thread) and
//      additional context pw::Thread requires.
//
//   2) StaticContextWithStack which contains the stack.
//
// Only StaticContextWithStack can be instantiated directly.
class Context {
 public:
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

 protected:
  // We can't use `= default` here, because it allows to create an Context
  // instance in C++17 with `pw::thread::zephyr::Context{}` syntax.
  Context() {}

 private:
  friend Thread;

  static void CreateThread(const Options& options,
                           Function<void()>&& thread_fn,
                           Context*& native_type_out);

  k_tid_t task_handle() const { return task_handle_; }
  void set_task_handle(const k_tid_t task_handle) {
    task_handle_ = task_handle;
  }

  k_thread& thread_info() { return thread_info_; }

  void set_thread_routine(Function<void()>&& rvalue) {
    fn_ = std::move(rvalue);
  }

  bool detached() const { return detached_; }
  void set_detached(bool value = true) { detached_ = value; }

  bool thread_done() const { return thread_done_; }
  void set_thread_done(bool value = true) { thread_done_ = value; }

  const char* name() const { return name_.data(); }
  void set_name(const char* name) { string::Assign(name_, name).IgnoreError(); }

  static void ThreadEntryPoint(void* void_context_ptr, void*, void*);

  k_tid_t task_handle_ = nullptr;
  k_thread thread_info_;
  Function<void()> fn_;
  bool detached_ = false;
  bool thread_done_ = false;

  // The TCB may have storage for the name, depending on the setting of
  // CONFIG_THREAD_NAME, and if storage is present, the reserved
  // space will depend on CONFIG_THREAD_MAX_NAME_LEN. In order to provide
  // a consistent interface, we always store the string here, and use
  // k_thread_name_set to set the name for the thread, if it is available.
  // We will defer to our storage when queried for the name, but by setting
  // the name with the RTOS call, raw RTOS access to the thread's name should
  // work properly, though possibly with a truncated name.
  pw::InlineString<config::kMaximumNameLength> name_;
};

// Intermediate class to type-erase kStackSizeBytes parameter of
// StaticContextWithStack.
class StaticContext : public Context {
 protected:
  explicit StaticContext(z_thread_stack_element* stack,
                         size_t available_stack_size)
      : stack_(stack), available_stack_size_(available_stack_size) {}

 private:
  friend Context;

  z_thread_stack_element* stack() { return stack_; }
  size_t available_stack_size() { return available_stack_size_; }

  // Zephyr RTOS doesn't specify how Zephyr-owned thread information is
  // stored in the stack, how much spaces it takes, etc.
  // All we know is that K_THREAD_STACK(stack, size) macro will allocate
  // enough memory to hold size bytes of user-owned stack and that
  // we must pass that stack pointer to k_thread_create.
  z_thread_stack_element* stack_;
  size_t available_stack_size_;
};

// Static thread context allocation including the stack along with the Context.
//
// See docs.rst for an usage example.
template <size_t kStackSizeBytes>
class StaticContextWithStack final : public StaticContext {
 public:
  constexpr StaticContextWithStack()
      : StaticContext(stack_storage_, kStackSizeBytes) {}

 private:
  K_KERNEL_STACK_MEMBER(stack_storage_, kStackSizeBytes);
};

}  // namespace pw::thread::zephyr
