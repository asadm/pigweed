# Copyright 2025 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

pw_add_library(pw_interrupt_freertos.context INTERFACE
  HEADERS
    public/pw_interrupt_freertos/context_inline.h
    public_overrides/pw_interrupt_backend/context_inline.h
  PUBLIC_INCLUDES
    public
    public_overrides
  PUBLIC_DEPS
    pw_third_party.freertos
)
