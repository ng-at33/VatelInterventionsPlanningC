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

TimeSlot::TimeSlot(int idx, std::string name, std::string hours, int day, int slot_of_day)
        : idx(idx), name(name), hours(hours), day(day), slot_of_day(slot_of_day) {};

StudentGroup::StudentGroup() {};

StudentGroup::StudentGroup(int idx, std::string name,
                           std::vector<const shared_ptr<Professional>>& r_pros)
        : idx(idx), name(name), pros(std::move(r_pros)) {};

Professional::Professional() {};

Professional::Professional(int idx, std::string name,
                           std::vector<const shared_ptr<TimeSlot>>& r_slots,
                           std::vector<const shared_ptr<StudentGroup>>& r_groups)
        : idx(idx), name(name), slots(std::move(r_slots)), groups(std::move(r_groups)) {};

Data::Data() {};

Data::Data(Dimension& r_dimensions, Config& r_config,
           std::vector<shared_ptr<Professional>>& r_professionals,
           std::vector<shared_ptr<StudentGroup>>& r_groups, std::vector<shared_ptr<TimeSlot>>& r_slots)
        : dimensions(r_dimensions), config(r_config), professionals(std::move(r_professionals)),
          groups(std::move(r_groups)), slots(std::move(r_slots))
    {};

Config::Config() {};

Config::Config(vector<string> &days, vector<string>& slots, int nb_weeks, int nb_days,
               int nb_slots_by_day, int nb_pros)
        : days(days), slots(slots), nb_weeks(nb_weeks), nb_days(nb_days),
          nb_slots_by_day(nb_slots_by_day), nb_pros(nb_pros) {};

bool StudentGroup::isProCompatible(shared_ptr<Professional> p_pro) {
    return find(pros.begin(), pros.end(), p_pro) != pros.end();
}

bool Professional::isProAvailOnSlot(shared_ptr<TimeSlot> p_slot) {
    return find(slots.begin(), slots.end(), p_slot) != slots.end();
}

bool Professional::isGroupCompatible(shared_ptr<StudentGroup> p_group) {
    return find(groups.begin(), groups.end(), p_group) != groups.end();
}

unique_ptr<Dimension> readXLSDimensions(XLWorksheet& r_sheet) {
    // Number of slots
    auto num_slots = 0;
    auto row_hours = 5;
    auto col_hour_off = 1;
    auto iterate_slots = true;
    while (iterate_slots) {
        auto col_idx = num_slots + col_hour_off;
        auto hour = r_sheet.cell(XLCellReference(row_hours + 1, col_idx + 1));
        if (hour.valueType() == XLValueType::Empty) {
            iterate_slots = false;
        } else {
            num_slots++;
        }
    }
    // Number of pros
    auto num_pros = 0;
    auto col_pros = 0;
    auto row_offset = 6;
    bool iterate_pros = true;
    while (iterate_pros) {
        auto row_idx = num_pros + row_offset;
        auto cell_pro_name = r_sheet.cell(XLCellReference(row_idx + 1, col_pros + 1));
        auto pro_name = cell_pro_name.value().get<string>();
        if (pro_name.compare("Nombre") == 0) {
            iterate_pros = false;
        } else {
            num_pros++;
        }
    }
    auto num_groups = 0;
    auto num_languages = 0;
    auto p_dimensions = make_unique<Dimension>(num_pros, num_groups, num_languages, num_slots);
    return p_dimensions;
};

unique_ptr<Config> readXLSConfig(XLWorksheet& r_sheet) {
    auto nb_weeks = 2;
    // Reading days and slot names
    vector<string> days {};
    set<string> slots {};
    auto row_day = 4;
    auto row_hour = 5;
    auto nb_slots = 0;
    auto nb_days = 0;
    auto nb_slots_by_day = 0;
    auto nb_slots_by_dayTmp = 0;
    auto col_offset = 1;
    bool iterate_slots = true;
    string cur_month = "";
    string cur_day = "";
    while (iterate_slots) {
        auto col_idx = nb_slots + col_offset;
        auto hour = r_sheet.cell(XLCellReference(row_hour + 1, col_idx + 1));
        string cur_hour;
        if (hour.valueType() == XLValueType::Empty) {
            iterate_slots = false;
        } else {
            slots.insert(string(hour.value().get<string>()));
            auto day = r_sheet.cell(XLCellReference(row_day + 1, col_idx + 1));
            if (day.valueType() != XLValueType::Empty) {
                days.push_back(day.value().get<string>());
                nb_days++;
                nb_slots_by_dayTmp = 0;
            }
        }
        nb_slots_by_day = max(nb_slots_by_day, nb_slots_by_dayTmp);
        nb_slots_by_dayTmp++;
        nb_slots++;
    }
    auto nb_pros = 0;
    std::vector<string> slots_vec(slots.begin(), slots.end());
    auto p_config = make_unique<Config>(days, slots_vec, nb_weeks, nb_days, nb_slots_by_day, nb_pros);
    return p_config;
};

unique_ptr<vector<shared_ptr<TimeSlot>>> readXLSSlots(XLWorksheet& r_sheet) {
    unique_ptr<vector<shared_ptr<TimeSlot>>> p_slots = make_unique<vector<shared_ptr<TimeSlot>>>();
    // Reading slots
    auto row_month = 3;
    auto row_day = 4;
    auto row_hour = 5;
    auto col_iter = 0;
    auto col_offset = 1;
    bool iterate_slots = true;
    string cur_month = "";
    string cur_day = "";
    auto cur_d = -1;
    auto slot_of_day = 0;
    while (iterate_slots) {
        auto col_idx = col_iter + col_offset;
        auto hour = r_sheet.cell(XLCellReference(row_hour + 1, col_idx + 1));
        string cur_hour;
        if (hour.valueType() == XLValueType::Empty) {
            iterate_slots = false;
        } else {
            cur_hour = hour.value().get<string>();
            auto month = r_sheet.cell(XLCellReference(row_month + 1, col_idx + 1));
            if (month.valueType() != XLValueType::Empty) {
                cur_month = month.value().get<string>();
            }
            auto day = r_sheet.cell(XLCellReference(row_day + 1, col_idx + 1));
            if (day.valueType() != XLValueType::Empty) {
                cur_day = day.value().get<string>();
                slot_of_day = 0;
                cur_d++;
            } else {
                slot_of_day++;
            }
            auto slot_name = cur_month + " " + cur_day + " " + cur_hour;
            auto slot = make_shared<TimeSlot>(col_iter, slot_name, cur_hour, cur_d, slot_of_day);
            p_slots->push_back(std::move(slot));
        }
        col_iter++;
    }
    return p_slots;
};

unique_ptr<vector<shared_ptr<Professional>>> readXLSProfessionals(unique_ptr<Data>& data,
                                                                  XLWorksheet& r_sheet) {
    auto p_pros = make_unique<vector<shared_ptr<Professional>>>();
    auto col_slot_off = 1;
    auto col_pros = 0;
    auto row_iter = 0;
    auto row_offset = 6;
    bool iterate_pros = true;
    // Iterating on professionals names until "Nombre"
    while (iterate_pros) {
        auto row_idx = row_iter + row_offset;
        auto pro_name = r_sheet.cell(XLCellReference(row_idx + 1, col_pros + 1)).value().get<string>();
        if (pro_name.compare("Nombre") == 0) {
            iterate_pros = false;
        } else {
            vector<const shared_ptr<TimeSlot>> slots {};
            // Reading availabilities for this professional
            for (auto slotIdx = 0; slotIdx < data->dimensions.num_slots; slotIdx++) {
                auto col_idx = slotIdx + col_slot_off;
                auto slot_cell = r_sheet.cell(XLCellReference(row_idx + 1, col_idx + 1));
                if (slot_cell.valueType() == XLValueType::String) {
                    auto slot = data->slots[slotIdx];
                    slots.push_back(slot);
                }
            }
            vector<const shared_ptr<StudentGroup>> groups {};
            auto pro = make_shared<Professional>(row_iter, string(pro_name), slots, groups);
            p_pros->push_back(pro);
        }
        row_iter++;
    }
    return p_pros;
};

unique_ptr<vector<shared_ptr<StudentGroup>>> readXLSGroups(XLWorksheet& r_sheet,
                                                           Dimension& dimensions) {
    auto p_groups = make_unique<vector<shared_ptr<StudentGroup>>>();
    auto row_groups = 0;
    auto group_idx = 0;
    auto col_offset = 1;
    bool iterate_groups = true;
    // Iterating on group names until empty cell
    while (iterate_groups) {
        auto row = row_groups;
        auto col = group_idx + col_offset;
        auto group_name_cell = r_sheet.cell(XLCellReference(row + 1, col + 1));
        if (group_name_cell.valueType() == XLValueType::String) {
            auto group_name = group_name_cell.value().get<string>();
            vector<const shared_ptr<Professional>> pros {};
            auto group = make_shared<StudentGroup>(group_idx, string(group_name), pros);
            p_groups->push_back(std::move(group));
            group_idx++;
        } else {
            iterate_groups = false;
        }
    }
    dimensions.num_groups = group_idx;
    return p_groups;
};

void readXLSCompatibilities(unique_ptr<Data>& data, XLWorksheet& r_sheet) {
    auto col_pros = 0;
    auto row_groups = 0;
    auto col_groups_off = 1;
    auto row_pros_off  = 1;
    auto col_iter = 0;
    bool iterate_groups = true;
    // Iterating on group names until empty cell
    while (iterate_groups) {
        auto group_name_cell = r_sheet.cell(XLCellReference(row_groups + 1,
                                                          col_iter + col_groups_off + 1));
        if (group_name_cell.valueType() == XLValueType::String) {
            auto group_name = group_name_cell.value().get<string>();
            auto p_group = data->getGroupPtrByName(group_name);
            // If group is in the data set
            if (p_group != nullptr) {
                auto row_iter = 0;
                bool iterate_pros = true;
                // Iterating on professionals
                while (iterate_pros) {
                    auto pros_name_cell = r_sheet.cell(XLCellReference(row_iter + row_pros_off + 1,
                                                                    col_pros + 1));
                    if (pros_name_cell.valueType() == XLValueType::String) {
                        auto pro_name = pros_name_cell.value().get<string>();
                        auto p_pro = data->getProPtrByName(pro_name);
                        // If pro is in the data set
                        if (p_pro != nullptr) {
                            auto compat_cell = r_sheet.cell(XLCellReference(row_iter + row_pros_off + 1,
                                                                          col_iter + col_groups_off + 1));
                            if (compat_cell.valueType() == XLValueType::Empty) {
                                p_pro->groups.push_back(p_group);
                                p_group->pros.push_back(p_pro);
                            }
                        } else { // Consider full compatibility
                            for (auto pro : data->professionals) {
                                p_group->pros.push_back(pro);
                                pro->groups.push_back(p_group);
                            }
                        }
                        row_iter++;
                    } else {
                        iterate_pros = false;
                    }
                }
            }
            col_iter++;
        } else {
            iterate_groups = false;
        }
    }
};

// IO functions
unique_ptr<Data> readXLS(string& r_filename) {
    auto p_data = make_unique<Data>();
    XLDocument doc;
    doc.open(r_filename.c_str());
    auto sheet1 = doc.workbook().worksheet("Sondage");
    p_data->dimensions = *readXLSDimensions(sheet1);
    p_data->config = *readXLSConfig(sheet1);
    p_data->slots = *readXLSSlots(sheet1);
    p_data->professionals = *readXLSProfessionals(p_data, sheet1);
    auto sheet2 = doc.workbook().worksheet("FPP");
    p_data->groups = *readXLSGroups(sheet2, p_data->dimensions);
    readXLSCompatibilities(p_data, sheet2);
    doc.close();

    return p_data;
};

unique_ptr<Data> generateData(int num_pros, int num_groups, float slot_compatProb,
                              float pro_group_compa_prob) {
    // Init DisplayConfig
    auto num_weeks = 2;
    auto n_max_pro_interv = 3;
    vector<string> days{ "2020-11-09", "2020-11-11", "2020-11-12", "2020-11-13", "2020-11-14", "2020-11-15", "2020-11-16", "2020-11-17", "2020-11-18", "2020-11-19" };
    vector<string> base_slots_start { "9h", "10h30", "14h", "15h30" };
    vector<string> bast_slots_end{ "10h30", "12h", "15h30", "17h" };

    // Initializing random number generation
    srand(123);
    // srand(time(nullptr));
    static string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    string pro_name;
    int str_length = 10;
    pro_name.resize(str_length);
    // Generating names for professionals
    vector<string> pros_str {};
    for (int proI = 0; proI < num_pros; proI++) {
        for (int i = 0; i < str_length; i++) {
            pro_name[i] = charset[rand() % charset.length()];
        }
        pros_str.push_back("pro_" + pro_name);
    }

    string group_name;
    str_length = 6;
    group_name.resize(str_length);
    // Generating names for students groups
    vector<string> groups_str {};
    for (int grpI = 0; grpI < num_groups; grpI++) {
        for (int i = 0; i < str_length; i++) {
            group_name[i] = charset[rand() % charset.length()];
        }
        groups_str.push_back("group_" + group_name);
    }

    auto config = make_shared<Config>();

    vector<string> slots_str;
    vector<shared_ptr<TimeSlot>> slots;
    auto cnt_slots = 0;
    auto cnt_days = 0;
    // Generating time slots
    for (auto day : days) {
        for (unsigned long slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
            auto new_slot = make_shared<TimeSlot>();
            new_slot->idx = cnt_slots;
            new_slot->name = day + " " + base_slots_start [slot_idx] + "-" + bast_slots_end[slot_idx];
            new_slot->hours = base_slots_start [slot_idx] + "-" + bast_slots_end[slot_idx];
            new_slot->day = cnt_days;
            new_slot->slot_of_day = slot_idx;
            slots.push_back(new_slot);
            cnt_slots++;
        }
        cnt_days++;
    }

    for (unsigned long slot_idx = 0; slot_idx < base_slots_start .size(); ++slot_idx) {
        slots_str.push_back(base_slots_start [slot_idx] + "-" + bast_slots_end[slot_idx]);
    }
    // Initializing config
    config->days = days;
    config->slots = slots_str;
    config->nb_weeks = num_weeks;
    config->nb_days = days.size();
    config->nb_slots_by_day = slots_str.size();
    config->nb_pros = pros_str.size();

    shared_ptr<Dimension> dimensions = make_shared<Dimension>(pros_str.size(), groups_str.size(), 1,
                                                              slots.size());

    int** dispo = new int*[dimensions->num_pros];
    // Generating random availabilities for professionals
    for (int i = 0; i < dimensions->num_pros; ++i) {
        dispo[i] = new int[slots.size()];
        for (int j = 0; j < dimensions->num_slots; ++j) {
            auto rng = rand() % 2;
            dispo[i][j] = rng;
        }
    }
    vector<shared_ptr<StudentGroup>> groups;
    auto cnt_group = 0;
    // Initializing students groups info
    for (auto const& r_group : groups_str) {
        auto new_group = make_shared<StudentGroup>();
        new_group->idx = cnt_group;
        new_group->name = r_group;
        groups.push_back(new_group);
        cnt_group++;
    }
    vector<shared_ptr<Professional>> pros;
    auto cntPro = 0;
    // Initializing professionals info
    for (auto const& r_pro : pros_str) {
        auto p_new_pro = make_shared<Professional>();
        p_new_pro->idx = cntPro;
        p_new_pro->name = r_pro;
        // Generating random pro availabilities
        for (auto& r_slot : slots) {
            auto randFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (randFloat <= slot_compatProb) {
                p_new_pro->slots.push_back(r_slot);
                r_slot->pros.push_back(p_new_pro);
            }
        }
        // Generating random compatibilities between pro and students groups
        for (auto const& r_group : groups) {
            auto randFloat = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (randFloat <= pro_group_compa_prob) {
                p_new_pro->groups.push_back(r_group);
            }
        }
        pros.push_back(p_new_pro);
        cntPro++;
    }
    // Completing groups with compatible pros
    for (auto& r_group : groups) {
        for (auto const& r_pro : pros) {
            if (find(r_pro->groups.begin(), r_pro->groups.end(), r_group) != r_pro->groups.end()) {
                r_group->pros.push_back(r_pro);
            }
        }
    }
    // Constructing Data struct info
    auto p_data = make_unique<Data>();
    p_data->dimensions = *dimensions;
    p_data->config = *config;
    p_data->slots = slots;
    p_data->professionals = pros;
    p_data->groups = groups;
    return p_data;
};

// Getters
shared_ptr<Professional> Data::getProPtrByName(std::string& pro_name) {
    vector<shared_ptr<Professional>>::iterator p_pro = find_if(professionals.begin(),
        professionals.end(),
        [&](shared_ptr<Professional>& p_pro) {
            return p_pro->name.compare(pro_name) == 0;
        });
    if (p_pro == professionals.end()) return nullptr;
    return *p_pro;
}

shared_ptr<StudentGroup> Data::getGroupPtrByName(std::string& group_name) {
    vector<shared_ptr<StudentGroup>>::iterator p_group = find_if(groups.begin(), groups.end(),
        [&](shared_ptr<StudentGroup>& p_group_tmp) {
            return p_group_tmp->name.compare(group_name) == 0;
        });
    if (p_group == groups.end()) return nullptr;
    return *p_group;
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
    os << "Dimension(" << "num_pros : " << num_pros << endl << "num_groups : " << num_groups << endl;
    os << "num_languages : " << num_languages << endl << "num_slots : " << num_slots << endl << ")";
    os << endl;
    return os;
};

ostream& Config::print(ostream& os) const {
    os << "Config(" << endl << "days " << endl;
    for (auto& day : days) os << day << "," << endl;
    os << "slots " << endl;
    for (auto& slot : slots) os << slot << "," << endl;
    os << "nb_weeks : " << nb_weeks << endl << "nb_days : " << nb_days << endl << "nb_base_slots : "
        << nb_slots_by_day << endl << "nb_pros : " << nb_pros << endl;
    os << ")" << endl;
    return os;
};

ostream& TimeSlot::print(ostream& os) const {
    os << "TimeSlot(idx : " << idx << ", name : " << name << ", time interval : (" << hours << ")";
    os << ", day : " << day << ", slot_of_day : " << slot_of_day << ")" << endl;
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