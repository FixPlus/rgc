#include "rgc/Action.hpp"
#include "rgc/Graph.hpp"
#include "rgc/Types.hpp"
#include <iostream>

class MyAllocation : public rgc::Allocation {
public:
  explicit MyAllocation(rgc::TypePool &tp)
      : rgc::Allocation(tp.get<rgc::BufferType>(
            rgc::ScalarType::OwnerType::Device, 4u, 4u)){};
};

class OneUseAction : public rgc::Composition {
public:
  explicit OneUseAction(rgc::Value *v)
      : rgc::Composition(v->type(), std::span<rgc::Value *>{&v, 1}) {}

  void dump(std::ostream &os) const override {
    os << "OneUseAction: " << std::endl;
    os << "\t";
    rgc::Action::dump(os);
  }
};

class TwoUseAction : public rgc::RealAction {
public:
  TwoUseAction(rgc::Value *v1, rgc::Value *v2) : rgc::RealAction(v1, v2) {}

  void dump(std::ostream &os) const override {
    os << "TwoUseAction: " << std::endl;
    os << "\t";
    rgc::Action::dump(os);
  }
};

int main() {
  auto graph = rgc::Graph{};
  auto *a1 = new MyAllocation{graph.types()};
  auto *a2 = new OneUseAction{a1};
  auto *a3 = new TwoUseAction{a1, a2};

  auto *a4 = new OneUseAction{a1};
  auto *nc = graph.getConstant<rgc::NullConstant>(graph.types());
  graph.push_back(a1);
  graph.push_back(a2);
  graph.push_back(a3);
  graph.push_back(a4);

  assert(a1->type() == a2->type());
  assert(a1->type() == a3->type());
  assert(nc->type() != a1->type());

  a2->replaceAllUsesWith(nc);

  for (auto *action : graph) {
    action->dump(std::cout);
    std::cout << std::endl;
  }
}