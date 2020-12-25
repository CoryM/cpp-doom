#include "compiler_feature.hpp"

#include <algorithm> // for sort
#include <cstring>   // for size_t, strcmp
#include <iterator>  // for begin, end
#include <memory>    // for allocator_traits<>::value_type
#include <vector>    // for vector

#include "config.hpp" // for PrintOptions, print


void show(char const *title, const std::vector<CompilerFeature> &features)
{
    if (Options::titles)
    {
        std::cout << '\n'
                  << std::left << title << '\n';
    }

    std::vector<size_t> index;
    index.reserve(features.size());
    for (size_t i = 0; i < features.size(); i++)
    {
        index.emplace_back(i);
    }

    if (Options::sorted_by_value)
    {
        std::sort(std::begin(index), std::end(index),
            [features](size_t const &lhs, size_t const &rhs) {
                return (features.at(lhs).value < features.at(rhs).value);
            });
    }
    for (const size_t x : index)
    {
        print_compiler_feature(features.at(x));
    }
}
