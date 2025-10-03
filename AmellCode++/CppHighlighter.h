#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class CppHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CppHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QVector<Rule> m_rules;
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;
    QTextCharFormat m_commentFormat;
};
