#include "../include/FixedSizeContainer.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include <boost/mpl/lambda.hpp>
#include <memory>

#define TEMPLATE template <typename T>

TEMPLATE
FixedSizeContainer<T>::FixedSizeContainer(const size_t _size) : m_Size{_size} {
      if (m_Size <= 0) {
            throw InvalidContainerSize("Container size must be unsigned integer above 0!");
      }

      m_Container(new T[m_Size]);
}

TEMPLATE
auto FixedSizeContainer<T>::operator==(const FixedSizeContainer<typename T><T> &other) const noexcept -> bool {
      bool f_cond = (this->m_First == other.First());
      bool s_cond = (this->m_Second == other.Second());
      bool t_cond = (this->m_Third == other.Third());

      return (f_cond && s_cond && t_cond);
}

TEMPLATE
auto Triplet<T>::operator!=(const Triplet<T> &other) const noexcept -> bool {
      if (*this == other) {
            return false;
      }
      return true;
}

TEMPLATE
auto Triplet<T>::operator[](size_t idx) const -> T & {
      switch (idx) {
      case 0:
            return this->m_First;
            break;
      case 1:
            return this->m_Second;
            break;
      case 2:
            return this->m_Third;
            break;
      default:
            throw std::invalid_argument("Index out the range!\n");
            break;
      }
}

TEMPLATE
Triplet<T>::itterator::itterator(POINTER _ptr) : m_Ptr{_ptr} {};
TEMPLATE
Triplet<T>::itterator::itterator(const itterator &other) : m_Ptr{other.m_Ptr} {}
TEMPLATE
auto Triplet<T>::itterator::operator++() noexcept -> REFERENCE {
      this->m_Ptr++;
      return this;
}
TEMPLATE
auto Triplet<T>::itterator::operator++(T) -> itterator {
      itterator temp(m_Ptr++);
      *this = temp;
      return *this;
}

TEMPLATE
auto Triplet<T>::begin() const -> itterator { return itterator(&(this->m_First)); }
TEMPLATE
auto Triplet<T>::end() const -> itterator { return itterator(&(this->m_Third)); }