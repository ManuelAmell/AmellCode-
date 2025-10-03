#pragma once

#include <QPlainTextEdit>

class LineNumberArea;
class CppHighlighter;

class Editor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);
    void newDocument();
    void openFile(const QString &filePath);
    void save();

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

signals:
    void zoomLevelChanged(int newZoomLevel);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *m_lineNumberArea;
    QString m_currentFile;
    CppHighlighter *m_highlighter;

    int m_zoomLevel = 0;
    static constexpr int MAX_ZOOM = 10;
    static constexpr int MIN_ZOOM = -10;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(Editor *editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return QSize(m_editor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override { m_editor->lineNumberAreaPaintEvent(event); }

private:
    Editor *m_editor;
};