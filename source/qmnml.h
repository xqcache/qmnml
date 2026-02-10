#pragma once

#include <QList>
#include <QString>
#include <map>
#include <optional>
#include <stdexcept>

class QTextStream;

namespace qmnml {

template <typename... Ts> struct Visitor : Ts... {
    using Ts::operator()...;
};

template <typename... Ts> Visitor(Ts...) -> Visitor<Ts...>;

using Variant = std::variant<std::monostate, bool, int, double, QString, QList<bool>, QList<int>, QList<double>, QList<QString>>;

struct VariantEx {
    Variant val;
    QString comment;
};

template <typename T, typename V> struct is_support_t : std::false_type { };

template <typename T, typename... Ts> struct is_support_t<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };

template <typename T, typename V> inline constexpr bool is_support_v = is_support_t<T, V>::value;

template <typename T>
concept is_support = is_support_v<T, Variant>;

class Value {
public:
    enum class Type : uint8_t {
        None = 0,
        Boolean,
        Integer,
        Double,
        String,
        BooleanList,
        IntegerList,
        DoubleList,
        StringList,
    };

    Value();
    explicit Value(const QString& key);
    Value(Value&& other);
    Value& operator=(Value&& other);
    Value(const Value& other) = default;
    Value& operator=(const Value& other) = default;

    bool contains(const QString& key) const;

    template <typename T>
        requires is_support<T>
    T as_(const std::optional<T>& def = std::nullopt) const
    {
        if (std::holds_alternative<T>(data_)) {
            return std::get<T>(data_);
        } else if (def.has_value()) {
            return *def;
        }
        throw std::runtime_error("Invalid type");
    };

    Value& operator=(const Variant& data);
    Value& operator=(const VariantEx& data);

    template <typename T>
        requires is_support<T>
    Value& operator=(const std::initializer_list<T>& data)
    {
        data_ = QList<T>(data);
        return *this;
    }

    Value& operator[](const QString& key);
    Value& at(const QString& key);
    const Value& at(const QString& key) const;

    inline QString key() const;

    QString dump() const;

    Type type() const;
    bool isNull() const;

private:
    friend QTextStream& operator<<(QTextStream& stream, const Value& nml);

private:
    QString key_;
    QString comment_;
    Variant data_;
    std::map<QString, Value> children_;
};

inline QString Value::key() const
{
    return key_;
}

inline bool Value::isNull() const
{
    return type() == Type::None;
}

Value parse(const QString& text);

QTextStream& operator<<(QTextStream& stream, const Value& nml);

} // namespace qmnml
