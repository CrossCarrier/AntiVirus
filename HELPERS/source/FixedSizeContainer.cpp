#include "../include/FixedSizeContainer.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"

#define TEMPLATE template <typename T>

namespace {
      TEMPLATE
      auto categorize_shared_ptr(std::shared_ptr<T> _ptr, FixedSizeContainer<T> *struct_ptr,
                                 char flag) -> std::shared_ptr<T> {
            if (_ptr == nullptr) {
                  _ptr = struct_ptr->begin();
            }
            if (_ptr >= struct_ptr->begin() && _ptr < struct_ptr->end()) {
                  _ptr++;
            }
            if (_ptr == struct_ptr->end()) {
                  throw FullContainer("Cannot add any more objects to this container!\n");
            }
            return _ptr;
      }
} // namespace

TEMPLATE
FixedSizeContainer<T>::FixedSizeContainer(const size_t _size) : m_Size{_size} {
      if (m_Size <= 0) {
            throw InvalidContainerSize("Container size must be unsigned integer above 0!");
      }
      m_Container(new T[m_Size]);
      m_HELPER = nullptr;
}

TEMPLATE
auto FixedSizeContainer<T>::operator<<(const T &other) -> void {
      *(categorize_shared_ptr(this->m_HELPER, this));
}
TEMPLATE
auto FixedSizeContainer<T>::operator<<(T &&other) -> void {
      *(categorize_shared_ptr(this->m_HELPER, this)) = std::move(other);
}

TEMPLATE
auto FixedSizeContainer<T>::operator==(const FixedSizeContainer<T> &other) const noexcept -> bool {
      if (this->m_Size == other.m_Size) {
            for (size_t ptr_offset = 0; ptr_offset < this->m_Size; ptr_offset++) {
                  if (*(this->m_Container + ptr_offset) != *(other.m_Container + ptr_offset)) {
                        return false;
                  }
            }
            return true;
      }
      return false;
}

TEMPLATE
auto FixedSizeContainer<T>::operator!=(const FixedSizeContainer<T> &other) const noexcept -> bool {
      if (*this == other) {
            return false;
      }
      return true;
}

TEMPLATE
auto FixedSizeContainer<T>::operator[](size_t idx) const -> T & {
      if (idx > this->m_Size) {
            throw InvalidIndex("Provided index exceed conatiner size!\n");
      }
      return *(this->m_Container + (idx - 1));
}

TEMPLATE
FixedSizeContainer<T>::itterator::itterator(POINTER _ptr) : m_Ptr{_ptr} {};
TEMPLATE
FixedSizeContainer<T>::itterator::itterator(const itterator &other) : m_Ptr{other.m_Ptr} {}
TEMPLATE
auto FixedSizeContainer<T>::itterator::operator++() noexcept -> REFERENCE {
      this->m_Ptr++;
      return this;
}
TEMPLATE
auto FixedSizeContainer<T>::itterator::operator++(T) -> itterator {
      itterator temp(m_Ptr++);
      *this = temp;
      return *this;
}

TEMPLATE
auto FixedSizeContainer<T>::begin() const -> itterator { return itterator(&(this->m_First)); }
TEMPLATE
auto FixedSizeContainer<T>::end() const -> itterator { return itterator(&(this->m_Third)); }
