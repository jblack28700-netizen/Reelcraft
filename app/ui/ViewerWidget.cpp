#include "ViewerWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPointF>

namespace {

const QColor kBackgroundColor(18, 20, 24);
const QColor kGridColor(66, 74, 86);
const QColor kLabelColor(226, 228, 232);

constexpr double kMarkerRadius = 4.0;
constexpr int kGridStepDegrees = 30;

// Deterministic marker palette; index 0 is the FRONT marker.
const QColor kMarkerPalette[] = {
    QColor(255, 213, 79),  // amber
    QColor(79, 195, 247),  // light blue
    QColor(129, 199, 132), // green
    QColor(240, 98, 146),  // pink
    QColor(186, 104, 200), // purple
    QColor(255, 138, 101), // deep orange
    QColor(77, 182, 172),  // teal
    QColor(149, 117, 205), // deep purple
    QColor(255, 167, 38),  // orange
    QColor(100, 181, 246), // blue
};
constexpr int kMarkerPaletteSize = static_cast<int>(sizeof(kMarkerPalette) / sizeof(kMarkerPalette[0]));

} // namespace

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget(parent)
{
    setScene(ViewerScene::createDeterministicTestScene());
}

void ViewerWidget::setScene(const ViewerScene &scene)
{
    m_scene = scene;
    update();
}

QSize ViewerWidget::sizeHint() const
{
    return QSize(640, 320);
}

QSize ViewerWidget::minimumSizeHint() const
{
    return QSize(160, 80);
}

double ViewerWidget::pixelXForYaw(double yaw, double width)
{
    return width * (yaw + 180.0) / 360.0;
}

double ViewerWidget::pixelYForPitch(double pitch, double height)
{
    return height * (90.0 - pitch) / 180.0;
}

void ViewerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    drawBackground(painter);
    drawOrientationGrid(painter);
    drawMarkers(painter);
}

void ViewerWidget::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), kBackgroundColor);
}

void ViewerWidget::drawOrientationGrid(QPainter &painter)
{
    painter.setPen(QPen(kGridColor, 1.0));

    // Meridians (vertical) every 30 degrees of yaw across the whole sphere.
    for (int yaw = -180; yaw <= 180; yaw += kGridStepDegrees) {
        const double x = pixelXForYaw(yaw, width());
        painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));
    }

    // Parallels (horizontal) every 30 degrees of pitch across the whole sphere.
    for (int pitch = -90; pitch <= 90; pitch += kGridStepDegrees) {
        const double y = pixelYForPitch(pitch, height());
        painter.drawLine(QPointF(0.0, y), QPointF(width(), y));
    }
}

void ViewerWidget::drawMarkers(QPainter &painter)
{
    QFont labelFont = font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);
    painter.setPen(kLabelColor);

    const QList<ViewerSceneMarker> markers = m_scene.markers();
    for (int i = 0; i < markers.size(); ++i) {
        const ViewerSceneMarker &marker = markers.at(i);
        const QColor color = markerColor(i);
        const double x = pixelXForYaw(marker.yaw, width());
        const double y = pixelYForPitch(marker.pitch, height());

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(QPointF(x, y), kMarkerRadius, kMarkerRadius);

        painter.setPen(kLabelColor);
        painter.drawText(QPointF(x + kMarkerRadius + 2.0, y - kMarkerRadius - 2.0), marker.label);
    }
}

QColor ViewerWidget::markerColor(int index) const
{
    return kMarkerPalette[index % kMarkerPaletteSize];
}
