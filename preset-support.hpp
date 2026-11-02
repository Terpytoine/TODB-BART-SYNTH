#pragma once
// Factory patches are decoded once at creation, never by an audio/MIDI callback.
struct FactoryProgram {
    std::string name, category, metadata;
    std::array<double, PARAM_COUNT> values{};
};
inline std::vector<FactoryProgram> readPrograms(const std::filesystem::path& path) {
    std::ifstream file(path); auto bank = nlohmann::json::parse(file);
    if (!bank.is_array() || bank.empty() || bank.size() > 10000) throw std::runtime_error("Invalid factory program list");
    std::vector<FactoryProgram> programs; programs.reserve(bank.size());
    for (const auto& patch : bank) {
        FactoryProgram program; program.name = patch.at("name").get<std::string>(); program.category = patch.value("category", "Synth");
        program.metadata = nlohmann::json({{"name", program.name}}).dump();
        const auto& values = patch.at("params");
        for (int i = 0; i < PARAM_COUNT; ++i) {
            double value = specs[i].def; auto key = specs[i].key;
            if (values.contains(key)) {
                const auto& v = values[key];
                if (v.is_string()) {
                    std::string choices = specs[i].choices, want = v.get<std::string>(); size_t position = 0; int index = 0; bool found = false;
                    while (position <= choices.size()) {
                        auto end = choices.find('|', position);
                        if (choices.substr(position, end - position) == want) { value = index; found = true; break; }
                        if (end == std::string::npos) break; position = end + 1; ++index;
                    }
                    if (!found) throw std::runtime_error("Factory choice is invalid");
                } else value = v.get<double>();
            }
            if (!std::isfinite(value)) throw std::runtime_error("Factory value is invalid");
            program.values[i] = normalized(i, value);
        }
        programs.push_back(std::move(program));
    }
    return programs;
}
