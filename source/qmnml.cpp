#include "qmnml.h"
#include <QBuffer>
#include <QTextStream>

namespace qmnml {

Value::Value(const QString& key)
    : key_(key)
{
}

bool Value::contains(const QString& key) const
{
    return children_.contains(key);
}

Value& Value::operator=(const Variant& data)
{
    data_ = data;
    return *this;
}

Value& Value::operator=(const VariantEx& data)
{
    data_ = data.val;
    comment_ = data.comment;
    return *this;
}

Value& Value::operator[](const QString& key)
{
    auto& child = children_[key];
    child.key_ = key;
    return child;
}

Value& Value::at(const QString& key)
{
    return children_.at(key);
}

const Value& Value::at(const QString& key) const
{
    return children_.at(key);
}

QString Value::dump() const
{

    QString buffer;
    QTextStream stream(&buffer);

    if (children_.empty()) {
        stream << *this;
        return buffer;
    }

    stream << "&" << key_ << "\r\n";
    for (const auto [_, child] : children_) {
        stream << child << "\r\n";
    }
    stream << "/\r\n";
    return buffer;
}

QTextStream& operator<<(QTextStream& stream, const Value& nml)
{
    stream << nml.key_ << " = ";
    std::visit(Visitor { [&stream](bool val) { stream << QString(".%1.").arg(val ? "true" : "false"); }, [&stream](int val) { stream << val; },
                   [&stream](double val) { stream << val; }, [&stream](QString val) { stream << QString("\"%1\"").arg(val); },
                   [&stream](const QList<bool>& val) {
                       for (auto it = val.cbegin(); it != val.cend(); ++it) {
                           if (it != val.cbegin()) {
                               stream << ", ";
                           }
                           stream << QString(".%1.").arg(*it ? "true" : "false");
                       }
                   },
                   [&stream](const QList<int>& val) {
                       for (auto it = val.cbegin(); it != val.cend(); ++it) {
                           if (it != val.cbegin()) {
                               stream << ", ";
                           }
                           stream << *it;
                       }
                   },
                   [&stream](const QList<double>& val) {
                       for (auto it = val.cbegin(); it != val.cend(); ++it) {
                           if (it != val.cbegin()) {
                               stream << ", ";
                           }
                           stream << *it;
                       }
                   },
                   [&stream](const QList<QString>& val) {
                       for (auto it = val.cbegin(); it != val.cend(); ++it) {
                           if (it != val.cbegin()) {
                               stream << ", ";
                           }
                           stream << QString("\"%1\"").arg(*it);
                       }
                   },
                   [](auto) {} },
        nml.data_);
    if (!nml.comment_.isEmpty()) {
        stream << " ! " << nml.comment_;
    }
    return stream;
}

Value::Type Value::type() const
{
    // clang-format off
    return std::visit(Visitor { 
        [](bool) { return Type::Boolean; },
        [](int) { return Type::Integer; },
        [](double) { return Type::Double; },
        [](QString) { return Type::String; },
        [](const QList<bool>&) { return Type::BooleanList; },
        [](const QList<int>&) { return Type::IntegerList; },
        [](const QList<double>&) { return Type::DoubleList; },
        [](const QList<QString>&) { return Type::StringList; },
        [](auto) { return Type::None; } }, data_);
    // clang-format on
}

Value parse(const QString& text)
{
    Value res;
    Value* current = nullptr;
    const auto lines = text.split('\n');

    enum class Type {
        Bool,
        Int,
        Double,
        String
    };

    auto detectType = [](const QString& token) {
        const auto trimmed = token.trimmed();
        const auto lower = trimmed.toLower();
        if (trimmed.size() >= 2 && trimmed.front() == '"' && trimmed.back() == '"') {
            return Type::String;
        }
        if (lower == ".true." || lower == ".false.") {
            return Type::Bool;
        }
        if (trimmed.contains('.') || trimmed.contains('e') || trimmed.contains('E')) {
            return Type::Double;
        }
        return Type::Int;
    };

    auto stripQuotes = [](const QString& token) {
        if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
            return token.mid(1, token.size() - 2);
        }
        return token;
    };

    auto assignValue = [&](Value& target, const QString& key, const QString& valueText) {
        auto items = valueText.split(',', Qt::KeepEmptyParts);
        for (auto& item : items) {
            item = item.trimmed();
        }
        if (items.isEmpty()) {
            return;
        }

        const auto type = detectType(items.front());

        switch (type) {
        case Type::Bool: {
            if (items.size() == 1) {
                target[key] = items.front().toLower() == ".true.";
            } else {
                QList<bool> list;
                list.reserve(items.size());
                for (const auto& item : items) {
                    list.append(item.toLower() == ".true.");
                }
                target[key] = list;
            }
            break;
        }
        case Type::Int: {
            if (items.size() == 1) {
                target[key] = items.front().toInt();
            } else {
                QList<int> list;
                list.reserve(items.size());
                for (const auto& item : items) {
                    list.append(item.toInt());
                }
                target[key] = list;
            }
            break;
        }
        case Type::Double: {
            if (items.size() == 1) {
                target[key] = items.front().toDouble();
            } else {
                QList<double> list;
                list.reserve(items.size());
                for (const auto& item : items) {
                    list.append(item.toDouble());
                }
                target[key] = list;
            }
            break;
        }
        case Type::String: {
            if (items.size() == 1) {
                target[key] = stripQuotes(items.front());
            } else {
                QList<QString> list;
                list.reserve(items.size());
                for (const auto& item : items) {
                    list.append(stripQuotes(item));
                }
                target[key] = list;
            }
            break;
        }
        }
    };

    auto stripComment = [](const QString& src) {
        QString out;
        out.reserve(src.size());
        bool inString = false;
        for (const QChar ch : src) {
            if (ch == '"') {
                inString = !inString;
                out.append(ch);
                continue;
            }
            if (!inString && ch == '!') {
                break;
            }
            out.append(ch);
        }
        return out.trimmed();
    };

    for (QString line : lines) {
        line = line.trimmed();
        if (line.startsWith('!')) {
            continue;
        }
        line = stripComment(line);
        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith('&')) {
            auto& standalone = res[line.mid(1).trimmed()];
            current = &standalone;
            continue;
        }

        if (line.startsWith('/')) {
            current = nullptr;
            continue;
        }

        const auto eqPos = line.indexOf('=');
        if (eqPos == -1) {
            continue;
        }

        const auto key = line.left(eqPos).trimmed();
        const auto valueText = stripComment(line.mid(eqPos + 1));

        if (key.isEmpty() || valueText.isEmpty()) {
            continue;
        }

        if (current) {
            assignValue(*current, key, valueText);
        } else {
            assignValue(res[key], key, valueText);
        }
    }
    return res;
}

} // namespace qmnml
