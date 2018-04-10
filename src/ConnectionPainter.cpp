#include "ConnectionPainter.hpp"

//#define DEBUG_DRAWING 1

#include "ConnectionGeometry.hpp"
#include "ConnectionState.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "Connection.hpp"

#include "NodeData.hpp"

#include "StyleCollection.hpp"

using QtNodes::ConnectionPainter;
using QtNodes::ConnectionGeometry;
using QtNodes::Connection;


static
QPainterPath
cubicPath(ConnectionGeometry const& geom)
{
  QPointF const& source = geom.source();
  QPointF const& sink   = geom.sink();

  auto c1c2 = geom.pointsC1C2();

  // cubic spline
  QPainterPath cubic(source);

  cubic.cubicTo(c1c2.first, c1c2.second, sink);

  return cubic;
}


QPainterPath
ConnectionPainter::
getPainterStroke(ConnectionGeometry const& geom)
{
  auto cubic = cubicPath(geom);

  QPointF const& source = geom.source();
  QPainterPath result(source);

  unsigned segments = 20;

  for (auto i = 0ul; i < segments; ++i)
  {
    double ratio = double(i + 1) / segments;
    result.lineTo(cubic.pointAtPercent(ratio));
  }

  QPainterPathStroker stroker; stroker.setWidth(10.0);

  return stroker.createStroke(result);
}


#ifdef DEBUG_DRAWING
static
void
debugDrawing(QPainter * painter,
             Connection const & connection)
{
  Q_UNUSED(painter);
  Q_UNUSED(connection);
  ConnectionGeometry const& geom =
    connection.connectionGeometry();

  {
    QPointF const& source = geom.source();
    QPointF const& sink   = geom.sink();

    auto points = geom.pointsC1C2();

    painter->setPen(Qt::red);
    painter->setBrush(Qt::red);

    painter->drawLine(QLineF(source, points.first));
    painter->drawLine(QLineF(points.first, points.second));
    painter->drawLine(QLineF(points.second, sink));
    painter->drawEllipse(points.first, 3, 3);
    painter->drawEllipse(points.second, 3, 3);

    painter->setBrush(Qt::NoBrush);

    painter->drawPath(cubicPath(geom));
  }

  {
    painter->setPen(Qt::yellow);

    painter->drawRect(geom.boundingRect());
  }
}
#endif

static
void
drawSketchLine(QPainter * painter,
               Connection const & connection)
{
  using QtNodes::ConnectionState;

  ConnectionState const& state =
    connection.connectionState();

  if (state.requiresPort())
  {
    auto const & connectionStyle =
      QtNodes::StyleCollection::connectionStyle();

    QPen p;
    p.setWidth(connectionStyle.constructionLineWidth());
    p.setColor(connectionStyle.constructionColor());
    p.setStyle(Qt::DashLine);

    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    using QtNodes::ConnectionGeometry;
    ConnectionGeometry const& geom = connection.connectionGeometry();

    auto cubic = cubicPath(geom);
    // cubic spline
    painter->drawPath(cubic);
  }
}

static
void
drawHoveredOrSelected(QPainter * painter,
                      Connection const & connection)
{
  using QtNodes::ConnectionGeometry;

  ConnectionGeometry const& geom = connection.connectionGeometry();
  bool const hovered = geom.hovered();

  auto const& graphicsObject =
    connection.getConnectionGraphicsObject();

  bool const selected = graphicsObject.isSelected();

  // drawn as a fat background
  if (hovered || selected)
  {
    QPen p;

    auto const &connectionStyle =
      QtNodes::StyleCollection::connectionStyle();
    double const lineWidth     = connectionStyle.lineWidth();

    p.setWidth(2 * lineWidth);
    p.setColor(selected ?
               connectionStyle.selectedHaloColor() :
               connectionStyle.hoveredColor());

    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    // cubic spline
    auto cubic = cubicPath(geom);
    painter->drawPath(cubic);
  }
}


static
void
drawNormalLine(QPainter * painter,
               Connection const & connection)
{
  using QtNodes::ConnectionState;

  ConnectionState const& state =
    connection.connectionState();

  if (state.requiresPort())
    return;

  // colors

  auto const &connectionStyle =
    QtNodes::StyleCollection::connectionStyle();

  QColor normalColorOut  = connectionStyle.normalColor();
  QColor normalColorIn   = connectionStyle.normalColor();
  QColor selectedColor = connectionStyle.selectedColor();

  bool gradientColor = false;

  if (connectionStyle.useDataDefinedColors())
  {
    using QtNodes::PortType;

    auto dataTypeOut = connection.dataType(PortType::Out);
    auto dataTypeIn = connection.dataType(PortType::In);

    gradientColor = (dataTypeOut.id != dataTypeIn.id);

    normalColorOut  = connectionStyle.normalColor(dataTypeOut.id);
    normalColorIn   = connectionStyle.normalColor(dataTypeIn.id);
    selectedColor = normalColorOut.darker(200);
  }

  // geometry

  ConnectionGeometry const& geom = connection.connectionGeometry();

  double const lineWidth = connectionStyle.lineWidth();

  // draw normal line
  QPen p;

  p.setWidth(lineWidth);

  auto const& graphicsObject = connection.getConnectionGraphicsObject();
  bool const selected = graphicsObject.isSelected();

  if (gradientColor)
  {
    double dr = normalColorIn.red() - normalColorOut.red();
    double dg = normalColorIn.green() - normalColorOut.green();
    double db = normalColorIn.blue() - normalColorOut.blue();

    painter->setBrush(Qt::NoBrush);

    unsigned int const segments = 20;

    for (unsigned int i = 0ul; i < segments; ++i)
    {
      double ratioPrev = double(i) / segments;
      double ratio = double(i + 1) / segments;

      QColor c(normalColorOut.red()   + dr * ratio,
               normalColorOut.green() + dg * ratio,
               normalColorOut.blue()  + db * ratio);

      if (selected)
        c = c.darker(200);

      p.setColor(c);

      painter->setPen(p);

      auto cubic = cubicPath(geom);
      painter->drawLine(cubic.pointAtPercent(ratioPrev),
                        cubic.pointAtPercent(ratio));
    }
  }
  else
  {
    p.setColor(normalColorOut);

    if (selected)
    {
      p.setColor(selectedColor);
    }

    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    // cubic spline
    auto cubic = cubicPath(geom);
    painter->drawPath(cubic);
  }
}


void
ConnectionPainter::
paint(QPainter* painter,
      Connection const &connection)
{
  drawHoveredOrSelected(painter, connection);

  drawSketchLine(painter, connection);

  drawNormalLine(painter, connection);

#ifdef DEBUG_DRAWING
  debugDrawing(painter, connection);
#endif

  // draw end points
  ConnectionGeometry const& geom = connection.connectionGeometry();

  QPointF const & source = geom.source();
  QPointF const & sink   = geom.sink();

  auto const & connectionStyle =
    QtNodes::StyleCollection::connectionStyle();

  double const pointDiameter = connectionStyle.pointDiameter();

  painter->setPen(connectionStyle.constructionColor());
  painter->setBrush(connectionStyle.constructionColor());
  double const pointRadius = pointDiameter / 2.0;
  painter->drawEllipse(source, pointRadius, pointRadius);
  painter->drawEllipse(sink, pointRadius, pointRadius);
}
