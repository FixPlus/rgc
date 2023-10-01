#ifndef RENDERGRAPHCOMPILER_GRAPH_HPP
#define RENDERGRAPHCOMPILER_GRAPH_HPP

#include <list>
#include <memory>

#include "rgc/Action.hpp"
#include "rgc/Constant.hpp"
#include "rgc/IList.hpp"

namespace rgc {

class Action;

class Graph : public IList<Action> {
public:
  virtual ~Graph();

  template <class CT, typename... Args>
  requires std::derived_from<CT, Constant>
  auto *getConstant(Args &&...args) {
    return m_constants.template get<CT>(std::forward<Args>(args)...);
  }

  template <class CT, typename... Args>
  requires std::derived_from<CT, Type>
  auto *getType(Args &&...args) {
    return m_types.template get<CT>(std::forward<Args>(args)...);
  }
  auto &types() const { return m_types; }

  auto &constants() const { return m_constants; }

  auto &types() { return m_types; }

  auto &constants() { return m_constants; }

private:
  ConstantPool m_constants;
  TypePool m_types;
};

} // namespace rgc
#endif // RENDERGRAPHCOMPILER_GRAPH_HPP
