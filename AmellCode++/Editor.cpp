#include "Editor.h"
#include "CppHighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QTextFormat>
#include <QWheelEvent>
#include <QKeyEvent>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent),
      m_lineNumberArea(new LineNumberArea(this)),
      m_highlighter(new CppHighlighter(document())) {

    connect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
    connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setTabStopDistance(fontMetrics().horizontalAdvance(" ") * 4);
}

void Editor::newDocument() {
    setPlainText("");
    m_currentFile.clear();

    if (m_zoomLevel != 0) {
        if (m_zoomLevel > 0) for (int i = 0; i < m_zoomLevel; ++i) zoomOut(1);
        else for (int i = 0; i < -m_zoomLevel; ++i) zoomIn(1);

        m_zoomLevel = 0;
        emit zoomLevelChanged(m_zoomLevel);
    }
}

void Editor::openFile(const QString &filePath) {
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setPlainText(QString::fromUtf8(f.readAll()));
        m_currentFile = filePath;

        if (m_zoomLevel != 0) {
            if (m_zoomLevel > 0) for (int i = 0; i < m_zoomLevel; ++i) zoomOut(1);
            else for (int i = 0; i < -m_zoomLevel; ++i) zoomIn(1);

            m_zoomLevel = 0;
            emit zoomLevelChanged(m_zoomLevel);
        }
    }
}

void Editor::save() {
    QString path = m_currentFile;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath());
        if (path.isEmpty()) return;
        m_currentFile = path;
    }
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write(toPlainText().toUtf8());
    }
}

int Editor::lineNumberAreaWidth() const {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space + 6;
}

void Editor::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void Editor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(24, 40, 68));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(140, 170, 210));
            painter.drawText(0, top, m_lineNumberArea->width() - 8, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void Editor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(30, 90, 170, 60);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void Editor::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        int steps = delta / 120;
        if (steps == 0) steps = (delta > 0) ? 1 : -1;

        int allowedSteps = steps;
        int newZoom = m_zoomLevel + steps;
        if (newZoom > MAX_ZOOM) allowedSteps = MAX_ZOOM - m_zoomLevel;
        if (newZoom < MIN_ZOOM) allowedSteps = MIN_ZOOM - m_zoomLevel;

        if (allowedSteps > 0) for (int i = 0; i < allowedSteps; ++i) zoomIn(1);
        else if (allowedSteps < 0) for (int i = 0; i < -allowedSteps; ++i) zoomOut(1);

        m_zoomLevel += allowedSteps;
        emit zoomLevelChanged(m_zoomLevel);
        event->accept();
        return;
    }

    QPlainTextEdit::wheelEvent(event);
}

void Editor::keyPressEvent(QKeyEvent *event) {
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_0) {
        if (m_zoomLevel != 0) {
            if (m_zoomLevel > 0) for (int i = 0; i < m_zoomLevel; ++i) zoomOut(1);
            else for (int i = 0; i < -m_zoomLevel; ++i) zoomIn(1);

            m_zoomLevel = 0;
            emit zoomLevelChanged(m_zoomLevel);
        }
        event->accept();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}
