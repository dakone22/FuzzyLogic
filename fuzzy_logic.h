#ifndef FUZZYLOGIC_FUZZY_LOGIC_H
#define FUZZYLOGIC_FUZZY_LOGIC_H

#include <iostream>
#include <utility>
#include <functional>
#include <memory>
#include <cassert>
#include <map>
#include <stack>


struct Range {
    double l, r;
    bool operator==(const Range & another) const {
        return l == another.l and r == another.r;
    }
};

struct LinearPiece {
    enum Type {
        Const, Range
    } type;
    double x_start{ }, x_end{ };

    LinearPiece(Type type_, std::tuple<double, double> x_range) : type(type_) {
        std::tie(x_start, x_end) = x_range;
    }

    virtual std::shared_ptr<LinearPiece> getY(double x) = 0;
    virtual std::shared_ptr<LinearPiece> getX(double y) = 0;

    virtual ~LinearPiece() = 0;
};

struct LinearPieceRange : LinearPiece {
    double y_start{ }, y_end{ };

    LinearPieceRange(std::tuple<double, double> x_range, std::tuple<double, double> y_range) :
            LinearPiece(LinearPiece::Type::Range, x_range) {
        std::tie(y_start, y_end) = y_range;
    }
};


//struct LinearPieceConst : LinearPiece {
//    double y_value;
//
//    LinearPieceConst(std::tuple<double, double> x_range, double y_value_) :
//            LinearPiece(LinearPiece::Type::Range, x_range), y_value(y_value_) {
//    }
//
//    std::shared_ptr<LinearPiece> getY(double x) override {
//
//    }
//
//    std::shared_ptr<LinearPiece> getX(double y) override {
//        if (y_value == y) {  // todo: float precision
//            return std::make_shared<LinearPieceRange>({x_start, x_end}, {})
//        }
//    }
//};



class FunctionBuilder {
    double _start, _end;

public:
    FunctionBuilder(double start, double end) : _start(start), _end(end) { }

    const FunctionBuilder & set(const Range & x_range, double y_value) const {

        return *this;
    }

    const FunctionBuilder & set(const Range & x_range, const Range & y_value) const {

        return *this;
    }
};


class IRuleAggregation {
public:
    virtual double And(double a, double b) = 0;

    virtual double Or(double a, double b) = 0;

    virtual double Not(double a) { return 1 - a; };
};

class MaxMinRuleAggregation : public IRuleAggregation {
public:
    double And(double a, double b) override { return std::min(a, b); }
    double Or(double a, double b) override { return std::max(a, b); }
};

class ColorimetryRuleAggregation : public IRuleAggregation {
public:
    double And(double a, double b) override { return a + b - a * b; }
    double Or(double a, double b) override { return a * b; }
};

class IDefuzzifier {
    virtual double implication(double a, double b) = 0;
};

class ZadehDefuzzifier : public IDefuzzifier {
    double implication(double a, double b) override { return std::max(std::min(a, b), 1 - a); }
    /*
     * if (a <= 0.5) return 1 - a;
     * return [1-a, a];
     */
};
class LukaszewiczDefuzzifier : public IDefuzzifier {
    double implication(double a, double b) override { return std::min(1 - a + b, 1.); }
};
class GauguinDefuzzifier : public IDefuzzifier {
    double implication(double a, double b) override { return std::min(a / b, 1.); }
};

class LinguisticVariable;

class Term {
private:
    std::string _name;
    std::function<double(double)> _func;
public:
    Term(std::string name, std::function<double(double)> func) : _name(std::move(name)), _func(std::move(func)) { }

    std::string getName() const { return _name; }

    bool operator==(const Term & another) const {
        return this->_name == another._name;
    }

    double operator()(double value) const {
        return _func(value);
    }
};


struct Rule {
    enum Type {
        VarIsTerm, And, Or, Not, Implication
    } type;

    explicit Rule(Type type_) : type(type_) { }

    virtual ~Rule() = default;
};

struct VarIsTermRule : Rule {
    const LinguisticVariable & var;
    const Term & term;

    VarIsTermRule(const LinguisticVariable & var_, const Term & term_) : Rule(Rule::Type::VarIsTerm),
                                                                         var(var_),
                                                                         term(term_) { };
};

struct AndRule : Rule {
    std::shared_ptr<Rule> a, b;

    AndRule(std::shared_ptr<Rule> a_, std::shared_ptr<Rule> b_) : Rule(Rule::Type::And),
                                                                  a(std::move(a_)), b(std::move(b_)) { };
};

struct OrRule : Rule {
    std::shared_ptr<Rule> a, b;

    OrRule(std::shared_ptr<Rule> a_, std::shared_ptr<Rule> b_) : Rule(Rule::Type::Or),
                                                                 a(std::move(a_)), b(std::move(b_)) { };
};

struct NotRule : Rule {
    std::shared_ptr<Rule> a;

    explicit NotRule(std::shared_ptr<Rule> a_) : Rule(Rule::Type::Not), a(std::move(a_)) { };
};

struct ImplicationRule : Rule {
    std::shared_ptr<Rule> a, b;

    ImplicationRule(std::shared_ptr<Rule> a_, std::shared_ptr<Rule> b_) : Rule(Rule::Type::Implication),
                                                                          a(std::move(a_)), b(std::move(b_)) { };
};


class RuleComposer {
public:
    std::shared_ptr<Rule> root_rule;

    RuleComposer(const LinguisticVariable & var, const Term & term)
            : root_rule(std::make_shared<VarIsTermRule>(var, term)) {
    }

    explicit RuleComposer(std::shared_ptr<Rule> rule)
            : root_rule(std::move(rule)) {
    }

    RuleComposer operator||(const RuleComposer & another) const {
        return RuleComposer{ std::make_shared<OrRule>(this->root_rule, another.root_rule) };
    }

    RuleComposer operator&&(const RuleComposer & another) const {
        return RuleComposer{ std::make_shared<AndRule>(this->root_rule, another.root_rule) };
    }

    RuleComposer operator!() const {
        return RuleComposer{ std::make_shared<NotRule>(this->root_rule) };
    }

    RuleComposer operator>>=(const RuleComposer & another) const {
        return RuleComposer{ std::make_shared<ImplicationRule>(this->root_rule, another.root_rule) };
    }
};


class TermSet {
private:
    std::vector<Term> _terms;
public:
    TermSet(std::initializer_list<Term> terms) : _terms(terms) { }

    const std::vector<Term> & get() const { return _terms; }

    const Term & getByName(const std::string & name) {
        for (const auto & term : _terms) {
            if (term.getName() == name)
                return term;
        }
        throw std::runtime_error("No such term!");
    }
};

class LinguisticVariable {
protected:
    std::string _name;
    TermSet _terms;

public:
    LinguisticVariable(std::string name, TermSet terms) : _name(std::move(name)), _terms(std::move(terms)) { };

    RuleComposer operator==(const Term & term) {
        return { *this, term };
    }

    RuleComposer operator==(const std::string & term_string) {
        return { *this, _terms.getByName(term_string) };
    }

    std::string getName() const { return _name; }

    const TermSet & getTerms() const { return _terms; }

    bool operator==(const LinguisticVariable & another) const {
        return this->_name == another._name;
    }
};

constexpr double lined(double x_value, double x_from, double x_to, double y_from, double y_to) {
    if (x_value < x_from) { return y_from; }
    if (x_to < x_value) { return y_to; }
    auto k = (y_to - y_from) / (x_to - x_from);
    auto b = y_from - (k * x_from);
    return k * x_value + b;
}

// Check if vector contains an element
template <typename T>
bool contains(
        const std::vector<T>& vecObj,
        const T& element)
{
    // Get the iterator of first occurrence
    // of given element in vector
    auto it = std::find(
            vecObj.begin(),
            vecObj.end(),
            element) ;
    return it != vecObj.end();
}

bool ruleContains(const std::shared_ptr<Rule> & rule, const VarIsTermRule & varIsTermRule) {
    if (rule->type == Rule::Type::VarIsTerm) {
        auto r = std::dynamic_pointer_cast<VarIsTermRule>(rule);
        return r->term == varIsTermRule.term and r->var == varIsTermRule.var;
    }
    if (rule->type == Rule::Type::Implication) {
        throw std::runtime_error("Unexpected implication!");
    }
    if (rule->type == Rule::Type::And) {
        auto r = std::dynamic_pointer_cast<AndRule>(rule);
        return ruleContains(r->a, varIsTermRule) or ruleContains(r->b, varIsTermRule);
    }
    if (rule->type == Rule::Type::Or) {
        auto r = std::dynamic_pointer_cast<OrRule>(rule);
        return ruleContains(r->a, varIsTermRule) or ruleContains(r->b, varIsTermRule);
    }
    if (rule->type == Rule::Type::Not) {
        auto r = std::dynamic_pointer_cast<NotRule>(rule);
        return ruleContains(r->a, varIsTermRule);
    }
    throw std::runtime_error("Unexpected rule type!");
}

struct VarIsTerm {
    const LinguisticVariable & var;
    const Term & term;

    bool operator ==(const VarIsTerm & another) const {
        return this->var == another.var and this->term == another.term;
    }
};

template<>
struct std::hash<VarIsTerm> {
    std::size_t operator()(const VarIsTerm & k) const {
        using std::size_t;
        using std::hash;
        using std::string;

        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:

        return ((hash<string>()(k.var.getName()) ^ (hash<string>()(k.term.getName()) << 1)) >> 1);
    }
};

void print(const std::shared_ptr<Rule> & rule) {
    if (rule->type == Rule::Type::Implication) {
        auto r = std::dynamic_pointer_cast<ImplicationRule>(rule);
        std::cout << "Если [";
        print(r->a);
        std::cout << "], ТО ";
        print(r->b);
    } else if (rule->type == Rule::Type::And) {
        auto r = std::dynamic_pointer_cast<AndRule>(rule);
        std::cout << "(";
        print(r->a);
        std::cout << " И ";
        print(r->b);
        std::cout << ")";
    } else if (rule->type == Rule::Type::Or) {
        auto r = std::dynamic_pointer_cast<OrRule>(rule);
        std::cout << "(";
        print(r->a);
        std::cout << " ИЛИ ";
        print(r->b);
        std::cout << ")";
    } else if (rule->type == Rule::Type::Not) {
        auto r = std::dynamic_pointer_cast<NotRule>(rule);
        std::cout << "( НЕ ";
        print(r->a);
        std::cout << ")";
    } else if (rule->type == Rule::Type::VarIsTerm) {
        auto r = std::dynamic_pointer_cast<VarIsTermRule>(rule);
        std::cout << "(";
        std::cout << r->var.getName();
        std::cout << " = ";
        std::cout << r->term.getName();
        std::cout << ")";
    }

}

class FuzzyLogicEngine {
private:
    std::vector<LinguisticVariable> inputVariables;

    struct Output {
        const LinguisticVariable & var;
        std::shared_ptr<IRuleAggregation> ruleAggregation;
        std::shared_ptr<IDefuzzifier> defuzzifier;
    };

    std::vector<Output> outputs;

    std::vector<std::shared_ptr<Rule>> rules;

public:
    void addInputVariable(const LinguisticVariable & var) {
        inputVariables.push_back(var);
    }

    void addOutputVariable(const LinguisticVariable & var, std::shared_ptr<IRuleAggregation> ruleAggregation,
                           std::shared_ptr<IDefuzzifier> defuzzifier) {
        outputs.push_back({ var, std::move(ruleAggregation), std::move(defuzzifier) });
    }

    void addRule(const RuleComposer & ruleComposer) {
        auto rule = ruleComposer.root_rule;
        assert(rule->type == Rule::Implication);
        rules.push_back(rule);
    }

    bool checkBase() {
        for (const auto & output : outputs) {
            if (not _checkBaseVar(output.var, false)) return false;
        }

        for (const auto & inputVariable : inputVariables) {
            if (not _checkBaseVar(inputVariable, true)) return false;
        }

        return true;
    }

    void process(const std::vector<std::tuple<const LinguisticVariable &, double>> & data) {
        _checkInputData(data);

        // fuzzification
        std::unordered_map<VarIsTerm, double> fuzzification_results{ };

        for (auto [variable, value] : data) {
            for (const auto & term : variable.getTerms().get()) {
                auto membershipDegree = term(value);
                fuzzification_results.insert({{ variable, term }, membershipDegree });

                std::cout << variable.getName() << "(" << value << ") = \"\\text{" << term.getName() << "}\", \\mu_{\\widetilde{"
                          << term.getName() << "}} (" << value << ") = " << membershipDegree << std::endl;
            }
        }

        // aggregation

        for (auto [outputVariable, ruleAggregation, defuzzifier] : outputs) {

            std::unordered_map<std::shared_ptr<Rule>, double> aggregation_results{ };
            for (const auto & rule : rules) {
                auto r = std::dynamic_pointer_cast<ImplicationRule>(rule);
                double uncertaintyDegree = _applyAggregationRule(r->a, ruleAggregation, fuzzification_results);

//                std::cout << "\n";
                print(rule);
                std::cout << "  :  " << uncertaintyDegree << std::endl;

                aggregation_results.insert({ rule, uncertaintyDegree });
            }

            //


        }
    }

private:

    static double _applyAggregationRule(const std::shared_ptr<Rule>& rule,
                                        const std::shared_ptr<IRuleAggregation>& ruleAggregation,
                                        const std::unordered_map<VarIsTerm, double>& fuzzification_results) {

        if (rule->type == Rule::Type::Implication) {
            throw std::runtime_error("Unexpected implication rule!");
        }
        if (rule->type == Rule::Type::VarIsTerm) {
            auto r = std::dynamic_pointer_cast<VarIsTermRule>(rule);
            auto x = fuzzification_results.at({ r->var, r->term});
//            std::cout << "(" << r->var.getName() << " == " << r->term.getName() << " : " << x << " )";
            std::cout << x;
            return x;
        }
        if (rule->type == Rule::Type::And) {
            auto r = std::dynamic_pointer_cast<AndRule>(rule);

            std::cout << "min(";
            auto a = _applyAggregationRule(r->a, ruleAggregation, fuzzification_results);
            std::cout << ", ";
            auto b = _applyAggregationRule(r->b, ruleAggregation, fuzzification_results);
            std::cout << ")";

            auto res = ruleAggregation->And(a, b);
//            std::cout << ":" << res;

            return res;
        }
        if (rule->type == Rule::Type::Or) {
            auto r = std::dynamic_pointer_cast<OrRule>(rule);

            std::cout << "max(";
            auto a = _applyAggregationRule(r->a, ruleAggregation, fuzzification_results);
            std::cout << ", ";
            auto b = _applyAggregationRule(r->b, ruleAggregation, fuzzification_results);
            std::cout << ")";

            auto res = ruleAggregation->Or(a, b);
//            std::cout << ":" << res;

            return res;
        }
        if (rule->type == Rule::Type::Not) {
            auto r = std::dynamic_pointer_cast<OrRule>(rule);

//            std::cout << "[ not ";
            auto a = _applyAggregationRule(r->a, ruleAggregation, fuzzification_results);
//            std::cout << "]";

            auto res = ruleAggregation->Not(a);
//            std::cout << ":" << res;

            return res;
        }

        throw std::runtime_error("Unexpected rule!");
    }

    void _assertInputData(const std::vector<std::tuple<const LinguisticVariable &, double>> & data) {
        for (auto [variable, value] : data) {
            assert(contains(inputVariables, variable));
        }

        for (const auto & variable  : inputVariables) {
            bool present = false;
            for (auto [inputed_variable, value] : data) {
                if (variable == inputed_variable) {
                    present = true;
                    break;
                }
            }
            assert(present);
        }
    }

    bool _checkBaseVar(const LinguisticVariable & linguisticVariable, bool check_in_left) {
        for (const auto & term : linguisticVariable.getTerms().get()) {
            bool isPresentInAnyRule = false;
            for (const auto & rule : rules) {
                assert(rule->type == Rule::Implication);
                auto implicationRule = std::dynamic_pointer_cast<ImplicationRule>(rule);

                auto where_to_check = check_in_left ? implicationRule->a : implicationRule->b;

                if (ruleContains(where_to_check, VarIsTermRule(linguisticVariable, term))) {
                    isPresentInAnyRule = true;
                    break;
                }
            }

            if (not isPresentInAnyRule) {
                std::cout << term.getName() << " is not present in any rule" << std::endl;
                return false;
            }
        }
        return true;
    }

};

#endif //FUZZYLOGIC_FUZZY_LOGIC_H
