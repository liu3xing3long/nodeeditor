#include "DataModelRegistry.hpp"

#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>

using QtNodes::DataModelRegistry;
using QtNodes::NodeDataModel;
using QtNodes::Converter;

std::unique_ptr<NodeDataModel>
DataModelRegistry::
create(QString const &modelName)
{
  auto it = _registeredModels.find(modelName);

  if (it != _registeredModels.end())
  {
    return it->second->clone();
  }

  return nullptr;
}


DataModelRegistry::RegisteredModelsMap const &
DataModelRegistry::
registeredModels() const
{
  return _registeredModels;
}


DataModelRegistry::RegisteredModelsCategoryMap const &
DataModelRegistry::
registeredModelsCategoryAssociation() const
{
  return _registeredModelsCategory;
}


DataModelRegistry::CategoriesSet const &
DataModelRegistry::
categories() const
{
  return _categories;
}


Converter
DataModelRegistry::
getTypeConverter(NodeDataType const & d1,
                 NodeDataType const & d2) const
{
  ConverterType typeConverterKey = std::make_pair(d1, d2);

  auto it = _registeredTypeConverters.find(typeConverterKey);

  if (it != _registeredTypeConverters.end())
  {
    return it->second;
  }

  return Converter{};
}
