#include "fuzzy_logic.h"

int main() {
    /*
     * LinguisticVariable x = new NumericLinguisticVariable(
     *    name: "Температура в комнате",
     *    U: [1, 5],
     *    Terms: {"Высокая"=f(x) => x > 4, "Средняя", "Низкая"},
     *    )
     */
//    FuzzyLogicEngine engine;
//
//
//    Term educationHigh = { "высокая", [](double x) -> double { return lined(x, 4.5, 5, 0, 1); }};
//    Term educationMedium = { "средняя", [](double x) -> double {
//        return (x <= 4) ? lined(x, 3.5, 4, 0, 1) : lined(x, 4, 4.5, 1, 0);
//    }};
//    Term educationLow = { "низкая", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//    auto education = LinguisticVariable("успеваемость студента", {
//            educationHigh,
//            educationMedium,
//            educationLow,
//    });
//
////    FunctionBuilder(0, 20)
////    .set({0, 1}, 0)
////    .set({1, 2}, {0, 1})
////    .set({2, 3}, 1);
//
//    Term workLow = { "мало", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//    Term workMedium = { "достаточно", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//    Term workHigh = { "много", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//
//    auto work = LinguisticVariable("количество решаемых задач", {
//            workLow,
//            workMedium,
//            workHigh,
//    });
//
//    Term timeLow = { "мало", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//    Term timeMedium = { "продолжительно", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//    Term timeHigh = { "много", [](double x) -> double {
//        return (x <= 2) ? lined(x, 1.5, 2, 0, 1) : lined(x, 3, 3.5, 1, 0);
//    }};
//
//    auto time = LinguisticVariable("кол-во времени", {
//            timeLow,
//            timeMedium,
//            timeHigh,
//    });
//
//    engine.addInputVariable(education);
//    engine.addInputVariable(work);
//
//    engine.addOutputVariable(time, std::make_shared<MaxMinRuleAggregation>(), std::make_shared<ZadehDefuzzifier>());
//
//    engine.addRule((education == educationHigh or education == educationMedium) and work == workLow >>= time == timeLow);
//    engine.addRule((education == educationHigh or education == educationMedium) and work == workHigh >>= time == timeMedium);
//    engine.addRule(education == educationLow and work == workHigh >>= time == timeHigh);
//    engine.addRule(education == educationMedium and work == workMedium >>= time == timeMedium);
//
//    std::cout << "Полнота базы: " << (engine.checkBase() ? "YES" : "NO") << std::endl;
//
//    engine.process({
//                           { education, 3.2 },
//                           { work,      9 },
//                   });

    auto power = LinguisticVariable("X", {
            { "малая",            [](double x) -> double { return lined(x, 1.8, 3.8, 1, 0); }},
            { "не очень высокая", [](double x) -> double { return (x <= 4.8) ? lined(x, 2.8, 4.8, 0, 1) : lined(x, 5.8, 7.8, 1, 0); }},
            { "большая",          [](double x) -> double { return lined(x, 6.8, 8.8, 0, 1); }},
    });

    auto temperature = LinguisticVariable("Y", {
            { "холодно",       [](double x) -> double { return lined(x, 15, 19, 1, 0); }},
            { "тепло",         [](double x) -> double { return (x <= 19) ? lined(x, 15, 19, 0, 1) : lined(x, 19, 23, 1, 0); }},
            { "слишком тепло", [](double x) -> double { return (x <= 25) ? lined(x, 21, 25, 0, 1) : lined(x, 25, 29, 1, 0); }},
            { "жарко",         [](double x) -> double { return lined(x, 6.8, 8.8, 0, 1); }},
    });

    auto room = LinguisticVariable("S", {
            { "комната", [](double x) -> double { return lined(x, 20, 28, 1, 0); }},
            { "студия",  [](double x) -> double { return (x <= 24) ? lined(x, 16, 24, 0, 1) : lined(x, 24, 32, 1, 0); }},
            { "зал",     [](double x) -> double { return (x <= 36) ? lined(x, 28, 36, 0, 1) : lined(x, 48, 51, 1, 0.75); }},
    });

    auto mode = LinguisticVariable("Z", {
            { "низкая",  [](double x) -> double { return lined(x, 14, 16, 1, 0); }},
            { "средняя", [](double x) -> double { return (x <= 17) ? lined(x, 13, 17, 0, 1) : lined(x, 18, 25, 1, 0); }},
            { "высокая", [](double x) -> double { return lined(x, 17, 21, 0, 1); }},
    });

    FuzzyLogicEngine engine;
    for (const auto& var : { power, temperature, room }) {
        engine.addInputVariable(var);
    }
    engine.addOutputVariable(mode,
                             std::make_shared<MaxMinRuleAggregation>(),
                             std::make_shared<ZadehDefuzzifier>());

    for (const auto & rule : {
            ((power == "малая") or (power == "не очень высокая")) and (temperature == "тепло") and (room == "комната") >>= (mode == "средняя"),
            (power == "большая") and ((temperature == "тепло") or (temperature == "холодно")) and (room == "зал") >>= (mode == "высокая"),
            ((power == "малая") or (power == "не очень высокая")) and (temperature == "жарко") and ((room == "комната") or (room == "студия")) >>= (mode == "низкая"),
            (power == "не очень высокая") and (temperature == "жарко") >>= (mode == "низкая"),
            (power == "большая") and ((temperature == "жарко") or (temperature == "слишком тепло")) >>= (mode == "низкая"),
    }) {
        engine.addRule(rule);
    }

    std::cout << "Полнота базы: " << (engine.checkBase() ? "YES" : "NO") << std::endl;

    int day = 27, month = 9;

    std::cout << (day * month) % 10 + 0.8 << " "
              << day + 9 << " "
              << day + month + 8 << std::endl,

    engine.process({
                           { power, (day * month) % 10 + 0.8 },
                           { temperature, day + 9 },
                           { room, day + month + 8 },
                   });

    return 0;
}
