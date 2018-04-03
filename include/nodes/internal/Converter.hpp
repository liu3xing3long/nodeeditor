#pragma once

#include "NodeData.hpp"

#include <functional>
#include <tuple>

namespace QtNodes
{

using SharedNodeData = std::shared_ptr<NodeData>;

// a function taking in NodeData and returning NodeData
using Converter =
  std::function<SharedNodeData(SharedNodeData)>;

// data-type-in, data-type-out
using ConverterType =
  std::pair<NodeDataType, NodeDataType>;

// convertor function
using ConverterDataModel =
  std::pair<ConverterType, Converter>;

}
