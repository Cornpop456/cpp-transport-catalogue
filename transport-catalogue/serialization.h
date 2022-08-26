#pragma once

#include <filesystem>



namespace serialize {


struct Settings {
    std::filesystem::path file;
};


class Serializator final {
public:
    Serializator(const Settings& settings) : settings_(settings) {};


private:
    Settings settings_;
};

} // serialize