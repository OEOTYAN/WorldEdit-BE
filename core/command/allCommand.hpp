//
// Created by OEOTYAN on 2022/05/16.
//
#include <DynamicCommandAPI.h>
#include "llapi/utils/StringHelper.h"

namespace worldedit {
    // Direct setup of dynamic command with necessary information
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    inline void setArg(std::string_view args) {
        std::vector<std::string> arg;
        arg.clear();
        for (auto& c : args.substr(1)) {
            arg.push_back(std::string("-") + c);
        }
        Global<CommandRegistry>->setSoftEnumValues(asString(args), arg);
    }

    void commandsSetup();
    void brushCommandSetup();
    void regionCommandSetup();
    void historyCommandSetup();
    void handToolCommandSetup();
    void clipboardCommandSetup();
    void regionInfoCommandSetup();
    void generationCommandSetup();
    void regionOperationCommandSetup();
}  // namespace worldedit