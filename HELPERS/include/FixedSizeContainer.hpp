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
      FixedSizeContainer(const FixedSizeContainer<T>& _other);
      FixedSizeContainer(FixedSizeContainer<T>&& _other) noexcept;
      ~FixedSizeContainer() = default;

      auto operator=(const FixedSizeContainer<T>& _other) -> FixedSizeContainer<T>;
      auto operator=(FixedSizeContainer<T>&& _other) noexcept -> FixedSizeContainer<T>;

      // Custom itterator
      struct iterator {
            using DISTANCE  = std::ptrdiff_t;
            using VALUE     = T;
            using POINTER   = T *;
            using REFERENCE = T &;
            POINTER m_Ptr;
      
            iterator(POINTER ptr);
            iterator(const iterator &other);

            auto operator++() noexcept -> iterator&;
            auto operator++(int) -> iterator;
            auto operator*() const -> REFERENCE;
            auto operator->() -> POINTER;

            friend auto operator==(const iterator& _first, const iterator& _second) -> bool {
                  return _first.m_Ptr == _second.m_Ptr;
            }
            friend auto operator!=(const iterator& _first, const iterator& _second) -> bool {
                  return _first.m_Ptr != _second.m_Ptr;
            }
      };

      auto begin() const -> iterator;
      auto end() const -> iterator;

      auto operator[](size_t idx) const -> T &;
      auto operator==(const FixedSizeContainer &other) const noexcept -> bool;
      auto operator!=(const FixedSizeContainer &other) const noexcept -> bool;
      auto operator<<(const T &_value) -> void;
      auto operator<<(T &&_value) -> void;
};
