/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * data.h: define and builds a data structure holding all the information
 * needed to solve the problem
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <OpenXLSX.hpp>

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
        numPros(numPros), numGroups(numGroups), numLanguages(numLanguages), numSlots(numSlots)
        {};
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Dimension& rDim) { return rDim.print(os); };

// Contain useful information to display a Solution
struct Config {
    std::vector<std::string> days;
    std::vector<std::string> slots;
    int nbWeeks;
    int nbDays;
    int nbSlotsByDay;
    int nbPros;
    Config();
    Config(std::vector<std::string>& days, std::vector<std::string>& rSlots, int nbWeeks, int nbDays,
        int nbSlotsByDay, int nbPros);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Config& rConfig) { return rConfig.print(os); };

// Contain the information relative to a time slot
struct TimeSlot {
    int idx;
    std::string name;
    std::string hours;
    int day;
    int slotOfDay;
    std::vector<const Professional *> pros; // Professionals available on this time slot
    std::ostream& print(std::ostream& os = std::cout) const;
    TimeSlot();
    TimeSlot(int idx, std::string name, std::string hours, int day, int slotOfDay);
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const TimeSlot& rSlot) { return rSlot.print(os); };

// Contain the information of a single student group
struct StudentGroup {
    int idx;
    std::string name;
    std::vector<const Professional*> pros; // Professionals compatible with this students group
    StudentGroup();
    StudentGroup(int idx, std::string name, std::vector<const Professional*>& rPros);
    bool isProCompatible(Professional* pro);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const StudentGroup& rGroup) { return rGroup.print(os); };

// Contain the information of a single professional
struct Professional {
    int idx;
    std::string name;
    std::vector<const TimeSlot *> slots; // Time slots when this professional is available
    std::vector<const StudentGroup*> groups; // Groups compatible with this professional
    Professional();
    Professional(int idx, std::string name, std::vector<const TimeSlot *>& slots,
        std::vector<const StudentGroup*>& groups);
    bool isProAvailOnSlot(TimeSlot* slot);
    bool isGroupCompatible(StudentGroup* group);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Professional& rPro) { return rPro.print(os); };

// Main structure that holds the information of the problem
struct Data {
    Dimension                   dimensions;
    Config                      config;
    std::vector<Professional *> professionals;
    std::vector<StudentGroup *> groups;
    std::vector<TimeSlot *>     slots;
    Data();
    Data(Dimension& rDimensions, Config& rConfig, std::vector<Professional *>& rProfessionals,
        std::vector<StudentGroup *>& rGroups, std::vector<TimeSlot *>& rSlots);
    Professional* getProPtrByName(std::string& rProName);
    StudentGroup* getGroupPtrByName(std::string& rGroupName);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Data& rData) { return rData.print(os); };

// XLS reading functions
Dimension* readXLSDimensions(OpenXLSX::XLWorksheet& rSheet);
Config* readXLSConfig(OpenXLSX::XLWorksheet& rSheet);
std::vector<Professional *>* readXLSProfessionals(std::unique_ptr<Data>& data, OpenXLSX::XLWorksheet& rSheet);
std::vector<StudentGroup *>* readXLSGroups(OpenXLSX::XLWorksheet& rSheet, Dimension& rDimensions);
std::vector<TimeSlot *>* readXLSSlots(OpenXLSX::XLWorksheet& rSheet);
void readXLSCompatibilities(std::unique_ptr<Data>& data, OpenXLSX::XLWorksheet& rSheet);
std::unique_ptr<Data> readXLS(std::string& rFilename);

// Generate a random data set with the given number of professionals and students groups
std::unique_ptr<Data> generateData(int numPros, int numGroups, float slotCompatProb, float proGroupCompatProb);