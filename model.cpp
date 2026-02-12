#include "model.h"

#include <windows.h>
#include <fstream>
#include <filesystem>

void IncrementFile(QString& file) {
    auto pos = file.lastIndexOf(").");
    if (pos == -1) {
        auto insert_pos = file.lastIndexOf('.');
        file = file.insert(insert_pos, "(1)");
    }
    else {
        auto pre_pos = file.lastIndexOf('(');
        if (pre_pos != 0) {
            bool ok;
            auto sliced = file.sliced(pre_pos + 1, pos - pre_pos - 1);
            uint sliced_val = sliced.toUInt(&ok);
            if (ok) {
                file = file.replace(pre_pos + 1, pos - pre_pos - 1, QString(std::to_string(++sliced_val).data()));
            }
            else {
                auto insert_pos = file.lastIndexOf('.');
                file = file.insert(insert_pos, "(1)");
            }
        }
    }
}

QString model::Worker::Modify(ModifyInput&& data) {
    uint files_count = data.i_files.size();
    uint current_file = 0;
    uint progress = 0;
    for (auto& file : data.i_files) {
        auto file_name = std::filesystem::current_path().string() + '\\' + file.toStdString();
        auto in = std::ifstream(file_name, std::ios::binary);
        if (!in) {
            return "Error on opening " + file;
        }
        QString new_file;
        if (data.o_path.isEmpty()) {
            new_file.append(std::filesystem::current_path().string() + '\\' + file.toStdString());
        }
        else {
            new_file.append(data.o_path + '\\' + file);
        }
        if (std::filesystem::exists(new_file.toStdString()) && !data.override_output) {
            while(std::filesystem::exists(new_file.toStdString())) {
                IncrementFile(new_file);
            }
        }
        auto out = std::ofstream(new_file.toStdString(), std::ios::binary);
        if (!out) {
            return "Error on opening or creating for output " + file;
        }
        char b;
        auto value_iter = data.value.begin();
        unsigned long long counter = 0;
        uint size = std::filesystem::file_size(file_name);
        while (in.get(b)) {
            if (value_iter == data.value.end()) {
                value_iter = data.value.begin();
            }
            out << char(b ^ *value_iter);
            if (counter != 0 && counter % (size >> 2) == 0) {
                progress = (current_file * 100 + (counter * 100 / size)) / files_count;
                emit statusUpdated(progress);
            }
            ++value_iter;
            ++counter;
        }
        in.close();
        out.close();
        ++current_file;
        if (data.delete_input) {
            std::remove(std::filesystem::current_path().append(file.toStdString()).string().data());
        }
    }
    return "";
}
