#include "ViewerScene.h"

#include <QtGlobal>

namespace {

// Mirrors ViewportState's canonicalization so scene markers and viewer state
// always speak the same angle language.
double normalizeYaw(double value)
{
    while (value < -180.0) {
        value += 360.0;
    }
    while (value >= 180.0) {
        value -= 360.0;
    }
    return value;
}

double clampPitch(double value)
{
    return qBound(-90.0, value, 90.0);
}

} // namespace

ViewerScene ViewerScene::createDeterministicTestScene()
{
    ViewerScene scene;

    // Cardinal and pole markers define the primary orientation frame.
    scene.addMarker(0.0, 0.0, QStringLiteral("FRONT"));
    // +180 is intentionally supplied to prove canonicalization to -180.
    scene.addMarker(180.0, 0.0, QStringLiteral("BACK"));
    scene.addMarker(-90.0, 0.0, QStringLiteral("LEFT"));
    scene.addMarker(90.0, 0.0, QStringLiteral("RIGHT"));
    scene.addMarker(0.0, 90.0, QStringLiteral("UP"));
    scene.addMarker(0.0, -90.0, QStringLiteral("DOWN"));

    // Diagonal markers make intermediate directions verifiable too.
    scene.addMarker(0.0, 45.0, QStringLiteral("FRONT_UP"));
    scene.addMarker(0.0, -45.0, QStringLiteral("FRONT_DOWN"));
    scene.addMarker(90.0, 45.0, QStringLiteral("RIGHT_UP"));
    scene.addMarker(-90.0, -45.0, QStringLiteral("LEFT_DOWN"));

    return scene;
}

bool ViewerScene::isValid() const
{
    for (const ViewerSceneMarker &marker : m_markers) {
        if (marker.label.isEmpty()) {
            return false;
        }
        if (marker.yaw < -180.0 || marker.yaw >= 180.0) {
            return false;
        }
        if (marker.pitch < -90.0 || marker.pitch > 90.0) {
            return false;
        }
    }

    for (int i = 0; i < m_markers.size(); ++i) {
        for (int j = i + 1; j < m_markers.size(); ++j) {
            if (m_markers.at(i).label == m_markers.at(j).label) {
                return false;
            }
        }
    }

    return true;
}

int ViewerScene::markerIndex(const QString &label) const
{
    for (int i = 0; i < m_markers.size(); ++i) {
        if (m_markers.at(i).label == label) {
            return i;
        }
    }
    return -1;
}

void ViewerScene::addMarker(double yaw, double pitch, const QString &label)
{
    ViewerSceneMarker marker;
    marker.yaw = normalizeYaw(yaw);
    marker.pitch = clampPitch(pitch);
    marker.label = label;
    m_markers.append(marker);
}
