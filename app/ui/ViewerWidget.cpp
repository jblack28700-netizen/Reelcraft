#include "ViewerWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPointF>

#include "viewer/ViewerProjection.h"
#include "viewer/ViewportState.h"

namespace {

const QColor kBackgroundColor(18, 20, 24);
const QColor kGridColor(66, 74, 86);
const QColor kLabelColor(226, 228, 232);

constexpr double kMarkerRadius = 4.0;
constexpr int kReticleArmLength = 12;

constexpr double kDefaultYaw = 0.0;
constexpr double kDefaultPitch = 0.0;
constexpr double kDefaultRoll = 0.0;
constexpr double kDefaultFieldOfView = 90.0;

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

void ViewerWidget::setViewportState(const ViewportState *viewportState)
{
    if (m_viewportState == viewportState) {
        return;
    }
    m_viewportState = viewportState;

    if (m_viewportState) {
        connect(m_viewportState, &ViewportState::yawChanged, this,
                [this](double) { update(); });
        connect(m_viewportState, &ViewportState::pitchChanged, this,
                [this](double) { update(); });
        connect(m_viewportState, &ViewportState::rollChanged, this,
                [this](double) { update(); });
        connect(m_viewportState, &ViewportState::fieldOfViewChanged, this,
                [this](double) { update(); });
    }
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

void ViewerWidget::cameraValues(double *yawDeg, double *pitchDeg, double *rollDeg,
                                double *fieldOfViewDeg) const
{
    if (m_viewportState) {
        *yawDeg = m_viewportState->yaw();
        *pitchDeg = m_viewportState->pitch();
        *rollDeg = m_viewportState->roll();
        *fieldOfViewDeg = m_viewportState->fieldOfView();
        return;
    }
    *yawDeg = kDefaultYaw;
    *pitchDeg = kDefaultPitch;
    *rollDeg = kDefaultRoll;
    *fieldOfViewDeg = kDefaultFieldOfView;
}

void ViewerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    drawBackground(painter);
    drawCenterReticle(painter);
    drawMarkers(painter);
}

void ViewerWidget::drawBackground(QPainter &painter)
{
    painter.fillRect(rect(), kBackgroundColor);
}

void ViewerWidget::drawCenterReticle(QPainter &painter)
{
    painter.setPen(QPen(kGridColor, 1.0));

    const double centerX = width() / 2.0;
    const double centerY = height() / 2.0;

    painter.drawLine(QPointF(centerX - kReticleArmLength, centerY),
                     QPointF(centerX + kReticleArmLength, centerY));
    painter.drawLine(QPointF(centerX, centerY - kReticleArmLength),
                     QPointF(centerX, centerY + kReticleArmLength));
}

void ViewerWidget::drawMarkers(QPainter &painter)
{
    double cameraYaw = 0.0;
    double cameraPitch = 0.0;
    double cameraRoll = 0.0;
    double fieldOfView = 90.0;
    cameraValues(&cameraYaw, &cameraPitch, &cameraRoll, &fieldOfView);

    QFont labelFont = font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);
    painter.setPen(kLabelColor);

    const QList<ViewerSceneMarker> markers = m_scene.markers();
    for (int i = 0; i < markers.size(); ++i) {
        const ViewerSceneMarker &marker = markers.at(i);
        double x = 0.0;
        double y = 0.0;
        const bool visible = ViewerProjection::project(
            marker.yaw, marker.pitch, cameraYaw, cameraPitch, cameraRoll,
            fieldOfView, width(), height(), &x, &y);
        if (!visible) {
            continue;
        }

        const QColor color = markerColor(i);

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
