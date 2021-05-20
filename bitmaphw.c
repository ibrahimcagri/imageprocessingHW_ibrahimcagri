




//USE cygwin as compiler, preferably CLion as IDE
//output .bmp's for hw2 and hw3 are provided under cmake-build-debug





#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"

int main() {

    // 1/scale
    int n = 2;

    // names of in/out files
    char *infile = "itu.bmp";
    char *outfile = "itu-downscaled.bmp";

    // open input file
    FILE *inptr = fopen(infile, "r");

    // open output file
    FILE *outptr = fopen(outfile, "w");

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);


    // determine dimensions of the new image
    int oldWidth = bi.biWidth;
    int oldHeight = bi.biHeight;
    int newWidth = oldWidth / n;
    int newHeight = oldHeight / n;

    // reconfigure headers
    bi.biHeight = newHeight;
    bi.biWidth = newWidth;
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * newWidth)) * abs(newHeight);
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // allocate memory to store input and output lines
    RGBTRIPLE inputLine1[oldWidth * sizeof(RGBTRIPLE)];
    RGBTRIPLE inputLine2[oldWidth * sizeof(RGBTRIPLE)];
    RGBTRIPLE outputLine[newWidth * sizeof(RGBTRIPLE)];

    // input lines
    for (int i = 0, biHeight = abs(oldHeight); i < biHeight; i++) {

        for (int j = 0; j < oldWidth; j++) {

            RGBTRIPLE triple;

            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            inputLine1[j] = triple;
        }

        for (int j = 0; j < oldWidth; j++) {

            RGBTRIPLE triple;

            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            inputLine2[j] = triple;
        }

        for (int j = 0; j < newWidth; ++j) {
            RGBTRIPLE input1 = inputLine1[2 * j], input2 = inputLine1[2 * j + 1],
                    input3 = inputLine2[2 * j], input4 = inputLine2[2 * j + 1];
            RGBTRIPLE output;
            output.rgbtBlue = (input1.rgbtBlue + input2.rgbtBlue + input3.rgbtBlue + input4.rgbtBlue) / 4;
            output.rgbtGreen = (input1.rgbtGreen + input2.rgbtGreen + input3.rgbtGreen + input4.rgbtGreen) / 4;
            output.rgbtRed = (input1.rgbtRed + input2.rgbtRed + input3.rgbtRed + input4.rgbtRed) / 4;
            outputLine[j] = output;
        }

        // write the current output line
        fwrite(outputLine, sizeof(RGBTRIPLE), newWidth, outptr);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // names of in/out files
    infile = "itu.bmp";
    outfile = "itu-filtered.bmp";

    // open input file
    inptr = fopen(infile, "r");

    // open output file
    outptr = fopen(outfile, "w");

    // read infile's BITMAPFILEHEADER
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // determine dimensions of the image
    int width = bi.biWidth;
    int height = bi.biHeight;
    printf("width: %d\n", width);
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // define kernel matrix
    double kernel[3][3] = {{0.0625, 0.125, 0.0625},
                           {0.125,  0.25,  0.125},
                           {0.0625, 0.125, 0.0625}};

    // define 0 pixel
    RGBTRIPLE zeros;
    zeros.rgbtRed = 0;
    zeros.rgbtGreen = 0;
    zeros.rgbtBlue = 0;

    // add padding
    RGBTRIPLE *inputLine = calloc(width + 2, sizeof(RGBTRIPLE));
    RGBTRIPLE *imageMatrix = calloc((width + 2) * (height + 2), sizeof(RGBTRIPLE));

    for (int i = 0; i < width + 2; ++i) {
        *(imageMatrix + i) = zeros;
    }

    for (int i = 1; i < height + 1; ++i) {
        for (int j = 0; j < width + 2; j++) {
            RGBTRIPLE triple;

            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            inputLine[j - 1] = triple;
        }

        inputLine[0] = zeros;
        inputLine[width + 1] = zeros;

        for (int j = 0; j < width + 2; ++j) {
            *(imageMatrix + (i * (width + 2)) + j) = inputLine[j];
        }
    }

    for (int i = 0; i < width + 2; ++i) {
        *(imageMatrix + ((width + 2) * (height + 1)) + i) = zeros;
    }

    // define adjacent pixel matrix
    RGBTRIPLE *adjacentMatrix = calloc(3 * 3, sizeof(RGBTRIPLE));
    RGBTRIPLE newPixel;

    double redValue;
    double blueValue;
    double greenValue;

    RGBTRIPLE *filteredImage = calloc(width * height, sizeof(RGBTRIPLE));

    for (int i = 1; i < height + 1; ++i) {
        for (int j = 1; j < width + 1; ++j) {
            *(adjacentMatrix + 0) = *(imageMatrix + j + ((i - 1) * width) - 1);
            *(adjacentMatrix + 1) = *(imageMatrix + j + ((i - 1) * width));
            *(adjacentMatrix + 2) = *(imageMatrix + j + ((i - 1) * width) + 1);
            *(adjacentMatrix + 3) = *(imageMatrix + j + (i * width) - 1);
            *(adjacentMatrix + 4) = *(imageMatrix + j + (i * width));
            *(adjacentMatrix + 5) = *(imageMatrix + j + (i * width) + 1);
            *(adjacentMatrix + 6) = *(imageMatrix + j + ((i + 1) * width) - 1);
            *(adjacentMatrix + 7) = *(imageMatrix + j + ((i + 1) * width));
            *(adjacentMatrix + 8) = *(imageMatrix + j + ((i + 1) * width) + 1);

            redValue = 0;
            greenValue = 0;
            blueValue = 0;

            // calculate new values of each color pixel
            for (int k = 0; k < 3; ++k) {
                for (int l = 0; l < 3; ++l) {
                    redValue += (adjacentMatrix + (k * 3) + l)->rgbtRed * kernel[k][l];
                    greenValue += (adjacentMatrix + (k * 3) + l)->rgbtGreen * kernel[k][l];
                    blueValue += (adjacentMatrix + (k * 3) + l)->rgbtBlue * kernel[k][l];
                }
            }

            newPixel.rgbtRed = redValue;
            newPixel.rgbtGreen = greenValue;
            newPixel.rgbtBlue = blueValue;

            *(filteredImage + ((i - 1) * width) + j - 1) = newPixel;
        }
    }

    // write output line
    RGBTRIPLE filterOutputLine[width * sizeof(RGBTRIPLE)];

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            filterOutputLine[j] = *(filteredImage + (i * width) + j);
        }
        fwrite(filterOutputLine, sizeof(RGBTRIPLE), width, outptr);
    }

    fclose(inptr);
    fclose(outptr);

    // end main
    return 0;
}