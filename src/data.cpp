/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * data.cpp
 */

#include <array>
#include <string>

#include <boost/algorithm/string.hpp>

#include <OpenXLSX.hpp>

#include "data.h"

using namespace std;
using namespace OpenXLSX;

// Constructors
TimeSlot::TimeSlot(){}

TimeSlot::TimeSlot(int idx, std::string name, std::string hours, int day, int slotOfDay) :
    idx(idx), name(name), hours(hours), day(day), slotOfDay(slotOfDay)
    {};

StudentGroup::StudentGroup(){};

StudentGroup::StudentGroup(int idx, std::string name, std::vector<const Professional*>& pros) :
    idx(idx), name(name), pros(pros) {};

Professional::Professional(){};

Professional::Professional(int idx, std::string name, std::vector<const TimeSlot *>& slots,
        std::vector<const StudentGroup*>& groups) :
    idx(idx), name(name), slots(slots), groups(groups)
    {};

Data::Data(){};

Data::Data(Dimension dimensions, Config config, std::vector<Professional *>& professionals,
    std::vector<StudentGroup *>& groups, std::vector<TimeSlot *>& slots) :
    dimensions(dimensions), config(config), professionals(professionals), groups(groups),
    slots(slots)
    {};

Config::Config() {};

Config::Config(vector<string> &days, vector<string>& slots, int nbWeeks, int nbDays,
    int nbSlotsByDay, int nbPros, int maxInter) :
    days(days), slots(slots), nbWeeks(nbWeeks), nbDays(nbDays), nbSlotsByDay(nbSlotsByDay),
    nbPros(nbPros), maxInter(maxInter)
    {};

bool StudentGroup::isProCompatible(Professional* pro) {
    return find(pros.begin(), pros.end(), pro) != pros.end();
}

bool Professional::isProAvailOnSlot(TimeSlot* slot) {
    return find(slots.begin(), slots.end(), slot) != slots.end();
}

bool Professional::isGroupCompatible(StudentGroup* group) {
    return find(groups.begin(), groups.end(), group) != groups.end();
}

Dimension* readXLSDimensions(XLWorksheet& sheet) {
    // Number of slots
    auto numSlots = 0;
    auto rowHours = 5;
    auto colHourOff = 1;
    auto iterateSlots = true;
    while (iterateSlots) {
        auto colIdx = numSlots + colHourOff;
        auto hour = sheet.cell(XLCellReference(rowHours + 1, colIdx + 1));
        if (hour.valueType() == XLValueType::Empty) {
            iterateSlots = false;
        } else {
            numSlots++;
        }
    }
    // Number of pros
    auto numPros = 0;
    auto colPros = 0;
    auto rowOffset = 6;
    bool iteratePros = true;
    while (iteratePros) {
        auto rowIdx = numPros + rowOffset;
        auto cellProName = sheet.cell(XLCellReference(rowIdx + 1, colPros + 1));
        auto proName = cellProName.value().get<string>();
        if (proName.compare("Nombre") == 0) {
            iteratePros = false;
        } else {
            numPros++;
        }
    }
    auto numGroups = 0;
    auto numLanguages = 0;
    Dimension* dimensions = new Dimension(numPros, numGroups, numLanguages, numSlots);
    return dimensions;
};

Config* readXLSConfig(XLWorksheet& sheet) {
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
    string cur_month = "";
    string cur_day = "";
    while (iterateSlots) {
        auto colIdx = nbSlots + colOffset;
        auto hour = sheet.cell(XLCellReference(rowHour + 1, colIdx + 1));
        string cur_hour;
        if (hour.valueType() == XLValueType::Empty) {
            iterateSlots = false;
        } else {
            slots.insert(string(hour.value().get<string>()));
            auto day = sheet.cell(XLCellReference(rowDay + 1, colIdx + 1));
            if (day.valueType() != XLValueType::Empty) {
                days.push_back(day.value().get<string>());
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
    Config* config = new Config(days, slotsVec, nbWeeks, nbDays, nbSlotsByDay, nbPros, 3);
    return config;
};

vector<TimeSlot *>* readXLSSlots(XLWorksheet& sheet) {
    vector<TimeSlot *>* slots = new vector<TimeSlot *>();
    // Reading slots
    auto row_month = 3;
    auto row_day = 4;
    auto row_hour = 5;
    auto colIter = 0;
    auto colOffset = 1;
    bool iterateSlots = true;
    string cur_month = "";
    string cur_day = "";
    auto cur_d = -1;
    auto slotOfDay = 0;
    while (iterateSlots) {
        auto colIdx = colIter + colOffset;
        auto hour = sheet.cell(XLCellReference(row_hour + 1, colIdx + 1));
        string cur_hour;
        if (hour.valueType() == XLValueType::Empty) {
            iterateSlots = false;
        } else {
            cur_hour = hour.value().get<string>();
            auto month = sheet.cell(XLCellReference(row_month + 1, colIdx + 1));
            if (month.valueType() != XLValueType::Empty) {
                cur_month = month.value().get<string>();
            }
            auto day = sheet.cell(XLCellReference(row_day + 1, colIdx + 1));
            if (day.valueType() != XLValueType::Empty) {
                cur_day = day.value().get<string>();
                slotOfDay = 0;
                cur_d++;
            } else {
                slotOfDay++;
            }
            auto slotName = cur_month + " " + cur_day + " " + cur_hour;
            TimeSlot* slot = new TimeSlot(colIter, slotName, cur_hour, cur_d, slotOfDay);
            slots->push_back(slot);
        }
        colIter++;
    } 
    return slots;
};

vector<Professional *>* readXLSProfessionals(Data* data, XLWorksheet& sheet) {
    vector<Professional *>* pros = new vector<Professional *>();
    auto colSlotOff = 1;
    auto colPros = 0;
    auto rowIter = 0;
    auto rowOffset = 6;
    bool iteratePros = true;
    // Iterating on professionals names until "Nombre"
    while (iteratePros) {
        auto rowIdx = rowIter + rowOffset;
        auto proName = sheet.cell(XLCellReference(rowIdx + 1, colPros + 1)).value().get<string>();
        if (proName.compare("Nombre") == 0) {
            iteratePros = false;
        } else {
            vector<const TimeSlot *> slots {};
            // Reading availabilities for this professional
            for (auto slotIdx = 0; slotIdx < data->dimensions.numSlots; slotIdx++) {
                auto colIdx = slotIdx + colSlotOff;
                auto slotCell = sheet.cell(XLCellReference(rowIdx + 1, colIdx + 1));
                if (slotCell.valueType() == XLValueType::String) {
                    auto slot = data->slots[slotIdx];
                    slots.push_back(slot);
                }
            }
            vector<const StudentGroup *> groups {};
            Professional* pro = new Professional(rowIter, string(proName), slots, groups);
            pros->push_back(pro);
        }
        rowIter++;
    }
    return pros;
};

vector<StudentGroup *>* readXLSGroups(XLWorksheet& sheet, Dimension& dimensions) {
    vector<StudentGroup *>* groups = new vector<StudentGroup *>();
    auto rowGroups = 0;
    auto groupIdx = 0;
    auto colOffset = 1;
    bool iterateGroups = true;
    // Iterating on group names until empty cell
    while (iterateGroups) {
        auto row = rowGroups;
        auto col = groupIdx + colOffset;
        auto groupNameCell = sheet.cell(XLCellReference(row + 1, col + 1));
        if (groupNameCell.valueType() == XLValueType::String) {
            auto groupName = groupNameCell.value().get<string>();
            vector<const Professional *> pros {};
            StudentGroup* group = new StudentGroup(groupIdx, string(groupName), pros);
            groups->push_back(group);
            groupIdx++;
        } else {
            iterateGroups = false;
        }
    }
    dimensions.numGroups = groupIdx;
    return groups;
};

void readXLSCompatibilities(Data* data, XLWorksheet& sheet) {
    auto colPros = 0;
    auto rowGroups = 0;
    auto colGroupsOff = 1;
    auto rowProsOff  = 1;
    auto colIter = 0;
    bool iterateGroups = true;
    // Iterating on group names until empty cell
    while (iterateGroups) {
        auto groupNameCell = sheet.cell(XLCellReference(rowGroups + 1, colIter + colGroupsOff + 1));
        if (groupNameCell.valueType() == XLValueType::String) {
            auto groupName = groupNameCell.value().get<string>();
            StudentGroup* group = data->getGroupPtrByName(groupName);
            // If group is in the data set
            if (group != NULL) {
                auto rowIter = 0;
                bool iteratePros = true;
                // Iterating on professionals
                while (iteratePros) {
                    auto prosNameCell = sheet.cell(XLCellReference(rowIter + rowProsOff + 1, colPros + 1));
                    if (prosNameCell.valueType() == XLValueType::String) {
                        auto proName = prosNameCell.value().get<string>();
                        Professional* pro = data->getProPtrByName(proName);
                        // If pro is in the data set
                        if (pro != NULL) {
                            auto compatCell = sheet.cell(XLCellReference(rowIter + rowProsOff + 1, colIter + colGroupsOff + 1));
                            if (compatCell.valueType() == XLValueType::Empty) {
                                pro->groups.push_back(group);
                                group->pros.push_back(pro);
                            }
                        } else { // Consider full compatibility
                            for (auto& pro : data->professionals) {
                                group->pros.push_back(pro);
                                pro->groups.push_back(group);
                            }
                        }
                        rowIter++;
                    } else {
                        iteratePros = false;
                    }
                }
            }
            colIter++;
        } else {
            iterateGroups = false;
        }
    }
};

// IO functions
Data* readXLS(string& filename) {
    Data* data = new Data();
    XLDocument doc;
    doc.open(filename.c_str());
    auto sheet1 = doc.workbook().worksheet("Sondage");
    data->dimensions = *readXLSDimensions(sheet1);
    data->config = *readXLSConfig(sheet1);
    data->slots = *readXLSSlots(sheet1);
    data->professionals = *readXLSProfessionals(data, sheet1);
    auto sheet2 = doc.workbook().worksheet("FPP");
    data->groups = *readXLSGroups(sheet2, data->dimensions);
    readXLSCompatibilities(data, sheet2);
    doc.close();

    return data;
};

Data* generateData(int numPros, int numGroups, float slotCompatProb, float proGroupCompatProb) {
    // Init DisplayConfig
    auto num_weeks = 2;
    auto n_max_pros_interv = 3;
    vector<string> days{ "2020-11-09", "2020-11-11", "2020-11-12", "2020-11-13", "2020-11-14", "2020-11-15", "2020-11-16", "2020-11-17", "2020-11-18", "2020-11-19" };
    vector<string> base_slots_start{ "9h", "10h30", "14h", "15h30" };
    vector<string> base_slots_end{ "10h30", "12h", "15h30", "17h" };

    // Initializing random number generation
    srand(123);
    // srand(time(NULL));
    static string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    string proName;
    int strLength = 10;
    proName.resize(strLength);
    // Generating names for professionals
    vector<string> pros_str {};
    for (int proI = 0; proI < numPros; proI++) {
        for (int i = 0; i < strLength; i++) {
            proName[i] = charset[rand() % charset.length()];
        }
        pros_str.push_back("pro_" + proName);
    }

    string groupName;
    strLength = 6;
    groupName.resize(strLength);
    // Generating names for students groups
    vector<string> groups_str {};
    for (int grpI = 0; grpI < numGroups; grpI++) {
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
    // Generating time slots
    for (auto day : days) {
        for (unsigned long slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
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

    for (unsigned long slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
        slots_str.push_back(base_slots_start[slot_idx] + "-" + base_slots_end[slot_idx]);
    }
    // Initializing config
    config->days = days;
    config->slots = slots_str;
    config->nbWeeks = num_weeks;
    config->nbDays = days.size();
    config->nbSlotsByDay = slots_str.size();
    config->nbPros = pros_str.size();
    config->maxInter = n_max_pros_interv;

    Dimension* dimensions = new Dimension(pros_str.size(), groups_str.size(), 1, slots.size());

    int** dispo = new int*[dimensions->numPros];
    // Generating random availabilities for professionals
    for (int i = 0; i < dimensions->numPros; ++i) {
        dispo[i] = new int[slots.size()];
        for (int j = 0; j < dimensions->numSlots; ++j) {
            auto rng = rand() % 2;
            dispo[i][j] = rng;
        }
    }
    vector<StudentGroup *> groups;
    auto cnt_group = 0;
    // Initializing students groups info
    for (auto& group : groups_str) {
        StudentGroup* new_group = new StudentGroup();
        new_group->idx = cnt_group;
        new_group->name = group;
        groups.push_back(new_group);
        cnt_group++;
    }
    vector<Professional *> pros;
    auto cnt_pro = 0;
    // Initializing professionals info
    for (auto& pro : pros_str) {
        Professional* new_pro = new Professional();
        new_pro->idx = cnt_pro;
        new_pro->name = pro;
        // Generating random pro availabilities
        for (auto& slot : slots) {
            auto randFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (randFloat <= slotCompatProb) {
                new_pro->slots.push_back(slot);
                slot->pros.push_back(new_pro);
            }
        }
        // Generating random compatibilities between pro and students groups
        for (auto& group : groups) {
            auto randFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (randFloat <= proGroupCompatProb) {
                new_pro->groups.push_back(group);
            }
        }
        pros.push_back(new_pro);
        cnt_pro++;
    }
    // Completing groups with compatible pros
    for (auto& group : groups) {
        for (auto& pro : pros) {
            if (find(pro->groups.begin(), pro->groups.end(), group) != pro->groups.end()) {
                group->pros.push_back(pro);
            }
        }
    }
    // Constructing Data struct info
    Data* data = new Data();
    data->dimensions = *dimensions;
    data->config = *config;
    data->slots = slots;
    data->professionals = pros;
    data->groups = groups;
    return data;
};

// Getters
Professional* Data::getProPtrByName(std::string& proName) {
    vector<Professional*>::iterator pPro = find_if(professionals.begin(), professionals.end(),
        [&] (Professional* proTmp) {
            return proTmp->name.compare(proName) == 0;
        });
    if (pPro == professionals.end()) return NULL;
    return *pPro;
}

StudentGroup* Data::getGroupPtrByName(std::string& groupName) {
    vector<StudentGroup*>::iterator pGroup = find_if(groups.begin(), groups.end(),
        [&] (StudentGroup* groupTmp) {
            return groupTmp->name.compare(groupName) == 0;
        });
    if (pGroup == groups.end()) return NULL;
    return *pGroup;
}

// Printers
ostream& Data::print(ostream& os) const {
    os << "Data(" << endl << config << endl << dimensions << endl;
    os << "Professionals" << endl;
    for (auto& pro : professionals) pro->print(os);
    os << "Students groups" << endl;
    for (auto& group : groups) group->print(os);
    os << "Slots" << endl;
    for (auto& slot : slots) slot->print(os);
    os << ")" << endl;
    return os;
};

ostream& Dimension::print(ostream& os) const {
    os << "Dimension(" << "numPros : " << numPros << endl << "numGroups : " << numGroups << endl;
    os << "numLanguages : " << numLanguages << endl << "numSlots : " << numSlots << endl << ")";
    os << endl;
    return os;
};

ostream& Config::print(ostream& os) const {
    os << "Config(" << endl << "days " << endl;
    for (auto& day : days) os << day << "," << endl;
    os << "slots " << endl;
    for (auto& slot : slots) os << slot << "," << endl;
    os << "nb_weeks : " << nbWeeks << endl << "nb_days : " << nbDays << endl << "nb_base_slots : "
        << nbSlotsByDay << endl << "nb_pros : " << nbPros << endl << "max_inter : " << maxInter;
    os << ")" << endl;
    return os;
};

ostream& TimeSlot::print(ostream& os) const {
    os << "TimeSlot(idx : " << idx << ", name : " << name << ", time interval : (" << hours << ")";
    os << ", day : " << day << ", slotOfDay : " << slotOfDay << ")" << endl;
    return os;
};

ostream& StudentGroup::print(ostream& os) const {
    os << "StudentGroup(idx : " << idx << ", name : " << name << ")" << endl;
    return os;
};

ostream& Professional::print(ostream& os) const {
    os << "Professional(idx : " << idx << ", name : " << name << ", slots " << endl;
    for (auto& slot : slots) slot->print(os);
    os << ")" << endl;
    return os;
};