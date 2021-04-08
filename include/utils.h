/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * utils.h: define some useful functions
 */


#pragma once

#include <iostream>
#include <numeric>
#include <vector>

template <typename T>
float computeSDVec(std::vector<T>& v) {
    if (v.size() == 1) return 0.0;
    float sum = accumulate(begin(v), end(v), 0.0);
    float m = sum / v.size();
    float accum = 0.0;
    for_each (begin(v), end(v), [&](const float d) {
        accum += (d - m) * (d - m);
    });
    float stdev = sqrt(accum / (v.size()-1));
    return stdev;
}