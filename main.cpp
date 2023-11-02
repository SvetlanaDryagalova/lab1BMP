//Дрягалова Светлана б18
//st119022@student.spbu.ru

#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

#pragma pack(push, 1)
struct BMPHeader {
    char signature[2];   // Сигнатура файла ('B' и 'M')
    int fileSize;        // Размер файла в байтах
    short reserved1;     // Зарезервированное значение
    short reserved2;     // Зарезервированное значение
    int dataOffset;      // Смещение данных пикселей от начала файла

    // Дополнительные поля для заголовка BMP
    int headerSize;      // Размер структуры заголовка BMP
    int width;           // Ширина изображения в пикселях
    int height;          // Высота изображения в пикселях
    short colorPlanes;   // Количество плоскостей (должно быть 1)
    short bitsPerPixel;  // Глубина цвета в битах
    int compression;     // Метод сжатия (обычно без сжатия = 0)
    int imageSize;       // Размер изображения в байтах (ширина * высота * глубина цвета)
    int horizontalRes;   // Горизонтальное разрешение в пикселях на метр
    int verticalRes;     // Вертикальное разрешение в пикселях на метр
    int numColors;       // Количество использованных цветов (0 - все)
    int importantColors; // Количество важных цветов (0 - все)
};

#pragma pack(pop)

void right(const unsigned char* oldFile, unsigned char* newFile, int width, int height) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            int newIndex = ((width-i-1) * height +j) *3;
            int oldIndex = (j*width +i) *3;
            for (int k = 0; k<3; ++k)
                newFile[newIndex + k] = oldFile[oldIndex + k];
        }
    }
}

void left(const unsigned char* oldFile, unsigned char* newFile, int width, int height) {
    for (int i = 0; i < height; ++i){
        for (int j = 0; j < width; ++j){
            int newIndex = ((height - i - 1) + j*height) * 3;
            int oldIndex = (i * width + j) * 3;

            for (int k = 0; k < 3; ++k)
                newFile[newIndex + k] = oldFile[oldIndex + k];
        }
    }
}

float* GaussianFilter(int filterSize, int radius, float sigma) {
    float* gMatrix = new float[filterSize * filterSize];
    float sum = 0.0;

    for (int j = -radius; j <= radius; j++) {
        for (int i = -radius; i <= radius; i++) {
            int index = (j + radius) * filterSize + (i + radius);
            gMatrix[index] = exp(-(i * i + j * j) / (2 * M_PI * sigma * sigma));
            sum += gMatrix[index];
        }
    }
    for (int i = 0; i < filterSize * filterSize; i++) {
        gMatrix[i] /= sum;
    }
    return gMatrix;
}
void applyGF(unsigned char* oldMas, unsigned char* newMas, int width, int height, float* gFilter, int filterSize, int radius) {
    for (int y = radius; y < height - radius; y++) {
        for (int x = radius; x < width - radius; x++) {
            float sumR = 0.0, sumG = 0.0, sumB = 0.0;

            for (int j = -radius; j <= radius; j++) {
                for (int i = -radius; i <= radius; i++) {
                    int index = ((y + j) * width + (x + i)) * 3;
                    int fIndex = (j + radius) * filterSize + (i + radius);
                    sumR += oldMas[index] * gFilter[fIndex];
                    sumG += oldMas[index + 1] * gFilter[fIndex];
                    sumB += oldMas[index + 2] * gFilter[fIndex];
                }
            }

            int index = (y * width + x) * 3;
            newMas[index] = round(sumR);
            newMas[index + 1] = round(sumG);
            newMas[index + 2] = round(sumB);
        }
    }
}

int main() {
    // открываем изображение
    ifstream bmpImage("2.bmp", ios::binary);
    if (!bmpImage) {
        cout << "Failed to open the input file!" << endl;
        return 1;
    }
    // читаем заголовок
    BMPHeader bmpHeader{};
    bmpImage.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BMPHeader));
    int weight = bmpHeader.fileSize;
    cout << "The size of memory being allocated: " << weight << " bytes" << endl;
    int width = bmpHeader.width;
    int height = bmpHeader.height;
    int size = width * height * 3;
    int heightRot = width;
    int widthRot = height;

    // считываем массив с данными
    unsigned char* buff = new unsigned char[size];
    bmpImage.read(reinterpret_cast<char*>(buff), size);
    bmpImage.close();

    //создаем новый массив
    unsigned char* rotatedImage = new unsigned char[size];
    bmpHeader.dataOffset = sizeof(BMPHeader);
    bmpHeader.width = widthRot;
    bmpHeader.height = heightRot;

    // создаем матрицу для фильтра Гаусса
    int radius = 3;
    int filterSize = 2 * radius + 1;
    float sigma = 1.0;
    float* gFilter = GaussianFilter(filterSize, radius, sigma);
    unsigned char* filteredImage = new unsigned char[size];

    // поворачиваем вправо и применяем фильтр Гаусса
    right(buff, rotatedImage, width, height);

    ofstream rightImage("right_rotated.bmp", ios::binary);
    rightImage.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));
    rightImage.write(reinterpret_cast<const char*>(rotatedImage), size);
    //filter
    applyGF(rotatedImage, filteredImage, widthRot, heightRot, gFilter, filterSize, radius);
    ofstream rightFilteredImage("right_rotated_and_filtered.bmp", ios::binary);
    rightFilteredImage.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));
    rightFilteredImage.write(reinterpret_cast<const char*>(filteredImage), size);
    rightImage.close();
    rightFilteredImage.close();

    // поворачиваем влево и применяем фильтр Гаусса
    left(buff, rotatedImage, width, height);

    ofstream leftImage("left_rotated.bmp", ios::binary);
    leftImage.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));
    leftImage.write(reinterpret_cast<const char*>(rotatedImage), size);
    //filter
    applyGF(rotatedImage, filteredImage, widthRot, heightRot, gFilter, filterSize, radius);
    ofstream leftFilteredImage("left_rotated_and_filtered.bmp", ios::binary);
    leftFilteredImage.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));
    leftFilteredImage.write(reinterpret_cast<const char*>(filteredImage), size);
    leftImage.close();
    leftFilteredImage.close();


    // освобождаем память
    delete[] buff;
    delete[] rotatedImage;
    delete[] filteredImage;

    return 0;
}
