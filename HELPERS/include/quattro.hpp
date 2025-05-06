#pragma once

#include "FixedSizeContainer.hpp"
#include <vector>

#define QUATTRO_SIZE 4
#define TEMPLATE template <typename Content>

TEMPLATE
class QuattroList {
public:
    QuattroList(std::vector<Content> _data);

    auto insert(const Content &_content) -> void;
    auto pop() -> void;

    auto operator[](size_t idx) -> std::vector<Content> &;

private:
    FixedSizeContainer<std::vector<Content>> m_RootList;
    size_t root_ptr;
};