#pragma once

#include "image/color_range.hpp"
#include "transform/factory.hpp"
#include "common.hpp"

template <typename IO>
bool flif_encode(IO& io, Images &images, std::vector<std::string> transDesc = {"YIQ","BND","PLA","PLT","ACB","DUP","FRS","FRA"}, flifEncoding encoding = flifEncoding::interlaced, int learn_repeats = 3, int acb = -1, int palette_size = 1024, int lookback = 1, int divisor=CONTEXT_TREE_COUNT_DIV, int min_size=CONTEXT_TREE_MIN_SUBTREE_SIZE, int split_threshold=CONTEXT_TREE_SPLIT_THRESHOLD, int cutoff=2, int alpha=19);
