#pragma once

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QSize>
#include <QWidget>

#include "viewer/ViewerScene.h"

class QMouseEvent;
class QPaintEvent;
class QPainter;
class QWheelEvent;
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

    // Selects flat vs equirectangular presentation for the source image
    // (Objective 12). Flat mode (opt-in, default false) draws the source
    // aspect-preserving, centered, and letterboxed with NO equirectangular or
    // camera transformation. Pointer/keyboard camera controls may still run,
    // but they do not transform flat presentation.
    void setFlatSourceMode(bool flat);
    bool isFlatSourceMode() const { return m_flatSourceMode; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

    // Pointer-based orientation control (Objective 11): left-drag emits yaw
    // (drag right = yaw increases) and pitch (drag up = pitch increases)
    // deltas; the mouse wheel adjusts FOV (wheel up = FOV decreases = zoom in,
    // wheel down = FOV increases = zoom out). The widget only emits requests;
    // it never owns or mutates viewport state.
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

signals:
    // Viewport orientation/FOV delta requests (consumed by Application's
    // adjust slots through main.cpp wiring).
    void viewportYawDeltaRequested(double delta);
    void viewportPitchDeltaRequested(double delta);
    void viewportFovDeltaRequested(double delta);

private:
    void cameraValues(double *yawDeg, double *pitchDeg, double *rollDeg,
                      double *fieldOfViewDeg) const;

    void drawBackground(QPainter &painter);
    void drawCenterReticle(QPainter &painter);
    void drawMarkers(QPainter &painter);
    QColor markerColor(int index) const;

    void drawSourceImage(QPainter &painter);
    void drawFlatSourceImage(QPainter &painter);

    bool m_dragging = false;
    QPoint m_lastDragPosition;

    ViewerScene m_scene;
    const ViewportState *m_viewportState = nullptr;
    QImage m_sourceImage;
    bool m_flatSourceMode = false;
};
