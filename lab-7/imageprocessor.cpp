#include "imageprocessor.h"

QImage ImageProcessor::applyNearestNeighbourScaling(const QImage& src, float scale) {
    if (src.isNull()) return QImage();

    int newWidth  = std::roundf(src.width() * scale);
    int newHeight = std::roundf(src.height() * scale);

    QImage result(newWidth, newHeight, src.format());

    float xRatio = static_cast<float>(src.width()) / newWidth;
    float yRatio = static_cast<float>(src.height()) / newHeight;

    for (int y = 0; y < newHeight; ++y) {
        QRgb *resultRow = reinterpret_cast<QRgb*>(result.scanLine(y));
        int srcY = static_cast<int>(y * yRatio);
        const QRgb *srcRow = reinterpret_cast<const QRgb*>(src.scanLine(srcY));

        for (int x = 0; x < newWidth; ++x) {
            int srcX = static_cast<int>(x * xRatio);
            resultRow[x] = srcRow[srcX];
        }
    }

    return result;
}

QImage ImageProcessor::applyBilinearScaling(const QImage &src, float scale) {
    if (src.isNull()) return QImage();

    int newWidth = std::roundf(src.width() * scale);
    int newHeight = std::roundf(src.height() * scale);

    QImage result(newWidth, newHeight, src.format());

    float xRatio = static_cast<float>(src.width() - 1) / (newWidth > 1 ? newWidth - 1 : 1);
    float yRatio = static_cast<float>(src.height() - 1) / (newHeight > 1 ? newHeight - 1 : 1);

    for (int y = 0; y < newHeight; ++y) {
        // Source Y coordinates
        float srcY = y * yRatio;
        int y1 = static_cast<int>(srcY);
        int y2 = (y1 < src.height() - 1) ? y1 + 1 : y1;
        float dy = srcY - y1;
        float one_minus_dy = 1.0f - dy;

        const QRgb *row1 = reinterpret_cast<const QRgb*>(src.scanLine(y1));
        const QRgb *row2 = reinterpret_cast<const QRgb*>(src.scanLine(y2));
        QRgb* resultRow = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < newWidth; ++x) {
            // Source X coordinates
            float srcX = x * xRatio;
            int x1 = static_cast<int>(srcX);
            int x2 = (x1 < src.width() - 1) ? x1 + 1 : x1;
            float dx = srcX - x1;
            float one_minus_dx = 1.0f - dx;

            // Get 4 neighbors
            QRgb p1 = row1[x1]; // Top-Left
            QRgb p2 = row1[x2]; // Top-Right
            QRgb p3 = row2[x1]; // Bottom-Left
            QRgb p4 = row2[x2]; // Bottom-Right

            // Red
            int r = (qRed(p1) * one_minus_dx * one_minus_dy) +
                    (qRed(p2) * dx * one_minus_dy) +
                    (qRed(p3) * one_minus_dx * dy) +
                    (qRed(p4) * dx * dy);

            // Green
            int g = (qGreen(p1) * one_minus_dx * one_minus_dy) +
                    (qGreen(p2) * dx * one_minus_dy) +
                    (qGreen(p3) * one_minus_dx * dy) +
                    (qGreen(p4) * dx * dy);

            // Blue
            int b = (qBlue(p1) * one_minus_dx * one_minus_dy) +
                    (qBlue(p2) * dx * one_minus_dy) +
                    (qBlue(p3) * one_minus_dx * dy) +
                    (qBlue(p4) * dx * dy);

            // Alpha
            int a = (qAlpha(p1) * one_minus_dx * one_minus_dy) +
                    (qAlpha(p2) * dx * one_minus_dy) +
                    (qAlpha(p3) * one_minus_dx * dy) +
                    (qAlpha(p4) * dx * dy);

            resultRow[x] = qRgba(r, g, b, a);
        }
    }

    return result;
}

