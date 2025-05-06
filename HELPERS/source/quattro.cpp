#include "../include/quattro.hpp"

TEMPLATE
QuattroList<Content>::QuattroList(std::vector<Content> _data) {
    this->m_RootList = FixedSizeContainer<std::vector<Content>>(QUATTRO_SIZE);
    this->root_ptr   = 0;

    for (const auto &container : _data) {
        m_RootList[root_ptr % 4].push_back(container);
        root_ptr++;
    }
}

TEMPLATE
auto QuattroList<Content>::insert(const Content &_data) -> void {
    this->m_RootList[root_ptr % 4].push_back(_data);
    root_ptr++;
}

TEMPLATE
auto QuattroList<Content>::pop() -> void {
    this->m_RootList[root_ptr % 4].pop_back();
    root_ptr--;
}

TEMPLATE
auto QuattroList<Content>::operator[](size_t idx) -> std::vector<Content> & {
    if (idx < 0 | idx > 3) {
        throw std::invalid_argument("Index out of the range!\n");
    }
    return this->m_RootList[idx];
}