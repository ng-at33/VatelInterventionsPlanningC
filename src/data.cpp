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
TimeSlot::TimeSlot(int idx, std::string name, std::string hours,
    int day, int slot_of_day) :
    idx(idx), name(name), hours(hours), day(day), slotOfDay(slotOfDay)
    {}
    
TimeSlot::TimeSlot(){}

// IO functions
Data* readXLS(string& filename) {
    Data* data = new Data();
    cout << filename << endl;
    Book* book = xlCreateXMLBook();
    // Book* book;
    cout << filename << endl;
    if (book->load(filename.c_str())) {
        Sheet* sheet = book->getSheet(1);
        cout << " first Col " << sheet->firstCol() << endl;
        cout << " last Col " << sheet->lastCol() << endl;
        if (sheet) {
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
                const char* hour = sheet->readStr(row_hour, colIdx);
                string cur_hour;
                // cout << " m " << row_month << " d " << row_day << " h " << row_hour << " c " << colIdx << endl;
                if (!hour) {
                    iterateSlots = false;
                } else {
                    cur_hour = string(hour);
                    if (auto month = sheet->readStr(row_month, colIdx)) { cur_month = month; }
                    if (auto day = sheet->readStr(row_day, colIdx)) {
                        cur_day = day;
                        cur_d++;
                        slotOfDay = 0;
                    }
                    cout << " cur_month " << string(cur_month) ;
                    cout << " cur_day " << string(cur_day);
                    cout << " hour " << cur_hour;

                    cout << endl;
                    auto slotName = cur_month + "/" + cur_day + "h" + hour;
                    // vector<string> strs;
                    // boost::split(strs, cur_hour, boost::is_any_of(" - "));
                    // cout << "* size of the vector: " << strs.size() << endl;
                    // for (vector<string>::iterator it = strs.begin(); it != strs.end(); ++it)
                    // {
                    //     cout << *it << endl;
                    // }
                    // auto startHour;
                    // auto endHour;
                    // Creating slot
                    TimeSlot* slot = new TimeSlot(colIter, slotName, cur_hour, cur_d, slotOfDay);
                }
                colIter++;
                slotOfDay++;
            }
            // for (int row = sheet->firstRow(); row < sheet->lastRow(); ++row)
            // {
            //     for (int col = sheet->firstCol(); col < sheet->lastCol(); ++col)
            //     {
            //         CellType cellType = sheet->cellType(row, col);
            //         std::cout << "(" << row << ", " << col << ") = ";
            //         if (sheet->isFormula(row, col))
            //         {
            //             const char* s = sheet->readFormula(row, col);
            //             std::cout << (s ? s : "null") << " [formula]";
            //         }
            //         else
            //         {
            //             switch (cellType)
            //             {
            //                 case CELLTYPE_EMPTY: std::cout << "[empty]"; break;
            //                 case CELLTYPE_NUMBER:
            //                 {
            //                     double d = sheet->readNum(row, col);
            //                     std::cout << d << " [number]";
            //                     break;
            //                 }
            //                 case CELLTYPE_STRING:
            //                 {
            //                     const char* s = sheet->readStr(row, col);
            //                     std::cout << (s ? s : "null") << " [string]";
            //                     break;        
            //                 }
            //                 case CELLTYPE_BOOLEAN:
            //                 {
            //                     bool b = sheet->readBool(row, col);
            //                     std::cout << (b ? "true" : "false") << " [boolean]";
            //                     break;
            //                 }
            //                 case CELLTYPE_BLANK: std::cout << "[blank]"; break;
            //                 case CELLTYPE_ERROR: std::cout << "[error]"; break;
            //             }
            //         }
            //         std::cout << std::endl;
            //     }
            // }
        }
    }
    book->release();

    return data;
};

Data* tempInit() {
    // Init DisplayConfig
    auto num_weeks = 2;
    auto n_max_pros_interv = 3;
    auto n_min_class_interv = 1;
    vector<string> days{ "2020-11-09", "2020-11-11", "2020-11-12", "2020-11-13", "2020-11-14", "2020-11-15", "2020-11-16", "2020-11-17", "2020-11-18", "2020-11-19" };
    vector<string> base_slots_start{ "9h", "10h30", "14h", "15h30" };
    vector<string> base_slots_end{ "10h30", "12h", "15h30", "17h" };

    array pros_str{ "Aude Becquart", "Thomas Galharague", "Olivier Medel", "Jeanne Gourmaud", "Julie Kassabji", "Anna Godillon", "Camille Auger", "Emeric Autier", "Emilie Catesson", "Romain Tarate", "Marine Alengry", "Mathieu Jude", "Boris Mancambou", "Ghislain Boucard", "Géraldine Mabit" };

    array groups{ "M1", "M2/Prépa", "M3" };

    Config* config = new Config();

    vector<string> slots_str;
    vector<TimeSlot *> slots;
    auto cnt_slots = 0;
    auto cnt_days = 0;
    for (auto day : days) {
        for (auto slot_idx = 0; slot_idx < base_slots_start.size(); ++slot_idx) {
            slots_str.push_back(day + " " + base_slots_start[slot_idx] + "-" + base_slots_end[slot_idx]);
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
    config->days = days;
    config->slots = slots_str;
    config->nbWeeks = num_weeks;
    config->nbDays = days.size();
    config->nbBaseSlots = slots_str.size();
    config->nbSlots = slots.size();
    config->nbPros = pros_str.size();
    config->maxInter = n_max_pros_interv;
    
    Dimension* dimensions = new Dimension(pros_str.size(), groups.size(), 1, slots.size());

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
            if (dispo[cnt_pro][i]) {
                new_pro->slots.push_back(slots[i]);
            }
        }
        pros.push_back(new_pro);
        cnt_pro++;
    }

    for (auto& slot : slots)
        cout << slot->name << endl;

    Data* data = new Data();
    data->dimensions = *dimensions;
    data->config = *config;
    data->slots = slots;
    data->professionals = pros;
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
    os << "nb_base_slots : " << nbBaseSlots << endl;
    os << "nb_slots : " << nbSlots << endl;
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

ostream& Professional::print(ostream& os) const {
    os << "Professional(idx : " << idx; 
    os << ", name : " << name;
    os << ", slots " << endl;
    for (auto& slot : slots) slot->print(os);
    os << ")" << endl;
    return os;
};