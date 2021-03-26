/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * data.cpp
 */

#include <array>
#include <string>

#include <boost/algorithm/string.hpp>

#include "libxl.h"

#include "data.h"

using namespace std;
using namespace libxl;

// Constructors
TimeSlot::TimeSlot(){}

TimeSlot::TimeSlot(int idx, std::string name, std::string hours,
    int day, int slotOfDay) :
    idx(idx), name(name), hours(hours), day(day), slotOfDay(slotOfDay)
    {};

StudentGroup::StudentGroup(){};

StudentGroup::StudentGroup(int idx, std::string name) :
    idx(idx), name(name)
    {};

Professional::Professional(){};

Professional::Professional(int idx, std::string name,
    std::vector<TimeSlot *> slots) :
    idx(idx), name(name), slots(slots)
    {};

Data::Data(){};

Data::Data(Dimension dimensions, Config config,
    std::vector<Professional *> professionals,
    std::vector<StudentGroup *> groups, std::vector<TimeSlot *> slots) :
    dimensions(dimensions), config(config), professionals(professionals),
    groups(groups), slots(slots)
    {};

Config::Config(){};

Config::Config(vector<string> days, vector<string> slots, int nbWeeks,
    int nbDays, int nbSlotsByDay, int nbPros, int maxInter) :
    days(days), slots(slots), nbWeeks(nbWeeks), nbDays(nbDays),
    nbSlotsByDay(nbSlotsByDay), nbPros(nbPros), maxInter(maxInter)
    {};

Dimension* readXLSDimensions(Sheet* sheet) {
    // Number of slots
    auto numSlots = 0;
    auto rowHours = 5;
    auto colHourOff = 1;
    auto iterateSlots = true;
    while (iterateSlots) {
        auto colIdx = numSlots + colHourOff;
        auto hour = sheet->readStr(rowHours, colIdx);
        if (!hour) {
            iterateSlots = false;
        } else {
            numSlots++;
        }
    }
    // Number of pros
    auto numPros = 0;
    auto colPros = 0;
    auto rowIter = 0;
    auto rowOffset = 6;
    bool iteratePros = true;
    while (iteratePros) {
        auto rowIdx = numPros + rowOffset;
        auto proName = sheet->readStr(rowIdx, colPros);
        if (strcmp(proName, "Nombre") == 0) {
            iteratePros = false;
        } else {
            numPros++;
        }
    }
    auto numGroups = 4;
    auto numLanguages = 0;
    Dimension* dimensions = new Dimension(numPros, numGroups, numLanguages,
        numSlots);
    return dimensions;
};

Config* readXLSConfig(Sheet* sheet) {
    auto nbWeeks = 2;
    // Reading days and slot names
    vector<string> days {};
    set<string> slots {};
    auto rowDay = 4;
    auto rowHour = 5;
    auto nbSlots = 0;
    auto nbDays = 0;
    auto nbSlotsByDay = 0;
    auto nbSlotsByDayTmp = 0;
    auto colOffset = 1;
    bool iterateSlots = true;
    const char* cur_hour = "";
    string cur_month = "";
    string cur_day = "";
    auto cur_d = 0;
    auto slotOfDay = 0;
    while (iterateSlots) {
        auto colIdx = nbSlots + colOffset;
        auto hour = sheet->readStr(rowHour, colIdx);
        string cur_hour;
        if (!hour) {
            iterateSlots = false;
        } else {
            slots.insert(string(hour));
            if (auto day = sheet->readStr(rowDay, colIdx)) {
                days.push_back(string(day));
                nbDays++;
                nbSlotsByDayTmp = 0;
            }
        }
        nbSlotsByDay = max(nbSlotsByDay, nbSlotsByDayTmp);
        nbSlotsByDayTmp++;
        nbSlots++;
    }
    auto nbPros = 0;
    std::vector<string> slotsVec(slots.begin(), slots.end());
    Config* config = new Config(days, slotsVec, nbWeeks, nbDays, nbSlotsByDay,
        nbPros, 3);
    return config;
};

vector<TimeSlot *>* readXLSSlots(Sheet* sheet) {
    vector<TimeSlot *>* slots = new vector<TimeSlot *>();
    // Reading slots
    auto row_month = 3;
    auto row_day = 4;
    auto row_hour = 5;
    auto colIter = 0;
    auto colOffset = 1;
    bool iterateSlots = true;
    const char* cur_hour = "";
    string cur_month = "";
    string cur_day = "";
    auto cur_d = 0;
    auto slotOfDay = 0;
    while (iterateSlots) {
        auto colIdx = colIter + colOffset;
        auto hour = sheet->readStr(row_hour, colIdx);
        string cur_hour;
        if (!hour) {
            iterateSlots = false;
        } else {
            cur_hour = string(hour);
            if (auto month = sheet->readStr(row_month, colIdx)) {
                cur_month = month;
            }
            if (auto day = sheet->readStr(row_day, colIdx)) {
                cur_day = day;
                cur_d++;
                slotOfDay = 0;
            } else {
                slotOfDay++;
            }
            auto slotName = cur_month + " " + cur_day + " " + hour;
            TimeSlot* slot = new TimeSlot(colIter, slotName, cur_hour, cur_d, slotOfDay);
            slots->push_back(slot);
        }
        colIter++;
    } 
    return slots;
};

vector<Professional *>* readXLSProfessionals(Data* data, 
        Sheet* sheet) {
    vector<Professional *>* pros = new vector<Professional *>();
    auto colSlotOff = 1;
    auto colPros = 0;
    auto rowIter = 0;
    auto rowOffset = 6;
    bool iteratePros = true;
    // Iterating on professionals names until "Nombre"
    while (iteratePros) {
        auto rowIdx = rowIter + rowOffset;
        auto proName = string(sheet->readStr(rowIdx, colPros));
        if (proName.compare("Nombre") == 0) {
            iteratePros = false;
        } else {
            vector<TimeSlot *> slots {};
            // Reading availabilities for this professional
            for (auto slotIdx = 0; slotIdx < data->dimensions.numSlots; slotIdx++) {
                auto colIdx = slotIdx + colSlotOff;
                auto cellType = sheet->cellType(rowIdx, colIdx);
                if (cellType == CELLTYPE_STRING) {
                    auto slot = data->slots[slotIdx];
                    slots.push_back(slot);
                }
            }
            Professional* pro = new Professional(rowIter, string(proName),
                slots);
            pros->push_back(pro);
        }
        rowIter++;
    } 
    return pros;
};

vector<StudentGroup *>* readXLSGroups(Sheet* sheet) {
    vector<StudentGroup *>* groups = new vector<StudentGroup *>();
    return groups;
};

// IO functions
Data* readXLS(string& filename) {
    Data* data = new Data();
    Book* book = xlCreateXMLBook();
    if (book->load(filename.c_str())) {
        Sheet* sheet = book->getSheet(1);
        if (sheet) {
            data->dimensions = *readXLSDimensions(sheet);
            data->config = *readXLSConfig(sheet);
            data->slots = *readXLSSlots(sheet);
            data->professionals = *readXLSProfessionals(data, sheet);
            data->groups = *readXLSGroups(sheet);
        }
    }
    book->release();

    return data;
};

Data* generateData(int numPros, int numGroups, float slotCompatProb) {
    // Init DisplayConfig
    auto num_weeks = 2;
    auto n_max_pros_interv = 3;
    auto n_min_class_interv = 1;
    vector<string> days{ "2020-11-09", "2020-11-11", "2020-11-12", "2020-11-13", "2020-11-14", "2020-11-15", "2020-11-16", "2020-11-17", "2020-11-18", "2020-11-19" };
    vector<string> base_slots_start{ "9h", "10h30", "14h", "15h30" };
    vector<string> base_slots_end{ "10h30", "12h", "15h30", "17h" };

    srand(time(NULL));
    static string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    string proName;
    int strLength = 10;
    proName.resize(strLength);
    vector<string> pros_str {};
    for (int proI; proI < numPros; proI++) {
        for (int i = 0; i < strLength; i++) {
            proName[i] = charset[rand() % charset.length()];
        }
        pros_str.push_back("pro_" + proName);
    }

    string groupName;
    strLength = 6;
    groupName.resize(strLength);
    vector<string> groups_str {};
    for (int grpI; grpI < numGroups; grpI++) {
        for (int i = 0; i < strLength; i++) {
            groupName[i] = charset[rand() % charset.length()];
        }
        groups_str.push_back("group_" + groupName);
    }

    Config* config = new Config();

    vector<string> slots_str;
    vector<TimeSlot *> slots;
    auto cnt_slots = 0;
    auto cnt_days = 0;
    for (auto day : days) {
        for (auto slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
            TimeSlot* new_slot = new TimeSlot();
            new_slot->idx = cnt_slots;
            new_slot->name = day + " " + base_slots_start[slot_idx] + "-" + base_slots_end[slot_idx];
            new_slot->hours = base_slots_start[slot_idx] + "-" + base_slots_end[slot_idx];
            new_slot->day = cnt_days;
            new_slot->slotOfDay = slot_idx;
            slots.push_back(new_slot);
            cnt_slots++;
        }
        cnt_days++;
    }

    for (auto slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
        slots_str.push_back(base_slots_start[slot_idx] + "-" + base_slots_end[slot_idx]);
    }
    config->days = days;
    config->slots = slots_str;
    config->nbWeeks = num_weeks;
    config->nbDays = days.size();
    config->nbSlotsByDay = slots_str.size();
    config->nbPros = pros_str.size();
    config->maxInter = n_max_pros_interv;

    Dimension* dimensions = new Dimension(pros_str.size(), groups_str.size(), 1, slots.size());

    int** dispo = new int*[dimensions->numPros];
    for (int i = 0; i < dimensions->numPros; ++i) {
        dispo[i] = new int[slots.size()];
        for (int j = 0; j < dimensions->numSlots; ++j) {
            auto rng = rand() % 2;
            dispo[i][j] = rng;
        }
    }
    vector<Professional *> pros;
    auto cnt_pro = 0;
    for (auto& pro : pros_str) {
        Professional* new_pro = new Professional();
        new_pro->idx = cnt_pro;
        new_pro->name = pro;
        for (int i = 0; i < dimensions->numSlots; ++i) {
            auto randFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (randFloat <= slotCompatProb) {
                new_pro->slots.push_back(slots[i]);
                slots[i]->pros.push_back(new_pro);
            }
        }
        pros.push_back(new_pro);
        cnt_pro++;
    }
    vector<StudentGroup *> groups;
    auto cnt_group = 0;
    for (auto& group : groups_str) {
        StudentGroup* new_group = new StudentGroup();
        new_group->idx = cnt_group;
        new_group->name = group;
        groups.push_back(new_group);
        cnt_group++;
    }


    Data* data = new Data();
    data->dimensions = *dimensions;
    data->config = *config;
    data->slots = slots;
    data->professionals = pros;
    data->groups = groups;
    return data;
};

// Printers
ostream& operator<<(ostream& os, const Data& data) {
    os << "Data(" << endl;
    // os << data.config << endl;
    os << data.config << endl;
    os << data.dimensions << endl;
    os << "Professionals" << endl;
    for (auto& pro : data.professionals) pro->print(os);
    os << "Slots" << endl;
    for (auto& slot : data.slots) slot->print(os);
    os << ")" << endl;
    return os;
};

ostream& Dimension::print(ostream& os) const {
    os << "Dimension(";
    os << "numPros : " << numPros << endl;
    os << "numGroups : " << numGroups << endl;
    os << "numLanguages : " << numLanguages << endl;
    os << "numSlots : " << numSlots << endl;
    os << ")" << endl;
    return os;
};

ostream& Config::print(ostream& os) const {
    os << "Config(" << endl;
    os << "days " << endl;
    for (auto& day : days) os << day << "," << endl;
    os << "slots " << endl;
    for (auto& slot : slots) os << slot << "," << endl;
    os << "nb_weeks : " << nbWeeks << endl;
    os << "nb_days : " << nbDays << endl;
    os << "nb_base_slots : " << nbSlotsByDay << endl;
    os << "nb_pros : " << nbPros << endl;
    os << "max_inter : " << maxInter;
    os << ")" << endl;
    return os;
};

ostream& TimeSlot::print(ostream& os) const {
    os << "TimeSlot(idx : " << idx; 
    os << ", name : " << name;
    os << ", time interval : (" << hours << ")";
    os << ", day : " << day;
    os << ", slotOfDay : " << slotOfDay;
    os << ")" << endl;
    return os;
};

ostream& StudentGroup::print(ostream& os) const {
    os << "StudentGroup(idx : " << idx; 
    os << ", name : " << name << ")" << endl;
    return os;
};

ostream& Professional::print(ostream& os) const {
    os << "Professional(idx : " << idx; 
    os << ", name : " << name;
    os << ", slots " << endl;
    for (auto& slot : slots) slot->print(os);
    os << ")" << endl;
    return os;
};