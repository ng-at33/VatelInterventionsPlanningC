/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * data.h: define and builds a data structure holding all the information
 * needed to solve the problem
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "libxl.h"

struct VatelData;
struct Dimension;
struct DisplayConfig;
struct Professional;
struct StudentGroup;
struct TimeSlot;

// Hold the dimensions of the main vectors of a Data struct
struct Dimension {
    int numPros;
    int numGroups;
    int numLanguages;
    int numSlots;
    std::ostream& print(std::ostream& os = std::cout) const;

    Dimension(){};
    Dimension(int numPros, int numGroups, int numLanguages, int numSlots) :
        numPros(numPros), numGroups(numGroups), numLanguages(numLanguages),
        numSlots(numSlots)
        {};
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Dimension& dim) {
    return dim.print(os);
};

// Contain useful information to display a Solution
struct Config {
    std::vector<std::string> days;
    std::vector<std::string> slots;
    int nbWeeks;
    int nbDays;
    int nbSlotsByDay;
    int nbPros;
    int maxInter;
    Config();
    Config(std::vector<std::string> days, std::vector<std::string> slots,
        int nbWeeks, int nbDays, int nbSlotsByDay, int nbPros, int maxInter);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Config& config) {
    return config.print(os);
};

// Contain the information relative to a time slot
struct TimeSlot {
    int idx;
    std::string name;
    std::string hours;
    int day;
    int slotOfDay;
    std::ostream& print(std::ostream& os = std::cout) const;
    TimeSlot();
    TimeSlot(int idx, std::string name, std::string hours,
        int day, int slotOfDay);
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const TimeSlot& ts) {
    return ts.print(os);
};

// Contain the information of a single professional
struct Professional {
    int idx;
    std::string name;
    std::vector<TimeSlot *> slots;
    Professional();
    Professional(int idx, std::string name, std::vector<TimeSlot *> slots);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Professional& pf) {
    return pf.print(os);
};

// Main structure that holds the information of the problem
struct Data {
    Dimension                   dimensions;
    Config                      config;
    std::vector<Professional *> professionals;
    std::vector<StudentGroup *> groups;
    std::vector<TimeSlot *>     slots;
    Data();
    Data(Dimension dimensions, Config config,
        std::vector<Professional *> professionals,
        std::vector<StudentGroup *> groups, std::vector<TimeSlot *> slots);
};

// Overrides cout
std::ostream& operator<<(std::ostream& os, const Data& data);

// XLS reading functions
Dimension* readXLSDimensions(libxl::Sheet* sheet);
Config* readXLSConfig(libxl::Sheet* sheet);
std::vector<Professional *>* readXLSProfessionals(Data* data,
    libxl::Sheet* sheet);
std::vector<StudentGroup *>* readXLSGroups(libxl::Sheet* sheet);
std::vector<TimeSlot *>* readXLSSlots(libxl::Sheet* sheet);
Data* readXLS(std::string& filename);