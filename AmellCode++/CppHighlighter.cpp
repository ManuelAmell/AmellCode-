#include "CppHighlighter.h"
#include <QTextCharFormat>
#include <QFont>
#include <QColor>
#include <QRegularExpression>

// Paleta aproximada One Dark
static const QColor kColorText       = QColor("#ABB2BF");
static const QColor kColorKeyword    = QColor("#C678DD");
static const QColor kColorType       = QColor("#56B6C2");
static const QColor kColorFunction   = QColor("#E5C07B");
static const QColor kColorString     = QColor("#98C379");
static const QColor kColorNumber     = QColor("#D19A66");
static const QColor kColorComment    = QColor("#5C6370");
static const QColor kColorError      = QColor("#E06C75");
static const QColor kColorOperator   = QColor("#61AFEF");
static const QColor kColorClassName  = QColor("#E06C75"); // nombres de clase/struct

// Clase encargada de aplicar resaltado de sintaxis en un QTextDocument
CppHighlighter::CppHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    m_rules.clear();

    // ---------- Keywords ----------
    const QStringList keywords = {
        "alignas","alignof","and","and_eq","asm","atomic_cancel","atomic_commit","atomic_noexcept",
        "auto","bool","break","case","catch","char","char16_t","char32_t","class","compl","const",
        "constexpr","const_cast","continue","decltype","default","delete","do","double","dynamic_cast",
        "else","enum","explicit","export","extern","false","float","for","friend","goto","if","inline",
        "int","long","mutable","namespace","new","noexcept","not","not_eq","nullptr","operator",
        "or","or_eq","private","protected","public","register","reinterpret_cast","return","short",
        "signed","sizeof","static","static_assert","static_cast","struct","switch","template","this",
        "thread_local","throw","true","try","typedef","typeid","typename","union","unsigned","using",
        "virtual","void","volatile","wchar_t","while"
    };

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(kColorKeyword);
    keywordFormat.setFontWeight(QFont::Bold);
    for (const QString &kw : keywords) {
        Rule r;
        r.pattern = QRegularExpression(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(kw)));
        r.format = keywordFormat;
        m_rules.push_back(r);
    }

    // ---------- Types / builtins ----------
    QTextCharFormat typeFormat;
    typeFormat.setForeground(kColorType);
    typeFormat.setFontWeight(QFont::Bold);
    const QStringList types = {"size_t","uint32_t","uint64_t","int32_t","int64_t","std","string","QString","QWidget","QMainWindow"};
    for (const QString &t : types) {
        Rule r;
        r.pattern = QRegularExpression(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(t)));
        r.format = typeFormat;
        m_rules.push_back(r);
    }

    // ---------- Class / Struct names (identifier starting with uppercase) ----------
    QTextCharFormat classFormat;
    classFormat.setForeground(kColorClassName);
    classFormat.setFontWeight(QFont::Bold);
    m_rules.push_back({ QRegularExpression(QStringLiteral("\\b[A-Z][A-Za-z0-9_]*\\b")), classFormat });

    // ---------- Funciones (identificador seguido de '(' ) ----------
    QTextCharFormat functionFormat;
    functionFormat.setForeground(kColorFunction);
    functionFormat.setFontWeight(QFont::Normal);
    // lookahead para "("
    m_rules.push_back({ QRegularExpression(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()")), functionFormat });

    // ---------- Strings (soportando escapes) ----------
    QTextCharFormat stringFormat;
    stringFormat.setForeground(kColorString);
    stringFormat.setFontItalic(false);
    // Cadenas con escapes
    m_rules.push_back({ QRegularExpression(QStringLiteral("\"(?:\\\\.|[^\\\\\"])*\"")), stringFormat });
    // Caracteres 'a' o '\n'
    m_rules.push_back({ QRegularExpression(QStringLiteral("'(?:\\\\.|[^\\\\'])'")), stringFormat });
    // Raw string literals básicos (bastante permissive)
    m_rules.push_back({ QRegularExpression(QStringLiteral("R\"\\((?:.|\\n)*?\\)\"")), stringFormat });

    // ---------- Números (decimal, float, exponent, hex) ----------
    QTextCharFormat numberFormat;
    numberFormat.setForeground(kColorNumber);
    m_rules.push_back({ QRegularExpression(QStringLiteral("\\b0x[0-9A-Fa-f]+\\b")), numberFormat });
    m_rules.push_back({ QRegularExpression(QStringLiteral("\\b[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?[uUlLfF]*\\b")), numberFormat });

    // ---------- Preprocesador / directivas ----------
    QTextCharFormat preprocFormat;
    preprocFormat.setForeground(kColorOperator);
    preprocFormat.setFontWeight(QFont::Bold);
    m_rules.push_back({ QRegularExpression(QStringLiteral("^\\s*#\\s*\\w+")), preprocFormat });
    // Highlight include filename between <> or ""
    QTextCharFormat includeFileFormat;
    includeFileFormat.setForeground(kColorString);
    includeFileFormat.setFontItalic(true);
    m_rules.push_back({ QRegularExpression(QStringLiteral("#\\s*include\\s*[<\"][^>\"]+[>\"]")), includeFileFormat });

    // ---------- Comentarios ----------
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(kColorComment);
    singleLineCommentFormat.setFontItalic(true);
    m_rules.push_back({ QRegularExpression(QStringLiteral("//[^\\n]*")), singleLineCommentFormat });

    // Multiline comments (handled specially below too)
    m_commentStart = QRegularExpression(QStringLiteral("/\\*"));
    m_commentEnd = QRegularExpression(QStringLiteral("\\*/"));
    m_commentFormat = QTextCharFormat();
    m_commentFormat.setForeground(kColorComment);
    m_commentFormat.setFontItalic(true);

    // ---------- Operadores / símbolos ----------
    QMap<QString, QColor> symbolColors = {
        {"<<", kColorOperator}, {">>", kColorOperator},
        {"==", kColorOperator}, {"!=", kColorOperator}, {">=", kColorOperator}, {"<=", kColorOperator},
        {"+", kColorOperator}, {"-", kColorOperator}, {"*", kColorOperator}, {"/", kColorOperator},
        {"%", kColorOperator}, {"=", kColorOperator}, {"&", kColorOperator}, {"|", kColorOperator},
        {"^", kColorOperator}, {"~", kColorOperator}, {"->", kColorOperator},
        {"{", kColorOperator}, {"}", kColorOperator}, {"(", kColorOperator}, {")", kColorOperator},
        {"[", kColorOperator}, {"]", kColorOperator}, {";", kColorOperator}, {":", kColorOperator},
        {"::", kColorOperator}, {",", kColorOperator}
    };
    for (auto it = symbolColors.begin(); it != symbolColors.end(); ++it) {
        QTextCharFormat fmt;
        fmt.setForeground(it.value());
        fmt.setFontWeight(QFont::Normal);
        m_rules.push_back({ QRegularExpression(QRegularExpression::escape(it.key())), fmt });
    }

    // ---------- Errores simples (por ejemplo TODO/FIXME) ----------
    QTextCharFormat todoFormat;
    todoFormat.setForeground(kColorError);
    todoFormat.setFontWeight(QFont::Bold);
    m_rules.push_back({ QRegularExpression(QStringLiteral("\\b(TODO|FIXME|BUG)\\b")), todoFormat });
}

// Método que aplica los formatos en cada bloque de texto
void CppHighlighter::highlightBlock(const QString &text) {
    // Aplica todas las reglas (keywords, tipos, strings, símbolos, etc.)
    for (const Rule &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }

    // ---------- Comentarios multilínea ----------
    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1)
        startIndex = text.indexOf(m_commentStart);
    else
        startIndex = 0;

    while (startIndex >= 0) {
        int endIndex = text.indexOf(m_commentEnd, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + m_commentEnd.pattern().length();
        }
        setFormat(startIndex, commentLength, m_commentFormat);
        startIndex = text.indexOf(m_commentStart, startIndex + commentLength);
    }
}
