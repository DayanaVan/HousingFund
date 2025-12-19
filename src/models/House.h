#ifndef HOUSE_H
#define HOUSE_H

#include <string>
#include <ctime>
#include <cmath>   // для C++

using namespace std;

struct House {
    int id;
    string address;
    int apartments;
    double totalArea;
    int buildYear;
    int floors;
    time_t createdAt;
    
    House() : id(0), apartments(0), totalArea(0.0), buildYear(0), floors(0), createdAt(0) {}
    
    // Проверка корректности данных
    bool isValid() const {
        return !address.empty() && 
               apartments > 0 && 
               totalArea > 0 && 
               buildYear >= 1500 && 
               buildYear <= (time(nullptr) / 31556952 + 1970) && // текущий год
               floors > 0 && floors <= 100;
    }
    
    // Расчет возраста дома
    int getAge() const {
        time_t now = time(nullptr);
        tm* now_tm = localtime(&now);
        int currentYear = now_tm->tm_year + 1900;
        return currentYear - buildYear;
    }
    
    // Проверка, старше ли дома заданного возраста
    bool isOlderThan(int years) const {
        return getAge() > years;
    }
    
    // Проверка на схожесть с другим домом
    bool isSimilarTo(const House& other, double threshold = 0.7) const {
        double similarity = 0.0;
        
        if (address == other.address) {
            similarity += 0.5;
        }
        
        if (abs(buildYear - other.buildYear) <= 5) {
            similarity += 0.1;
        }
        
        int aptDiff = abs(apartments - other.apartments);
        double aptRatio = static_cast<double>(aptDiff) / max(apartments, other.apartments);
        if (aptRatio <= 0.2) {
            similarity += 0.1;
        }
        
        double areaDiff = fabs(totalArea - other.totalArea);
        double areaRatio = areaDiff / max(totalArea, other.totalArea);
        if (areaRatio <= 0.15) {
            similarity += 0.1;
        }
        
        if (floors == other.floors) {
            similarity += 0.1;
        }
        
        return similarity >= threshold;
    }
};

#endif
