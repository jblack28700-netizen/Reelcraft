#pragma once

#include <QColor>
#include <QSize>
#include <QWidget>

#include "viewer/ViewerScene.h"

class QPaintEvent;
class QPainter;
class ViewportState;

// ViewerWidget is the viewer presentation surface.
//
// It presents the deterministic synthetic 360-degree test scene through a
// deterministic camera view (see ViewerProjection). The camera follows the
// authoritative, application-owned ViewportState supplied via
// setViewportState(); the widget never owns or modifies viewport state and
// therefore never becomes a second source of yaw/pitch/roll/FOV.
//
// When no ViewportState is supplied, the identity defaults are used
// (yaw/pitch/roll 0, FOV 90) so the widget remains deterministic standalone.
class ViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewerWidget(QWidget *parent = nullptr);

    const ViewerScene &scene() const { return m_scene; }
    void setScene(const ViewerScene &scene);

    // Supplies the authoritative (read-only) viewport state that drives the
    // presented camera view. The widget repaints when that state changes.
    void setViewportState(const ViewportState *viewportState);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void cameraValues(double *yawDeg, double *pitchDeg, double *rollDeg,
                      double *fieldOfViewDeg) const;

    void drawBackground(QPainter &painter);
    void drawCenterReticle(QPainter &painter);
    void drawMarkers(QPainter &painter);
    QColor markerColor(int index) const;

    ViewerScene m_scene;
    const ViewportState *m_viewportState = nullptr;
};
