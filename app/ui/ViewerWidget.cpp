#include "ViewerWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QWheelEvent>

#include "viewer/EquirectView.h"
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

// Pointer-interaction sensitivity (Objective 11). Deterministic constants;
// revisitable later without contract change.
constexpr double kLookDegreesPerPixel = 0.25;
constexpr double kFovDegreesPerWheelStep = 5.0;
constexpr int kWheelStepDenominator = 120;

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

void ViewerWidget::setSourceImage(const QImage &sourceImage)
{
    m_sourceImage = sourceImage;
    update();
}

void ViewerWidget::setFlatSourceMode(bool flat)
{
    if (m_flatSourceMode == flat) {
        return;
    }
    m_flatSourceMode = flat;
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

    if (hasSourceImage()) {
        drawSourceImage(painter);
        return;
    }

    // Synthetic marker-scene path (protected regression contract).
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

void ViewerWidget::drawSourceImage(QPainter &painter)
{
    if (width() <= 0 || height() <= 0) {
        return;
    }

    if (m_flatSourceMode) {
        drawFlatSourceImage(painter);
        return;
    }

    double cameraYaw = 0.0;
    double cameraPitch = 0.0;
    double cameraRoll = 0.0;
    double fieldOfView = 90.0;
    cameraValues(&cameraYaw, &cameraPitch, &cameraRoll, &fieldOfView);

    // Capped CPU rendering resolution (implementation/performance safeguard;
    // not an architectural limit). Aspect is preserved from the widget.
    const int renderWidth = qMin(width(), EquirectView::MaxOutputWidth);
    const int renderHeight =
        qMax(1, static_cast<int>(renderWidth * static_cast<double>(height()) / width()));

    QImage view;
    if (!EquirectView::render(m_sourceImage, cameraYaw, cameraPitch, cameraRoll,
                              fieldOfView, renderWidth, renderHeight, &view)) {
        // Deterministic fallback: the background is already painted; no
        // partial frame is drawn.
        return;
    }

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(QRect(0, 0, width(), height()), view);
}

void ViewerWidget::drawFlatSourceImage(QPainter &painter)
{
    if (m_sourceImage.isNull()) {
        return;
    }
    const int widgetWidth = width();
    const int widgetHeight = height();
    const int imageWidth = m_sourceImage.width();
    const int imageHeight = m_sourceImage.height();
    if (widgetWidth <= 0 || widgetHeight <= 0 || imageWidth <= 0 || imageHeight <= 0) {
        return;
    }

    // Fit and center, preserving aspect ratio; the background (already drawn)
    // letterboxes any unused space. No camera/equirectangular transformation.
    const double scale = qMin(static_cast<double>(widgetWidth) / imageWidth,
                              static_cast<double>(widgetHeight) / imageHeight);
    const int contentWidth = qMax(1, static_cast<int>(imageWidth * scale));
    const int contentHeight = qMax(1, static_cast<int>(imageHeight * scale));
    const int x = (widgetWidth - contentWidth) / 2;
    const int y = (widgetHeight - contentHeight) / 2;

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(QRect(x, y, contentWidth, contentHeight), m_sourceImage);
}

void ViewerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastDragPosition = event->pos();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        const QPoint position = event->pos();
        const int deltaX = position.x() - m_lastDragPosition.x();
        const int deltaY = position.y() - m_lastDragPosition.y();
        m_lastDragPosition = position;

        if (deltaX != 0) {
            // Drag right => yaw increases.
            emit viewportYawDeltaRequested(kLookDegreesPerPixel * deltaX);
        }
        if (deltaY != 0) {
            // Drag up (negative deltaY) => pitch increases.
            emit viewportPitchDeltaRequested(-kLookDegreesPerPixel * deltaY);
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void ViewerWidget::wheelEvent(QWheelEvent *event)
{
    const int angleDelta = event->angleDelta().y();
    if (angleDelta != 0) {
        // Wheel up => FOV decreases (zoom in); wheel down => FOV increases.
        const double steps = static_cast<double>(angleDelta) / kWheelStepDenominator;
        emit viewportFovDeltaRequested(-kFovDegreesPerWheelStep * steps);
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}
