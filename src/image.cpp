/* 
 *  Copyright (c) 2015  Rafael Pagés (rps (at) gti.ssr.upm.es)
 *    and Universidad Politécnica de Madrid
 *
 *  This file is part of Multitex
 *
 *  Multitex is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Multitex is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include "image.h"


Image::Image(){
}

Image::Image(const std::string& _fileName){

	if(!this->imageFile_.load(_fileName.c_str())){
		std::cerr << "Image " << _fileName << " could not be read" << std::endl;
		std::cerr << "Filename length " << _fileName.length() << std::endl;
	}

	width_ = imageFile_.getWidth();
	height_ = imageFile_.getHeight();
}

Image::Image(unsigned int _height, unsigned int _width, Color _background){

    width_ = _width;
    height_ = _height;

    fipImage tmp(FIT_BITMAP,width_,height_, 24);

    RGBQUAD color;
    color.rgbBlue = _background.getBlue();
    color.rgbGreen = _background.getGreen();
    color.rgbRed = _background.getRed();
    FreeImage_FillBackground(tmp, &color);

    imageFile_ = tmp;

}

Color Image::getColor (unsigned int _row, unsigned int _column) const{

    if ( _row > height_){
        std::cerr << _row << "/" << height_ << " row is wrong!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    if ( _column > width_){
        std::cerr << _column << "/" << width_ << " column is wrong!!!!!!!!!!!!!!!!!" << std::endl;
    }

    RGBQUAD col;
    imageFile_.getPixelColor(_column,_row, &col);
    Color color ((float)col.rgbRed, (float)col.rgbGreen, (float)col.rgbBlue);
    return color;

}

Color Image::interpolate (float _row, float _column, InterpolateMode _mode) const {

	const float c = _column - 0.5;
    const float r = _row - 0.5;
    int r_base = floor(r);
    int c_base = floor(c);

    const float x = r - (float)r_base;
    const float y = c - (float)c_base;

    r_base = std::max(r_base, 0);
    c_base = std::max(c_base, 0);

    Color final;
    Color A,B,C,D;

    // Bilinear
    if (_mode == BILINEAR){

        if (r_base + 1 > height_ || c_base + 1 > width_){
            // We are in the edge of an image, then we cannot interpolate
            final = getColor(r_base, c_base);
        } else {

           A = getColor(r_base, c_base);         // f(0,0)
           B = getColor(r_base + 1, c_base);     // f(1,0)
           C = getColor(r_base, c_base + 1);     // f(0,1)
           D = getColor(r_base + 1, c_base + 1); // f(1,1)

           // Bilinear interpolation: 
           // f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)(x)(1-y) + f(0,1)(1-x)(y) + f(1,1)(x)(y)
           final = A + (B-A) * x + (C-A) * y + (A+D-B-C) * x * y;

 
        }
    
    } else if (_mode == BICUBIC){
    // p(x,y) = a00*x^0^y^0 + a01*x^0^y^1 + a02*x^0^y^2 + a03*x^0^y^3 +
    // a10*x^1^y^0 + a11*x^1^y^1 + a12*x^1^y^2 + a13*x^1^y^3 +
    // a20*x^2^y^0 + a21*x^2^y^1 + a22*x^3^y^2 + a23*x^2^y^3 +
    // a30*x^3^y^0 + a31*x^3^y^1 + a32*x^3^y^2 + a33*x^3^y^3
    // Bi-Cubic

    	Color row_temp [4];

    	for (int i = -1; i < 3; i++) {


    		const Color M = getColor(r_base + i, c_base - 1);
    		const Color N = getColor(r_base + i, c_base);
    		const Color O = getColor(r_base + i, c_base + 1);
    		const Color P = getColor(r_base + i, c_base + 2);

    		A = P - O;
    		B = N - M;
    		C = O - M;

    		row_temp[i+1] = N + ( ( (A+B) * y - A - B - B) * y + C) * y;
    	}

    	A = row_temp[3] - row_temp[2];
    	B = row_temp[1] - row_temp[0];
    	C = row_temp[2] - row_temp[0];

    	final = row_temp[1] + ( ( (A+B) * x - A - B - B) * x + C) * x;

    } else {
    	std::cerr << "Mode " << _mode << " is unknown" << std::endl;
    	final = Color(0,0,0);
    }
    return final;
}


void Image::setColor (const Color& _color, unsigned int _row, unsigned int _column){
    
    RGBQUAD col;
    col.rgbBlue = _color.getBlue();
    col.rgbGreen = _color.getGreen();
    col.rgbRed = _color.getRed();

    imageFile_.setPixelColor(_column,_row,&col);
}

void Image::save(const std::string& _fileName){

    char * name = new char[_fileName.length()+1];
    strcpy(name, _fileName.c_str());

    this->imageFile_.convertTo24Bits();
    this->imageFile_.save(name);

}

unsigned int Image::getWidth() const{
	return width_;
}

unsigned int Image::getHeight() const{
	return height_;
}
