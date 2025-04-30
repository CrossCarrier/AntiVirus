#pragma once

#include <cstddef>
#include <memory>

template <typename T> class FixedSizeContainer {
private:
      size_t m_Size;
      std::shared_ptr<T> m_Container;
      std::shared_ptr<T> m_HELPER;

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
      auto operator<<(const T &other) -> void;
      auto operator<<(T &&other) -> void;
};
