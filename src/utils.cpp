/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * utils.cpp
 */

#include <numeric>
#include <vector>

using namespace std;

template <typename T>
float computeSDVec(vector<T>& v) {
    float sum = accumulate(begin(v), end(v), 0.0);
    float m = sum / v.size();
    float accum = 0.0;
    for_each (begin(v), end(v), [&](const float d) {
        accum += (d - m) * (d - m);
    });
    float stdev = sqrt(accum / (v.size()-1));
    return stdev;
}