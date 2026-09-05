#pragma once

#include <QColor>
#include <QImage>
#include <QSize>
#include <QWidget>

#include "viewer/ViewerScene.h"

class QPaintEvent;
class QPainter;
class ViewportState;

// ViewerWidget is the viewer presentation surface.
//
// Two presentation paths exist:
//   - Source-image path (Objective 8): when a valid in-memory equirectangular
//     image is supplied via setSourceImage(), the widget renders it through
//     the current authoritative ViewportState camera using EquirectView.
//   - Synthetic scene path (Objectives 2-3, protected regression contract):
//     when no source image is present (or it is cleared), the widget renders
//     the deterministic marker scene through the camera exactly as before.
//
// The camera always follows the authoritative, application-owned ViewportState
// supplied via setViewportState(); the widget never owns or modifies viewport
// state and therefore never becomes a second source of yaw/pitch/roll/FOV.
// The in-memory source image is low-level presentation data only and defines
// no decoder/media-engine contract. When no ViewportState is supplied, the
// identity defaults are used (yaw/pitch/roll 0, FOV 90).
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

    // Supplies an optional equirectangular source image for the pixel
    // presentation path. Passing an empty/null image (or a cleared QImage)
    // returns the widget to the synthetic marker-scene rendering.
    void setSourceImage(const QImage &sourceImage);
    bool hasSourceImage() const { return !m_sourceImage.isNull(); }

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

    void drawSourceImage(QPainter &painter);

    ViewerScene m_scene;
    const ViewportState *m_viewportState = nullptr;
    QImage m_sourceImage;
};
