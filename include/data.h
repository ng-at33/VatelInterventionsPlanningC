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
    int num_pros;
    int num_groups;
    int num_languages;
    int num_slots;
    std::ostream& print(std::ostream& os = std::cout) const;

    Dimension(){};
    Dimension(int num_pros, int num_groups, int num_languages, int num_slots)
        : num_pros(num_pros), num_groups(num_groups), num_languages(num_languages),
          num_slots(num_slots) {};
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Dimension& r_dim) { return r_dim.print(os); };

// Contain useful information to display a Solution
struct Config {
    std::vector<std::string> days;
    std::vector<std::string> slots;
    int nb_weeks;
    int nb_days;
    int nb_slots_by_day;
    int nb_pros;
    Config();
    Config(std::vector<std::string>& days, std::vector<std::string>& rSlots, int nb_weeks,
           int nb_days, int nb_slots_by_day, int nb_pros);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const Config& r_config) { return r_config.print(os); };

// Contain the information relative to a time slot
struct TimeSlot {
    int idx;
    std::string name;
    std::string hours;
    int day;
    int slot_of_day;
    std::vector<const std::shared_ptr<Professional>> pros; // Professionals available on this time slot
    std::ostream& print(std::ostream& os = std::cout) const;
    TimeSlot();
    TimeSlot(int idx, std::string name, std::string hours, int day, int slot_of_day);
};

// Override cout
inline std::ostream& operator<<(std::ostream& os, const TimeSlot& r_slot) { return r_slot.print(os); };

// Contain the information of a single student group
struct StudentGroup {
    int idx;
    std::string name;
    std::vector<const std::shared_ptr<Professional>> pros; // Professionals compatible with this students group
    StudentGroup();
    StudentGroup(int idx, std::string name, std::vector<const std::shared_ptr<Professional>>& r_pros);
    bool isProCompatible(std::shared_ptr<Professional> pro);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const StudentGroup& r_group) { return r_group.print(os); };

// Contain the information of a single professional
struct Professional {
    int idx;
    std::string name;
    std::vector<const std::shared_ptr<TimeSlot>> slots; // Time slots when this professional is available
    std::vector<const std::shared_ptr<StudentGroup>> groups; // Groups compatible with this professional
    Professional();
    Professional(int idx, std::string name, std::vector<const std::shared_ptr<TimeSlot>>& slots,
                 std::vector<const std::shared_ptr<StudentGroup>>& groups);
    bool isProAvailOnSlot(std::shared_ptr<TimeSlot> slot);
    bool isGroupCompatible(std::shared_ptr<StudentGroup> group);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Professional& r_pro) { return r_pro.print(os); };

// Main structure that holds the information of the problem
struct Data {
    Dimension                                  dimensions;
    Config                                     config;
    std::vector<std::shared_ptr<Professional>> professionals;
    std::vector<std::shared_ptr<StudentGroup>> groups;
    std::vector<std::shared_ptr<TimeSlot>>     slots;
    Data();
    Data(Dimension& r_dimensions, Config& r_config,
         std::vector<std::shared_ptr<Professional>>& r_professionals,
         std::vector<std::shared_ptr<StudentGroup>>& r_groups,
         std::vector<std::shared_ptr<TimeSlot>>& r_slots);
    std::shared_ptr<Professional> getProPtrByName(std::string& r_proName);
    std::shared_ptr<StudentGroup> getGroupPtrByName(std::string& r_groupName);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Data& r_data) { return r_data.print(os); };

// XLS reading functions
std::unique_ptr<Dimension> readXLSDimensions(OpenXLSX::XLWorksheet& r_sheet);
std::unique_ptr<Config> readXLSConfig(OpenXLSX::XLWorksheet& r_sheet);
std::unique_ptr<std::vector<std::shared_ptr<Professional>>> readXLSProfessionals(
    std::unique_ptr<Data>& data, OpenXLSX::XLWorksheet& r_sheet);
std::unique_ptr<std::vector<std::shared_ptr<StudentGroup>>> readXLSGroups(
    OpenXLSX::XLWorksheet& r_sheet, Dimension& r_dimensions);
std::unique_ptr<std::vector<std::shared_ptr<TimeSlot>>> readXLSSlots(OpenXLSX::XLWorksheet& r_sheet);
void readXLSCompatibilities(std::unique_ptr<Data>& data, OpenXLSX::XLWorksheet& r_sheet);
std::unique_ptr<Data> readXLS(std::string& r_dilename);

// Generate a random data set with the given number of professionals and students groups
std::unique_ptr<Data> generateData(int num_pros, int num_groups, float slot_compat_prob,
                                   float pro_group_compat_prob);