#include <algorithm>

#include "rgc/Graph.hpp"

namespace rgc {
Graph::~Graph() {
  while (!empty()) {
    std::vector<Action *> unused;
    std::copy_if(begin(), end(), std::back_inserter(unused),
                 [](auto *action) { return action->unused(); });
    assert(!unused.empty() && "cyclic dependency");
    for (auto *action : unused) {
      erase(action);
    }
  }
}
} // namespace rgc