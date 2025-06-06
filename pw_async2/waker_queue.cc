// Copyright 2025 The Pigweed Authors
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

#include "pw_async2/waker_queue.h"

#include "pw_log/tokenized_args.h"

namespace pw::async2::internal {

bool StoreWaker(Context& cx, WakerQueueBase& queue, log::Token wait_reason) {
  Waker waker;
  CloneWaker(*cx.waker_, waker, wait_reason);
  return queue.Add(std::move(waker));
}

void WakerQueueBase::WakeMany(size_t count) {
  while (count > 0 && !empty()) {
    Waker& waker = queue_.front();
    std::move(waker).Wake();
    queue_.pop();
    count--;
  }
}

}  // namespace pw::async2::internal
