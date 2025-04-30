#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>

template <typename T> class FixedSizeContainer {
      using pointer = std::unique_ptr<T>;

  private:
      size_t m_Size;
      pointer m_Container;

  public:
      FixedSizeContainer(const size_t _size);
      ~FixedSizeContainer();

      // Custom itterator
      struct itterator {
            using DISTANCE  = std::ptrdiff_t;
            using VALUE     = T;
            using POINTER   = T *;
            using REFERENCE = T &;

            POINTER m_Ptr;
            itterator(POINTER ptr);
            itterator(const itterator &other);

            auto operator++() noexcept -> REFERENCE;
            auto operator++(T) -> itterator;
      };

      auto begin() const -> itterator;
      auto end() const -> itterator;

      auto operator[](size_t idx) const -> T &;
      auto operator==(const FixedSizeContainer &other) const noexcept -> bool;
      auto operator!=(const FixedSizeContainer &other) const noexcept -> bool;
};
