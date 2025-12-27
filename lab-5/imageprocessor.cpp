#include "imageprocessor.h"
#include <QDebug>

ImageProcessor::ImageProcessor() = default;

QImage ImageProcessor::applyTransformations(const QImage& src, int brightness, int contrast, bool isGray, bool isNegative) {
    if (src.isNull()) return QImage();

    QImage result = src;

    // Apply grayscale and/or negative
    if (isGray || isNegative) applyGrayscaleAndNegative(result, isGray, isNegative);

    int width      = result.width();
    int height     = result.height();
    int pixelCount = width * height;

    size_t sumR = 0, sumG = 0, sumB = 0;

    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            QRgb p = row[x];
            sumR += qRed(p);
            sumG += qGreen(p);
            sumB += qBlue(p);
        }
    }

    double avgR = sumR / static_cast<double>(pixelCount);
    double avgG = sumG / static_cast<double>(pixelCount);
    double avgB = sumB / static_cast<double>(pixelCount);

    double t = (contrast - 5000) / 5000.0f;
    double K = std::pow(2.0, t * 2.0);

    // Brightness offset
    double shift = static_cast<double>(brightness);

    // Lookup tables
    uchar lutR[256];
    uchar lutG[256];
    uchar lutB[256];

    // Y = K * (Y_old - Y_av) + Y_av + Brightness
    for (int i = 0; i < 256; ++i) {
        lutR[i] = static_cast<uchar>(std::clamp(K * (i - avgR) + avgR + shift, 0.0, 255.0)); // Red
        lutG[i] = static_cast<uchar>(std::clamp(K * (i - avgG) + avgG + shift, 0.0, 255.0)); // Green
        lutB[i] = static_cast<uchar>(std::clamp(K * (i - avgB) + avgB + shift, 0.0, 255.0)); // Blue
    }

    // Write to the result image
    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            QRgb p = row[x];
            row[x] = qRgb(
                lutR[qRed(p)],
                lutG[qGreen(p)],
                lutB[qBlue(p)]
            );
        }
    }

    return result;
}

void ImageProcessor::applyGrayscaleAndNegative(QImage& img, bool isGray, bool isNegative) {
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(img.scanLine(y));

        for (int x = 0; x < img.width(); ++x) {
            QRgb p = row[x];

            int r = qRed(p);
            int g = qGreen(p);
            int b = qBlue(p);

            if (isGray) {
                int y = qGray(p);
                r = g = b = y;
            }

            if (isNegative) {
                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
            }

            row[x] = qRgb(r, g, b);
        }
    }
}

std::vector<int> ImageProcessor::calculateHistogram(const QImage& src) {
    std::vector<int> histogram(256, 0);
    if (src.isNull()) return histogram;

    QImage img = src.convertToFormat(QImage::Format_RGB32);
    int w = src.width();
    int h = src.height();

    for (int y = 0; y < h; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(img.constScanLine(y));

        for (int x = 0; x < w; ++x) {
            int l = qGray(row[x]);
            ++histogram[l];
        }
    }

    return histogram;
}

QImage ImageProcessor::binarizeManual(const QImage& src, int threshold) {
    if (src.isNull()) return QImage();

    QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    int w = src.width();
    int h = src.height();

    // Allocate RGB result
    QImage result(w, h, QImage::Format_RGB32);

    for (int y = 0; y < h; ++y) {
        const uchar* row = gray.constScanLine(y);
        QRgb* dstRow = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < w; ++x) {
            dstRow[x] = row[x] >= threshold ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        }
    }

    return result;
}

QImage ImageProcessor::binarizeOtsu(const QImage& src) {
    QImage gray = src.convertToFormat(QImage::Format_Grayscale8);
    int w = gray.width();
    int h = gray.height();

    auto hist = calculateHistogram(gray);

    int total = w * h;
    double sum = 0;
    for (int i = 0; i < 256; ++i) sum += i * hist[i];

    int wB = 0;
    double sumB = 0;
    double maxVar = 0;
    int threshold = 0;

    for (int i = 0; i < 256; ++i) {
        wB += hist[i];
        if (wB == 0) continue;
        int wF = total - wB;
        if (wF == 0) break;

        sumB += i * hist[i];
        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;
        double varBetween = static_cast<double>(wB) * static_cast<double>(wF) * (mB - mF) * (mB - mF);

        if (varBetween > maxVar) {
            maxVar = varBetween;
            threshold = i;
        }
    }

    QImage result(w, h, QImage::Format_RGB32);

    for (int y = 0; y < h; ++y) {
        const uchar* row = gray.constScanLine(y);
        QRgb* dstRow = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < w; ++x) {
            dstRow[x] = row[x] > threshold ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        }
    }

    return result;
}

QImage ImageProcessor::binarizeAdaptive(const QImage& src) {
    if (src.isNull()) return QImage();

    QImage source = src.convertToFormat(QImage::Format_RGB32);
    QImage result = source;

    int w = source.width();
    int h = source.height();

    int blockSize = 11;
    int C = 2;

    int halfBlock = blockSize / 2;

    // Iterate every pixel
    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        QRgb *dstRow = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < w; ++x) {
            //  Calculate Local Mean
            int sum = 0;
            int count = 0;

            // Loop over the neighbor block
            for (int dy = -halfBlock; dy <= halfBlock; ++dy) {
                int ny = y + dy;
                // Boundary check Y
                if (ny < 0 || ny >= h) continue;

                const QRgb *neighborRow = reinterpret_cast<const QRgb*>(source.constScanLine(ny));

                for (int dx = -halfBlock; dx <= halfBlock; ++dx) {
                    int nx = x + dx;
                    // Boundary check X
                    if (nx < 0 || nx >= w) continue;

                    sum += qGray(neighborRow[nx]);
                    count++;
                }
            }

            int localMean = sum / count;
            int pixelVal = qGray(srcRow[x]);

            // Apply Threshold
            if (pixelVal > (localMean - C)) {
                dstRow[x] = qRgb(255, 255, 255);
            } else {
                dstRow[x] = qRgb(0, 0, 0);
            }
        }
    }

    return result;
}
